"""Recursive-descent parser for D++."""

from __future__ import annotations

from . import ast
from .errors import ParserError
from .lexer import Token, TokenType


class Parser:
    def __init__(self, tokens: list[Token]) -> None:
        self.tokens = tokens
        self.current = 0

    def parse(self) -> ast.Program:
        statements: list[ast.Statement] = []
        while not self._is_at_end():
            statements.append(self._statement())
        return ast.Program(statements)

    def _statement(self) -> ast.Statement:
        if self._match(TokenType.LET):
            return self._var_declaration()
        if self._match(TokenType.FN):
            return self._function_declaration()
        if self._match(TokenType.IF):
            return self._if_statement()
        if self._match(TokenType.FOR):
            return self._for_statement()
        if self._match(TokenType.RETURN):
            return self._return_statement()
        if self._match(TokenType.PRINT):
            return self._print_statement()
        if self._check(TokenType.IDENTIFIER) and self._check_next(TokenType.ASSIGN):
            return self._assignment()
        return self._expression_statement()

    def _var_declaration(self) -> ast.VarDeclaration:
        name = self._consume(TokenType.IDENTIFIER, "Expected variable name.")
        self._consume(TokenType.ASSIGN, "Expected '=' after variable name.")
        value = self._expression()
        return ast.VarDeclaration(name.lexeme, value, name.line)

    def _assignment(self) -> ast.Assignment:
        name = self._consume(TokenType.IDENTIFIER, "Expected variable name.")
        self._consume(TokenType.ASSIGN, "Expected '=' after variable name.")
        value = self._expression()
        return ast.Assignment(name.lexeme, value, name.line)

    def _function_declaration(self) -> ast.FunctionDeclaration:
        name = self._consume(TokenType.IDENTIFIER, "Expected function name.")
        self._consume(TokenType.LEFT_PAREN, "Expected '(' after function name.")

        params: list[str] = []
        if not self._check(TokenType.RIGHT_PAREN):
            while True:
                params.append(self._consume(TokenType.IDENTIFIER, "Expected parameter name.").lexeme)
                if not self._match(TokenType.COMMA):
                    break

        self._consume(TokenType.RIGHT_PAREN, "Expected ')' after parameters.")
        body = self._block()
        return ast.FunctionDeclaration(name.lexeme, params, body, name.line)

    def _if_statement(self) -> ast.IfStatement:
        keyword = self._previous()
        condition = self._expression()
        then_branch = self._block()
        else_branch: list[ast.Statement] | None = None

        if self._match(TokenType.ELSE):
            if self._match(TokenType.IF):
                else_branch = [self._if_statement()]
            else:
                else_branch = self._block()

        return ast.IfStatement(condition, then_branch, else_branch, keyword.line)

    def _for_statement(self) -> ast.ForStatement:
        keyword = self._previous()
        iterator = self._consume(TokenType.IDENTIFIER, "Expected loop variable name.")
        self._consume(TokenType.IN, "Expected 'in' after loop variable.")
        start = self._expression()
        self._consume(TokenType.RANGE, "Expected '..' in for loop range.")
        end = self._expression()
        body = self._block()
        return ast.ForStatement(iterator.lexeme, start, end, body, keyword.line)

    def _return_statement(self) -> ast.ReturnStatement:
        keyword = self._previous()
        if self._check(TokenType.RIGHT_BRACE) or self._check(TokenType.EOF):
            return ast.ReturnStatement(None, keyword.line)
        value = self._expression()
        return ast.ReturnStatement(value, keyword.line)

    def _print_statement(self) -> ast.PrintStatement:
        keyword = self._previous()
        if self._match(TokenType.LEFT_PAREN):
            value = self._expression()
            self._consume(TokenType.RIGHT_PAREN, "Expected ')' after print value.")
        else:
            value = self._expression()
        return ast.PrintStatement(value, keyword.line)

    def _expression_statement(self) -> ast.ExpressionStatement:
        expression = self._expression()
        return ast.ExpressionStatement(expression, self._line_of(expression))

    def _block(self) -> list[ast.Statement]:
        self._consume(TokenType.LEFT_BRACE, "Expected '{' to start block.")
        statements: list[ast.Statement] = []
        while not self._check(TokenType.RIGHT_BRACE) and not self._is_at_end():
            statements.append(self._statement())
        self._consume(TokenType.RIGHT_BRACE, "Expected '}' after block.")
        return statements

    def _expression(self) -> ast.Expression:
        return self._equality()

    def _equality(self) -> ast.Expression:
        expression = self._comparison()
        while self._match(TokenType.EQEQ, TokenType.BANGEQ):
            operator = self._previous()
            right = self._comparison()
            expression = ast.Binary(expression, operator.lexeme, right, operator.line)
        return expression

    def _comparison(self) -> ast.Expression:
        expression = self._term()
        while self._match(TokenType.GT, TokenType.GTE, TokenType.LT, TokenType.LTE):
            operator = self._previous()
            right = self._term()
            expression = ast.Binary(expression, operator.lexeme, right, operator.line)
        return expression

    def _term(self) -> ast.Expression:
        expression = self._factor()
        while self._match(TokenType.PLUS, TokenType.MINUS):
            operator = self._previous()
            right = self._factor()
            expression = ast.Binary(expression, operator.lexeme, right, operator.line)
        return expression

    def _factor(self) -> ast.Expression:
        expression = self._unary()
        while self._match(TokenType.STAR, TokenType.SLASH):
            operator = self._previous()
            right = self._unary()
            expression = ast.Binary(expression, operator.lexeme, right, operator.line)
        return expression

    def _unary(self) -> ast.Expression:
        if self._match(TokenType.MINUS):
            operator = self._previous()
            operand = self._unary()
            return ast.Unary(operator.lexeme, operand, operator.line)
        return self._call()

    def _call(self) -> ast.Expression:
        expression = self._primary()
        while self._match(TokenType.LEFT_PAREN):
            expression = self._finish_call(expression)
        return expression

    def _finish_call(self, callee: ast.Expression) -> ast.Call:
        paren = self._previous()
        arguments: list[ast.Expression] = []
        if not self._check(TokenType.RIGHT_PAREN):
            while True:
                arguments.append(self._expression())
                if not self._match(TokenType.COMMA):
                    break
        self._consume(TokenType.RIGHT_PAREN, "Expected ')' after arguments.")
        return ast.Call(callee, arguments, paren.line)

    def _primary(self) -> ast.Expression:
        if self._match(TokenType.NUMBER, TokenType.STRING):
            token = self._previous()
            return ast.Literal(token.literal, token.line)
        if self._match(TokenType.TRUE, TokenType.FALSE, TokenType.NULL):
            token = self._previous()
            return ast.Literal(token.literal, token.line)
        if self._match(TokenType.IDENTIFIER):
            token = self._previous()
            return ast.Variable(token.lexeme, token.line)
        if self._match(TokenType.LEFT_PAREN):
            token = self._previous()
            expression = self._expression()
            self._consume(TokenType.RIGHT_PAREN, "Expected ')' after expression.")
            return ast.Grouping(expression, token.line)

        token = self._peek()
        raise self._error(token, f"Expected expression, found '{token.lexeme or 'end of file'}'.")

    def _match(self, *token_types: TokenType) -> bool:
        for token_type in token_types:
            if self._check(token_type):
                self._advance()
                return True
        return False

    def _consume(self, token_type: TokenType, message: str) -> Token:
        if self._check(token_type):
            return self._advance()
        raise self._error(self._peek(), message)

    def _check(self, token_type: TokenType) -> bool:
        if self._is_at_end():
            return token_type == TokenType.EOF
        return self._peek().type == token_type

    def _check_next(self, token_type: TokenType) -> bool:
        if self.current + 1 >= len(self.tokens):
            return False
        return self.tokens[self.current + 1].type == token_type

    def _advance(self) -> Token:
        if not self._is_at_end():
            self.current += 1
        return self._previous()

    def _is_at_end(self) -> bool:
        return self._peek().type == TokenType.EOF

    def _peek(self) -> Token:
        return self.tokens[self.current]

    def _previous(self) -> Token:
        return self.tokens[self.current - 1]

    def _line_of(self, expression: ast.Expression) -> int:
        return getattr(expression, "line", 0)

    def _error(self, token: Token, message: str) -> ParserError:
        return ParserError(f"Error: {message} at line {token.line}, column {token.column}")
