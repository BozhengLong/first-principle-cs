[中文](README.md) | [English](README.en.md)

---

# Tiny Interpreter

> Build a programming language interpreter from scratch to understand how programs are "read" and "executed."

## What You Will Learn

After completing this project, you will be able to answer:

- **How are programs "read"?** (Lexical analysis + parsing)
- **How are variables "remembered"?** (Environment model)
- **How do functions "work"?** (Evaluation rules)
- **How do closures "capture" their environment?** (Lexical scoping)

---

## Getting Started

### Option 1: Interactive Learning (Recommended)

This is a **learning product**, not just code. We designed a progressive learning path:

```bash
# 1. Clone the repository
git clone https://github.com/first-principles-cs/tiny-interpreter.git
cd tiny-interpreter

# 2. Install dependencies
pip install pytest

# 3. Start your learning journey
cd learn/00-introduction
python playground.py
```

### Option 2: View the Implementation Directly

If you want to jump straight to the full implementation, the code is in `src/`:

```bash
# Run the REPL
python -m src.tiny_interpreter.main

# Run an example
python -m src.tiny_interpreter.main examples/factorial.lisp

# Run tests
pytest tests/ -v
```

---

## Learning Path

```
Module 0: Introduction ────── "Why learn about interpreters?"
    ↓
Module 1: Lexical Analysis ── "How to turn strings into meaningful tokens?"
    ↓
Module 2: Parsing ─────────── "How to organize tokens into a tree structure?"
    ↓
Module 3: Environment Model ─ "Where are variable values stored?"
    ↓
Module 4: Basic Evaluation ── "How to compute the value of an expression?"
    ↓
Module 5: Lambda & Closures ─ "How do functions remember their environment?" (Aha moment!)
    ↓
Module 6: Integration ─────── "How to extend the language?"
```

---

## Project Structure

```
tiny-interpreter/
├── learn/                  # Learning modules (start here)
│   ├── 00-introduction/   # Introduction and motivation
│   ├── 01-lexer/          # Lexical analysis
│   ├── 02-parser/         # Parsing
│   ├── 03-environment/    # Environment model
│   ├── 04-evaluator-basic/# Basic evaluation
│   ├── 05-evaluator-lambda/# Lambda & closures
│   └── 06-putting-together/# Integration & extension
│
├── src/tiny_interpreter/   # Complete reference implementation
├── tests/                  # Test suite
├── examples/               # Example programs
├── tools/                  # Learning helper tools
└── docs/                   # Documentation
```

---

## Language Features

Tiny Interpreter implements a Lisp-style language:

```lisp
; Define a variable
(define x 42)

; Define a function
(define square (lambda (x) (* x x)))

; Conditional expression
(if (< x 10) "small" "large")

; Closures
(define make-adder
  (lambda (x)
    (lambda (y) (+ x y))))

(define add5 (make-adder 5))
(add5 3)  ; => 8
```

---

## Helper Tools

```bash
# Check learning progress
python tools/check_progress.py

# AST visualization
python tools/visualize_ast.py "(+ 1 (* 2 3))"
```

---

## Core API

```python
# Lexical analysis
lexer = Lexer(source_code)
tokens = lexer.tokenize()

# Parsing
parser = Parser(tokens)
ast = parser.parse()

# Evaluation
evaluator = Evaluator()
result = evaluator.run(source_code)
```

---

## Key Invariants

1. **Lexical scoping**: Variable lookup follows static scope rules
2. **Closure capture**: Closures capture the environment at definition time, not call time
3. **Parenthesis matching**: Every opening parenthesis must have a matching closing one
4. **Type safety**: Operators can only be applied to operands of the correct type

---

## Design Document

See [docs/design.md](docs/design.md) for detailed technical design.

---

## License

MIT License — see [LICENSE](LICENSE)

---

## Related Course

This project is part of the [first-principles-cs](https://github.com/first-principles-cs/guide) curriculum.
