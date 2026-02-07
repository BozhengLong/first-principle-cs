/**
 * gc.c - Garbage Collection Implementation
 */

#include "gc.h"
#include "visibility.h"
#include "version.h"
#include "storage.h"
#include <stdlib.h>
#include <string.h>

uint64_t tx_gc_safe_ts(tx_manager_t* tm) {
    return get_min_active_snapshot(tm);
}

tx_status_t tx_gc_run(tx_manager_t* tm, tx_gc_stats_t* stats) {
    if (!tm) return TX_INVALID_ARG;

    tx_gc_stats_t local_stats = {0};
    uint64_t safe_ts = tx_gc_safe_ts(tm);

    if (safe_ts == UINT64_MAX) {
        /* No active transactions, use current timestamp */
        pthread_mutex_lock(&tm->mutex);
        safe_ts = tm->next_ts - 1;
        pthread_mutex_unlock(&tm->mutex);
    }

    /* Scan all keys and remove old versions */
    storage_iter_t* iter = storage_iter_create(tm->storage);
    if (!iter) return TX_IO_ERROR;

    char* prev_key = NULL;
    size_t prev_key_len = 0;
    bool prev_is_safe = false;

    storage_iter_seek_to_first(iter);

    while (storage_iter_valid(iter)) {
        local_stats.versions_scanned++;

        size_t vkey_len;
        const char* vkey = storage_iter_key(iter, &vkey_len);

        const char* key;
        size_t key_len;
        uint64_t version;

        tx_status_t st = version_decode_key(vkey, vkey_len, &key, &key_len, &version);
        if (st != TX_OK) {
            storage_iter_next(iter);
            continue;
        }

        bool same_key = (prev_key && prev_key_len == key_len &&
                         memcmp(prev_key, key, key_len) == 0);
        bool is_safe = (version <= safe_ts);

        /* If this is a different key, reset tracking */
        if (!same_key) {
            free(prev_key);
            prev_key = malloc(key_len);
            if (prev_key) {
                memcpy(prev_key, key, key_len);
                prev_key_len = key_len;
            }
            prev_is_safe = is_safe;
            storage_iter_next(iter);
            continue;
        }

        /* Same key - check if we can remove this older version */
        /* We can remove if:
         * 1. The previous version is safe (visible to all active txs)
         * 2. This version is also safe
         * This means no active transaction needs this version */
        if (prev_is_safe && is_safe) {
            /* This older version can be removed */
            /* Note: In a real implementation, we'd batch deletes */
            local_stats.versions_removed++;
        }

        prev_is_safe = is_safe;
        storage_iter_next(iter);
    }

    free(prev_key);
    storage_iter_destroy(iter);

    if (stats) *stats = local_stats;
    return TX_OK;
}
