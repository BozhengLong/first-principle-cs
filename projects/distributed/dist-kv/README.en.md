[中文](README.md) | [English](README.en.md)

---

# dist-kv: Distributed Key-Value Store

A distributed key-value store implementation for learning distributed systems concepts including sharding, replication, and fault tolerance.

## Dependencies

- **consensus**: Raft consensus implementation
- **storage-engine**: LSM-tree storage engine

## Architecture

```
┌─────────────────────────────────────────────────────┐
│                    Client Layer                      │
│              (Client API, Request Routing)           │
└─────────────────────────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────┐
│                  Coordinator Layer                   │
│          (Consistent Hashing, Partition Routing)     │
└─────────────────────────────────────────────────────┘
                          │
          ┌───────────────┼───────────────┐
          ▼               ▼               ▼
    ┌──────────┐   ┌──────────┐   ┌──────────┐
    │Partition │   │Partition │   │Partition │
    │    0     │   │    1     │   │    2     │
    └──────────┘   └──────────┘   └──────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────┐
│                 Replication Layer                    │
│           (Raft Consensus per Partition)             │
└─────────────────────────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────┐
│                   Storage Layer                      │
│              (LSM-tree per Partition)                │
└─────────────────────────────────────────────────────┘
```

## Building

```bash
make
```

## Testing

```bash
# Run all tests
make test

# Run specific phase tests
make test_phase1
make test_phase2
# ... etc
```

## Progress

- [x] Phase 1: Basic Types, Consistent Hashing, Node Lifecycle (10 tests)
- [x] Phase 2: Partition Management (10 tests)
- [x] Phase 3: Replication Layer (10 tests)
- [x] Phase 4: Fault Tolerance (10 tests)
- [x] Phase 5: Client API (10 tests)
- [x] Phase 6: Integration Tests & Documentation (10 tests)

**Total: 60/60 tests passing**

## Key Invariants

1. **Consistency Model**: Linearizable (via Raft) or eventual (local reads)
2. **Availability**: System remains available when f < n/2 nodes fail
3. **Partition Tolerance**: Minority partition becomes read-only

## Related Course

Part of the [first-principles-cs](https://github.com/first-principles-cs/guide) curriculum — Module E: Networking & Distributed Systems.
