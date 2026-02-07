#ifndef STORAGE_CACHE_H
#define STORAGE_CACHE_H

#include "types.h"
#include <stdint.h>
#include <stddef.h>

// Cache entry (internal)
typedef struct cache_entry {
    char* key;
    size_t key_len;
    uint8_t* data;
    size_t data_len;
    struct cache_entry* prev;  // LRU list
    struct cache_entry* next;
    struct cache_entry* hash_next;  // Hash chain
    uint32_t hash;
} cache_entry_t;

// LRU Block Cache
struct block_cache {
    cache_entry_t** buckets;
    size_t bucket_count;
    cache_entry_t* head;  // Most recently used
    cache_entry_t* tail;  // Least recently used
    size_t capacity;
    size_t usage;
    size_t hits;
    size_t misses;
};

// Create/destroy
block_cache_t* cache_create(size_t capacity);
void cache_destroy(block_cache_t* cache);

// Operations
uint8_t* cache_get(block_cache_t* cache, const char* key, size_t key_len,
                   size_t* data_len);
void cache_put(block_cache_t* cache, const char* key, size_t key_len,
               const uint8_t* data, size_t data_len);
void cache_invalidate(block_cache_t* cache, const char* key, size_t key_len);
void cache_clear(block_cache_t* cache);

// Statistics
double cache_hit_rate(block_cache_t* cache);
size_t cache_usage(block_cache_t* cache);
size_t cache_count(block_cache_t* cache);

#endif // STORAGE_CACHE_H
