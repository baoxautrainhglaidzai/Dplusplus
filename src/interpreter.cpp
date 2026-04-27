#include "dpp/interpreter.hpp"

#include <cmath>
#include <cstddef>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "dpp/error.hpp"

namespace dpp {

namespace {

class ReturnSignal final : public std::exception {
public:
    explicit ReturnSignal(Value return_value) : value(std::move(return_value)) {}
    Value value;
};

class BreakSignal final : public std::exception {};
class ContinueSignal final : public std::exception {};

class NativeFunction final : public Callable {
public:
    using Implementation = std::function<Value(Interpreter&, const std::vector<Value>&, int)>;

    NativeFunction(std::string function_name, std::size_t expected_arity, Implementation implementation)
        : name_(std::move(function_name)), arity_(expected_arity), implementation_(std::move(implementation)) {}

    std::optional<std::size_t> arity() const override {
        return arity_;
    }

    Value call(Interpreter& interpreter, const std::vector<Value>& arguments, int line) override {
        return implementation_(interpreter, arguments, line);
    }

    std::string describe() const override {
        return "<native fn " + name_ + ">";
    }

private:
    std::string name_;
    std::size_t arity_;
    Implementation implementation_;
};

class DppFunction final : public Callable {
public:
    DppFunction(const ast::FunctionDeclaration* declaration, std::shared_ptr<Environment> closure)
        : declaration_(declaration), closure_(std::move(closure)) {}

    std::optional<std::size_t> arity() const override {
        return declaration_->params.size();
    }

    Value call(Interpreter& interpreter, const std::vector<Value>& arguments, int) override {
        auto local = std::make_shared<Environment>(closure_);
        for (std::size_t index = 0; index < declaration_->params.size(); ++index) {
            local->define(declaration_->params[index], arguments[index]);
        }

        try {
            interpreter.executeBlock(declaration_->body, local);
        } catch (const ReturnSignal& signal) {
            return signal.value;
        }

        return std::monostate{};
    }

    std::string describe() const override {
        return "<fn " + declaration_->name + ">";
    }

private:
    const ast::FunctionDeclaration* declaration_;
    std::shared_ptr<Environment> closure_;
};

std::string formatNumber(double value) {
    if (std::isfinite(value) && std::floor(value) == value) {
        std::ostringstream stream;
        stream << static_cast<long long>(value);
        return stream.str();
    }

    std::ostringstream stream;
    stream << value;
    return stream.str();
}

}  // namespace

std::string stringifyValue(const Value& value) {
    if (std::holds_alternative<std::monostate>(value)) {
        return "null";
    }
    if (const auto* number = std::get_if<double>(&value)) {
        return formatNumber(*number);
    }
    if (const auto* boolean = std::get_if<bool>(&value)) {
        return *boolean ? "true" : "false";
    }
    if (const auto* text = std::get_if<std::string>(&value)) {
        return *text;
    }
    if (const auto* list = std::get_if<ListPtr>(&value)) {
        if (*list == nullptr) {
            return "[]";
        }
        std::ostringstream stream;
        stream << "[";
        for (std::size_t index = 0; index < (*list)->elements.size(); ++index) {
            if (index > 0) {
                stream << ", ";
            }
            stream << stringifyValue((*list)->elements[index]);
        }
        stream << "]";
        return stream.str();
    }
    return std::get<CallablePtr>(value)->describe();
}

bool valuesEqual(const Value& left, const Value& right) {
    if (left.index() != right.index()) {
        return false;
    }
    if (std::holds_alternative<std::monostate>(left)) {
        return true;
    }
    if (const auto* left_number = std::get_if<double>(&left)) {
        return *left_number == std::get<double>(right);
    }
    if (const auto* left_boolean = std::get_if<bool>(&left)) {
        return *left_boolean == std::get<bool>(right);
    }
    if (const auto* left_text = std::get_if<std::string>(&left)) {
        return *left_text == std::get<std::string>(right);
    }
    if (const auto* left_list = std::get_if<ListPtr>(&left)) {
        const auto right_list = std::get<ListPtr>(right);
        if (*left_list == nullptr || right_list == nullptr) {
            return *left_list == right_list;
        }
        if ((*left_list)->elements.size() != right_list->elements.size()) {
            return false;
        }
        for (std::size_t index = 0; index < (*left_list)->elements.size(); ++index) {
            if (!valuesEqual((*left_list)->elements[index], right_list->elements[index])) {
                return false;
            }
        }
        return true;
    }
    return std::get<CallablePtr>(left) == std::get<CallablePtr>(right);
}

Environment::Environment(std::shared_ptr<Environment> parent) : parent_(std::move(parent)) {}

void Environment::define(const std::string& name, Value value) {
    values_[name] = std::move(value);
}

void Environment::assign(const std::string& name, Value value, int line) {
    const auto found = values_.find(name);
    if (found != values_.end()) {
        found->second = std::move(value);
        return;
    }

    if (parent_ != nullptr) {
        parent_->assign(name, std::move(value), line);
        return;
    }

    throw RuntimeError("Error: Undefined variable '" + name + "' at line " + std::to_string(line));
}

Value Environment::get(const std::string& name, int line) const {
    const auto found = values_.find(name);
    if (found != values_.end()) {
        return found->second;
    }

    if (parent_ != nullptr) {
        return parent_->get(name, line);
    }

    throw RuntimeError("Error: Undefined variable '" + name + "' at line " + std::to_string(line));
}

std::shared_ptr<Environment> Environment::parent() const {
    return parent_;
}

Interpreter::Interpreter(InputProvider input_provider, OutputHandler output_handler, RandomProvider random_provider)
    : globals_(std::make_shared<Environment>()),
      environment_(globals_),
      inputProvider_(std::move(input_provider)),
      outputHandler_(std::move(output_handler)),
      randomProvider_(std::move(random_provider)) {
    if (!inputProvider_) {
        inputProvider_ = []() {
            std::string line;
            std::getline(std::cin, line);
            return line;
        };
    }

    if (!outputHandler_) {
        outputHandler_ = [](const std::string& text) {
            std::cout << text << '\n';
        };
    }

    if (!randomProvider_) {
        auto engine = std::make_shared<std::mt19937>(std::random_device{}());
        auto distribution = std::make_shared<std::uniform_real_distribution<double>>(0.0, 1.0);
        randomProvider_ = [engine, distribution]() mutable {
            return (*distribution)(*engine);
        };
    }

    installBuiltins();
}

void Interpreter::execute(const ast::Program& program) {
    try {
        for (const auto& statement : program.statements) {
            executeStatement(*statement);
        }
    } catch (const ReturnSignal&) {
        throw RuntimeError("Error: 'return' can only be used inside a function");
    } catch (const BreakSignal&) {
        throw RuntimeError("Error: 'break' can only be used inside a loop");
    } catch (const ContinueSignal&) {
        throw RuntimeError("Error: 'continue' can only be used inside a loop");
    }
}

void Interpreter::executeBlock(const std::vector<ast::StmtPtr>& statements, std::shared_ptr<Environment> environment) {
    const auto previous = environment_;
    environment_ = std::move(environment);
    try {
        for (const auto& statement : statements) {
            executeStatement(*statement);
        }
    } catch (...) {
        environment_ = previous;
        throw;
    }
    environment_ = previous;
}

void Interpreter::installBuiltins() {
    globals_->define(
        "input",
        std::make_shared<NativeFunction>(
            "input",
            0,
            [this](Interpreter&, const std::vector<Value>&, int) {
                return Value(inputProvider_());
            }));

    globals_->define(
        "sqrt",
        std::make_shared<NativeFunction>(
            "sqrt",
            1,
            [](Interpreter& interpreter, const std::vector<Value>& arguments, int line) {
                const double value = interpreter.requireNumber(arguments[0], line);
                if (value < 0) {
                    throw RuntimeError("Error: sqrt expects a non-negative number at line " + std::to_string(line));
                }
                return Value(std::sqrt(value));
            }));

    globals_->define(
        "random",
        std::make_shared<NativeFunction>(
            "random",
            0,
            [this](Interpreter&, const std::vector<Value>&, int) {
                return Value(randomProvider_());
            }));

    globals_->define(
        "len",
        std::make_shared<NativeFunction>(
            "len",
            1,
            [](Interpreter& interpreter, const std::vector<Value>& arguments, int line) {
                if (const auto* text = std::get_if<std::string>(&arguments[0])) {
                    return Value(static_cast<double>(text->size()));
                }
                const auto list = interpreter.requireList(arguments[0], line);
                return Value(static_cast<double>(list->elements.size()));
            }));

    globals_->define(
        "push",
        std::make_shared<NativeFunction>(
            "push",
            2,
            [](Interpreter& interpreter, const std::vector<Value>& arguments, int line) {
                const auto list = interpreter.requireList(arguments[0], line);
                list->elements.push_back(arguments[1]);
                return Value(static_cast<double>(list->elements.size()));
            }));

    globals_->define(
        "pop",
        std::make_shared<NativeFunction>(
            "pop",
            1,
            [](Interpreter& interpreter, const std::vector<Value>& arguments, int line) {
                const auto list = interpreter.requireList(arguments[0], line);
                if (list->elements.empty()) {
                    throw RuntimeError("Error: pop from empty list at line " + std::to_string(line));
                }
                const Value value = list->elements.back();
                list->elements.pop_back();
                return value;
            }));
}

void Interpreter::executeStatement(const ast::Stmt& statement) {
    if (const auto* var_declaration = dynamic_cast<const ast::VarDeclaration*>(&statement)) {
        environment_->define(var_declaration->name, evaluate(*var_declaration->value));
        return;
    }

    if (const auto* assignment = dynamic_cast<const ast::Assignment*>(&statement)) {
        environment_->assign(assignment->name, evaluate(*assignment->value), assignment->line);
        return;
    }

    if (const auto* function_declaration = dynamic_cast<const ast::FunctionDeclaration*>(&statement)) {
        environment_->define(function_declaration->name, std::make_shared<DppFunction>(function_declaration, environment_));
        return;
    }

    if (const auto* return_statement = dynamic_cast<const ast::ReturnStatement*>(&statement)) {
        if (return_statement->value.has_value()) {
            throw ReturnSignal(evaluate(*return_statement->value.value()));
        }
        throw ReturnSignal(std::monostate{});
    }

    if (const auto* print_statement = dynamic_cast<const ast::PrintStatement*>(&statement)) {
        outputHandler_(stringifyValue(evaluate(*print_statement->value)));
        return;
    }

    if (const auto* if_statement = dynamic_cast<const ast::IfStatement*>(&statement)) {
        if (isTruthy(evaluate(*if_statement->condition))) {
            executeBlock(if_statement->then_branch, std::make_shared<Environment>(environment_));
        } else if (if_statement->else_branch.has_value()) {
            executeBlock(if_statement->else_branch.value(), std::make_shared<Environment>(environment_));
        }
        return;
    }

    if (const auto* while_statement = dynamic_cast<const ast::WhileStatement*>(&statement)) {
        executeWhileStatement(*while_statement);
        return;
    }

    if (const auto* for_statement = dynamic_cast<const ast::ForStatement*>(&statement)) {
        executeForStatement(*for_statement);
        return;
    }

    if (dynamic_cast<const ast::BreakStatement*>(&statement) != nullptr) {
        throw BreakSignal();
    }

    if (dynamic_cast<const ast::ContinueStatement*>(&statement) != nullptr) {
        throw ContinueSignal();
    }

    if (const auto* expression_statement = dynamic_cast<const ast::ExpressionStatement*>(&statement)) {
        static_cast<void>(evaluate(*expression_statement->expression));
        return;
    }

    throw RuntimeError("Error: Unsupported statement.");
}

void Interpreter::executeForStatement(const ast::ForStatement& statement) {
    const int start = requireInteger(evaluate(*statement.start), statement.line);
    const int end = requireInteger(evaluate(*statement.end), statement.line);
    const int step = start <= end ? 1 : -1;

    for (int value = start; value != end + step; value += step) {
        auto loopEnvironment = std::make_shared<Environment>(environment_);
        loopEnvironment->define(statement.iterator, static_cast<double>(value));
        try {
            executeBlock(statement.body, loopEnvironment);
        } catch (const ContinueSignal&) {
            continue;
        } catch (const BreakSignal&) {
            break;
        }
    }
}

void Interpreter::executeWhileStatement(const ast::WhileStatement& statement) {
    while (isTruthy(evaluate(*statement.condition))) {
        try {
            executeBlock(statement.body, std::make_shared<Environment>(environment_));
        } catch (const ContinueSignal&) {
            continue;
        } catch (const BreakSignal&) {
            break;
        }
    }
}

Value Interpreter::evaluate(const ast::Expr& expression) {
    if (const auto* literal = dynamic_cast<const ast::Literal*>(&expression)) {
        return literal->value;
    }

    if (const auto* variable = dynamic_cast<const ast::Variable*>(&expression)) {
        return environment_->get(variable->name, variable->line);
    }

    if (const auto* grouping = dynamic_cast<const ast::Grouping*>(&expression)) {
        return evaluate(*grouping->expression);
    }

    if (const auto* unary = dynamic_cast<const ast::Unary*>(&expression)) {
        if (unary->operator_symbol == "-") {
            return Value(-requireNumber(evaluate(*unary->operand), unary->line));
        }
        if (unary->operator_symbol == "!") {
            return Value(!isTruthy(evaluate(*unary->operand)));
        }
        throw RuntimeError("Error: Unsupported unary operator '" + unary->operator_symbol + "' at line " + std::to_string(unary->line));
    }

    if (const auto* binary = dynamic_cast<const ast::Binary*>(&expression)) {
        return evaluateBinary(*binary);
    }

    if (const auto* logical = dynamic_cast<const ast::Logical*>(&expression)) {
        return evaluateLogical(*logical);
    }

    if (const auto* call = dynamic_cast<const ast::Call*>(&expression)) {
        return evaluateCall(*call);
    }

    if (const auto* list_literal = dynamic_cast<const ast::ListLiteral*>(&expression)) {
        auto list = std::make_shared<ListValue>();
        list->elements.reserve(list_literal->elements.size());
        for (const auto& element : list_literal->elements) {
            list->elements.push_back(evaluate(*element));
        }
        return list;
    }

    if (const auto* access = dynamic_cast<const ast::IndexAccess*>(&expression)) {
        const Value target = evaluate(*access->target);
        const int index = requireInteger(evaluate(*access->index), access->line);
        if (index < 0) {
            throw RuntimeError("Error: Index out of bounds at line " + std::to_string(access->line));
        }

        if (const auto* text = std::get_if<std::string>(&target)) {
            if (static_cast<std::size_t>(index) >= text->size()) {
                throw RuntimeError("Error: Index out of bounds at line " + std::to_string(access->line));
            }
            return Value(std::string(1, text->at(static_cast<std::size_t>(index))));
        }

        const auto list = requireList(target, access->line);
        if (static_cast<std::size_t>(index) >= list->elements.size()) {
            throw RuntimeError("Error: Index out of bounds at line " + std::to_string(access->line));
        }
        return list->elements[static_cast<std::size_t>(index)];
    }

    if (const auto* assign = dynamic_cast<const ast::AssignExpression*>(&expression)) {
        const Value value = evaluate(*assign->value);
        environment_->assign(assign->name, value, assign->line);
        return value;
    }

    if (const auto* assign = dynamic_cast<const ast::IndexAssignExpression*>(&expression)) {
        const Value target = evaluate(*assign->target);
        const int index = requireInteger(evaluate(*assign->index), assign->line);
        if (index < 0) {
            throw RuntimeError("Error: Index out of bounds at line " + std::to_string(assign->line));
        }
        const auto list = requireList(target, assign->line);
        if (static_cast<std::size_t>(index) >= list->elements.size()) {
            throw RuntimeError("Error: Index out of bounds at line " + std::to_string(assign->line));
        }
        const Value value = evaluate(*assign->value);
        list->elements[static_cast<std::size_t>(index)] = value;
        return value;
    }

    throw RuntimeError("Error: Unsupported expression.");
}

Value Interpreter::evaluateBinary(const ast::Binary& expression) {
    const Value left = evaluate(*expression.left);
    const Value right = evaluate(*expression.right);

    if (expression.operator_symbol == "+") {
        if (std::holds_alternative<std::string>(left) || std::holds_alternative<std::string>(right)) {
            return Value(stringifyValue(left) + stringifyValue(right));
        }
        return Value(requireNumber(left, expression.line) + requireNumber(right, expression.line));
    }

    if (expression.operator_symbol == "-") {
        return Value(requireNumber(left, expression.line) - requireNumber(right, expression.line));
    }

    if (expression.operator_symbol == "*") {
        return Value(requireNumber(left, expression.line) * requireNumber(right, expression.line));
    }

    if (expression.operator_symbol == "/") {
        const double denominator = requireNumber(right, expression.line);
        if (denominator == 0.0) {
            throw RuntimeError("Error: Division by zero at line " + std::to_string(expression.line));
        }
        return Value(requireNumber(left, expression.line) / denominator);
    }

    if (expression.operator_symbol == "%") {
        const double denominator = requireNumber(right, expression.line);
        if (denominator == 0.0) {
            throw RuntimeError("Error: Modulo by zero at line " + std::to_string(expression.line));
        }
        return Value(std::fmod(requireNumber(left, expression.line), denominator));
    }

    if (expression.operator_symbol == "==") {
        return Value(valuesEqual(left, right));
    }

    if (expression.operator_symbol == "!=") {
        return Value(!valuesEqual(left, right));
    }

    if (expression.operator_symbol == ">") {
        return Value(requireNumber(left, expression.line) > requireNumber(right, expression.line));
    }

    if (expression.operator_symbol == ">=") {
        return Value(requireNumber(left, expression.line) >= requireNumber(right, expression.line));
    }

    if (expression.operator_symbol == "<") {
        return Value(requireNumber(left, expression.line) < requireNumber(right, expression.line));
    }

    if (expression.operator_symbol == "<=") {
        return Value(requireNumber(left, expression.line) <= requireNumber(right, expression.line));
    }

    throw RuntimeError("Error: Unsupported operator '" + expression.operator_symbol + "' at line " + std::to_string(expression.line));
}

Value Interpreter::evaluateLogical(const ast::Logical& expression) {
    const Value left = evaluate(*expression.left);
    const bool is_or = expression.operator_symbol == "||" || expression.operator_symbol == "or";
    const bool is_and = expression.operator_symbol == "&&" || expression.operator_symbol == "and";

    if (is_or) {
        if (isTruthy(left)) {
            return left;
        }
        return evaluate(*expression.right);
    }

    if (is_and) {
        if (!isTruthy(left)) {
            return left;
        }
        return evaluate(*expression.right);
    }

    throw RuntimeError("Error: Unsupported logical operator '" + expression.operator_symbol + "' at line " + std::to_string(expression.line));
}

Value Interpreter::evaluateCall(const ast::Call& expression) {
    const Value callee = evaluate(*expression.callee);
    std::vector<Value> arguments;
    arguments.reserve(expression.arguments.size());
    for (const auto& argument : expression.arguments) {
        arguments.push_back(evaluate(*argument));
    }

    const auto* callable = std::get_if<CallablePtr>(&callee);
    if (callable == nullptr || *callable == nullptr) {
        throw RuntimeError("Error: Value is not callable at line " + std::to_string(expression.line));
    }

    const auto expected_arity = (*callable)->arity();
    if (expected_arity.has_value() && arguments.size() != expected_arity.value()) {
        std::ostringstream stream;
        stream << "Error: Expected " << expected_arity.value() << " arguments but got " << arguments.size()
               << " at line " << expression.line;
        throw RuntimeError(stream.str());
    }

    return (*callable)->call(*this, arguments, expression.line);
}

double Interpreter::requireNumber(const Value& value, int line) const {
    const auto* number = std::get_if<double>(&value);
    if (number != nullptr) {
        return *number;
    }
    throw RuntimeError("Error: Expected a number at line " + std::to_string(line));
}

int Interpreter::requireInteger(const Value& value, int line) const {
    const double number = requireNumber(value, line);
    if (std::floor(number) == number) {
        return static_cast<int>(number);
    }
    throw RuntimeError("Error: Expected an integer at line " + std::to_string(line));
}

ListPtr Interpreter::requireList(const Value& value, int line) const {
    const auto* list = std::get_if<ListPtr>(&value);
    if (list == nullptr || *list == nullptr) {
        throw RuntimeError("Error: Expected a list at line " + std::to_string(line));
    }
    return *list;
}

bool Interpreter::isTruthy(const Value& value) const {
    if (std::holds_alternative<std::monostate>(value)) {
        return false;
    }
    if (const auto* boolean = std::get_if<bool>(&value)) {
        return *boolean;
    }
    if (const auto* number = std::get_if<double>(&value)) {
        return *number != 0.0;
    }
    if (const auto* text = std::get_if<std::string>(&value)) {
        return !text->empty();
    }
    if (const auto* list = std::get_if<ListPtr>(&value)) {
        return *list != nullptr && !(*list)->elements.empty();
    }
    return true;
}

bool Interpreter::isNumber(const Value& value) const {
    return std::holds_alternative<double>(value);
}

}  // namespace dpp
