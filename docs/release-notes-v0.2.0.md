# Release Notes - v0.2.0

Release date: 2026-04-27

## Highlights

- Added real control-flow power: `while`, `break`, and `continue`.
- Added lists: list literals (`[1, 2]`), indexing (`items[0]`), and index assignment (`items[0] = 7`).
- Added logical operators with short-circuit evaluation: `!`, `&&`, `||`, plus `and` / `or`.
- Added `%` modulo operator.
- Added built-ins for data work: `len`, `push`, `pop`.

## Language Changes

### New syntax

- `while <condition> { ... }`
- `break` and `continue` inside loops
- list literals: `[a, b, c]`
- index access: `items[0]`
- index assignment: `items[0] = value`

### New operators

- `%` modulo
- `!` not
- `&&` / `||` and `and` / `or` (short-circuit)

### New built-ins

- `len(value)` for strings and lists
- `push(list, value)` appends and returns new size
- `pop(list)` removes and returns the last element

## Examples

- `examples/systems_demo.dpp` demonstrates `while`, modulo, list mutation, `break`/`continue`, and `len/push/pop`.

## Developer Notes

- The parser now treats assignment as an expression (enables indexed assignment targets cleanly).
- The native regression suite was expanded to cover all features listed above.
