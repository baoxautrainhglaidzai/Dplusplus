# D++

D++ is a lightweight experimental language with a native C++ interpreter.

Latest release: `v0.1.1` ([release notes](docs/release-notes-v0.1.1.md))

Documentation:

- [Developer Guide](docs/developer-guide.md)
- [Changelog](CHANGELOG.md)

## Build

```sh
make
```

## Run a D++ program

```sh
./build/dpp examples/moonlit_tea_party.dpp
```

## Run the native test suite

```sh
make test
```

The reference Python implementation is still in the repository, but the primary runtime is now the native C++ executable.
