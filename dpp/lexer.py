"""Lexer for the D++ language."""

from __future__ import annotations

from dataclasses import dataclass
from enum import Enum, auto

from .errors import LexerError


class TokenType(Enum):
    LEFT_PAREN = auto()
    RIGHT_PAREN = auto()
    LEFT_BRACE = auto()
    RIGHT_BRACE = auto()
    COMMA = auto()
    PLUS = auto()
    MINUS = auto()
    STAR = auto()
    SLASH = auto()
    ASSIGN = auto()
    EQEQ = auto()
    BANGEQ = auto()
    GT = auto()
    GTE = auto()
    LT = auto()
    LTE = auto()
    RANGE = auto()
    IDENTIFIER = auto()
    NUMBER = auto()
    STRING = auto()
    LET = auto()
    FN = auto()
    RETURN = auto()
    IF = auto()
    ELSE = auto()
    FOR = auto()
    IN = auto()
    PRINT = auto()
    TRUE = auto()
    FALSE = auto()
    NULL = auto()
    EOF = auto()


KEYWORDS = {
    "let": TokenType.LET,
    "fn": TokenType.FN,
    "return": TokenType.RETURN,
    "if": TokenType.IF,
    "else": TokenType.ELSE,
    "for": TokenType.FOR,
    "in": TokenType.IN,
    "print": TokenType.PRINT,
    "true": TokenType.TRUE,
    "false": TokenType.FALSE,
    "null": TokenType.NULL,
}


@dataclass(slots=True)
class Token:
    type: TokenType
    lexeme: str
    literal: object
    line: int
    column: int


class Lexer:
    def __init__(self, source: str) -> None:
        self.source = source
        self.length = len(source)
        self.start = 0
        self.current = 0
        self.line = 1
        self.column = 1
        self.start_line = 1
        self.start_column = 1
        self.tokens: list[Token] = []

    def tokenize(self) -> list[Token]:
        while not self._is_at_end():
            self.start = self.current
            self.start_line = self.line
            self.start_column = self.column
            self._scan_token()

        self.tokens.append(Token(TokenType.EOF, "", None, self.line, self.column))
        return self.tokens

    def _scan_token(self) -> None:
        char = self._advance()
        match char:
            case "(":
                self._add_token(TokenType.LEFT_PAREN)
            case ")":
                self._add_token(TokenType.RIGHT_PAREN)
            case "{":
                self._add_token(TokenType.LEFT_BRACE)
            case "}":
                self._add_token(TokenType.RIGHT_BRACE)
            case ",":
                self._add_token(TokenType.COMMA)
            case "+":
                self._add_token(TokenType.PLUS)
            case "-":
                self._add_token(TokenType.MINUS)
            case "*":
                self._add_token(TokenType.STAR)
            case "/":
                if self._match("/"):
                    while self._peek() not in ("\n", "\0"):
                        self._advance()
                else:
                    self._add_token(TokenType.SLASH)
            case "=":
                token_type = TokenType.EQEQ if self._match("=") else TokenType.ASSIGN
                self._add_token(token_type)
            case "!":
                if self._match("="):
                    self._add_token(TokenType.BANGEQ)
                else:
                    self._error("Unexpected character '!'.")
            case ">":
                token_type = TokenType.GTE if self._match("=") else TokenType.GT
                self._add_token(token_type)
            case "<":
                token_type = TokenType.LTE if self._match("=") else TokenType.LT
                self._add_token(token_type)
            case ".":
                if self._match("."):
                    self._add_token(TokenType.RANGE)
                else:
                    self._error("Unexpected character '.'.")
            case " " | "\r" | "\t":
                return
            case "\n":
                return
            case "\"":
                self._string()
            case _:
                if char.isdigit():
                    self._number()
                elif char.isalpha() or char == "_":
                    self._identifier()
                else:
                    self._error(f"Unexpected character '{char}'.")

    def _identifier(self) -> None:
        while self._peek().isalnum() or self._peek() == "_":
            self._advance()

        lexeme = self.source[self.start : self.current]
        token_type = KEYWORDS.get(lexeme, TokenType.IDENTIFIER)
        literal = None
        if token_type == TokenType.TRUE:
            literal = True
        elif token_type == TokenType.FALSE:
            literal = False
        elif token_type == TokenType.NULL:
            literal = None

        self._add_token(token_type, literal)

    def _number(self) -> None:
        while self._peek().isdigit():
            self._advance()

        if self._peek() == "." and self._peek_next().isdigit():
            self._advance()
            while self._peek().isdigit():
                self._advance()

        lexeme = self.source[self.start : self.current]
        literal = float(lexeme) if "." in lexeme else int(lexeme)
        self._add_token(TokenType.NUMBER, literal)

    def _string(self) -> None:
        value_chars: list[str] = []

        while not self._is_at_end():
            char = self._advance()
            if char == "\"":
                self._add_token(TokenType.STRING, "".join(value_chars))
                return
            if char == "\\":
                if self._is_at_end():
                    self._error("Unterminated string.")
                escape = self._advance()
                value_chars.append(self._translate_escape(escape))
                continue
            value_chars.append(char)

        self._error("Unterminated string.")

    def _translate_escape(self, escape: str) -> str:
        mapping = {
            "\"": "\"",
            "\\": "\\",
            "n": "\n",
            "t": "\t",
            "r": "\r",
        }
        if escape not in mapping:
            self._error(f"Unsupported escape sequence '\\{escape}'.")
        return mapping[escape]

    def _add_token(self, token_type: TokenType, literal: object = None) -> None:
        lexeme = self.source[self.start : self.current]
        self.tokens.append(Token(token_type, lexeme, literal, self.start_line, self.start_column))

    def _advance(self) -> str:
        char = self.source[self.current]
        self.current += 1
        if char == "\n":
            self.line += 1
            self.column = 1
        else:
            self.column += 1
        return char

    def _match(self, expected: str) -> bool:
        if self._is_at_end() or self.source[self.current] != expected:
            return False
        self._advance()
        return True

    def _peek(self) -> str:
        if self._is_at_end():
            return "\0"
        return self.source[self.current]

    def _peek_next(self) -> str:
        if self.current + 1 >= self.length:
            return "\0"
        return self.source[self.current + 1]

    def _is_at_end(self) -> bool:
        return self.current >= self.length

    def _error(self, message: str) -> None:
        raise LexerError(f"Error: {message} at line {self.start_line}, column {self.start_column}")
