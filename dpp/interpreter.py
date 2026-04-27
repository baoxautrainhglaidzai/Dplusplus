"""Tree-walking interpreter for D++."""

from __future__ import annotations

import math
import random as random_module
from dataclasses import dataclass
from typing import Callable

from . import ast
from .errors import DppRuntimeError


class Environment:
    def __init__(self, parent: "Environment | None" = None) -> None:
        self.parent = parent
        self.values: dict[str, object] = {}

    def define(self, name: str, value: object) -> None:
        self.values[name] = value

    def assign(self, name: str, value: object, line: int) -> None:
        if name in self.values:
            self.values[name] = value
            return
        if self.parent is not None:
            self.parent.assign(name, value, line)
            return
        raise DppRuntimeError(f"Error: Undefined variable '{name}' at line {line}")

    def get(self, name: str, line: int) -> object:
        if name in self.values:
            return self.values[name]
        if self.parent is not None:
            return self.parent.get(name, line)
        raise DppRuntimeError(f"Error: Undefined variable '{name}' at line {line}")


class ReturnSignal(Exception):
    def __init__(self, value: object) -> None:
        self.value = value
        super().__init__()


class CallableValue:
    def arity(self) -> int | None:
        raise NotImplementedError

    def call(self, interpreter: "Interpreter", arguments: list[object], line: int) -> object:
        raise NotImplementedError


@dataclass(slots=True)
class NativeFunction(CallableValue):
    name: str
    expected_arity: int
    implementation: Callable[[list[object], int], object]

    def arity(self) -> int:
        return self.expected_arity

    def call(self, interpreter: "Interpreter", arguments: list[object], line: int) -> object:
        try:
            return self.implementation(arguments, line)
        except ValueError as exc:
            raise DppRuntimeError(f"Error: {exc} at line {line}") from exc

    def __repr__(self) -> str:
        return f"<native fn {self.name}>"


@dataclass(slots=True)
class DppFunction(CallableValue):
    declaration: ast.FunctionDeclaration
    closure: Environment

    def arity(self) -> int:
        return len(self.declaration.params)

    def call(self, interpreter: "Interpreter", arguments: list[object], line: int) -> object:
        local_env = Environment(self.closure)
        for name, value in zip(self.declaration.params, arguments, strict=True):
            local_env.define(name, value)

        try:
            interpreter._execute_block(self.declaration.body, local_env)
        except ReturnSignal as signal:
            return signal.value
        return None

    def __repr__(self) -> str:
        return f"<fn {self.declaration.name}>"


class Interpreter:
    def __init__(
        self,
        *,
        input_provider: Callable[[], str] | None = None,
        output: Callable[[str], None] | None = None,
        random_fn: Callable[[], float] | None = None,
    ) -> None:
        self.globals = Environment()
        self.environment = self.globals
        self.input_provider = input_provider or input
        self.output = output or print
        self.random_fn = random_fn or random_module.random
        self._install_builtins()

    def execute(self, program: ast.Program) -> None:
        try:
            for statement in program.statements:
                self._execute(statement)
        except ReturnSignal as exc:
            raise DppRuntimeError("Error: 'return' can only be used inside a function") from exc

    def _install_builtins(self) -> None:
        self.globals.define(
            "input",
            NativeFunction("input", 0, lambda args, line: self.input_provider()),
        )
        self.globals.define(
            "sqrt",
            NativeFunction("sqrt", 1, lambda args, line: math.sqrt(self._require_number(args[0], line))),
        )
        self.globals.define(
            "random",
            NativeFunction("random", 0, lambda args, line: self.random_fn()),
        )

    def _execute(self, statement: ast.Statement) -> None:
        if isinstance(statement, ast.VarDeclaration):
            self.environment.define(statement.name, self._evaluate(statement.value))
            return
        if isinstance(statement, ast.Assignment):
            self.environment.assign(statement.name, self._evaluate(statement.value), statement.line)
            return
        if isinstance(statement, ast.FunctionDeclaration):
            self.environment.define(statement.name, DppFunction(statement, self.environment))
            return
        if isinstance(statement, ast.ReturnStatement):
            value = self._evaluate(statement.value) if statement.value is not None else None
            raise ReturnSignal(value)
        if isinstance(statement, ast.PrintStatement):
            value = self._evaluate(statement.value)
            self.output(self._stringify(value))
            return
        if isinstance(statement, ast.IfStatement):
            branch = statement.then_branch if self._is_truthy(self._evaluate(statement.condition)) else statement.else_branch
            if branch is not None:
                self._execute_block(branch, Environment(self.environment))
            return
        if isinstance(statement, ast.ForStatement):
            self._execute_for(statement)
            return
        if isinstance(statement, ast.ExpressionStatement):
            self._evaluate(statement.expression)
            return
        raise DppRuntimeError("Error: Unsupported statement.")

    def _execute_for(self, statement: ast.ForStatement) -> None:
        start = self._require_integer(self._evaluate(statement.start), statement.line)
        end = self._require_integer(self._evaluate(statement.end), statement.line)
        step = 1 if start <= end else -1

        for value in range(start, end + step, step):
            loop_env = Environment(self.environment)
            loop_env.define(statement.iterator, value)
            self._execute_block(statement.body, loop_env)

    def _execute_block(self, statements: list[ast.Statement], environment: Environment) -> None:
        previous = self.environment
        self.environment = environment
        try:
            for statement in statements:
                self._execute(statement)
        finally:
            self.environment = previous

    def _evaluate(self, expression: ast.Expression | None) -> object:
        if expression is None:
            return None
        if isinstance(expression, ast.Literal):
            return expression.value
        if isinstance(expression, ast.Variable):
            return self.environment.get(expression.name, expression.line)
        if isinstance(expression, ast.Grouping):
            return self._evaluate(expression.expression)
        if isinstance(expression, ast.Unary):
            operand = self._evaluate(expression.operand)
            if expression.operator == "-":
                return -self._require_number(operand, expression.line)
            raise DppRuntimeError(f"Error: Unsupported unary operator '{expression.operator}' at line {expression.line}")
        if isinstance(expression, ast.Binary):
            return self._evaluate_binary(expression)
        if isinstance(expression, ast.Call):
            return self._evaluate_call(expression)
        raise DppRuntimeError("Error: Unsupported expression.")

    def _evaluate_binary(self, expression: ast.Binary) -> object:
        left = self._evaluate(expression.left)
        right = self._evaluate(expression.right)
        operator = expression.operator

        if operator == "+":
            if isinstance(left, str) and isinstance(right, str):
                return left + right
            if self._is_number(left) and self._is_number(right):
                return left + right
            raise DppRuntimeError(f"Error: '+' requires two numbers or two strings at line {expression.line}")
        if operator == "-":
            return self._require_number(left, expression.line) - self._require_number(right, expression.line)
        if operator == "*":
            return self._require_number(left, expression.line) * self._require_number(right, expression.line)
        if operator == "/":
            denominator = self._require_number(right, expression.line)
            if denominator == 0:
                raise DppRuntimeError(f"Error: Division by zero at line {expression.line}")
            return self._require_number(left, expression.line) / denominator
        if operator == "==":
            return left == right
        if operator == "!=":
            return left != right
        if operator == ">":
            return self._require_number(left, expression.line) > self._require_number(right, expression.line)
        if operator == ">=":
            return self._require_number(left, expression.line) >= self._require_number(right, expression.line)
        if operator == "<":
            return self._require_number(left, expression.line) < self._require_number(right, expression.line)
        if operator == "<=":
            return self._require_number(left, expression.line) <= self._require_number(right, expression.line)
        raise DppRuntimeError(f"Error: Unsupported operator '{operator}' at line {expression.line}")

    def _evaluate_call(self, expression: ast.Call) -> object:
        callee = self._evaluate(expression.callee)
        arguments = [self._evaluate(argument) for argument in expression.arguments]

        if not isinstance(callee, CallableValue):
            raise DppRuntimeError(f"Error: Value is not callable at line {expression.line}")

        arity = callee.arity()
        if arity is not None and len(arguments) != arity:
            raise DppRuntimeError(
                f"Error: Expected {arity} arguments but got {len(arguments)} at line {expression.line}"
            )
        return callee.call(self, arguments, expression.line)

    def _require_number(self, value: object, line: int) -> float | int:
        if self._is_number(value):
            return value
        raise DppRuntimeError(f"Error: Expected a number at line {line}")

    def _require_integer(self, value: object, line: int) -> int:
        if isinstance(value, bool):
            raise DppRuntimeError(f"Error: Expected an integer at line {line}")
        if isinstance(value, int):
            return value
        if isinstance(value, float) and value.is_integer():
            return int(value)
        raise DppRuntimeError(f"Error: Expected an integer at line {line}")

    def _is_number(self, value: object) -> bool:
        return isinstance(value, (int, float)) and not isinstance(value, bool)

    def _is_truthy(self, value: object) -> bool:
        return bool(value)

    def _stringify(self, value: object) -> str:
        if value is None:
            return "null"
        if isinstance(value, bool):
            return "true" if value else "false"
        return str(value)
