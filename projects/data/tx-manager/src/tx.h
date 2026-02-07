/**
 * tx.h - Transaction descriptor
 *
 * Represents an active transaction with its state and write set.
 */

#ifndef TX_TX_H
#define TX_TX_H

#include "types.h"

/**
 * Write entry in transaction's write set
 */
struct write_entry {
    char* key;
    size_t key_len;
    char* value;
    size_t value_len;
    bool is_delete;
};

/**
 * Transaction descriptor
 */
struct tx {
    uint64_t tx_id;           /* Unique transaction ID */
    uint64_t start_ts;        /* Snapshot timestamp (read visibility) */
    uint64_t commit_ts;       /* Commit timestamp (0 if not committed) */
    tx_state_t state;         /* Current state */

    write_entry_t* write_set; /* Buffered writes */
    size_t write_count;       /* Number of writes */
    size_t write_capacity;    /* Allocated capacity */
};

/**
 * Create a new transaction
 */
tx_t* tx_create(uint64_t tx_id, uint64_t start_ts);

/**
 * Destroy a transaction and free resources
 */
void tx_destroy(tx_t* tx);

/**
 * Add a write to the transaction's write set
 */
tx_status_t tx_add_write(tx_t* tx, const char* key, size_t key_len,
                         const char* value, size_t value_len,
                         bool is_delete);

/**
 * Find a write in the transaction's write set
 * Returns NULL if not found
 */
write_entry_t* tx_find_write(tx_t* tx, const char* key, size_t key_len);

/**
 * Clear all writes from the transaction
 */
void tx_clear_writes(tx_t* tx);

#endif /* TX_TX_H */
