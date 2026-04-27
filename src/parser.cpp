#include "dpp/parser.hpp"

#include <initializer_list>
#include <sstream>
#include <utility>

#include "dpp/error.hpp"

namespace dpp {

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

ast::Program Parser::parse() {
    ast::Program program;
    while (!isAtEnd()) {
        program.statements.push_back(statement());
    }
    return program;
}

ast::StmtPtr Parser::statement() {
    if (match({TokenType::Let})) {
        return varDeclaration();
    }
    if (match({TokenType::Fn})) {
        return functionDeclaration();
    }
    if (match({TokenType::If})) {
        return ifStatement();
    }
    if (match({TokenType::For})) {
        return forStatement();
    }
    if (match({TokenType::Return})) {
        return returnStatement();
    }
    if (match({TokenType::Print})) {
        return printStatement();
    }
    if (check(TokenType::Identifier) && checkNext(TokenType::Assign)) {
        return assignment();
    }
    return expressionStatement();
}

ast::StmtPtr Parser::varDeclaration() {
    const Token& name = consume(TokenType::Identifier, "Expected variable name.");
    consume(TokenType::Assign, "Expected '=' after variable name.");
    return std::make_unique<ast::VarDeclaration>(name.lexeme, expression(), name.line);
}

ast::StmtPtr Parser::assignment() {
    const Token& name = consume(TokenType::Identifier, "Expected variable name.");
    consume(TokenType::Assign, "Expected '=' after variable name.");
    return std::make_unique<ast::Assignment>(name.lexeme, expression(), name.line);
}

ast::StmtPtr Parser::functionDeclaration() {
    const Token& name = consume(TokenType::Identifier, "Expected function name.");
    consume(TokenType::LeftParen, "Expected '(' after function name.");

    std::vector<std::string> params;
    if (!check(TokenType::RightParen)) {
        do {
            params.push_back(consume(TokenType::Identifier, "Expected parameter name.").lexeme);
        } while (match({TokenType::Comma}));
    }

    consume(TokenType::RightParen, "Expected ')' after parameters.");
    return std::make_unique<ast::FunctionDeclaration>(name.lexeme, std::move(params), block(), name.line);
}

ast::StmtPtr Parser::ifStatement() {
    const Token keyword = previous();
    auto condition = expression();
    auto thenBranch = block();
    std::optional<std::vector<ast::StmtPtr>> elseBranch;

    if (match({TokenType::Else})) {
        if (match({TokenType::If})) {
            std::vector<ast::StmtPtr> nestedElse;
            nestedElse.push_back(ifStatement());
            elseBranch = std::move(nestedElse);
        } else {
            elseBranch = block();
        }
    }

    return std::make_unique<ast::IfStatement>(std::move(condition), std::move(thenBranch), std::move(elseBranch), keyword.line);
}

ast::StmtPtr Parser::forStatement() {
    const Token keyword = previous();
    const Token& iterator = consume(TokenType::Identifier, "Expected loop variable name.");
    consume(TokenType::In, "Expected 'in' after loop variable.");
    auto start = expression();
    consume(TokenType::Range, "Expected '..' in for loop range.");
    auto end = expression();
    return std::make_unique<ast::ForStatement>(iterator.lexeme, std::move(start), std::move(end), block(), keyword.line);
}

ast::StmtPtr Parser::returnStatement() {
    const Token keyword = previous();
    if (check(TokenType::RightBrace) || check(TokenType::EndOfFile)) {
        return std::make_unique<ast::ReturnStatement>(std::nullopt, keyword.line);
    }

    return std::make_unique<ast::ReturnStatement>(std::optional<ast::ExprPtr>(expression()), keyword.line);
}

ast::StmtPtr Parser::printStatement() {
    const Token keyword = previous();
    if (match({TokenType::LeftParen})) {
        auto value = expression();
        consume(TokenType::RightParen, "Expected ')' after print value.");
        return std::make_unique<ast::PrintStatement>(std::move(value), keyword.line);
    }

    return std::make_unique<ast::PrintStatement>(expression(), keyword.line);
}

ast::StmtPtr Parser::expressionStatement() {
    auto expr = expression();
    const int line = expr->line;
    return std::make_unique<ast::ExpressionStatement>(std::move(expr), line);
}

std::vector<ast::StmtPtr> Parser::block() {
    consume(TokenType::LeftBrace, "Expected '{' to start block.");

    std::vector<ast::StmtPtr> statements;
    while (!check(TokenType::RightBrace) && !isAtEnd()) {
        statements.push_back(statement());
    }

    consume(TokenType::RightBrace, "Expected '}' after block.");
    return statements;
}

ast::ExprPtr Parser::expression() {
    return equality();
}

ast::ExprPtr Parser::equality() {
    auto expr = comparison();

    while (match({TokenType::EqualEqual, TokenType::BangEqual})) {
        const Token op = previous();
        expr = std::make_unique<ast::Binary>(std::move(expr), op.lexeme, comparison(), op.line);
    }

    return expr;
}

ast::ExprPtr Parser::comparison() {
    auto expr = term();

    while (match({TokenType::Greater, TokenType::GreaterEqual, TokenType::Less, TokenType::LessEqual})) {
        const Token op = previous();
        expr = std::make_unique<ast::Binary>(std::move(expr), op.lexeme, term(), op.line);
    }

    return expr;
}

ast::ExprPtr Parser::term() {
    auto expr = factor();

    while (match({TokenType::Plus, TokenType::Minus})) {
        const Token op = previous();
        expr = std::make_unique<ast::Binary>(std::move(expr), op.lexeme, factor(), op.line);
    }

    return expr;
}

ast::ExprPtr Parser::factor() {
    auto expr = unary();

    while (match({TokenType::Star, TokenType::Slash})) {
        const Token op = previous();
        expr = std::make_unique<ast::Binary>(std::move(expr), op.lexeme, unary(), op.line);
    }

    return expr;
}

ast::ExprPtr Parser::unary() {
    if (match({TokenType::Minus})) {
        const Token op = previous();
        return std::make_unique<ast::Unary>(op.lexeme, unary(), op.line);
    }

    return call();
}

ast::ExprPtr Parser::call() {
    auto expr = primary();

    while (match({TokenType::LeftParen})) {
        expr = finishCall(std::move(expr), previous().line);
    }

    return expr;
}

ast::ExprPtr Parser::finishCall(ast::ExprPtr callee, int line) {
    std::vector<ast::ExprPtr> arguments;
    if (!check(TokenType::RightParen)) {
        do {
            arguments.push_back(expression());
        } while (match({TokenType::Comma}));
    }

    consume(TokenType::RightParen, "Expected ')' after arguments.");
    return std::make_unique<ast::Call>(std::move(callee), std::move(arguments), line);
}

ast::ExprPtr Parser::primary() {
    if (match({TokenType::Number})) {
        return std::make_unique<ast::Literal>(std::get<double>(previous().literal), previous().line);
    }
    if (match({TokenType::String})) {
        return std::make_unique<ast::Literal>(std::get<std::string>(previous().literal), previous().line);
    }
    if (match({TokenType::True})) {
        return std::make_unique<ast::Literal>(true, previous().line);
    }
    if (match({TokenType::False})) {
        return std::make_unique<ast::Literal>(false, previous().line);
    }
    if (match({TokenType::Null})) {
        return std::make_unique<ast::Literal>(std::monostate{}, previous().line);
    }
    if (match({TokenType::Identifier})) {
        return std::make_unique<ast::Variable>(previous().lexeme, previous().line);
    }
    if (match({TokenType::LeftParen})) {
        const Token opening = previous();
        auto expr = expression();
        consume(TokenType::RightParen, "Expected ')' after expression.");
        return std::make_unique<ast::Grouping>(std::move(expr), opening.line);
    }

    const Token& token = peek();
    error(token, "Expected expression, found '" + (token.lexeme.empty() ? std::string("end of file") : token.lexeme) + "'.");
}

bool Parser::match(std::initializer_list<TokenType> types) {
    for (const TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }

    return false;
}

const Token& Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) {
        return advance();
    }

    error(peek(), message);
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) {
        return type == TokenType::EndOfFile;
    }
    return peek().type == type;
}

bool Parser::checkNext(TokenType type) const {
    if (current_ + 1 >= tokens_.size()) {
        return false;
    }
    return tokens_[current_ + 1].type == type;
}

const Token& Parser::advance() {
    if (!isAtEnd()) {
        ++current_;
    }
    return previous();
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::EndOfFile;
}

const Token& Parser::peek() const {
    return tokens_[current_];
}

const Token& Parser::previous() const {
    return tokens_[current_ - 1];
}

void Parser::error(const Token& token, const std::string& message) const {
    std::ostringstream stream;
    stream << "Error: " << message << " at line " << token.line << ", column " << token.column;
    throw ParserError(stream.str());
}

}  // namespace dpp
