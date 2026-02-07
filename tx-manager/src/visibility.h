/**
 * visibility.h - MVCC Version Visibility Rules
 *
 * Determines which versions of data are visible to a transaction
 * based on Snapshot Isolation semantics.
 */

#ifndef TX_VISIBILITY_H
#define TX_VISIBILITY_H

#include "types.h"
#include "tx.h"

/**
 * Check if a version is visible to a transaction
 *
 * A version is visible if:
 * 1. It was committed before the transaction's snapshot (version <= start_ts)
 * 2. It was written by this transaction (in write set)
 *
 * @param version_ts The commit timestamp of the version
 * @param tx The transaction checking visibility
 * @return true if visible, false otherwise
 */
bool is_version_visible(uint64_t version_ts, tx_t* tx);

/**
 * Get the minimum active snapshot timestamp
 *
 * Used for garbage collection - versions older than this
 * can potentially be cleaned up.
 *
 * @param tm Transaction manager
 * @return Minimum snapshot timestamp among active transactions
 */
uint64_t get_min_active_snapshot(tx_manager_t* tm);

#endif /* TX_VISIBILITY_H */
