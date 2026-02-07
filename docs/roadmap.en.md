[中文](roadmap.md) | [English](roadmap.en.md)

---

# ROADMAP — 12-Month Roadmap

This document outlines the development roadmap for the next 12 months, broken down by quarter with goals and acceptance criteria.

## Overall Strategy

We adopt a "depth-first" strategy: each quarter focuses on one core system, from interface definition to full implementation, ensuring every system meets the standard of "verifiable, diagnosable, refactorable."

## Q1 (Months 1–3): Foundations & Language Execution

### Goals

Build the minimal computing infrastructure; understand "how programs run."

### Milestones

1. **Tiny Interpreter** — Minimal interpreter
   - Variables, functions, recursion
   - Lexical analysis, parsing, evaluation
   - Key invariants: scoping rules, tail-call optimization

2. **Simple Compiler** — Simple compiler
   - Compiles to bytecode or assembly
   - Register allocation, code generation
   - Key invariants: type safety, memory safety

### Acceptance Criteria

- [ ] Can run recursive Fibonacci
- [ ] Passes 100+ unit tests
- [ ] Property-based tests verify scoping rules
- [ ] Complete design docs and failure-model analysis

### Dependencies

No prerequisites — can start immediately.

## Q2 (Months 4–6): Single-Machine Systems

### Goals

Understand the core OS abstractions: processes, memory, files.

### Milestones

1. **Mini OS Kernel** — Minimal operating system kernel
   - Process scheduling, virtual memory, system calls
   - Key invariants: process isolation, memory protection

2. **Simple File System** — Simple file system
   - Inodes, directories, caching
   - Key invariants: crash consistency, atomicity

### Acceptance Criteria

- [ ] Can run multiple concurrent processes
- [ ] Passes fault-injection tests (process crash, disk corruption)
- [ ] Fuzz-tested file system interface
- [ ] Performance benchmarks and regression tests

### Dependencies

Requires Q1's compiler to compile kernel code.

## Q3 (Months 7–9): Data Systems

### Goals

Understand database fundamentals: storage, indexing, transactions.

### Milestones

1. **Storage Engine** — Storage engine
   - LSM-tree or B-tree implementation
   - Key invariants: durability, ordering

2. **Transaction Manager** — Transaction manager
   - MVCC or 2PL implementation
   - Key invariants: ACID properties

### Acceptance Criteria

- [ ] Can handle 1M+ records
- [ ] Passes Jepsen-style consistency tests
- [ ] Benchmarks comparing different index structures
- [ ] Complete failure-recovery tests

### Dependencies

Requires Q2's file system for data persistence.

## Q4 (Months 10–12): Networking & Distributed Systems

### Goals

Understand distributed systems fundamentals: communication, consistency, fault tolerance.

### Milestones

1. **Consensus Protocol** — Consensus protocol
   - Raft or Paxos implementation
   - Key invariants: safety, liveness

2. **Distributed KV Store** — Distributed key-value store
   - Sharding, replication, failover
   - Key invariants: linearizability or eventual consistency

### Acceptance Criteria

- [ ] Tolerates f node failures (f < n/2)
- [ ] Passes network-partition tests
- [ ] Jepsen-verified consistency
- [ ] Complete performance and availability analysis

### Dependencies

Requires Q3's storage engine for state persistence.

## Future Directions (Month 13+)

Based on the first 12 months of experience, possible extensions:

1. **AI Systems** — Implement a simple neural-network training framework
2. **Search Systems** — Implement inverted indexes and ranking algorithms
3. **Advanced Topics** — Formal verification, performance optimization, security analysis

## Risks & Mitigations

### Risk 1: Inaccurate time estimates

**Mitigation**: Re-evaluate at the end of each quarter; adjust subsequent plans as needed.

### Risk 2: Unexpected technical difficulty

**Mitigation**: Reduce system complexity; focus on core invariants; drop non-essential features.

### Risk 3: Documentation falling behind code

**Mitigation**: Design docs first; every milestone must include complete documentation to be considered done.

## Changelog

- 2025-01: Initial version

---

Next: See [PROJECTS.md](./projects.en.md) for the specific system repositories.
