[中文](README.md) | [English](README.en.md)

---

# tx-manager

A transaction manager for understanding concurrency control and isolation levels.

## Project Description

tx-manager is part of the [first-principles-cs](https://github.com/first-principles-cs) project, aiming to deeply understand database transaction concepts by building a transaction manager from scratch.

## Core Features

- MVCC (Multi-Version Concurrency Control) or 2PL (Two-Phase Locking)
- Deadlock detection
- Multiple isolation levels (Read Committed, Repeatable Read, Serializable)

## Key Invariants

1. **Atomicity**: Transactions either fully commit or fully roll back
2. **Isolation**: Concurrent transactions do not interfere with each other
3. **Consistency**: Transactions maintain database constraints
4. **Durability**: Committed transactions are never lost

## Quick Start

```bash
make
make test
./tx-test
```

## Verification

- Unit tests: test transaction operations
- Concurrency tests: test multi-threaded scenarios
- Jepsen-style tests: verify isolation levels

## Related Course

Module D — Data Systems
