#ifndef STORAGE_TYPES_H
#define STORAGE_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Status codes
typedef enum {
    STATUS_OK = 0,
    STATUS_NOT_FOUND = 1,
    STATUS_CORRUPTION = 2,
    STATUS_IO_ERROR = 3,
    STATUS_INVALID_ARG = 4,
    STATUS_NO_MEMORY = 5,
} status_t;

// Key-value entry
typedef struct {
    char* key;
    size_t key_len;
    char* value;
    size_t value_len;
    bool deleted;       // Tombstone marker
    uint64_t seq_num;   // Sequence number for MVCC
} kv_entry_t;

// Forward declarations
typedef struct storage storage_t;
typedef struct storage_iter storage_iter_t;
typedef struct memtable memtable_t;
typedef struct skiplist skiplist_t;
typedef struct skiplist_iter skiplist_iter_t;
typedef struct wal wal_t;
typedef struct sstable sstable_t;
typedef struct sstable_writer sstable_writer_t;
typedef struct sstable_reader sstable_reader_t;
typedef struct bloom_filter bloom_filter_t;
typedef struct level_manager level_manager_t;
typedef struct block_cache block_cache_t;

// Comparison function type
typedef int (*compare_fn)(const char* a, size_t a_len,
                          const char* b, size_t b_len);

// Default comparison (lexicographic)
int default_compare(const char* a, size_t a_len,
                    const char* b, size_t b_len);

#endif // STORAGE_TYPES_H
