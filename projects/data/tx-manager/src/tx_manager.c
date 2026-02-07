/**
 * tx_manager.c - Transaction Manager Implementation
 *
 * Phase 1: Basic transaction framework with single-threaded operations.
 */

#include "tx_manager.h"
#include "version.h"
#include "param.h"
#include "conflict.h"
#include <stdlib.h>
#include <string.h>

/* Include storage-engine headers */
#include "storage.h"

static void default_opts(tx_manager_opts_t* opts) {
    opts->max_active_txs = TX_DEFAULT_MAX_ACTIVE_TXS;
    opts->sync_on_commit = TX_DEFAULT_SYNC_ON_COMMIT;
}

tx_manager_t* tx_manager_open(const char* path, tx_manager_opts_t* opts) {
    if (!path) return NULL;

    tx_manager_t* tm = calloc(1, sizeof(tx_manager_t));
    if (!tm) return NULL;

    /* Copy path */
    tm->path = strdup(path);
    if (!tm->path) {
        free(tm);
        return NULL;
    }

    /* Set options */
    if (opts) {
        tm->opts = *opts;
    } else {
        default_opts(&tm->opts);
    }

    /* Open underlying storage */
    tm->storage = storage_open(path, NULL);
    if (!tm->storage) {
        free(tm->path);
        free(tm);
        return NULL;
    }

    /* Initialize transaction tracking */
    tm->next_tx_id = 1;
    tm->next_ts = 1;

    tm->active_capacity = tm->opts.max_active_txs;
    tm->active_txs = calloc(tm->active_capacity, sizeof(tx_t*));
    if (!tm->active_txs) {
        storage_close(tm->storage);
        free(tm->path);
        free(tm);
        return NULL;
    }
    tm->active_count = 0;

    /* Open transaction WAL */
    tm->wal = tx_wal_open(path);
    if (!tm->wal) {
        free(tm->active_txs);
        storage_close(tm->storage);
        free(tm->path);
        free(tm);
        return NULL;
    }

    pthread_mutex_init(&tm->mutex, NULL);

    return tm;
}

void tx_manager_close(tx_manager_t* tm) {
    if (!tm) return;

    pthread_mutex_lock(&tm->mutex);

    /* Abort all active transactions */
    for (size_t i = 0; i < tm->active_count; i++) {
        if (tm->active_txs[i]) {
            tm->active_txs[i]->state = TX_STATE_ABORTED;
            tx_destroy(tm->active_txs[i]);
        }
    }
    free(tm->active_txs);

    pthread_mutex_unlock(&tm->mutex);
    pthread_mutex_destroy(&tm->mutex);

    if (tm->storage) {
        storage_close(tm->storage);
    }
    if (tm->wal) {
        tx_wal_close(tm->wal);
    }
    free(tm->path);
    free(tm);
}

static void add_active_tx(tx_manager_t* tm, tx_t* tx) {
    /* Find empty slot or append */
    for (size_t i = 0; i < tm->active_capacity; i++) {
        if (!tm->active_txs[i]) {
            tm->active_txs[i] = tx;
            tm->active_count++;
            return;
        }
    }
}

static void remove_active_tx(tx_manager_t* tm, tx_t* tx) {
    for (size_t i = 0; i < tm->active_capacity; i++) {
        if (tm->active_txs[i] == tx) {
            tm->active_txs[i] = NULL;
            tm->active_count--;
            return;
        }
    }
}

tx_t* tx_begin(tx_manager_t* tm) {
    if (!tm) return NULL;

    pthread_mutex_lock(&tm->mutex);

    /* Check capacity */
    if (tm->active_count >= tm->active_capacity) {
        pthread_mutex_unlock(&tm->mutex);
        return NULL;
    }

    /* Allocate transaction ID and snapshot timestamp */
    uint64_t tx_id = tm->next_tx_id++;
    uint64_t start_ts = tm->next_ts++;

    tx_t* tx = tx_create(tx_id, start_ts);
    if (!tx) {
        pthread_mutex_unlock(&tm->mutex);
        return NULL;
    }

    /* Log begin to WAL */
    tx_wal_log_begin(tm->wal, tx);

    add_active_tx(tm, tx);

    pthread_mutex_unlock(&tm->mutex);
    return tx;
}

tx_status_t tx_abort(tx_manager_t* tm, tx_t* tx) {
    if (!tm || !tx) return TX_INVALID_ARG;
    if (tx->state != TX_STATE_ACTIVE) return TX_ABORTED;

    pthread_mutex_lock(&tm->mutex);

    tx->state = TX_STATE_ABORTED;
    tx_clear_writes(tx);
    remove_active_tx(tm, tx);

    /* Log abort to WAL */
    tx_wal_log_abort(tm->wal, tx);

    pthread_mutex_unlock(&tm->mutex);

    tx_destroy(tx);
    return TX_OK;
}

tx_status_t tx_commit(tx_manager_t* tm, tx_t* tx) {
    if (!tm || !tx) return TX_INVALID_ARG;
    if (tx->state != TX_STATE_ACTIVE) return TX_ABORTED;

    pthread_mutex_lock(&tm->mutex);

    /* Check for write-write conflicts (first-committer-wins) */
    tx_status_t conflict_st = check_write_conflicts(tm, tx);
    if (conflict_st == TX_CONFLICT) {
        tx->state = TX_STATE_ABORTED;
        remove_active_tx(tm, tx);
        pthread_mutex_unlock(&tm->mutex);
        tx_destroy(tx);
        return TX_CONFLICT;
    }

    /* Allocate commit timestamp */
    uint64_t commit_ts = tm->next_ts++;
    tx->commit_ts = commit_ts;

    /* Apply all writes to storage with versioned keys */
    for (size_t i = 0; i < tx->write_count; i++) {
        write_entry_t* w = &tx->write_set[i];

        char* versioned_key;
        size_t versioned_len;
        tx_status_t st = version_encode_key(w->key, w->key_len, commit_ts,
                                            &versioned_key, &versioned_len);
        if (st != TX_OK) {
            pthread_mutex_unlock(&tm->mutex);
            return st;
        }

        status_t sst;
        if (w->is_delete) {
            /* Write tombstone (empty value) for MVCC delete */
            sst = storage_put(tm->storage, versioned_key, versioned_len,
                              "", 0);
        } else {
            sst = storage_put(tm->storage, versioned_key, versioned_len,
                              w->value, w->value_len);
        }
        free(versioned_key);

        if (sst != STATUS_OK) {
            pthread_mutex_unlock(&tm->mutex);
            return TX_IO_ERROR;
        }
    }

    tx->state = TX_STATE_COMMITTED;
    remove_active_tx(tm, tx);

    /* Log commit to WAL and sync */
    tx_wal_log_commit(tm->wal, tx);
    if (tm->opts.sync_on_commit) {
        tx_wal_sync(tm->wal);
    }

    pthread_mutex_unlock(&tm->mutex);

    tx_destroy(tx);
    return TX_OK;
}

tx_status_t tx_put(tx_manager_t* tm, tx_t* tx,
                   const char* key, size_t key_len,
                   const char* value, size_t value_len) {
    if (!tm || !tx || !key || !value) return TX_INVALID_ARG;
    if (tx->state != TX_STATE_ACTIVE) return TX_ABORTED;

    return tx_add_write(tx, key, key_len, value, value_len, false);
}

tx_status_t tx_delete(tx_manager_t* tm, tx_t* tx,
                      const char* key, size_t key_len) {
    if (!tm || !tx || !key) return TX_INVALID_ARG;
    if (tx->state != TX_STATE_ACTIVE) return TX_ABORTED;

    return tx_add_write(tx, key, key_len, NULL, 0, true);
}

tx_status_t tx_get(tx_manager_t* tm, tx_t* tx,
                   const char* key, size_t key_len,
                   char** value, size_t* value_len) {
    if (!tm || !tx || !key || !value || !value_len) return TX_INVALID_ARG;
    if (tx->state != TX_STATE_ACTIVE) return TX_ABORTED;

    *value = NULL;
    *value_len = 0;

    /* First check write set (read-your-writes) */
    write_entry_t* local = tx_find_write(tx, key, key_len);
    if (local) {
        if (local->is_delete) {
            return TX_NOT_FOUND;
        }
        *value = malloc(local->value_len);
        if (!*value) return TX_NO_MEMORY;
        memcpy(*value, local->value, local->value_len);
        *value_len = local->value_len;
        return TX_OK;
    }

    /* Search storage for visible version */
    /* Phase 1: Simple approach - scan for key with version <= start_ts */
    storage_iter_t* iter = storage_iter_create(tm->storage);
    if (!iter) return TX_IO_ERROR;

    /* Build prefix to seek to */
    char* prefix;
    size_t prefix_len;
    tx_status_t st = version_encode_key(key, key_len, tx->start_ts,
                                        &prefix, &prefix_len);
    if (st != TX_OK) {
        storage_iter_destroy(iter);
        return st;
    }

    storage_iter_seek(iter, prefix, prefix_len);
    free(prefix);

    /* Check if we found a matching key */
    while (storage_iter_valid(iter)) {
        size_t found_key_len;
        const char* found_key = storage_iter_key(iter, &found_key_len);

        const char* orig_key;
        size_t orig_key_len;
        uint64_t version;
        st = version_decode_key(found_key, found_key_len,
                                &orig_key, &orig_key_len, &version);
        if (st != TX_OK) {
            storage_iter_next(iter);
            continue;
        }

        /* Check if this is our key */
        if (orig_key_len != key_len ||
            memcmp(orig_key, key, key_len) != 0) {
            /* Different key, stop searching */
            break;
        }

        /* Check visibility: version must be <= start_ts */
        if (version <= tx->start_ts) {
            /* Found visible version */
            size_t val_len;
            const char* val = storage_iter_value(iter, &val_len);

            /* Check for tombstone (empty value = deleted) */
            if (val_len == 0) {
                storage_iter_destroy(iter);
                return TX_NOT_FOUND;
            }

            *value = malloc(val_len);
            if (!*value) {
                storage_iter_destroy(iter);
                return TX_NO_MEMORY;
            }
            memcpy(*value, val, val_len);
            *value_len = val_len;
            storage_iter_destroy(iter);
            return TX_OK;
        }

        storage_iter_next(iter);
    }

    storage_iter_destroy(iter);
    return TX_NOT_FOUND;
}
