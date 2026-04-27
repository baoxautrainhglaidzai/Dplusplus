"""Public API for running D++ programs."""

from __future__ import annotations

from typing import Callable

from .ast import Program
from .interpreter import Interpreter
from .lexer import Lexer
from .parser import Parser


def parse_source(source: str) -> Program:
    tokens = Lexer(source).tokenize()
    return Parser(tokens).parse()


def run_source(
    source: str,
    *,
    input_provider: Callable[[], str] | None = None,
    output: Callable[[str], None] | None = None,
    random_fn: Callable[[], float] | None = None,
) -> Interpreter:
    program = parse_source(source)
    interpreter = Interpreter(input_provider=input_provider, output=output, random_fn=random_fn)
    interpreter.execute(program)
    return interpreter
