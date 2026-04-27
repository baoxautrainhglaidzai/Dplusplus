# Release Notes - v0.1.1

Release date: 2026-04-27

## Highlights

- Shipped a full native D++ runtime in C++20 with lexer, parser, AST, and interpreter.
- Added first-class build and test commands (`make`, `make test`).
- Added a complete developer guide for writing, embedding, and extending D++.
- Added a friendly showcase game (`moonlit_tea_party.dpp`) to demonstrate language features.
- Added native regression tests covering core syntax, built-ins, and runtime errors.

## Included in this release

- Runtime and CLI:
  - `src/lexer.cpp`
  - `src/parser.cpp`
  - `src/interpreter.cpp`
  - `src/runtime.cpp`
  - `src/main.cpp`
- Public headers:
  - `include/dpp/*`
- Documentation:
  - `README.md`
  - `docs/developer-guide.md`
  - `CHANGELOG.md`
- Tests:
  - `tests/test_dpp.cpp`
- Examples:
  - `examples/moonlit_tea_party.dpp`
  - `examples/treasure_cave.dpp`

## Notes

- `v0.1.1` is focused on a stable native baseline and developer onboarding.
- The Python reference implementation remains in the repo for comparison and experimentation.
