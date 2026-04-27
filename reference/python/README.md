# D++ Python Reference Runtime

This directory contains the Python reference implementation of D++.

It is not the primary runtime. The main runtime is the native C++20 interpreter in the repository root.

The Python runtime focuses on the core language surface: variables, functions, conditionals, `for` loops, arithmetic, `print`, `input`, `sqrt`, and `random`. Advanced features may appear in the native runtime first.

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
