/**
 * types.h - Status codes and type definitions for tx-manager
 *
 * Defines common types used throughout the transaction manager.
 */

#ifndef TX_TYPES_H
#define TX_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/**
 * Status codes for transaction operations
 */
typedef enum {
    TX_OK = 0,
    TX_NOT_FOUND = 1,
    TX_CONFLICT = 2,
    TX_ABORTED = 3,
    TX_IO_ERROR = 4,
    TX_INVALID_ARG = 5,
    TX_NO_MEMORY = 6,
    TX_CORRUPTION = 7,
} tx_status_t;

/**
 * Transaction state
 */
typedef enum {
    TX_STATE_ACTIVE = 0,
    TX_STATE_COMMITTED = 1,
    TX_STATE_ABORTED = 2,
} tx_state_t;

/* Forward declarations */
typedef struct tx tx_t;
typedef struct tx_manager tx_manager_t;
typedef struct write_entry write_entry_t;
typedef struct tx_manager_opts tx_manager_opts_t;

/**
 * Transaction manager options
 */
struct tx_manager_opts {
    size_t max_active_txs;    /* Maximum concurrent transactions */
    bool sync_on_commit;      /* fsync on commit */
};

#endif /* TX_TYPES_H */
