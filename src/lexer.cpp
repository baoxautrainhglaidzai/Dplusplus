#include "dpp/lexer.hpp"

#include <cctype>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>

#include "dpp/error.hpp"

namespace dpp {

namespace {

const std::unordered_map<std::string, TokenType> kKeywords = {
    {"let", TokenType::Let},
    {"fn", TokenType::Fn},
    {"return", TokenType::Return},
    {"if", TokenType::If},
    {"else", TokenType::Else},
    {"while", TokenType::While},
    {"for", TokenType::For},
    {"in", TokenType::In},
    {"break", TokenType::Break},
    {"continue", TokenType::Continue},
    {"and", TokenType::And},
    {"or", TokenType::Or},
    {"print", TokenType::Print},
    {"true", TokenType::True},
    {"false", TokenType::False},
    {"null", TokenType::Null},
};

}  // namespace

Lexer::Lexer(std::string source) : source_(std::move(source)) {}

std::vector<Token> Lexer::tokenize() {
    while (!isAtEnd()) {
        start_ = current_;
        startLine_ = line_;
        startColumn_ = column_;
        scanToken();
    }

    tokens_.push_back(Token{TokenType::EndOfFile, "", std::monostate{}, line_, column_});
    return tokens_;
}

void Lexer::scanToken() {
    const char character = advance();

    switch (character) {
        case '(':
            addToken(TokenType::LeftParen);
            return;
        case ')':
            addToken(TokenType::RightParen);
            return;
        case '{':
            addToken(TokenType::LeftBrace);
            return;
        case '}':
            addToken(TokenType::RightBrace);
            return;
        case '[':
            addToken(TokenType::LeftBracket);
            return;
        case ']':
            addToken(TokenType::RightBracket);
            return;
        case ',':
            addToken(TokenType::Comma);
            return;
        case '+':
            addToken(TokenType::Plus);
            return;
        case '-':
            addToken(TokenType::Minus);
            return;
        case '*':
            addToken(TokenType::Star);
            return;
        case '/':
            if (match('/')) {
                while (peek() != '\n' && peek() != '\0') {
                    advance();
                }
            } else {
                addToken(TokenType::Slash);
            }
            return;
        case '%':
            addToken(TokenType::Percent);
            return;
        case '=':
            addToken(match('=') ? TokenType::EqualEqual : TokenType::Assign);
            return;
        case '!':
            if (match('=')) {
                addToken(TokenType::BangEqual);
                return;
            }
            addToken(TokenType::Bang);
            return;
        case '&':
            if (match('&')) {
                addToken(TokenType::And);
                return;
            }
            error("Unexpected character '&'.");
        case '|':
            if (match('|')) {
                addToken(TokenType::Or);
                return;
            }
            error("Unexpected character '|'.");
        case '>':
            addToken(match('=') ? TokenType::GreaterEqual : TokenType::Greater);
            return;
        case '<':
            addToken(match('=') ? TokenType::LessEqual : TokenType::Less);
            return;
        case '.':
            if (match('.')) {
                addToken(TokenType::Range);
                return;
            }
            error("Unexpected character '.'.");
        case ' ':
        case '\r':
        case '\t':
        case '\n':
            return;
        case '"':
            string();
            return;
        default:
            if (std::isdigit(static_cast<unsigned char>(character))) {
                number();
                return;
            }
            if (std::isalpha(static_cast<unsigned char>(character)) || character == '_') {
                identifier();
                return;
            }
            error(std::string("Unexpected character '") + character + "'.");
    }
}

void Lexer::identifier() {
    while (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_') {
        advance();
    }

    const std::string lexeme = source_.substr(start_, current_ - start_);
    const auto found = kKeywords.find(lexeme);
    if (found != kKeywords.end()) {
        addToken(found->second);
        return;
    }

    addToken(TokenType::Identifier);
}

void Lexer::number() {
    while (std::isdigit(static_cast<unsigned char>(peek()))) {
        advance();
    }

    if (peek() == '.' && peekNext() != '.' && std::isdigit(static_cast<unsigned char>(peekNext()))) {
        advance();
        while (std::isdigit(static_cast<unsigned char>(peek()))) {
            advance();
        }
    }

    const std::string lexeme = source_.substr(start_, current_ - start_);
    addToken(TokenType::Number, std::stod(lexeme));
}

void Lexer::string() {
    std::string value;

    while (!isAtEnd()) {
        const char character = advance();
        if (character == '"') {
            addToken(TokenType::String, value);
            return;
        }

        if (character == '\\') {
            if (isAtEnd()) {
                error("Unterminated string.");
            }

            const char escaped = advance();
            switch (escaped) {
                case '"':
                    value.push_back('"');
                    break;
                case '\\':
                    value.push_back('\\');
                    break;
                case 'n':
                    value.push_back('\n');
                    break;
                case 't':
                    value.push_back('\t');
                    break;
                case 'r':
                    value.push_back('\r');
                    break;
                default:
                    error(std::string("Unsupported escape sequence '\\") + escaped + "'.");
            }
            continue;
        }

        value.push_back(character);
    }

    error("Unterminated string.");
}

void Lexer::addToken(TokenType type) {
    addToken(type, std::monostate{});
}

void Lexer::addToken(TokenType type, TokenLiteral literal) {
    tokens_.push_back(Token{
        type,
        source_.substr(start_, current_ - start_),
        std::move(literal),
        startLine_,
        startColumn_,
    });
}

char Lexer::advance() {
    const char character = source_[current_++];
    if (character == '\n') {
        ++line_;
        column_ = 1;
    } else {
        ++column_;
    }
    return character;
}

bool Lexer::match(char expected) {
    if (isAtEnd() || source_[current_] != expected) {
        return false;
    }

    advance();
    return true;
}

char Lexer::peek() const {
    if (isAtEnd()) {
        return '\0';
    }
    return source_[current_];
}

char Lexer::peekNext() const {
    if (current_ + 1 >= source_.size()) {
        return '\0';
    }
    return source_[current_ + 1];
}

bool Lexer::isAtEnd() const {
    return current_ >= source_.size();
}

void Lexer::error(const std::string& message) const {
    std::ostringstream stream;
    stream << "Error: " << message << " at line " << startLine_ << ", column " << startColumn_;
    throw LexerError(stream.str());
}

}  // namespace dpp
