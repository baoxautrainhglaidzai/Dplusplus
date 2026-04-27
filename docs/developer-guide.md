# D++ Developer Guide

This guide is for developers who want to write D++ programs, embed the runtime, or extend the language implementation.

## What D++ is

D++ is a small interpreted language designed to stay readable, easy to parse, and easy to extend.

Current goals:

- Keep the syntax minimal.
- Make the runtime simple enough to understand in one sitting.
- Be friendly to experiments, tooling, and AI-generated code.

Non-goals:

- Production-grade performance.
- A large standard library.
- Complex syntax or advanced type features.

## Quick Start

Build the native runtime:

```sh
make
```

Run a D++ program:

```sh
./build/dpp examples/moonlit_tea_party.dpp
```

Run the native test suite:

```sh
make test
```

The main executable is `build/dpp`.

## First Program

```dpp
let x = 5
let y = 10

fn multiply(a, b) {
    return a * b
}

let result = multiply(x, y)
print result
```

Expected output:

```text
50
```

## Language Overview

D++ programs are made of statements and expressions. Blocks use braces. Semicolons are not required.

### Comments

Use `//` for single-line comments.

```dpp
// This is a comment
print "Hello"
```

### Variables

Variables are dynamically typed.

```dpp
let name = "D++"
let score = 10
let ready = true

score = score + 1
```

Rules:

- Declare with `let`.
- Reassign with `name = value`.
- Reading an undefined variable is a runtime error.

### Functions

Functions use `fn`.

```dpp
fn greet(name) {
    print "Hello, " + name
}

greet("forest")
```

Rules:

- Parameters are positional.
- `return` is optional.
- `return` at top level is a runtime error.
- Functions close over outer variables, so basic closures work.

Example:

```dpp
let prefix = "Tea: "

fn announce(name) {
    print prefix + name
}

announce("mint")
```

### Conditionals

```dpp
if score > 10 {
    print "big"
} else if score > 5 {
    print "medium"
} else {
    print "small"
}
```

Notes:

- `else if` is supported.
- Conditions use D++ truthiness rules described below.

### Loops

D++ currently supports `for` loops over inclusive numeric ranges.

```dpp
for i in 0..5 {
    print i
}
```

Descending ranges also work:

```dpp
for i in 3..1 {
    print i
}
```

Notes:

- Both ends of the range are included.
- Loop bounds must evaluate to integers.
- There is no `while`, `break`, or `continue` yet.

### Print

`print` is a statement, not a function.

```dpp
print "Hello"
print 42
print "Score: " + 5
```

The parser also accepts `print(expression)` for convenience.

## Types and Runtime Behavior

Current runtime values:

- `number`
- `bool`
- `string`
- `null`
- function values

### Number

Internally numbers are stored as `double`.

```dpp
let x = 3
let y = 2.5
print x + y
```

### Boolean

Boolean literals are:

```dpp
true
false
```

### String

Strings use double quotes.

```dpp
let name = "Moonlit Tea Party"
```

Supported escapes:

- `\"`
- `\\`
- `\n`
- `\t`
- `\r`

### Null

Use `null` for an empty value.

```dpp
let current = null
```

### Truthiness

D++ treats values as truthy or falsy in `if` conditions.

- `null` is false.
- `false` is false.
- numeric `0` is false.
- empty string `""` is false.
- non-zero numbers are true.
- non-empty strings are true.
- functions are true.

## Operators

Supported operators:

- `+`
- `-`
- `*`
- `/`
- `==`
- `!=`
- `>`
- `>=`
- `<`
- `<=`

Behavior:

- `+` adds numbers.
- `+` concatenates strings.
- `+` also concatenates mixed values by stringifying them.
- `-`, `*`, `/` require numbers.
- comparison operators require numbers.
- equality compares values by type and value.

Examples:

```dpp
print 1 + 2
print "Tea " + "Party"
print "Joy: " + 5
print 10 > 3
```

## Built-in Functions

The standard library is intentionally small.

### `input()`

Reads one line of text from standard input.

```dpp
print "What is your name?"
let name = input()
print "Hello, " + name
```

### `sqrt(x)`

Returns the square root of a non-negative number.

```dpp
print sqrt(81)
```

### `random()`

Returns a floating-point value between `0.0` and `1.0`.

```dpp
print random()
```

## Error Handling

D++ tries to keep errors descriptive and human-readable.

Examples:

```text
Error: Undefined variable 'x' at line 3
Error: Expected variable name. at line 1, column 5
Error: Division by zero at line 8
```

Error categories:

- `LexerError`
- `ParserError`
- `RuntimeError`

The CLI prints the error message and exits with a non-zero status.

## Writing Programs

Recommended style:

- Prefer explicit names.
- Keep functions short.
- Favor straightforward control flow.
- Avoid clever tricks.
- Use comments sparingly and only when they clarify intent.

Recommended file extension:

- `.dpp`

## Embedding D++ in C++

The public runtime entrypoints are declared in [`include/dpp/runtime.hpp`](../include/dpp/runtime.hpp).

### Run a source string

```cpp
#include <string>
#include "dpp/runtime.hpp"

int main() {
    std::string source = R"(
        print "Hello from embedded D++"
    )";

    dpp::runSource(source);
}
```

### Provide custom input, output, or randomness

```cpp
#include <string>
#include <vector>
#include "dpp/runtime.hpp"

int main() {
    std::vector<std::string> outputs;

    dpp::runSource(
        R"(
            print input()
            print random()
        )",
        []() {
            return std::string("mint tea");
        },
        [&outputs](const std::string& line) {
            outputs.push_back(line);
        },
        []() {
            return 0.5;
        });
}
```

Use this when you want:

- deterministic tests
- custom REPL or editor integration
- embedding in a larger native application

## Runtime Pipeline

The execution flow is:

1. Source text goes through the lexer.
2. Tokens go through the parser.
3. The parser builds an AST.
4. The interpreter walks the AST and executes it.

Key files:

- [`src/lexer.cpp`](../src/lexer.cpp)
- [`src/parser.cpp`](../src/parser.cpp)
- [`src/interpreter.cpp`](../src/interpreter.cpp)
- [`src/runtime.cpp`](../src/runtime.cpp)

## Project Layout

Main directories:

- `include/dpp`: public headers and AST/value definitions
- `src`: native runtime implementation
- `examples`: sample D++ programs
- `tests`: native regression tests
- `dpp`: legacy Python reference implementation

The native C++ runtime is the primary implementation.

## Extending the Language

If you want to add a new language feature, keep the implementation order strict:

1. Add or update token definitions in [`include/dpp/token.hpp`](../include/dpp/token.hpp).
2. Teach the lexer to recognize the new syntax in [`src/lexer.cpp`](../src/lexer.cpp).
3. Add AST nodes if the feature needs new syntax forms in [`include/dpp/ast.hpp`](../include/dpp/ast.hpp).
4. Parse the new construct in [`src/parser.cpp`](../src/parser.cpp).
5. Execute it in [`src/interpreter.cpp`](../src/interpreter.cpp).
6. Add or update tests in [`tests/test_dpp.cpp`](../tests/test_dpp.cpp).

Guidelines for extensions:

- Preserve the minimal syntax style.
- Keep errors short and readable.
- Prefer explicit behavior over hidden magic.
- Do not add features that require large amounts of punctuation unless the value is clear.

## Known Limitations

Current limitations:

- No modules or imports.
- No arrays, maps, or objects.
- No user-defined classes.
- No `while`, `break`, or `continue`.
- No logical operators like `and` or `or`.
- No exception handling.
- No static typing.
- No bytecode compiler or VM.
- Single-file execution only.

D++ is still a playground language, not a production platform.

## Examples

Sample programs:

- [`examples/moonlit_tea_party.dpp`](../examples/moonlit_tea_party.dpp)
- [`examples/treasure_cave.dpp`](../examples/treasure_cave.dpp)

## Testing

The native tests live in [`tests/test_dpp.cpp`](../tests/test_dpp.cpp).

Run them with:

```sh
make test
```

The test suite covers:

- arithmetic
- variables and assignment
- functions and returns
- conditionals
- inclusive ascending and descending loops
- built-ins
- runtime errors
- example program execution

## Suggested Next Steps

If you want to keep evolving D++, the most natural next features are:

- `while` loops
- `break` and `continue`
- lists or arrays
- modules/imports
- a small REPL
- a bytecode backend
