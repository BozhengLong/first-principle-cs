/**
 * write_set.c - Transaction Write Set Implementation
 */

#include "write_set.h"
#include <stdlib.h>

tx_status_t write_set_get_keys(tx_t* tx, const char*** keys,
                               size_t** key_lens, size_t* count) {
    if (!tx || !keys || !key_lens || !count) {
        return TX_INVALID_ARG;
    }

    *count = tx->write_count;
    if (*count == 0) {
        *keys = NULL;
        *key_lens = NULL;
        return TX_OK;
    }

    *keys = malloc(*count * sizeof(char*));
    *key_lens = malloc(*count * sizeof(size_t));
    if (!*keys || !*key_lens) {
        free(*keys);
        free(*key_lens);
        return TX_NO_MEMORY;
    }

    for (size_t i = 0; i < *count; i++) {
        (*keys)[i] = tx->write_set[i].key;
        (*key_lens)[i] = tx->write_set[i].key_len;
    }

    return TX_OK;
}
