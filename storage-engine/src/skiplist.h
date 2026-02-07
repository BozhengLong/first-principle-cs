#ifndef STORAGE_SKIPLIST_H
#define STORAGE_SKIPLIST_H

#include "types.h"
#include "param.h"

// Skip list node
typedef struct skiplist_node {
    char* key;
    size_t key_len;
    char* value;
    size_t value_len;
    bool deleted;
    int level;
    struct skiplist_node** forward;  // Array of forward pointers
} skiplist_node_t;

// Skip list structure
struct skiplist {
    skiplist_node_t* header;
    int level;                  // Current max level
    size_t count;               // Number of entries
    size_t memory_usage;        // Approximate memory usage
    compare_fn compare;
};

// Skip list iterator
struct skiplist_iter {
    skiplist_t* list;
    skiplist_node_t* current;
};

// Skip list operations
skiplist_t* skiplist_create(compare_fn cmp);
void skiplist_destroy(skiplist_t* list);

// Insert or update a key-value pair
status_t skiplist_put(skiplist_t* list, const char* key, size_t key_len,
                      const char* value, size_t value_len);

// Get value for a key (returns STATUS_NOT_FOUND if not found)
status_t skiplist_get(skiplist_t* list, const char* key, size_t key_len,
                      char** value, size_t* value_len);

// Mark a key as deleted (tombstone)
status_t skiplist_delete(skiplist_t* list, const char* key, size_t key_len);

// Check if key exists (including tombstones)
bool skiplist_contains(skiplist_t* list, const char* key, size_t key_len);

// Get count and memory usage
size_t skiplist_count(skiplist_t* list);
size_t skiplist_memory_usage(skiplist_t* list);

// Iterator operations
skiplist_iter_t* skiplist_iter_create(skiplist_t* list);
void skiplist_iter_destroy(skiplist_iter_t* iter);
void skiplist_iter_seek_to_first(skiplist_iter_t* iter);
void skiplist_iter_seek(skiplist_iter_t* iter, const char* key, size_t key_len);
bool skiplist_iter_valid(skiplist_iter_t* iter);
void skiplist_iter_next(skiplist_iter_t* iter);
const char* skiplist_iter_key(skiplist_iter_t* iter, size_t* key_len);
const char* skiplist_iter_value(skiplist_iter_t* iter, size_t* value_len);
bool skiplist_iter_is_deleted(skiplist_iter_t* iter);

#endif // STORAGE_SKIPLIST_H
