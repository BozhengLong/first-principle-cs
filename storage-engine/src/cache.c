#include "cache.h"
#include <stdlib.h>
#include <string.h>

#define INITIAL_BUCKETS 256

// Hash function (FNV-1a)
static uint32_t hash_key(const char* key, size_t len) {
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < len; i++) {
        hash ^= (uint8_t)key[i];
        hash *= 16777619u;
    }
    return hash;
}

// Create a new cache entry
static cache_entry_t* entry_create(const char* key, size_t key_len,
                                   const uint8_t* data, size_t data_len,
                                   uint32_t hash) {
    cache_entry_t* entry = malloc(sizeof(cache_entry_t));
    if (!entry) return NULL;

    entry->key = malloc(key_len);
    if (!entry->key) {
        free(entry);
        return NULL;
    }
    memcpy(entry->key, key, key_len);
    entry->key_len = key_len;

    entry->data = malloc(data_len);
    if (!entry->data) {
        free(entry->key);
        free(entry);
        return NULL;
    }
    memcpy(entry->data, data, data_len);
    entry->data_len = data_len;

    entry->prev = NULL;
    entry->next = NULL;
    entry->hash_next = NULL;
    entry->hash = hash;

    return entry;
}

// Destroy a cache entry
static void entry_destroy(cache_entry_t* entry) {
    if (entry) {
        free(entry->key);
        free(entry->data);
        free(entry);
    }
}

// Create cache
block_cache_t* cache_create(size_t capacity) {
    block_cache_t* cache = malloc(sizeof(block_cache_t));
    if (!cache) return NULL;

    cache->bucket_count = INITIAL_BUCKETS;
    cache->buckets = calloc(cache->bucket_count, sizeof(cache_entry_t*));
    if (!cache->buckets) {
        free(cache);
        return NULL;
    }

    cache->head = NULL;
    cache->tail = NULL;
    cache->capacity = capacity;
    cache->usage = 0;
    cache->hits = 0;
    cache->misses = 0;

    return cache;
}

// Destroy cache
void cache_destroy(block_cache_t* cache) {
    if (!cache) return;

    cache_entry_t* entry = cache->head;
    while (entry) {
        cache_entry_t* next = entry->next;
        entry_destroy(entry);
        entry = next;
    }

    free(cache->buckets);
    free(cache);
}

// Remove entry from LRU list
static void lru_remove(block_cache_t* cache, cache_entry_t* entry) {
    if (entry->prev) {
        entry->prev->next = entry->next;
    } else {
        cache->head = entry->next;
    }
    if (entry->next) {
        entry->next->prev = entry->prev;
    } else {
        cache->tail = entry->prev;
    }
    entry->prev = NULL;
    entry->next = NULL;
}

// Insert entry at head of LRU list (most recently used)
static void lru_insert_head(block_cache_t* cache, cache_entry_t* entry) {
    entry->prev = NULL;
    entry->next = cache->head;
    if (cache->head) {
        cache->head->prev = entry;
    }
    cache->head = entry;
    if (!cache->tail) {
        cache->tail = entry;
    }
}

// Find entry in hash table
static cache_entry_t* hash_find(block_cache_t* cache, const char* key,
                                 size_t key_len, uint32_t hash) {
    size_t bucket = hash % cache->bucket_count;
    cache_entry_t* entry = cache->buckets[bucket];
    while (entry) {
        if (entry->hash == hash && entry->key_len == key_len &&
            memcmp(entry->key, key, key_len) == 0) {
            return entry;
        }
        entry = entry->hash_next;
    }
    return NULL;
}

// Insert entry into hash table
static void hash_insert(block_cache_t* cache, cache_entry_t* entry) {
    size_t bucket = entry->hash % cache->bucket_count;
    entry->hash_next = cache->buckets[bucket];
    cache->buckets[bucket] = entry;
}

// Remove entry from hash table
static void hash_remove(block_cache_t* cache, cache_entry_t* entry) {
    size_t bucket = entry->hash % cache->bucket_count;
    cache_entry_t** pp = &cache->buckets[bucket];
    while (*pp) {
        if (*pp == entry) {
            *pp = entry->hash_next;
            entry->hash_next = NULL;
            return;
        }
        pp = &(*pp)->hash_next;
    }
}

// Evict entries until we have enough space
static void evict_if_needed(block_cache_t* cache, size_t needed) {
    while (cache->usage + needed > cache->capacity && cache->tail) {
        cache_entry_t* victim = cache->tail;
        lru_remove(cache, victim);
        hash_remove(cache, victim);
        cache->usage -= (victim->key_len + victim->data_len);
        entry_destroy(victim);
    }
}

// Get data from cache (returns copy, caller must free)
uint8_t* cache_get(block_cache_t* cache, const char* key, size_t key_len,
                   size_t* data_len) {
    if (!cache || !key) return NULL;

    uint32_t hash = hash_key(key, key_len);
    cache_entry_t* entry = hash_find(cache, key, key_len, hash);

    if (entry) {
        cache->hits++;
        // Move to head (most recently used)
        lru_remove(cache, entry);
        lru_insert_head(cache, entry);
        // Return copy of data
        uint8_t* copy = malloc(entry->data_len);
        if (copy) {
            memcpy(copy, entry->data, entry->data_len);
            if (data_len) *data_len = entry->data_len;
        }
        return copy;
    }

    cache->misses++;
    return NULL;
}

// Put data into cache
void cache_put(block_cache_t* cache, const char* key, size_t key_len,
               const uint8_t* data, size_t data_len) {
    if (!cache || !key || !data) return;

    uint32_t hash = hash_key(key, key_len);

    // Check if key already exists
    cache_entry_t* existing = hash_find(cache, key, key_len, hash);
    if (existing) {
        // Update existing entry
        lru_remove(cache, existing);
        hash_remove(cache, existing);
        cache->usage -= (existing->key_len + existing->data_len);
        entry_destroy(existing);
    }

    size_t entry_size = key_len + data_len;

    // Don't cache if larger than capacity
    if (entry_size > cache->capacity) return;

    // Evict if needed
    evict_if_needed(cache, entry_size);

    // Create new entry
    cache_entry_t* entry = entry_create(key, key_len, data, data_len, hash);
    if (!entry) return;

    // Insert into hash table and LRU list
    hash_insert(cache, entry);
    lru_insert_head(cache, entry);
    cache->usage += entry_size;
}

// Invalidate a cache entry
void cache_invalidate(block_cache_t* cache, const char* key, size_t key_len) {
    if (!cache || !key) return;

    uint32_t hash = hash_key(key, key_len);
    cache_entry_t* entry = hash_find(cache, key, key_len, hash);

    if (entry) {
        lru_remove(cache, entry);
        hash_remove(cache, entry);
        cache->usage -= (entry->key_len + entry->data_len);
        entry_destroy(entry);
    }
}

// Clear all entries
void cache_clear(block_cache_t* cache) {
    if (!cache) return;

    cache_entry_t* entry = cache->head;
    while (entry) {
        cache_entry_t* next = entry->next;
        entry_destroy(entry);
        entry = next;
    }

    // Reset buckets
    memset(cache->buckets, 0, cache->bucket_count * sizeof(cache_entry_t*));

    cache->head = NULL;
    cache->tail = NULL;
    cache->usage = 0;
    // Keep hit/miss stats
}

// Get hit rate
double cache_hit_rate(block_cache_t* cache) {
    if (!cache) return 0.0;
    size_t total = cache->hits + cache->misses;
    if (total == 0) return 0.0;
    return (double)cache->hits / (double)total;
}

// Get current usage
size_t cache_usage(block_cache_t* cache) {
    return cache ? cache->usage : 0;
}

// Get entry count
size_t cache_count(block_cache_t* cache) {
    if (!cache) return 0;
    size_t count = 0;
    cache_entry_t* entry = cache->head;
    while (entry) {
        count++;
        entry = entry->next;
    }
    return count;
}
