# Changelog

All notable changes to D++ are documented in this file.

## [Unreleased]

No unreleased changes yet.

## [v0.2.1] - 2026-04-27

### Changed

- Standardized the repository layout to make the native C++ runtime the clear primary implementation.
- Moved the Python runtime into `reference/python/dpp_reference` and documented it as a reference implementation.
- Clarified that the Python runtime targets the core D++ feature set and may lag native-only extensions.
- Rewrote `README.md` with clearer project purpose, setup, usage, structure, roadmap, and contributing guidance.
- Reworked the developer guide to focus on architecture and contributor workflow.
- Added editable-install packaging for the Python reference runtime.
- Added `make install` and `make clean` targets for the native runtime.

### Removed

- Tracked Python bytecode and other generated artifacts from version control.
- Redundant per-version release-note documents that duplicated the changelog.

## [v0.2.0] - 2026-04-27

### Added

- `while` loops with `break` and `continue`.
- List literals, list indexing, and index assignment.
- Operators: `%`, `!`, `&&`, `||`, plus `and` / `or`.
- Built-ins: `len`, `push`, and `pop`.
- Advanced example script `examples/systems_demo.dpp`.

### Changed

- Parser now supports assignment as an expression, including indexed assignment targets.
- Logical operators short-circuit at runtime.
- Expanded native test coverage for advanced language features.

## [v0.1.1] - 2026-04-27

### Added

- Native C++20 interpreter pipeline (`lexer` -> `parser` -> `AST` -> `interpreter`).
- Build and test workflow with `Makefile` (`make`, `make test`).
- Developer-focused documentation in `docs/developer-guide.md`.
- Example game script `examples/moonlit_tea_party.dpp`.
- Native regression suite in `tests/test_dpp.cpp`.

### Changed

- Promoted the C++ runtime as the primary implementation.
- Improved developer onboarding in `README.md`.

## [v0.1.0] - 2026-04-27

### Added

- Initial repository setup.
- Base project license.
