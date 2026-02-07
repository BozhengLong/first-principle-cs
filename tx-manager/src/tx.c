/**
 * tx.c - Transaction descriptor implementation
 */

#include "tx.h"
#include "param.h"
#include <stdlib.h>
#include <string.h>

tx_t* tx_create(uint64_t tx_id, uint64_t start_ts) {
    tx_t* tx = calloc(1, sizeof(tx_t));
    if (!tx) return NULL;

    tx->tx_id = tx_id;
    tx->start_ts = start_ts;
    tx->commit_ts = 0;
    tx->state = TX_STATE_ACTIVE;

    tx->write_capacity = TX_WRITE_SET_INIT_CAPACITY;
    tx->write_set = calloc(tx->write_capacity, sizeof(write_entry_t));
    if (!tx->write_set) {
        free(tx);
        return NULL;
    }
    tx->write_count = 0;

    return tx;
}

static void free_write_entry(write_entry_t* entry) {
    if (entry->key) free(entry->key);
    if (entry->value) free(entry->value);
    entry->key = NULL;
    entry->value = NULL;
}

void tx_destroy(tx_t* tx) {
    if (!tx) return;

    for (size_t i = 0; i < tx->write_count; i++) {
        free_write_entry(&tx->write_set[i]);
    }
    free(tx->write_set);
    free(tx);
}

void tx_clear_writes(tx_t* tx) {
    if (!tx) return;

    for (size_t i = 0; i < tx->write_count; i++) {
        free_write_entry(&tx->write_set[i]);
    }
    tx->write_count = 0;
}

write_entry_t* tx_find_write(tx_t* tx, const char* key, size_t key_len) {
    if (!tx || !key) return NULL;

    for (size_t i = 0; i < tx->write_count; i++) {
        write_entry_t* entry = &tx->write_set[i];
        if (entry->key_len == key_len &&
            memcmp(entry->key, key, key_len) == 0) {
            return entry;
        }
    }
    return NULL;
}

tx_status_t tx_add_write(tx_t* tx, const char* key, size_t key_len,
                         const char* value, size_t value_len,
                         bool is_delete) {
    if (!tx || !key) return TX_INVALID_ARG;
    if (!is_delete && !value) return TX_INVALID_ARG;

    /* Check if key already exists in write set */
    write_entry_t* existing = tx_find_write(tx, key, key_len);
    if (existing) {
        /* Update existing entry */
        if (existing->value) free(existing->value);

        if (is_delete) {
            existing->value = NULL;
            existing->value_len = 0;
            existing->is_delete = true;
        } else {
            existing->value = malloc(value_len);
            if (!existing->value) return TX_NO_MEMORY;
            memcpy(existing->value, value, value_len);
            existing->value_len = value_len;
            existing->is_delete = false;
        }
        return TX_OK;
    }

    /* Grow write set if needed */
    if (tx->write_count >= tx->write_capacity) {
        size_t new_cap = tx->write_capacity * 2;
        write_entry_t* new_set = realloc(tx->write_set,
                                         new_cap * sizeof(write_entry_t));
        if (!new_set) return TX_NO_MEMORY;
        tx->write_set = new_set;
        tx->write_capacity = new_cap;
    }

    /* Add new entry */
    write_entry_t* entry = &tx->write_set[tx->write_count];
    entry->key = malloc(key_len);
    if (!entry->key) return TX_NO_MEMORY;
    memcpy(entry->key, key, key_len);
    entry->key_len = key_len;

    if (is_delete) {
        entry->value = NULL;
        entry->value_len = 0;
        entry->is_delete = true;
    } else {
        entry->value = malloc(value_len);
        if (!entry->value) {
            free(entry->key);
            return TX_NO_MEMORY;
        }
        memcpy(entry->value, value, value_len);
        entry->value_len = value_len;
        entry->is_delete = false;
    }

    tx->write_count++;
    return TX_OK;
}
