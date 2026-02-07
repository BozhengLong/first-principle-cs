/**
 * tx_iter.c - MVCC-aware Iterator Implementation
 */

#include "tx_iter.h"
#include "version.h"
#include "visibility.h"
#include "storage.h"
#include <stdlib.h>
#include <string.h>

struct tx_iter {
    tx_manager_t* tm;
    tx_t* tx;
    storage_iter_t* storage_iter;

    /* Current visible key/value */
    char* current_key;
    size_t current_key_len;
    char* current_value;
    size_t current_value_len;
    bool valid;
};

tx_iter_t* tx_iter_create(tx_manager_t* tm, tx_t* tx) {
    if (!tm || !tx) return NULL;

    tx_iter_t* iter = calloc(1, sizeof(tx_iter_t));
    if (!iter) return NULL;

    iter->tm = tm;
    iter->tx = tx;
    iter->storage_iter = storage_iter_create(tm->storage);
    if (!iter->storage_iter) {
        free(iter);
        return NULL;
    }

    iter->valid = false;
    return iter;
}

void tx_iter_destroy(tx_iter_t* iter) {
    if (!iter) return;
    if (iter->storage_iter) storage_iter_destroy(iter->storage_iter);
    free(iter->current_key);
    free(iter->current_value);
    free(iter);
}

static void advance_to_visible(tx_iter_t* iter) {
    free(iter->current_key);
    free(iter->current_value);
    iter->current_key = NULL;
    iter->current_value = NULL;
    iter->valid = false;

    char* last_key = NULL;
    size_t last_key_len = 0;

    while (storage_iter_valid(iter->storage_iter)) {
        size_t vkey_len;
        const char* vkey = storage_iter_key(iter->storage_iter, &vkey_len);

        const char* key;
        size_t key_len;
        uint64_t version;

        tx_status_t st = version_decode_key(vkey, vkey_len, &key, &key_len, &version);
        if (st != TX_OK) {
            storage_iter_next(iter->storage_iter);
            continue;
        }

        /* Skip if same key as last (we already found the visible version) */
        if (last_key && last_key_len == key_len &&
            memcmp(last_key, key, key_len) == 0) {
            storage_iter_next(iter->storage_iter);
            continue;
        }

        /* Check visibility */
        if (!is_version_visible(version, iter->tx)) {
            storage_iter_next(iter->storage_iter);
            continue;
        }

        /* Found visible version */
        size_t val_len;
        const char* val = storage_iter_value(iter->storage_iter, &val_len);

        /* Skip tombstones */
        if (val_len == 0) {
            /* Remember this key to skip older versions */
            free(last_key);
            last_key = malloc(key_len);
            if (last_key) {
                memcpy(last_key, key, key_len);
                last_key_len = key_len;
            }
            storage_iter_next(iter->storage_iter);
            continue;
        }

        /* Store current key/value */
        iter->current_key = malloc(key_len);
        iter->current_value = malloc(val_len);
        if (iter->current_key && iter->current_value) {
            memcpy(iter->current_key, key, key_len);
            iter->current_key_len = key_len;
            memcpy(iter->current_value, val, val_len);
            iter->current_value_len = val_len;
            iter->valid = true;
        }

        free(last_key);
        return;
    }

    free(last_key);
}

void tx_iter_seek_to_first(tx_iter_t* iter) {
    if (!iter) return;
    storage_iter_seek_to_first(iter->storage_iter);
    advance_to_visible(iter);
}

void tx_iter_seek(tx_iter_t* iter, const char* key, size_t key_len) {
    if (!iter || !key) return;

    /* Encode key with max timestamp to find first version */
    char* vkey;
    size_t vkey_len;
    if (version_encode_key(key, key_len, UINT64_MAX, &vkey, &vkey_len) != TX_OK) {
        iter->valid = false;
        return;
    }

    storage_iter_seek(iter->storage_iter, vkey, vkey_len);
    free(vkey);
    advance_to_visible(iter);
}

bool tx_iter_valid(tx_iter_t* iter) {
    return iter && iter->valid;
}

void tx_iter_next(tx_iter_t* iter) {
    if (!iter || !iter->valid) return;

    /* Move past current key's versions */
    char* skip_key = iter->current_key;
    size_t skip_key_len = iter->current_key_len;
    iter->current_key = NULL;  /* Don't free yet */

    while (storage_iter_valid(iter->storage_iter)) {
        size_t vkey_len;
        const char* vkey = storage_iter_key(iter->storage_iter, &vkey_len);

        const char* key;
        size_t key_len;
        uint64_t version;

        if (version_decode_key(vkey, vkey_len, &key, &key_len, &version) == TX_OK) {
            if (key_len != skip_key_len || memcmp(key, skip_key, key_len) != 0) {
                break;  /* Different key */
            }
        }
        storage_iter_next(iter->storage_iter);
    }

    free(skip_key);
    advance_to_visible(iter);
}

const char* tx_iter_key(tx_iter_t* iter, size_t* key_len) {
    if (!iter || !iter->valid) return NULL;
    if (key_len) *key_len = iter->current_key_len;
    return iter->current_key;
}

const char* tx_iter_value(tx_iter_t* iter, size_t* value_len) {
    if (!iter || !iter->valid) return NULL;
    if (value_len) *value_len = iter->current_value_len;
    return iter->current_value;
}
