/**
 * iterator.h - Range scan iterator
 *
 * Provides iteration over key ranges.
 */

#ifndef DIST_KV_ITERATOR_H
#define DIST_KV_ITERATOR_H

#include "types.h"
#include "storage_adapter.h"

/* Forward declaration */
typedef struct dkv_iterator dkv_iterator_t;

/* Iterator options */
typedef struct {
    const char *start_key;       /* Start of range (inclusive) */
    size_t start_key_len;
    const char *end_key;         /* End of range (exclusive), NULL for no limit */
    size_t end_key_len;
    int limit;                   /* Maximum entries to return, 0 for no limit */
    bool reverse;                /* Iterate in reverse order */
} iterator_options_t;

/* Create an iterator */
dkv_iterator_t *dkv_iterator_create(storage_adapter_t *storage,
                                     const iterator_options_t *options);

/* Destroy an iterator */
void dkv_iterator_destroy(dkv_iterator_t *iter);

/* Check if iterator has more entries */
bool dkv_iterator_valid(dkv_iterator_t *iter);

/* Move to next entry */
void dkv_iterator_next(dkv_iterator_t *iter);

/* Get current key */
const char *dkv_iterator_key(dkv_iterator_t *iter, size_t *key_len);

/* Get current value */
const void *dkv_iterator_value(dkv_iterator_t *iter, size_t *value_len);

/* Reset iterator to beginning */
void dkv_iterator_reset(dkv_iterator_t *iter);

/* Get number of entries iterated */
int dkv_iterator_count(dkv_iterator_t *iter);

/* Add an entry to the iterator (for testing) */
dkv_status_t dkv_iterator_add_entry(dkv_iterator_t *iter, const char *key, size_t key_len,
                                     const void *value, size_t value_len);

/* Sort the iterator entries */
void dkv_iterator_sort(dkv_iterator_t *iter);

#endif /* DIST_KV_ITERATOR_H */
