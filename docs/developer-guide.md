# D++ Developer Guide

This guide is for contributors and tool builders working on the D++ repository itself.

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

## Build and Test

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

## Python Reference Runtime

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
- `src/lexer.cpp`
- `src/parser.cpp`
- `src/interpreter.cpp`
- `src/runtime.cpp`

## Contributor Workflow

When changing the language:

1. Update tokens if syntax changes.
2. Update AST nodes if the feature needs new structure.
3. Update the parser and interpreter together.
4. Extend `tests/test_dpp.cpp`.
5. Update `README.md`, `CHANGELOG.md`, and release notes if behavior changes.

Keep changes:

- minimal
- explicit
- readable
- easy to test

## Release Hygiene

Before cutting a release:

1. Run `make test`.
2. Confirm generated files are ignored.
3. Update `README.md` if setup or usage changed.
4. Update `CHANGELOG.md`.

## Scope Notes

D++ is intentionally small. Favor simple, explainable features over large amounts of syntax or hidden behavior.
