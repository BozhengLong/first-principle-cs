/**
 * gc.h - Garbage Collection for MVCC
 *
 * Cleans up old versions that are no longer visible to any transaction.
 */

#ifndef TX_GC_H
#define TX_GC_H

#include "types.h"
#include "tx_manager.h"

/**
 * GC statistics
 */
typedef struct {
    size_t versions_scanned;
    size_t versions_removed;
} tx_gc_stats_t;

/**
 * Run garbage collection
 *
 * Removes versions older than the oldest active snapshot.
 * Only keeps the most recent version for each key that is
 * older than the oldest snapshot.
 *
 * @param tm Transaction manager
 * @param stats Output statistics (optional)
 * @return TX_OK on success
 */
tx_status_t tx_gc_run(tx_manager_t* tm, tx_gc_stats_t* stats);

/**
 * Get the safe GC timestamp
 *
 * Returns the oldest active snapshot timestamp.
 * Versions older than this can potentially be garbage collected.
 */
uint64_t tx_gc_safe_ts(tx_manager_t* tm);

#endif /* TX_GC_H */
