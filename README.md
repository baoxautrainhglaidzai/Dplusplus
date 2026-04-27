# D++

D++ is a small, experimental programming language for learning interpreter design, testing language ideas, and building automation-friendly tooling.

The language is intentionally minimal: readable syntax, simple runtime rules, and a codebase that stays easy to parse and extend.

Current release: `v0.2.1`

## What D++ Is For

- learning how lexers, parsers, and interpreters fit together
- experimenting with small language features without compiler-heavy infrastructure
- building predictable examples for tooling and AI-assisted code generation

## Runtime Layout

- **Main runtime:** native C++20 implementation in `src/` and `include/`
- **Reference runtime:** Python implementation in `reference/python/dpp_reference/`

The C++ runtime is the primary executable and the main tested target. The Python runtime is a smaller reference implementation for the core language surface.

## Quick Start

### Build and run the native runtime

```sh
make
make test
./build/dpp examples/systems_demo.dpp
```

Install it locally:

```sh
make install PREFIX=$HOME/.local
```

Termux install example:

```sh
make install PREFIX=$PREFIX
```

### Run the Python reference runtime

```sh
python -m pip install -e reference/python
dpp-reference examples/core_basics.dpp
```

Or without installation:

```sh
PYTHONPATH=reference/python python -m dpp_reference examples/core_basics.dpp
```

## Language Snapshot

Shared core example:

```dpp
let x = 5
let y = 10

fn multiply(a, b) {
    return a * b
}

let result = multiply(x, y)

if result > 20 {
    print "big"
} else {
    print "small"
}

for i in 1..3 {
    print i
}

print result
```

Native runtime extensions:

```dpp
let values = [1, 2, 3]
push(values, 4)

while len(values) > 0 {
    print pop(values)
}
```

## Features

- dynamic variables and explicit block syntax
- functions with closures and `return`
- `if`, `else if`, and `else`
- `for`, `while`, `break`, and `continue`
- arithmetic, comparison, modulo, and logical operators
- list literals, indexing, and indexed assignment
- built-ins: `input`, `sqrt`, `random`, `len`, `push`, `pop`

## Repository Structure

```text
src/                   main C++ runtime
include/dpp/           native headers, AST, tokens, values
reference/python/      Python reference runtime package
examples/              example D++ programs
tests/                 native regression tests
docs/                  technical and contributor documentation
```

## Examples

- `examples/core_basics.dpp` runs in both runtimes
- `examples/treasure_cave.dpp` is a core-language text adventure
- `examples/moonlit_tea_party.dpp` showcases the native runtime
- `examples/systems_demo.dpp` exercises advanced native features

## Documentation

- `README.md` gives the project overview and quick start
- `docs/developer-guide.md` covers build details, architecture, and contributor workflow
- `CHANGELOG.md` tracks release history

## Roadmap

- module/import support
- dictionary-style data structures
- expanded standard library
- REPL workflow
- bytecode or VM backend
- stronger cross-runtime conformance coverage

## Contributing

Keep changes small, explicit, and testable. Run `make test` when the native runtime or language behavior changes, and keep README and docs aligned with the current repository layout.
