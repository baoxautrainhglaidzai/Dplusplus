#pragma once

#include <string>
#include <variant>

namespace dpp {

enum class TokenType {
    LeftParen,
    RightParen,
    LeftBrace,
    RightBrace,
    LeftBracket,
    RightBracket,
    Comma,
    Plus,
    Minus,
    Star,
    Slash,
    Percent,
    Assign,
    Bang,
    EqualEqual,
    BangEqual,
    And,
    Or,
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
    While,
    For,
    In,
    Break,
    Continue,
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
