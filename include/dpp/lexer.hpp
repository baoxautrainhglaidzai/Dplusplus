#pragma once

#include <string>
#include <vector>

#include "dpp/token.hpp"

namespace dpp {

class Lexer {
public:
    explicit Lexer(std::string source);

    std::vector<Token> tokenize();

private:
    void scanToken();
    void identifier();
    void number();
    void string();
    void addToken(TokenType type);
    void addToken(TokenType type, TokenLiteral literal);
    char advance();
    bool match(char expected);
    char peek() const;
    char peekNext() const;
    bool isAtEnd() const;
    [[noreturn]] void error(const std::string& message) const;

    std::string source_;
    std::size_t start_ = 0;
    std::size_t current_ = 0;
    int line_ = 1;
    int column_ = 1;
    int startLine_ = 1;
    int startColumn_ = 1;
    std::vector<Token> tokens_;
};

}  // namespace dpp
