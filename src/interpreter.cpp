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

    if (const auto* for_statement = dynamic_cast<const ast::ForStatement*>(&statement)) {
        executeForStatement(*for_statement);
        return;
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
        executeBlock(statement.body, loopEnvironment);
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
        throw RuntimeError("Error: Unsupported unary operator '" + unary->operator_symbol + "' at line " + std::to_string(unary->line));
    }

    if (const auto* binary = dynamic_cast<const ast::Binary*>(&expression)) {
        return evaluateBinary(*binary);
    }

    if (const auto* call = dynamic_cast<const ast::Call*>(&expression)) {
        return evaluateCall(*call);
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
    return true;
}

bool Interpreter::isNumber(const Value& value) const {
    return std::holds_alternative<double>(value);
}

}  // namespace dpp
