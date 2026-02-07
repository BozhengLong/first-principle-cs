#ifndef STORAGE_PARAM_H
#define STORAGE_PARAM_H

#include <stddef.h>
#include <stdbool.h>
#include "types.h"

// MemTable parameters
#define MEMTABLE_SIZE_LIMIT     (4 * 1024 * 1024)   // 4 MB
#define SKIPLIST_MAX_LEVEL      12
#define SKIPLIST_P              0.25                 // Probability for level promotion

// WAL parameters
#define WAL_BLOCK_SIZE          32768               // 32 KB
#define WAL_HEADER_SIZE         12                  // length(4) + crc32(4) + type(4)

// SSTable parameters
#define SSTABLE_BLOCK_SIZE      4096                // 4 KB
#define SSTABLE_RESTART_INTERVAL 16                 // Keys between restart points
#define BLOOM_BITS_PER_KEY      10                  // Bloom filter bits per key

// Level parameters
#define MAX_LEVELS              7
#define L0_COMPACTION_TRIGGER   4                   // Trigger compaction when L0 has 4 files
#define L0_SLOWDOWN_TRIGGER     8                   // Slow down writes when L0 has 8 files
#define L0_STOP_TRIGGER         12                  // Stop writes when L0 has 12 files
#define LEVEL_SIZE_MULTIPLIER   10                  // Each level is 10x larger than previous
#define L1_MAX_BYTES            (10 * 1024 * 1024)  // 10 MB for L1

// Cache parameters
#define BLOCK_CACHE_SIZE        (8 * 1024 * 1024)   // 8 MB default cache size

// Storage options
typedef struct {
    size_t memtable_size;       // MemTable size limit
    size_t block_cache_size;    // Block cache size
    bool sync_writes;           // Sync WAL on every write
    compare_fn comparator;      // Key comparator
} storage_opts_t;

// Default options
#define STORAGE_OPTS_DEFAULT { \
    .memtable_size = MEMTABLE_SIZE_LIMIT, \
    .block_cache_size = BLOCK_CACHE_SIZE, \
    .sync_writes = false, \
    .comparator = NULL \
}

#endif // STORAGE_PARAM_H
