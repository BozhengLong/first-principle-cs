/**
 * conflict.c - Write-Write Conflict Detection Implementation
 */

#include "conflict.h"
#include "version.h"
#include "storage.h"
#include <stdlib.h>
#include <string.h>

tx_status_t get_latest_commit_ts(tx_manager_t* tm,
                                 const char* key, size_t key_len,
                                 uint64_t* commit_ts) {
    if (!tm || !key || !commit_ts) {
        return TX_INVALID_ARG;
    }

    *commit_ts = 0;

    /* Create iterator and seek to the key prefix */
    storage_iter_t* iter = storage_iter_create(tm->storage);
    if (!iter) return TX_IO_ERROR;

    /* Encode key with max timestamp to find latest version */
    char* prefix;
    size_t prefix_len;
    tx_status_t st = version_encode_key(key, key_len, UINT64_MAX,
                                        &prefix, &prefix_len);
    if (st != TX_OK) {
        storage_iter_destroy(iter);
        return st;
    }

    storage_iter_seek(iter, prefix, prefix_len);
    free(prefix);

    /* Check if we found a matching key */
    if (storage_iter_valid(iter)) {
        size_t found_key_len;
        const char* found_key = storage_iter_key(iter, &found_key_len);

        const char* orig_key;
        size_t orig_key_len;
        uint64_t version;
        st = version_decode_key(found_key, found_key_len,
                                &orig_key, &orig_key_len, &version);

        if (st == TX_OK && orig_key_len == key_len &&
            memcmp(orig_key, key, key_len) == 0) {
            *commit_ts = version;
        }
    }

    storage_iter_destroy(iter);
    return TX_OK;
}

tx_status_t check_write_conflicts(tx_manager_t* tm, tx_t* tx) {
    if (!tm || !tx) return TX_INVALID_ARG;

    /* Check each key in write set */
    for (size_t i = 0; i < tx->write_count; i++) {
        write_entry_t* w = &tx->write_set[i];

        uint64_t latest_ts;
        tx_status_t st = get_latest_commit_ts(tm, w->key, w->key_len, &latest_ts);
        if (st != TX_OK) return st;

        /* Conflict if someone committed after our snapshot */
        if (latest_ts > tx->start_ts) {
            return TX_CONFLICT;
        }
    }

    return TX_OK;
}
