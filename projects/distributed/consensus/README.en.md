[中文](README.md) | [English](README.en.md)

---

# Consensus (Raft)

A Raft consensus protocol implementation in C for learning distributed systems concepts.

## Overview

Raft is a consensus algorithm designed to be easy to understand. It provides the same fault-tolerance and performance as Paxos but is decomposed into relatively independent subproblems.

## Current Status

**Phase 1**: ✅ Complete — Basic structures and single-node operation
**Phase 2**: ✅ Complete — Leader election
**Phase 3**: ✅ Complete — Log replication
**Phase 4**: ✅ Complete — Persistence and recovery
**Phase 5**: ✅ Complete — Membership changes and optimization
**Phase 6**: ✅ Complete — Advanced Raft features

## Building

```bash
make test_phase1   # Build and run Phase 1 tests
make test_phase2   # Build and run Phase 2 tests
make test_phase3   # Build and run Phase 3 tests
make test_phase4   # Build and run Phase 4 tests
make test_phase5   # Build and run Phase 5 tests
make test_phase6   # Build and run Phase 6 tests
make clean         # Clean build artifacts
```

## Project Structure

```
consensus/
├── src/
│   ├── types.h          # Status codes and type definitions
│   ├── param.h          # Tunable parameters
│   ├── log.h/c          # Raft log management
│   ├── raft.h/c         # Core Raft node
│   ├── rpc.h            # RPC message structures
│   ├── election.h/c     # Leader election logic
│   ├── timer.h/c        # Timer management
│   ├── replication.h/c  # Log replication logic
│   ├── commit.h/c       # Commit index management
│   ├── crc32.h/c        # CRC32 checksum
│   ├── storage.h/c      # Persistent storage
│   ├── snapshot.h/c     # Snapshot support
│   ├── recovery.h/c     # Recovery from storage
│   ├── membership.h/c   # Cluster membership changes
│   ├── batch.h/c        # Batch operations
│   ├── read.h/c         # ReadIndex for linearizable reads
│   └── transfer.h/c     # Leadership transfer
├── tests/unit/
│   ├── test_phase1.c    # Phase 1 tests (10 tests)
│   ├── test_phase2.c    # Phase 2 tests (10 tests)
│   ├── test_phase3.c    # Phase 3 tests (10 tests)
│   ├── test_phase4.c    # Phase 4 tests (10 tests)
│   ├── test_phase5.c    # Phase 5 tests (10 tests)
│   └── test_phase6.c    # Phase 6 tests (10 tests)
└── docs/                # Documentation
```

## Test Results

```
Phase 1: 10/10 tests passed
Phase 2: 10/10 tests passed
Phase 3: 10/10 tests passed
Phase 4: 10/10 tests passed
Phase 5: 10/10 tests passed
Phase 6: 10/10 tests passed
Integration (Partition): 6/6 tests passed
Integration (Chaos): 5/5 tests passed
Total: 71/71 tests passed
```

## Key Invariants

- **Election Safety**: At most one leader per term
- **Leader Append-Only**: Leader never overwrites or deletes log entries
- **Log Matching**: If two logs contain an entry with same index and term, logs are identical up to that index
- **Leader Completeness**: If an entry is committed, it will be present in all future leaders' logs

## Quick Start

```bash
git clone https://github.com/first-principles-cs/consensus.git
cd consensus
make test_phase6
./test_phase6
```

## Phases

- **Phase 1**: Basic structures and single-node operation ✅
- **Phase 2**: Leader election ✅
- **Phase 3**: Log replication ✅
- **Phase 4**: Persistence and recovery ✅
- **Phase 5**: Membership changes and optimization ✅
- **Phase 6**: Advanced Raft features ✅
- **Phase 7**: Documentation ✅
- **Phase 8**: Integration tests and benchmarks ✅

## Related Course

Part of the [first-principles-cs](https://github.com/first-principles-cs/guide) curriculum — Module E: Networking & Distributed Systems.
