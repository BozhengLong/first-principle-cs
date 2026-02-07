/**
 * recovery.h - Crash Recovery for Transaction Manager
 *
 * Recovers transaction state from WAL after crash.
 */

#ifndef TX_RECOVERY_H
#define TX_RECOVERY_H

#include "types.h"
#include "tx_manager.h"

/**
 * Recovery result
 */
typedef struct {
    uint64_t max_tx_id;       /* Highest transaction ID seen */
    uint64_t max_ts;          /* Highest timestamp seen */
    size_t committed_count;   /* Number of committed transactions */
    size_t aborted_count;     /* Number of aborted transactions */
} tx_recovery_result_t;

/**
 * Recover transaction manager state from WAL
 *
 * This function:
 * 1. Scans the WAL for all records
 * 2. Tracks which transactions committed vs aborted
 * 3. Updates max_tx_id and max_ts in the manager
 *
 * @param tm Transaction manager
 * @param result Output recovery statistics (optional)
 * @return TX_OK on success
 */
tx_status_t tx_recover(tx_manager_t* tm, tx_recovery_result_t* result);

#endif /* TX_RECOVERY_H */
