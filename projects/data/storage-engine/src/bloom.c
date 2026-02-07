#include "bloom.h"
#include "param.h"
#include <stdlib.h>
#include <string.h>

// MurmurHash3 32-bit finalizer
static uint32_t murmur_hash3_32(const void* key, size_t len, uint32_t seed) {
    const uint8_t* data = (const uint8_t*)key;
    const int nblocks = (int)(len / 4);

    uint32_t h1 = seed;
    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;

    // Body
    const uint32_t* blocks = (const uint32_t*)(data + nblocks * 4);
    for (int i = -nblocks; i; i++) {
        uint32_t k1;
        memcpy(&k1, blocks + i, sizeof(k1));

        k1 *= c1;
        k1 = (k1 << 15) | (k1 >> 17);
        k1 *= c2;

        h1 ^= k1;
        h1 = (h1 << 13) | (h1 >> 19);
        h1 = h1 * 5 + 0xe6546b64;
    }

    // Tail
    const uint8_t* tail = (const uint8_t*)(data + nblocks * 4);
    uint32_t k1 = 0;

    switch (len & 3) {
        case 3: k1 ^= (uint32_t)tail[2] << 16; // fallthrough
        case 2: k1 ^= (uint32_t)tail[1] << 8;  // fallthrough
        case 1: k1 ^= tail[0];
                k1 *= c1;
                k1 = (k1 << 15) | (k1 >> 17);
                k1 *= c2;
                h1 ^= k1;
    }

    // Finalization
    h1 ^= (uint32_t)len;
    h1 ^= h1 >> 16;
    h1 *= 0x85ebca6b;
    h1 ^= h1 >> 13;
    h1 *= 0xc2b2ae35;
    h1 ^= h1 >> 16;

    return h1;
}

// Create bloom filter
bloom_filter_t* bloom_create(size_t estimated_keys) {
    if (estimated_keys == 0) {
        estimated_keys = 1;
    }

    bloom_filter_t* bf = malloc(sizeof(bloom_filter_t));
    if (!bf) return NULL;

    // Calculate number of bits (BLOOM_BITS_PER_KEY bits per key)
    bf->num_bits = estimated_keys * BLOOM_BITS_PER_KEY;
    // Round up to nearest byte
    size_t num_bytes = (bf->num_bits + 7) / 8;
    bf->num_bits = num_bytes * 8;

    bf->bits = calloc(num_bytes, 1);
    if (!bf->bits) {
        free(bf);
        return NULL;
    }

    bf->num_keys = 0;
    // Optimal k = (m/n) * ln(2) ≈ 0.693 * bits_per_key
    // For 10 bits/key: k ≈ 6.93, round to 7
    bf->num_hashes = 7;

    return bf;
}

// Destroy bloom filter
void bloom_destroy(bloom_filter_t* bf) {
    if (bf) {
        free(bf->bits);
        free(bf);
    }
}

// Add key to bloom filter using double hashing
void bloom_add(bloom_filter_t* bf, const char* key, size_t key_len) {
    if (!bf || !key) return;

    // Double hashing: h(i) = h1 + i * h2
    uint32_t h1 = murmur_hash3_32(key, key_len, 0);
    uint32_t h2 = murmur_hash3_32(key, key_len, h1);

    for (uint8_t i = 0; i < bf->num_hashes; i++) {
        uint32_t bit_pos = (h1 + i * h2) % bf->num_bits;
        bf->bits[bit_pos / 8] |= (1 << (bit_pos % 8));
    }

    bf->num_keys++;
}

// Check if key may be in the set
bool bloom_may_contain(bloom_filter_t* bf, const char* key, size_t key_len) {
    if (!bf || !key) return false;

    uint32_t h1 = murmur_hash3_32(key, key_len, 0);
    uint32_t h2 = murmur_hash3_32(key, key_len, h1);

    for (uint8_t i = 0; i < bf->num_hashes; i++) {
        uint32_t bit_pos = (h1 + i * h2) % bf->num_bits;
        if (!(bf->bits[bit_pos / 8] & (1 << (bit_pos % 8)))) {
            return false;
        }
    }

    return true;
}

// Get serialized size
// Format: num_bits(8) + num_hashes(1) + bits(variable)
size_t bloom_serialized_size(bloom_filter_t* bf) {
    if (!bf) return 0;
    size_t num_bytes = (bf->num_bits + 7) / 8;
    return 8 + 1 + num_bytes;  // num_bits + num_hashes + bit array
}

// Serialize bloom filter to buffer
status_t bloom_serialize(bloom_filter_t* bf, uint8_t* buf, size_t len) {
    if (!bf || !buf) return STATUS_INVALID_ARG;

    size_t required = bloom_serialized_size(bf);
    if (len < required) return STATUS_INVALID_ARG;

    size_t num_bytes = (bf->num_bits + 7) / 8;

    // Write num_bits (8 bytes, little-endian)
    uint64_t num_bits = bf->num_bits;
    memcpy(buf, &num_bits, 8);
    buf += 8;

    // Write num_hashes (1 byte)
    *buf++ = bf->num_hashes;

    // Write bit array
    memcpy(buf, bf->bits, num_bytes);

    return STATUS_OK;
}

// Deserialize bloom filter from buffer
bloom_filter_t* bloom_deserialize(const uint8_t* buf, size_t len) {
    if (!buf || len < 9) return NULL;  // Minimum: 8 + 1 bytes

    // Read num_bits
    uint64_t num_bits;
    memcpy(&num_bits, buf, 8);
    buf += 8;

    // Read num_hashes
    uint8_t num_hashes = *buf++;

    // Calculate expected size
    size_t num_bytes = (num_bits + 7) / 8;
    if (len < 9 + num_bytes) return NULL;

    // Allocate bloom filter
    bloom_filter_t* bf = malloc(sizeof(bloom_filter_t));
    if (!bf) return NULL;

    bf->bits = malloc(num_bytes);
    if (!bf->bits) {
        free(bf);
        return NULL;
    }

    bf->num_bits = num_bits;
    bf->num_hashes = num_hashes;
    bf->num_keys = 0;  // Unknown after deserialization

    // Copy bit array
    memcpy(bf->bits, buf, num_bytes);

    return bf;
}
