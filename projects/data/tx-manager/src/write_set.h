/**
 * write_set.h - Transaction Write Set Management
 *
 * Manages the buffered writes for a transaction.
 */

#ifndef TX_WRITE_SET_H
#define TX_WRITE_SET_H

#include "types.h"
#include "tx.h"

/**
 * Get all keys in the write set
 *
 * @param tx Transaction
 * @param keys Output array of key pointers (caller must free array, not keys)
 * @param key_lens Output array of key lengths
 * @param count Output number of keys
 * @return TX_OK on success
 */
tx_status_t write_set_get_keys(tx_t* tx, const char*** keys,
                               size_t** key_lens, size_t* count);

#endif /* TX_WRITE_SET_H */
