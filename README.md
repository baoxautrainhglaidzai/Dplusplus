# D++

D++ is a small, experimental programming language designed to be easy to read, easy to parse, and easy to extend.

The project is a practical sandbox for language design, interpreters, tooling, and AI-assisted code generation. It is not intended to replace production languages. The focus is clarity, minimal syntax, and fast experimentation.

## Purpose

D++ is built for:

- learning how lexers, parsers, and interpreters work
- testing language ideas without large compiler complexity
- building simple tooling around a predictable syntax
- exploring AI-friendly code generation and transformation

## Runtime Model

- **Main runtime:** native C++20 implementation in `src/` and `include/`
- **Reference runtime:** Python implementation in `reference/python/dpp_reference/`

The C++ runtime is the primary executable and the main tested target. The Python runtime is a reference implementation for the core language surface and may lag newer native features.

## Key Features

- minimal syntax with dynamic variables and explicit blocks
- functions with closures and `return`
- conditionals with `if`, `else if`, and `else`
- loops with `for`, `while`, `break`, and `continue`
- arithmetic, comparison, modulo, and short-circuit logical operators
- mutable lists with indexing and indexed assignment
- small built-in library: `input`, `sqrt`, `random`, `len`, `push`, `pop`

## Project Structure

```text
.
├── src/                      # Main C++ runtime implementation
├── include/dpp/             # Public headers, AST, tokens, values
├── tests/                   # Native regression tests
├── examples/                # Example D++ programs
├── docs/                    # Developer-facing repository documentation
├── reference/python/        # Python reference runtime package
├── Makefile                 # Build, test, install, clean
├── README.md
└── CHANGELOG.md
```

## Installation and Build

### C++ Runtime

Requirements:

- C++20 compiler (`clang++` or `g++`)
- `make`

Build:

```sh
make
```

Test:

```sh
make test
```

Run directly:

```sh
./build/dpp examples/systems_demo.dpp
```

Install locally:

```sh
make install PREFIX=$HOME/.local
```

If `$HOME/.local/bin` is on your `PATH`, you can then run:

```sh
dpp examples/moonlit_tea_party.dpp
```

Termux install example:

```sh
make install PREFIX=$PREFIX
```

### Python Reference Runtime

Requirements:

- Python 3.10+
- `pip`

Scope:

- supports the core D++ feature set from the original interpreter design
- useful for experimentation and behavior reference
- advanced native features may arrive in C++ first

Install in editable mode:

```sh
python -m pip install -e reference/python
```

Run after installation:

```sh
dpp-reference examples/core_basics.dpp
```

Run without installation:

```sh
PYTHONPATH=reference/python python -m dpp_reference examples/core_basics.dpp
```

## Usage Examples

### Functions and arithmetic

```dpp
let x = 5
let y = 10

fn multiply(a, b) {
    return a * b
}

print multiply(x, y)
```

### Lists and loops

```dpp
let values = [1, 2, 3]
push(values, 4)

for i in 0..3 {
    print values[i]
}
```

### While + control flow

```dpp
let i = 0

while i < 10 {
    i = i + 1

    if i % 2 == 0 {
        continue
    }

    if i > 7 {
        break
    }

    print i
}
```

## Included Examples

- `examples/core_basics.dpp` - shared core-language example for both runtimes
- `examples/treasure_cave.dpp` - interactive text adventure using the core language
- `examples/moonlit_tea_party.dpp` - native-focused showcase game
- `examples/systems_demo.dpp` - native-focused advanced feature demo

## Documentation

- `CHANGELOG.md`
- `docs/developer-guide.md`

## Roadmap

- module/import support
- maps or dictionaries
- expanded standard library
- REPL workflow
- bytecode or VM backend
- cross-runtime conformance testing between C++ and Python

## Contributing

Contributions should keep D++ small, explicit, and easy to understand.

Guidelines:

- prefer minimal syntax over clever syntax
- keep runtime behavior easy to reason about
- update tests when language behavior changes
- update docs when setup, layout, or features change
- do not commit generated files or build outputs

Recommended contributor flow:

```sh
make test
git status
```

## Release

Current release: `v0.2.1`
