/**
 * iterator.c - Range scan iterator implementation
 */

#include "iterator.h"
#include <stdlib.h>
#include <string.h>

#define MAX_ITER_ENTRIES 10000

typedef struct {
    char *key;
    size_t key_len;
    void *value;
    size_t value_len;
} iter_entry_t;

struct dkv_iterator {
    iterator_options_t options;
    storage_adapter_t *storage;
    iter_entry_t *entries;
    int entry_count;
    int current;
    int iterated;
};

/* Compare keys */
static int key_compare(const char *a, size_t a_len, const char *b, size_t b_len) {
    size_t min_len = a_len < b_len ? a_len : b_len;
    int cmp = memcmp(a, b, min_len);
    if (cmp != 0) return cmp;
    if (a_len < b_len) return -1;
    if (a_len > b_len) return 1;
    return 0;
}

/* Sort entries by key */
static int entry_compare(const void *a, const void *b) {
    const iter_entry_t *ea = (const iter_entry_t *)a;
    const iter_entry_t *eb = (const iter_entry_t *)b;
    return key_compare(ea->key, ea->key_len, eb->key, eb->key_len);
}

static int entry_compare_reverse(const void *a, const void *b) {
    return -entry_compare(a, b);
}

dkv_iterator_t *dkv_iterator_create(storage_adapter_t *storage,
                                     const iterator_options_t *options) {
    if (!storage) return NULL;

    dkv_iterator_t *iter = calloc(1, sizeof(dkv_iterator_t));
    if (!iter) return NULL;

    iter->storage = storage;
    if (options) {
        iter->options = *options;
    }

    /* Get storage stats to estimate size */
    storage_stats_t stats;
    storage_adapter_get_stats(storage, &stats);

    int capacity = stats.num_keys;
    if (capacity < 16) capacity = 16;
    if (capacity > MAX_ITER_ENTRIES) capacity = MAX_ITER_ENTRIES;

    iter->entries = calloc(capacity, sizeof(iter_entry_t));
    if (!iter->entries) {
        free(iter);
        return NULL;
    }

    /* Scan storage and collect matching entries */
    /* Note: This is a simplified implementation. A real implementation
     * would use the storage engine's native iteration support. */

    /* For now, we'll just mark the iterator as empty since we don't have
     * direct access to storage internals. The storage_adapter would need
     * to expose an iteration interface. */

    iter->entry_count = 0;
    iter->current = 0;

    return iter;
}

void dkv_iterator_destroy(dkv_iterator_t *iter) {
    if (!iter) return;

    for (int i = 0; i < iter->entry_count; i++) {
        free(iter->entries[i].key);
        free(iter->entries[i].value);
    }
    free(iter->entries);
    free(iter);
}

bool dkv_iterator_valid(dkv_iterator_t *iter) {
    if (!iter) return false;
    if (iter->options.limit > 0 && iter->iterated >= iter->options.limit) {
        return false;
    }
    return iter->current < iter->entry_count;
}

void dkv_iterator_next(dkv_iterator_t *iter) {
    if (!iter || !dkv_iterator_valid(iter)) return;
    iter->current++;
    iter->iterated++;
}

const char *dkv_iterator_key(dkv_iterator_t *iter, size_t *key_len) {
    if (!iter || !dkv_iterator_valid(iter)) return NULL;
    if (key_len) *key_len = iter->entries[iter->current].key_len;
    return iter->entries[iter->current].key;
}

const void *dkv_iterator_value(dkv_iterator_t *iter, size_t *value_len) {
    if (!iter || !dkv_iterator_valid(iter)) return NULL;
    if (value_len) *value_len = iter->entries[iter->current].value_len;
    return iter->entries[iter->current].value;
}

void dkv_iterator_reset(dkv_iterator_t *iter) {
    if (!iter) return;
    iter->current = 0;
    iter->iterated = 0;
}

int dkv_iterator_count(dkv_iterator_t *iter) {
    return iter ? iter->iterated : 0;
}

/* Add an entry to the iterator (for testing) */
dkv_status_t dkv_iterator_add_entry(dkv_iterator_t *iter, const char *key, size_t key_len,
                                     const void *value, size_t value_len) {
    if (!iter || !key || !value) return DKV_ERR_INVALID;
    if (iter->entry_count >= MAX_ITER_ENTRIES) return DKV_ERR_NOMEM;

    /* Check range bounds */
    if (iter->options.start_key) {
        if (key_compare(key, key_len, iter->options.start_key,
                        iter->options.start_key_len) < 0) {
            return DKV_OK;  /* Skip, before range */
        }
    }
    if (iter->options.end_key) {
        if (key_compare(key, key_len, iter->options.end_key,
                        iter->options.end_key_len) >= 0) {
            return DKV_OK;  /* Skip, after range */
        }
    }

    iter_entry_t *e = &iter->entries[iter->entry_count];
    e->key = malloc(key_len);
    e->value = malloc(value_len);
    if (!e->key || !e->value) {
        free(e->key);
        free(e->value);
        return DKV_ERR_NOMEM;
    }

    memcpy(e->key, key, key_len);
    e->key_len = key_len;
    memcpy(e->value, value, value_len);
    e->value_len = value_len;

    iter->entry_count++;
    return DKV_OK;
}

/* Sort the iterator entries */
void dkv_iterator_sort(dkv_iterator_t *iter) {
    if (!iter || iter->entry_count <= 1) return;

    if (iter->options.reverse) {
        qsort(iter->entries, iter->entry_count, sizeof(iter_entry_t), entry_compare_reverse);
    } else {
        qsort(iter->entries, iter->entry_count, sizeof(iter_entry_t), entry_compare);
    }
}
