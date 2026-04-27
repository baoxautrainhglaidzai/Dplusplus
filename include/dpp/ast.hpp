#pragma once

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "dpp/value.hpp"

namespace dpp::ast {

struct Expr {
    explicit Expr(int line_number) : line(line_number) {}
    virtual ~Expr() = default;

    int line;
};

using ExprPtr = std::unique_ptr<Expr>;

struct Stmt {
    explicit Stmt(int line_number) : line(line_number) {}
    virtual ~Stmt() = default;

    int line;
};

using StmtPtr = std::unique_ptr<Stmt>;

struct Literal final : Expr {
    Literal(Value literal_value, int line_number) : Expr(line_number), value(std::move(literal_value)) {}

    Value value;
};

struct Variable final : Expr {
    Variable(std::string variable_name, int line_number)
        : Expr(line_number), name(std::move(variable_name)) {}

    std::string name;
};

struct Grouping final : Expr {
    Grouping(ExprPtr grouped_expression, int line_number)
        : Expr(line_number), expression(std::move(grouped_expression)) {}

    ExprPtr expression;
};

struct Unary final : Expr {
    Unary(std::string operator_lexeme, ExprPtr unary_operand, int line_number)
        : Expr(line_number), operator_symbol(std::move(operator_lexeme)), operand(std::move(unary_operand)) {}

    std::string operator_symbol;
    ExprPtr operand;
};

struct Binary final : Expr {
    Binary(ExprPtr binary_left, std::string operator_lexeme, ExprPtr binary_right, int line_number)
        : Expr(line_number),
          left(std::move(binary_left)),
          operator_symbol(std::move(operator_lexeme)),
          right(std::move(binary_right)) {}

    ExprPtr left;
    std::string operator_symbol;
    ExprPtr right;
};

struct Call final : Expr {
    Call(ExprPtr call_target, std::vector<ExprPtr> call_arguments, int line_number)
        : Expr(line_number), callee(std::move(call_target)), arguments(std::move(call_arguments)) {}

    ExprPtr callee;
    std::vector<ExprPtr> arguments;
};

struct VarDeclaration final : Stmt {
    VarDeclaration(std::string variable_name, ExprPtr variable_value, int line_number)
        : Stmt(line_number), name(std::move(variable_name)), value(std::move(variable_value)) {}

    std::string name;
    ExprPtr value;
};

struct Assignment final : Stmt {
    Assignment(std::string variable_name, ExprPtr assigned_value, int line_number)
        : Stmt(line_number), name(std::move(variable_name)), value(std::move(assigned_value)) {}

    std::string name;
    ExprPtr value;
};

struct FunctionDeclaration final : Stmt {
    FunctionDeclaration(std::string function_name, std::vector<std::string> parameters, std::vector<StmtPtr> block, int line_number)
        : Stmt(line_number), name(std::move(function_name)), params(std::move(parameters)), body(std::move(block)) {}

    std::string name;
    std::vector<std::string> params;
    std::vector<StmtPtr> body;
};

struct ReturnStatement final : Stmt {
    ReturnStatement(std::optional<ExprPtr> return_value, int line_number)
        : Stmt(line_number), value(std::move(return_value)) {}

    std::optional<ExprPtr> value;
};

struct PrintStatement final : Stmt {
    PrintStatement(ExprPtr print_value, int line_number)
        : Stmt(line_number), value(std::move(print_value)) {}

    ExprPtr value;
};

struct IfStatement final : Stmt {
    IfStatement(ExprPtr condition_expr, std::vector<StmtPtr> then_block, std::optional<std::vector<StmtPtr>> else_block, int line_number)
        : Stmt(line_number),
          condition(std::move(condition_expr)),
          then_branch(std::move(then_block)),
          else_branch(std::move(else_block)) {}

    ExprPtr condition;
    std::vector<StmtPtr> then_branch;
    std::optional<std::vector<StmtPtr>> else_branch;
};

struct ForStatement final : Stmt {
    ForStatement(std::string iterator_name, ExprPtr range_start, ExprPtr range_end, std::vector<StmtPtr> loop_body, int line_number)
        : Stmt(line_number),
          iterator(std::move(iterator_name)),
          start(std::move(range_start)),
          end(std::move(range_end)),
          body(std::move(loop_body)) {}

    std::string iterator;
    ExprPtr start;
    ExprPtr end;
    std::vector<StmtPtr> body;
};

struct ExpressionStatement final : Stmt {
    ExpressionStatement(ExprPtr statement_expression, int line_number)
        : Stmt(line_number), expression(std::move(statement_expression)) {}

    ExprPtr expression;
};

struct Program {
    std::vector<StmtPtr> statements;
};

}  // namespace dpp::ast
