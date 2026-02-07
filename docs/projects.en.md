[中文](projects.md) | [English](projects.en.md)

---

# PROJECTS — Core System Repository List

This document lists all planned system repositories, including status, goals, key invariants, and more.

## Repository List

| Repository | Status | One-line Goal | Entry Command | Key Invariants | Course Module |
|-----------|--------|--------------|---------------|----------------|---------------|
| tiny-interpreter | active | Minimal interpreter — understand evaluation & scoping | `python -m src.tiny_interpreter.main examples/factorial.lisp` | Lexical scoping, closure capture | Module B |
| simple-compiler | active | Simple compiler — understand code generation | `python -m src.simple_compiler.main examples/gcd.sl` | Type safety, memory safety | Module B |
| mini-os | active | Minimal OS kernel — understand processes & memory | `make qemu` | Process isolation, memory protection | Module C |
| simple-fs | active | Simple file system — understand persistence & consistency | `./mkfs disk.img` | Crash consistency, atomicity | Module C |
| storage-engine | active | Storage engine — understand indexing & persistence | `./storage-bench` | Durability, ordering | Module D |
| tx-manager | planned | Transaction manager — understand concurrency control | `./tx-test` | ACID properties | Module D |
| consensus | planned | Consensus protocol — understand distributed consistency | `./raft-node` | Safety, liveness | Module E |
| dist-kv | planned | Distributed KV store — understand fault tolerance | `./kv-server` | Linearizability | Module E |

## Detailed Descriptions

### tiny-interpreter

**Status**: ✅ Active

**Goal**: Implement a minimal Lisp-style interpreter to understand how programs are evaluated.

**Core Features**:
- Lexical analysis and parsing
- Environment model and closures
- Recursion support

**Key Invariants**:
- Lexical scoping: variable lookup follows static scope rules
- Closure capture: closures capture the environment at definition time

**Verification**:
- Unit tests: 43 tests all passing
- Integration tests: factorial (recursion), closure examples
- Code coverage: 100% on core components

**Quick Start**:
```bash
git clone https://github.com/first-principles-cs/tiny-interpreter.git
cd tiny-interpreter
python3 -m src.tiny_interpreter.main examples/factorial.lisp
```

**Course Module**: Module B — Language & Execution

---

### simple-compiler

**Status**: ✅ Active

**Goal**: Implement a simple compiler that compiles SimpleLang (a C-like language) to stack-based bytecode and executes it on a virtual machine.

**Core Features**:
- Lexical analysis and parsing
- Static type checking
- Three-address-code intermediate representation
- Stack-based bytecode generation
- Virtual machine execution

**Key Invariants**:
- Type safety: well-typed programs do not produce runtime type errors
- Memory safety: stack-based execution prevents memory errors
- Semantic preservation: compiled programs behave identically to source semantics

**Verification**:
- Unit tests: 75 tests covering all compilation phases
  - Lexer: 15 tests
  - Parser: 19 tests
  - Type Checker: 26 tests
  - Integration: 15 tests
- Example programs: 6 SimpleLang programs verifying functionality
- Code stats: ~1982 lines of source, ~882 lines of tests

**Quick Start**:
```bash
git clone https://github.com/first-principles-cs/simple-compiler.git
cd simple-compiler
python -m src.simple_compiler.main examples/gcd.sl
```

**Course Module**: Module B — Language & Execution

---

### mini-os

**Status**: ✅ Active

**Goal**: Implement a minimal operating system kernel to understand processes, memory, and system calls.

**Core Features**:
- Process scheduling (round-robin)
- Virtual memory (two-level page tables)
- System call interface (fork, exit, wait, etc.)
- Context switching
- Interrupt handling

**Key Invariants**:
- Process isolation: processes cannot access other processes' memory
- Memory protection: user mode cannot access kernel memory
- Scheduling fairness: all ready processes are eventually scheduled

**Verification**:
- Unit tests: 35 files, ~2000 lines of code
- Scheduler tests: verify fairness and state transitions
- Memory tests: verify isolation and protection
- Fault injection: test process crash scenarios

**Quick Start**:
```bash
git clone https://github.com/first-principles-cs/mini-os.git
cd mini-os
make
make qemu
```

**Course Module**: Module C — Single-Machine Systems

---

### simple-fs

**Status**: ✅ Active

**Goal**: Implement a simple Unix-like file system based on xv6 design to understand persistence and crash consistency.

**Core Features**:
- Disk I/O simulation layer (Phase 1 ✅)
- Block allocator with bitmap management (Phase 1 ✅)
- Inode management and caching (Phase 2 ✅)
- Directory operations and path resolution (Phase 2 ✅)
- File creation, deletion, read/write (Phase 2 ✅)
- Buffer cache with LRU eviction (Phase 3 ✅)
- Write-ahead logging (WAL) and crash recovery (Phase 4 ✅)
- Interactive shell (Phase 5 ✅)

**Key Invariants**:
- Crash consistency: file system remains usable after a crash
- Atomicity: operations either fully complete or have no effect
- Reference count correctness: inode reference counts match actual references
- Block allocation consistency: allocated blocks are never double-allocated

**Verification**:
- Unit tests: 31 tests all passing
- Integration tests: file system creation, file operations, directory traversal, crash recovery
- Code stats: ~3500 lines of source and test code

**Quick Start**:
```bash
git clone https://github.com/first-principles-cs/simple-fs.git
cd simple-fs
make
./mkfs fs.img
make test
./shell fs.img  # Interactive shell
```

**Course Module**: Module C — Single-Machine Systems

---

### storage-engine

**Status**: ✅ Active

**Goal**: Implement an LSM-tree based storage engine to understand index structures and persistence.

**Core Features**:
- Skip List in-memory index (Phase 1 ✅)
- MemTable wrapper (Phase 1 ✅)
- In-memory put/get/delete (Phase 1 ✅)
- Write-ahead log WAL (Phase 2 ✅)
- SSTable on-disk storage (Phase 3 ✅)
- Multi-level LSM and compaction (Phase 4 ✅)
- Block cache and benchmarks (Phase 5 ✅)

**Key Invariants**:
- Durability: written data is never lost
- Ordering: range queries return ordered results
- Consistency: compaction does not lose data
- Atomicity: operations either fully complete or have no effect

**Verification**:
- Unit tests: 47 tests covering all phases
- Benchmarks: sequential/random read-write, mixed workloads

**Quick Start**:
```bash
git clone https://github.com/first-principles-cs/storage-engine.git
cd storage-engine
make test
```

**Course Module**: Module D — Data Systems

---

### tx-manager

**Goal**: Implement a transaction manager to understand concurrency control and isolation levels.

**Core Features**:
- MVCC or 2PL
- Deadlock detection
- Isolation levels

**Key Invariants**:
- Atomicity: transactions either fully commit or fully roll back
- Isolation: concurrent transactions do not interfere with each other
- Consistency: transactions maintain database constraints
- Durability: committed transactions are never lost

**Verification**:
- Unit tests: test transaction operations
- Concurrency tests: test multi-threaded scenarios
- Jepsen-style tests: verify isolation levels

**Course Module**: Module D — Data Systems

---

### consensus

**Goal**: Implement a consensus protocol (Raft or Paxos) to understand distributed consistency.

**Core Features**:
- Leader election
- Log replication
- Membership changes

**Key Invariants**:
- Safety: committed log entries are never lost
- Liveness: the system eventually reaches consensus

**Verification**:
- Unit tests: test protocol logic
- Network partition tests: simulate network failures
- Jepsen tests: verify consistency

**Course Module**: Module E — Networking & Distributed Systems

---

### dist-kv

**Goal**: Implement a distributed key-value store to understand sharding, replication, and fault tolerance.

**Core Features**:
- Consistent hashing
- Primary-replica replication
- Failure detection and failover

**Key Invariants**:
- Linearizability or eventual consistency
- Availability: system remains available when f < n/2 nodes fail

**Verification**:
- Unit tests: test sharding and replication logic
- Fault injection: test node crashes, network partitions
- Jepsen tests: verify consistency model
- Benchmarks: measure throughput and latency

**Course Module**: Module E — Networking & Distributed Systems

---

## Status Legend

- **planned**: Planned, not yet started
- **active**: Under active development
- **stable**: Core features complete, in maintenance

## Changelog

- 2025-01: Initial version, all projects in planned status

---

Next: See [ROADMAP.md](./roadmap.en.md) for the development timeline.
