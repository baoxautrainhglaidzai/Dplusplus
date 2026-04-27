#pragma once

#include <optional>
#include <vector>

#include "dpp/ast.hpp"
#include "dpp/token.hpp"

namespace dpp {

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);

    ast::Program parse();

private:
    ast::StmtPtr statement();
    ast::StmtPtr varDeclaration();
    ast::StmtPtr functionDeclaration();
    ast::StmtPtr ifStatement();
    ast::StmtPtr whileStatement();
    ast::StmtPtr forStatement();
    ast::StmtPtr breakStatement();
    ast::StmtPtr continueStatement();
    ast::StmtPtr returnStatement();
    ast::StmtPtr printStatement();
    ast::StmtPtr expressionStatement();
    std::vector<ast::StmtPtr> block();

    ast::ExprPtr expression();
    ast::ExprPtr assignment();
    ast::ExprPtr logicalOr();
    ast::ExprPtr logicalAnd();
    ast::ExprPtr equality();
    ast::ExprPtr comparison();
    ast::ExprPtr term();
    ast::ExprPtr factor();
    ast::ExprPtr unary();
    ast::ExprPtr call();
    ast::ExprPtr finishCall(ast::ExprPtr callee, int line);
    ast::ExprPtr primary();

    bool match(std::initializer_list<TokenType> types);
    const Token& consume(TokenType type, const std::string& message);
    bool check(TokenType type) const;
    bool checkNext(TokenType type) const;
    const Token& advance();
    bool isAtEnd() const;
    const Token& peek() const;
    const Token& previous() const;
    [[noreturn]] void error(const Token& token, const std::string& message) const;

    std::vector<Token> tokens_;
    std::size_t current_ = 0;
};

}  // namespace dpp
