/**
 * tx_manager.h - Transaction Manager Public API
 *
 * Provides MVCC-based transaction management with Snapshot Isolation.
 */

#ifndef TX_MANAGER_H
#define TX_MANAGER_H

#include "types.h"
#include "tx.h"
#include "tx_wal.h"
#include <pthread.h>

/**
 * Transaction manager structure
 */
struct tx_manager {
    void* storage;            /* Underlying storage engine (storage_t*) */
    char* path;               /* Database path */
    tx_wal_t* wal;            /* Transaction WAL */

    uint64_t next_tx_id;      /* Next transaction ID */
    uint64_t next_ts;         /* Next timestamp */

    tx_t** active_txs;        /* Array of active transactions */
    size_t active_count;      /* Number of active transactions */
    size_t active_capacity;   /* Capacity of active array */

    tx_manager_opts_t opts;   /* Configuration options */

    pthread_mutex_t mutex;    /* Protects manager state */
};

/**
 * Open a transaction manager
 *
 * @param path Database directory path
 * @param opts Configuration options (NULL for defaults)
 * @return Transaction manager or NULL on error
 */
tx_manager_t* tx_manager_open(const char* path, tx_manager_opts_t* opts);

/**
 * Close a transaction manager
 *
 * Aborts any active transactions.
 */
void tx_manager_close(tx_manager_t* tm);

/**
 * Begin a new transaction
 *
 * @param tm Transaction manager
 * @return New transaction or NULL on error
 */
tx_t* tx_begin(tx_manager_t* tm);

/**
 * Commit a transaction
 *
 * Atomically applies all writes to storage.
 *
 * @param tm Transaction manager
 * @param tx Transaction to commit
 * @return TX_OK on success, TX_CONFLICT on write conflict
 */
tx_status_t tx_commit(tx_manager_t* tm, tx_t* tx);

/**
 * Abort a transaction
 *
 * Discards all buffered writes.
 *
 * @param tm Transaction manager
 * @param tx Transaction to abort
 * @return TX_OK on success
 */
tx_status_t tx_abort(tx_manager_t* tm, tx_t* tx);

/**
 * Put a key-value pair within a transaction
 *
 * The write is buffered until commit.
 */
tx_status_t tx_put(tx_manager_t* tm, tx_t* tx,
                   const char* key, size_t key_len,
                   const char* value, size_t value_len);

/**
 * Get a value within a transaction
 *
 * Returns the value visible to this transaction's snapshot.
 * Caller must free *value.
 */
tx_status_t tx_get(tx_manager_t* tm, tx_t* tx,
                   const char* key, size_t key_len,
                   char** value, size_t* value_len);

/**
 * Delete a key within a transaction
 *
 * The delete is buffered until commit.
 */
tx_status_t tx_delete(tx_manager_t* tm, tx_t* tx,
                      const char* key, size_t key_len);

#endif /* TX_MANAGER_H */
