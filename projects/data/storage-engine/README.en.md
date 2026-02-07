[中文](README.md) | [English](README.en.md)

---

# storage-engine

An LSM-tree based key-value storage engine for understanding index structures and persistence.

## Project Status

**Phase 1: Basic Infrastructure** ✅ Complete

- [x] Skip List implementation
- [x] MemTable wrapper
- [x] In-memory put/get/delete
- [x] Unit tests (12)

**Phase 2: Write-Ahead Log (WAL)** ✅ Complete

- [x] WAL record format
- [x] CRC32 checksums
- [x] Crash recovery
- [x] Unit tests (8)

**Phase 3: SSTable** ✅ Complete

- [x] Bloom Filter implementation
- [x] SSTable writer (prefix compression, restart points)
- [x] SSTable reader (binary search, CRC32 verification)
- [x] Storage integration (flush, cross-level queries)
- [x] Unit tests (10)

**Phase 4: Multi-Level LSM** ✅ Complete

- [x] Level Manager implementation
- [x] L0 allows overlapping files
- [x] L1+ sorted by min_key
- [x] Compaction trigger detection
- [x] Manifest persistence and recovery
- [x] Unit tests (9)

**Phase 5: Block Cache & Benchmarks** ✅ Complete

- [x] LRU Block Cache implementation
- [x] Hash table accelerated lookups
- [x] Cache hit rate statistics
- [x] Benchmark tool (sequential/random read-write, mixed workloads)
- [x] Unit tests (8)

## Quick Start

```bash
# Build
make

# Run tests
make test

# Run benchmarks
./storage-bench
```

## Architecture

```
+------------------+
|  storage-bench   |  (Entry / CLI)
+--------+---------+
         |
+--------v---------+
|   Storage API    |  (Public interface)
+--------+---------+
         |
+--------+--------+--------+
|        |        |        |
v        v        v        v
MemTable SSTable  WAL    Cache
(SkipList)(Disk)  (Log)  (LRU)
```

## API

```c
// Lifecycle
storage_t* storage_open(const char* path, storage_opts_t* opts);
void storage_close(storage_t* db);

// Basic operations
status_t storage_put(storage_t* db, const char* key, size_t key_len,
                     const char* val, size_t val_len);
status_t storage_get(storage_t* db, const char* key, size_t key_len,
                     char** val, size_t* val_len);
status_t storage_delete(storage_t* db, const char* key, size_t key_len);

// Range operations
storage_iter_t* storage_iter_create(storage_t* db);
void storage_iter_seek(storage_iter_t* iter, const char* key, size_t key_len);
bool storage_iter_valid(storage_iter_t* iter);
void storage_iter_next(storage_iter_t* iter);
void storage_iter_destroy(storage_iter_t* iter);

// Maintenance
status_t storage_compact(storage_t* db);
status_t storage_flush(storage_t* db);
```

## Key Invariants

1. **Durability**: Written data is never lost
2. **Ordering**: Range queries return ordered results
3. **Consistency**: Compaction does not lose data
4. **Atomicity**: Operations either fully complete or have no effect

## Directory Structure

```
storage-engine/
├── Makefile
├── README.md
├── docs/design.md
├── src/
│   ├── types.h, param.h      # Types and parameters
│   ├── storage.h/c           # Public API
│   ├── skiplist.h/c          # Skip List
│   ├── memtable.h/c          # MemTable
│   ├── wal.h/c, crc32.h/c    # WAL
│   ├── sstable.h/c           # SSTable
│   ├── bloom.h/c             # Bloom Filter
│   ├── level.h/c             # Level management
│   ├── compact.h/c           # Compaction
│   ├── cache.h/c             # Block Cache
│   └── bench.c               # Benchmarks
└── tests/unit/
    └── test_phase[1-5].c
```

## Related Course

Module D — Data Systems
