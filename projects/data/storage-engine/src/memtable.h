#ifndef STORAGE_MEMTABLE_H
#define STORAGE_MEMTABLE_H

#include "types.h"
#include "param.h"
#include "skiplist.h"

// MemTable structure
struct memtable {
    skiplist_t* list;
    size_t size_limit;
    uint64_t seq_num;       // Current sequence number
};

// MemTable operations
memtable_t* memtable_create(size_t size_limit, compare_fn cmp);
void memtable_destroy(memtable_t* mt);

// Basic operations
status_t memtable_put(memtable_t* mt, const char* key, size_t key_len,
                      const char* value, size_t value_len);
status_t memtable_get(memtable_t* mt, const char* key, size_t key_len,
                      char** value, size_t* value_len);
status_t memtable_delete(memtable_t* mt, const char* key, size_t key_len);

// Check if memtable should be flushed
bool memtable_should_flush(memtable_t* mt);

// Get statistics
size_t memtable_count(memtable_t* mt);
size_t memtable_memory_usage(memtable_t* mt);

// Iterator (wraps skiplist iterator)
typedef skiplist_iter_t memtable_iter_t;

memtable_iter_t* memtable_iter_create(memtable_t* mt);
void memtable_iter_destroy(memtable_iter_t* iter);
void memtable_iter_seek_to_first(memtable_iter_t* iter);
void memtable_iter_seek(memtable_iter_t* iter, const char* key, size_t key_len);
bool memtable_iter_valid(memtable_iter_t* iter);
void memtable_iter_next(memtable_iter_t* iter);
const char* memtable_iter_key(memtable_iter_t* iter, size_t* key_len);
const char* memtable_iter_value(memtable_iter_t* iter, size_t* value_len);
bool memtable_iter_is_deleted(memtable_iter_t* iter);

#endif // STORAGE_MEMTABLE_H
