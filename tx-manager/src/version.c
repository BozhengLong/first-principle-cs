/**
 * version.c - Versioned key encoding implementation
 */

#include "version.h"
#include "param.h"
#include <stdlib.h>
#include <string.h>

tx_status_t version_encode_key(const char* key, size_t key_len,
                               uint64_t version,
                               char** out_key, size_t* out_len) {
    if (!key || !out_key || !out_len) {
        return TX_INVALID_ARG;
    }

    /* Format: key + \0 + inverted big-endian timestamp (8 bytes) */
    /* Invert timestamp so newer versions sort first */
    size_t total_len = key_len + 1 + 8;
    char* encoded = malloc(total_len);
    if (!encoded) {
        return TX_NO_MEMORY;
    }

    /* Copy key */
    memcpy(encoded, key, key_len);
    encoded[key_len] = TX_VERSION_SEP;

    /* Write inverted timestamp in big-endian (newer = smaller) */
    uint64_t inverted = ~version;
    for (int i = 0; i < 8; i++) {
        encoded[key_len + 1 + i] = (inverted >> (56 - i * 8)) & 0xFF;
    }

    *out_key = encoded;
    *out_len = total_len;
    return TX_OK;
}

tx_status_t version_decode_key(const char* versioned_key, size_t versioned_len,
                               const char** out_key, size_t* out_key_len,
                               uint64_t* out_version) {
    if (!versioned_key || !out_key || !out_key_len || !out_version) {
        return TX_INVALID_ARG;
    }

    /* Minimum: 1 byte key + 1 byte sep + 8 bytes timestamp */
    if (versioned_len < 10) {
        return TX_CORRUPTION;
    }

    /* Key length is total - separator - timestamp */
    size_t key_len = versioned_len - 1 - 8;

    /* Verify separator */
    if (versioned_key[key_len] != TX_VERSION_SEP) {
        return TX_CORRUPTION;
    }

    /* Read inverted big-endian timestamp */
    uint64_t inverted = 0;
    for (int i = 0; i < 8; i++) {
        inverted = (inverted << 8) | (unsigned char)versioned_key[key_len + 1 + i];
    }

    *out_key = versioned_key;
    *out_key_len = key_len;
    *out_version = ~inverted;
    return TX_OK;
}

int version_compare_keys(const char* key1, size_t len1,
                         const char* key2, size_t len2) {
    size_t min_len = len1 < len2 ? len1 : len2;
    int cmp = memcmp(key1, key2, min_len);
    if (cmp != 0) return cmp;
    if (len1 < len2) return -1;
    if (len1 > len2) return 1;
    return 0;
}