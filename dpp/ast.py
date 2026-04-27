"""AST node definitions for D++."""

from __future__ import annotations

from dataclasses import dataclass


@dataclass(slots=True)
class Program:
    statements: list["Statement"]


class Statement:
    pass


class Expression:
    pass


@dataclass(slots=True)
class VarDeclaration(Statement):
    name: str
    value: Expression
    line: int


@dataclass(slots=True)
class Assignment(Statement):
    name: str
    value: Expression
    line: int


@dataclass(slots=True)
class FunctionDeclaration(Statement):
    name: str
    params: list[str]
    body: list[Statement]
    line: int


@dataclass(slots=True)
class ReturnStatement(Statement):
    value: Expression | None
    line: int


@dataclass(slots=True)
class PrintStatement(Statement):
    value: Expression
    line: int


@dataclass(slots=True)
class IfStatement(Statement):
    condition: Expression
    then_branch: list[Statement]
    else_branch: list[Statement] | None
    line: int


@dataclass(slots=True)
class ForStatement(Statement):
    iterator: str
    start: Expression
    end: Expression
    body: list[Statement]
    line: int


@dataclass(slots=True)
class ExpressionStatement(Statement):
    expression: Expression
    line: int


@dataclass(slots=True)
class Literal(Expression):
    value: object
    line: int


@dataclass(slots=True)
class Variable(Expression):
    name: str
    line: int


@dataclass(slots=True)
class Grouping(Expression):
    expression: Expression
    line: int


@dataclass(slots=True)
class Unary(Expression):
    operator: str
    operand: Expression
    line: int


@dataclass(slots=True)
class Binary(Expression):
    left: Expression
    operator: str
    right: Expression
    line: int


@dataclass(slots=True)
class Call(Expression):
    callee: Expression
    arguments: list[Expression]
    line: int
