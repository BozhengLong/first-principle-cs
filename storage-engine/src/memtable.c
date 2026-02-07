#include "memtable.h"
#include <stdlib.h>

// Create a new memtable
memtable_t* memtable_create(size_t size_limit, compare_fn cmp) {
    memtable_t* mt = malloc(sizeof(memtable_t));
    if (!mt) return NULL;

    mt->list = skiplist_create(cmp);
    if (!mt->list) {
        free(mt);
        return NULL;
    }

    mt->size_limit = size_limit > 0 ? size_limit : MEMTABLE_SIZE_LIMIT;
    mt->seq_num = 0;

    return mt;
}

// Destroy memtable
void memtable_destroy(memtable_t* mt) {
    if (mt) {
        skiplist_destroy(mt->list);
        free(mt);
    }
}

// Put a key-value pair
status_t memtable_put(memtable_t* mt, const char* key, size_t key_len,
                      const char* value, size_t value_len) {
    if (!mt) return STATUS_INVALID_ARG;
    return skiplist_put(mt->list, key, key_len, value, value_len);
}

// Get value for a key
status_t memtable_get(memtable_t* mt, const char* key, size_t key_len,
                      char** value, size_t* value_len) {
    if (!mt) return STATUS_INVALID_ARG;
    return skiplist_get(mt->list, key, key_len, value, value_len);
}

// Delete a key (insert tombstone)
status_t memtable_delete(memtable_t* mt, const char* key, size_t key_len) {
    if (!mt) return STATUS_INVALID_ARG;
    return skiplist_delete(mt->list, key, key_len);
}

// Check if memtable should be flushed
bool memtable_should_flush(memtable_t* mt) {
    if (!mt) return false;
    return skiplist_memory_usage(mt->list) >= mt->size_limit;
}

// Get count
size_t memtable_count(memtable_t* mt) {
    return mt ? skiplist_count(mt->list) : 0;
}

// Get memory usage
size_t memtable_memory_usage(memtable_t* mt) {
    return mt ? skiplist_memory_usage(mt->list) : 0;
}

// Iterator operations (thin wrappers around skiplist iterator)
memtable_iter_t* memtable_iter_create(memtable_t* mt) {
    return mt ? skiplist_iter_create(mt->list) : NULL;
}

void memtable_iter_destroy(memtable_iter_t* iter) {
    skiplist_iter_destroy(iter);
}

void memtable_iter_seek_to_first(memtable_iter_t* iter) {
    skiplist_iter_seek_to_first(iter);
}

void memtable_iter_seek(memtable_iter_t* iter, const char* key, size_t key_len) {
    skiplist_iter_seek(iter, key, key_len);
}

bool memtable_iter_valid(memtable_iter_t* iter) {
    return skiplist_iter_valid(iter);
}

void memtable_iter_next(memtable_iter_t* iter) {
    skiplist_iter_next(iter);
}

const char* memtable_iter_key(memtable_iter_t* iter, size_t* key_len) {
    return skiplist_iter_key(iter, key_len);
}

const char* memtable_iter_value(memtable_iter_t* iter, size_t* value_len) {
    return skiplist_iter_value(iter, value_len);
}

bool memtable_iter_is_deleted(memtable_iter_t* iter) {
    return skiplist_iter_is_deleted(iter);
}
