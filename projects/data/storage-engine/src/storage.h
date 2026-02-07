#ifndef STORAGE_H
#define STORAGE_H

#include "types.h"
#include "param.h"
#include "memtable.h"
#include "wal.h"
#include "sstable.h"
#include "level.h"

// Storage engine structure
struct storage {
    char* path;
    storage_opts_t opts;
    memtable_t* memtable;
    wal_t* wal;  // Write-ahead log for durability
    // Phase 4: Level-based SSTable management
    level_manager_t* levels;
};

// Storage iterator
struct storage_iter {
    storage_t* db;
    memtable_iter_t* mt_iter;
    // Future phases will add SSTable iterators
};

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
void storage_iter_destroy(storage_iter_t* iter);
void storage_iter_seek_to_first(storage_iter_t* iter);
void storage_iter_seek(storage_iter_t* iter, const char* key, size_t key_len);
bool storage_iter_valid(storage_iter_t* iter);
void storage_iter_next(storage_iter_t* iter);
const char* storage_iter_key(storage_iter_t* iter, size_t* key_len);
const char* storage_iter_value(storage_iter_t* iter, size_t* val_len);

// Maintenance (stubs for Phase 1)
status_t storage_compact(storage_t* db);
status_t storage_flush(storage_t* db);

// Statistics
size_t storage_count(storage_t* db);
size_t storage_memory_usage(storage_t* db);

#endif // STORAGE_H
