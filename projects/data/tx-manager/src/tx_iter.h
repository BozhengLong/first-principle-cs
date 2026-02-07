/**
 * tx_iter.h - MVCC-aware Iterator
 *
 * Provides range scans that respect transaction visibility.
 */

#ifndef TX_ITER_H
#define TX_ITER_H

#include "types.h"
#include "tx.h"
#include "tx_manager.h"

/**
 * MVCC iterator
 */
typedef struct tx_iter tx_iter_t;

/**
 * Create an iterator for a transaction
 */
tx_iter_t* tx_iter_create(tx_manager_t* tm, tx_t* tx);

/**
 * Destroy an iterator
 */
void tx_iter_destroy(tx_iter_t* iter);

/**
 * Seek to first key
 */
void tx_iter_seek_to_first(tx_iter_t* iter);

/**
 * Seek to a specific key
 */
void tx_iter_seek(tx_iter_t* iter, const char* key, size_t key_len);

/**
 * Check if iterator is valid
 */
bool tx_iter_valid(tx_iter_t* iter);

/**
 * Move to next key
 */
void tx_iter_next(tx_iter_t* iter);

/**
 * Get current key
 */
const char* tx_iter_key(tx_iter_t* iter, size_t* key_len);

/**
 * Get current value
 */
const char* tx_iter_value(tx_iter_t* iter, size_t* value_len);

#endif /* TX_ITER_H */
