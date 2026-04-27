# D++ Developer Guide

This guide covers the technical structure of the D++ repository for `v0.2.1`.

## Documentation Boundaries

- `README.md` is the user-facing overview and quick start.
- `docs/` contains technical repository details and contributor guidance.
- `AGENTS.md` is reserved for AI/agent operating rules.

## Runtime Roles

- `src/` + `include/` contain the main D++ runtime written in C++20.
- `reference/python/dpp_reference/` contains the Python reference interpreter.
- The C++ runtime is the primary implementation and the main tested target.
- The Python runtime exists for core-language reference, experimentation, and implementation comparison.
- Advanced language features may land in the C++ runtime before they are mirrored in Python.

## Repository Layout

- `src/` native lexer, parser, interpreter, runtime API, and CLI
- `include/dpp/` public headers, AST, token, and value definitions
- `tests/` native regression tests
- `examples/` sample D++ programs
- `docs/` developer-facing repository documentation
- `reference/python/` Python reference package and packaging metadata

## Native Build, Test, and Install

Build the native runtime:

```sh
make
```

Run the native test suite:

```sh
make test
```

Install the native runtime:

```sh
make install PREFIX=$HOME/.local
```

Key native entrypoints:

- `src/lexer.cpp`
- `src/parser.cpp`
- `src/interpreter.cpp`
- `src/runtime.cpp`
- `src/main.cpp`

## Python Reference Runtime

The Python runtime is a reference implementation for the core language surface:

- variables and assignment
- functions and `return`
- conditionals
- `for` loops
- arithmetic and comparison
- `print`, `input`, `sqrt`, and `random`

It is intentionally not the primary runtime, and advanced native features may arrive there first.

Install the Python reference runtime in editable mode:

```sh
python -m pip install -e reference/python
```

Run it after installation:

```sh
dpp-reference examples/core_basics.dpp
```

Or run it directly from the repo without installation:

```sh
PYTHONPATH=reference/python python -m dpp_reference examples/core_basics.dpp
```

## Execution Pipeline

The native runtime flow is:

1. Source text is tokenized by `src/lexer.cpp`.
2. Tokens are parsed into an AST by `src/parser.cpp`.
3. The AST is executed by `src/interpreter.cpp`.
4. The public runtime API is exposed by `src/runtime.cpp`.

Main code references:

- `include/dpp/token.hpp`
- `include/dpp/ast.hpp`
- `include/dpp/value.hpp`

## Contributor Workflow

When changing the language:

1. Update tokens if syntax changes.
2. Update AST nodes if the feature needs new structure.
3. Update the parser and interpreter together.
4. Extend `tests/test_dpp.cpp`.
5. Update `README.md`, `CHANGELOG.md`, and any affected technical docs.

Keep changes:

- minimal
- explicit
- readable
- easy to test

## Release Hygiene

Before cutting a release:

1. Run `make test`.
2. Smoke-test documented examples that represent the current feature set.
3. Confirm generated files are ignored.
4. Update `README.md` if setup or usage changed.
5. Update `CHANGELOG.md`.

## Scope Notes

D++ is intentionally small. Favor simple, explainable features over large amounts of syntax or hidden behavior.
