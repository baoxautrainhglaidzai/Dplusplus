# AGENTS.md

Repository instructions for AI agents working on D++.

## Repository Priorities

- Treat `src/` and `include/dpp/` as the main runtime.
- Treat `reference/python/` as the reference runtime.
- Keep the repository layout clear: C++ is primary, Python is secondary.

## Language and Tooling Rules

- Keep D++ syntax minimal, explicit, and readable.
- Do not add external libraries unless explicitly requested.
- Prefer simple, human-readable errors.
- For language work, update lexer, parser, and interpreter in that order when appropriate.

## Documentation Boundaries

- Keep `README.md` user-facing: overview, quick start, usage.
- Keep `docs/` focused on technical and contributor detail.
- Keep `AGENTS.md` limited to agent behavior and repository rules.

## Change Hygiene

- Update tests when language behavior changes.
- Update docs when runtime behavior, setup, or structure changes.
- Do not commit generated files, caches, bytecode, virtual environments, or build outputs.

## Validation

- Run `make test` after changing the native runtime or shared language behavior.
- If you touch `reference/python/`, run `PYTHONPATH=reference/python python -m dpp_reference examples/core_basics.dpp`.
