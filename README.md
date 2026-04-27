# D++

D++ is a lightweight experimental language with a native C++20 interpreter.

Latest release: `v0.2.0` ([release notes](docs/release-notes-v0.2.0.md))

## Features (Current)

- Variables and functions (`let`, `fn`, `return`) with closures
- Conditionals (`if` / `else if` / `else`)
- Loops: `for` ranges and `while`, with `break` / `continue`
- Operators: arithmetic, comparisons, `%`, and logical short-circuit (`!`, `&&`, `||`, `and`, `or`)
- Lists: `[1, 2]`, indexing `items[0]`, assignment `items[0] = 5`
- Built-ins: `input`, `sqrt`, `random`, `len`, `push`, `pop`

## Install / Setup

### Requirements

- A C++20 compiler (`clang++` or `g++`)
- `make`

### Build

```sh
make
```

### Run

```sh
./build/dpp examples/moonlit_tea_party.dpp
./build/dpp examples/systems_demo.dpp
```

### Install to PATH (optional)

Linux/macOS (user-local install):

```sh
mkdir -p ~/.local/bin
install -m 755 build/dpp ~/.local/bin/dpp
```

Termux:

```sh
install -m 755 build/dpp $PREFIX/bin/dpp
```

After installing:

```sh
dpp examples/systems_demo.dpp
```

### Run tests

```sh
make test
```

## Docs

- [Developer Guide](docs/developer-guide.md)
- [Changelog](CHANGELOG.md)
- [Release Notes - v0.2.0](docs/release-notes-v0.2.0.md)
- [Release Notes - v0.1.1](docs/release-notes-v0.1.1.md)

## Notes

The Python interpreter in `dpp/` remains as a reference, but the primary runtime is the native C++ executable.
