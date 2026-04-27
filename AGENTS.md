Agents.md — D++ Programming Language

Overview

D++ is a lightweight, experimental programming language designed to be simple to parse, easy to extend, and friendly for AI-assisted code generation. It focuses on minimal syntax, strong readability, and automation-friendly constructs.

D++ is not intended to replace existing languages, but to act as a sandbox language for compilers, interpreters, and AI agents.


---

Core Philosophy

Simplicity over complexity

Human-readable syntax

Easy parsing for AI and machines

Minimal boilerplate

Built for experimentation and automation



---

Language Features

1. Variables

Variables are dynamically typed.

let x = 10
let name = "D++"


---

2. Functions

Functions are defined using fn keyword.

fn add(a, b) {
    return a + b
}


---

3. Print / Output

print "Hello D++"
print x


---

4. Conditionals

if x > 10 {
    print "big"
} else {
    print "small"
}


---

5. Loops

for i in 0..5 {
    print i
}


---

Operators

+ addition

- subtraction

* multiplication

/ division

== equality

!= not equal

> < comparisons



---

Runtime Model

D++ runs on a simple interpreted execution model:

1. Source code → Tokenizer


2. Tokens → Parser (AST)


3. AST → Interpreter execution



No compilation step is required in the base version.


---

Standard Library (minimal)

IO

print(value)
input()

Math

sqrt(x)
random()


---

Example Program

let x = 5
let y = 10

fn multiply(a, b) {
    return a * b
}

let result = multiply(x, y)
print result


---

Agent Instructions (for Codex / AI systems)

When generating or modifying D++ code:

Always follow minimal syntax rules

Do not introduce external libraries unless explicitly requested

Preserve readability over optimization

Prefer explicit constructs over implicit behavior

Ensure generated code is valid D++ syntax


When building tools around D++:

Implement lexer first

Then parser (AST builder)

Then interpreter

Keep error messages simple and human-readable



---

Error Handling

Errors should be descriptive:

Error: Undefined variable 'x' at line 3


---

Future Extensions

Type system (optional static typing)

Modules/import system

Async execution model

AI-native commands (experimental)



---

Goal

D++ is designed as a playground language for:

learning compilers

testing AI code generation

building interpreters quickly


Not for production systems.
