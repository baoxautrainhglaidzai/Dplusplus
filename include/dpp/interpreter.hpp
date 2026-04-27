#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "dpp/ast.hpp"
#include "dpp/value.hpp"

namespace dpp {

using InputProvider = std::function<std::string()>;
using OutputHandler = std::function<void(const std::string&)>;
using RandomProvider = std::function<double()>;

class Interpreter;

class Callable {
public:
    virtual ~Callable() = default;

    virtual std::optional<std::size_t> arity() const = 0;
    virtual Value call(Interpreter& interpreter, const std::vector<Value>& arguments, int line) = 0;
    virtual std::string describe() const = 0;
};

class Environment : public std::enable_shared_from_this<Environment> {
public:
    explicit Environment(std::shared_ptr<Environment> parent = nullptr);

    void define(const std::string& name, Value value);
    void assign(const std::string& name, Value value, int line);
    Value get(const std::string& name, int line) const;
    std::shared_ptr<Environment> parent() const;

private:
    std::shared_ptr<Environment> parent_;
    std::unordered_map<std::string, Value> values_;
};

class Interpreter {
public:
    Interpreter(InputProvider input_provider = {}, OutputHandler output_handler = {}, RandomProvider random_provider = {});

    void execute(const ast::Program& program);
    void executeBlock(const std::vector<ast::StmtPtr>& statements, std::shared_ptr<Environment> environment);

private:
    friend class Callable;

    void installBuiltins();
    void executeStatement(const ast::Stmt& statement);
    void executeForStatement(const ast::ForStatement& statement);
    void executeWhileStatement(const ast::WhileStatement& statement);
    Value evaluate(const ast::Expr& expression);
    Value evaluateBinary(const ast::Binary& expression);
    Value evaluateLogical(const ast::Logical& expression);
    Value evaluateCall(const ast::Call& expression);
    double requireNumber(const Value& value, int line) const;
    int requireInteger(const Value& value, int line) const;
    ListPtr requireList(const Value& value, int line) const;
    bool isTruthy(const Value& value) const;
    bool isNumber(const Value& value) const;

    std::shared_ptr<Environment> globals_;
    std::shared_ptr<Environment> environment_;
    InputProvider inputProvider_;
    OutputHandler outputHandler_;
    RandomProvider randomProvider_;
};

}  // namespace dpp
