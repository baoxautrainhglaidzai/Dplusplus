# D++ Python Reference Runtime

Python reference runtime for D++ `v0.2.1`.

This package is not the primary runtime. The main implementation is the native C++20 interpreter in the repository root.

The Python runtime focuses on the core language surface: variables, functions, conditionals, `for` loops, arithmetic, `print`, `input`, `sqrt`, and `random`. Native-only extensions may appear in C++ first.

## Install

```sh
python -m pip install -e reference/python
```

## Run

```sh
dpp-reference examples/core_basics.dpp
```

## Purpose

- reference behavior
- fast experimentation
- implementation comparison with the C++ runtime
