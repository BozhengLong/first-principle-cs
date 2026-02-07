/**
 * version.h - Versioned key encoding for MVCC
 *
 * Keys are encoded with their version (timestamp) to support multiple
 * versions of the same key. Format: key + \0 + big-endian timestamp
 *
 * Using big-endian ensures lexicographic ordering matches numeric ordering
 * (descending, so newer versions come first).
 */

#ifndef TX_VERSION_H
#define TX_VERSION_H

#include "types.h"

/**
 * Encode a key with version timestamp
 *
 * @param key Original key
 * @param key_len Length of key
 * @param version Version timestamp
 * @param out_key Output buffer (caller must free)
 * @param out_len Output length
 * @return TX_OK on success
 */
tx_status_t version_encode_key(const char* key, size_t key_len,
                               uint64_t version,
                               char** out_key, size_t* out_len);

/**
 * Decode a versioned key
 *
 * @param versioned_key Encoded key
 * @param versioned_len Length of encoded key
 * @param out_key Output original key (points into versioned_key, not copied)
 * @param out_key_len Output key length
 * @param out_version Output version timestamp
 * @return TX_OK on success
 */
tx_status_t version_decode_key(const char* versioned_key, size_t versioned_len,
                               const char** out_key, size_t* out_key_len,
                               uint64_t* out_version);

/**
 * Compare two keys (without version)
 */
int version_compare_keys(const char* key1, size_t len1,
                         const char* key2, size_t len2);

#endif /* TX_VERSION_H */
