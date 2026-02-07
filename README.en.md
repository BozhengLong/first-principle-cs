[中文](README.md) | [English](README.en.md)

---

# first-principles-cs

A first-principles computer science curriculum for the AI era.

## What Is This

This is a project that rebuilds computer science infrastructure from first principles. We're not satisfied with merely "knowing how to use tools" — we want to understand how the world is built, so that even if one day it's just us and an AI, we can rebuild it from scratch.

## Core Principles

We follow four principles to build and verify systems:

1. **Interfaces & Invariants** — Define system boundaries clearly; specify properties that must always hold
2. **Failure Models** — Identify all possible failure scenarios; design fault-tolerance mechanisms
3. **Verification First** — Test first; use property-based testing and fault injection to verify invariants
4. **Refactoring Log** — Continuously simplify implementations while keeping tests green; document design evolution

See the [principles/](./principles/) directory for details.

## Documentation

- [WHY — Why this project exists](./docs/why.en.md)
- [ROADMAP — 12-month roadmap](./docs/roadmap.en.md)
- [PROJECTS — Core system repository list](./docs/projects.en.md)
- [PROGRESS — Project progress tracker](./docs/progress.md) 📊
- [Principles](./principles/) — Detailed explanation of the four principles
- [Courses](./courses/) — Six learning modules

## AI Collaboration

We use a fixed collaboration workflow to build systems:

1. **Spec First** — Define interfaces, invariants, and failure models
2. **Generate Implementation** — AI generates the code
3. **Constrained Testing** — Verify invariants and failure scenarios
4. **Refactor to Reduce Complexity** — Simplify while keeping tests green

See [principles/ai-collaboration-protocol.md](./principles/ai-collaboration-protocol.md) for details.

## Templates

We provide unified templates to keep projects consistent:

- [Repository README Template](./templates/repo-readme-template.md)
- [Design Document Template](./templates/design-doc-template.md)
- [Experiment Template](./templates/experiment-template.md)

## License

This project is licensed under the MIT License. See [LICENSE](./LICENSE).
