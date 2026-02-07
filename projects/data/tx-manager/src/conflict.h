/**
 * conflict.h - Write-Write Conflict Detection
 *
 * Implements first-committer-wins conflict detection for MVCC.
 */

#ifndef TX_CONFLICT_H
#define TX_CONFLICT_H

#include "types.h"
#include "tx.h"
#include "tx_manager.h"

/**
 * Check for write-write conflicts
 *
 * A conflict occurs if another transaction committed a write to any
 * key in our write set after our snapshot was taken.
 *
 * @param tm Transaction manager
 * @param tx Transaction to check
 * @return TX_OK if no conflicts, TX_CONFLICT if conflict detected
 */
tx_status_t check_write_conflicts(tx_manager_t* tm, tx_t* tx);

/**
 * Get the latest commit timestamp for a key
 *
 * @param tm Transaction manager
 * @param key Key to check
 * @param key_len Key length
 * @param commit_ts Output commit timestamp (0 if not found)
 * @return TX_OK on success
 */
tx_status_t get_latest_commit_ts(tx_manager_t* tm,
                                 const char* key, size_t key_len,
                                 uint64_t* commit_ts);

#endif /* TX_CONFLICT_H */
