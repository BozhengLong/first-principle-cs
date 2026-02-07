#ifndef BLOOM_H
#define BLOOM_H

#include "types.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Bloom filter structure
struct bloom_filter {
    uint8_t* bits;          // Bit array
    size_t num_bits;        // Number of bits
    size_t num_keys;        // Number of keys added
    uint8_t num_hashes;     // Number of hash functions (k)
};

// Create a bloom filter for estimated number of keys
// Uses BLOOM_BITS_PER_KEY from param.h
bloom_filter_t* bloom_create(size_t estimated_keys);

// Destroy bloom filter
void bloom_destroy(bloom_filter_t* bf);

// Add a key to the bloom filter
void bloom_add(bloom_filter_t* bf, const char* key, size_t key_len);

// Check if key may be in the set (false = definitely not, true = maybe)
bool bloom_may_contain(bloom_filter_t* bf, const char* key, size_t key_len);

// Serialization
size_t bloom_serialized_size(bloom_filter_t* bf);
status_t bloom_serialize(bloom_filter_t* bf, uint8_t* buf, size_t len);
bloom_filter_t* bloom_deserialize(const uint8_t* buf, size_t len);

#endif // BLOOM_H
