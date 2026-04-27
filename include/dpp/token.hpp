#pragma once

#include <string>
#include <variant>

namespace dpp {

enum class TokenType {
    LeftParen,
    RightParen,
    LeftBrace,
    RightBrace,
    Comma,
    Plus,
    Minus,
    Star,
    Slash,
    Assign,
    EqualEqual,
    BangEqual,
    Greater,
    GreaterEqual,
    Less,
    LessEqual,
    Range,
    Identifier,
    Number,
    String,
    Let,
    Fn,
    Return,
    If,
    Else,
    For,
    In,
    Print,
    True,
    False,
    Null,
    EndOfFile,
};

using TokenLiteral = std::variant<std::monostate, double, std::string>;

struct Token {
    TokenType type;
    std::string lexeme;
    TokenLiteral literal;
    int line;
    int column;
};

}  // namespace dpp
