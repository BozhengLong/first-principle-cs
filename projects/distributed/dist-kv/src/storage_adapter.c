/**
 * storage_adapter.c - Storage engine adapter implementation
 *
 * Simplified in-memory storage for testing.
 * In production, this would integrate with the LSM-tree storage engine.
 */

#include "storage_adapter.h"
#include "param.h"
#include <stdlib.h>
#include <string.h>

#define MAX_ENTRIES 10000

typedef struct {
    char *key;
    size_t key_len;
    void *value;
    size_t value_len;
    bool deleted;
} storage_entry_t;

struct storage_adapter {
    storage_adapter_config_t config;
    storage_entry_t *entries;
    int entry_count;
    int entry_capacity;
    size_t data_size;
};

void storage_adapter_config_init(storage_adapter_config_t *config) {
    if (!config) return;
    memset(config, 0, sizeof(*config));
    config->memtable_size = DKV_DEFAULT_MEMTABLE_SIZE;
    config->block_size = DKV_DEFAULT_BLOCK_SIZE;
    strncpy(config->data_dir, "/tmp/dkv", sizeof(config->data_dir) - 1);
}

storage_adapter_t *storage_adapter_create(const storage_adapter_config_t *config) {
    storage_adapter_t *sa = calloc(1, sizeof(storage_adapter_t));
    if (!sa) return NULL;

    if (config) {
        sa->config = *config;
    } else {
        storage_adapter_config_init(&sa->config);
    }

    sa->entry_capacity = 64;
    sa->entries = calloc(sa->entry_capacity, sizeof(storage_entry_t));
    if (!sa->entries) {
        free(sa);
        return NULL;
    }

    return sa;
}

void storage_adapter_destroy(storage_adapter_t *sa) {
    if (!sa) return;

    for (int i = 0; i < sa->entry_count; i++) {
        if (!sa->entries[i].deleted) {
            free(sa->entries[i].key);
            free(sa->entries[i].value);
        }
    }
    free(sa->entries);
    free(sa);
}

/* Find entry by key, returns index or -1 if not found */
static int find_entry(storage_adapter_t *sa, const char *key, size_t key_len) {
    for (int i = 0; i < sa->entry_count; i++) {
        if (!sa->entries[i].deleted &&
            sa->entries[i].key_len == key_len &&
            memcmp(sa->entries[i].key, key, key_len) == 0) {
            return i;
        }
    }
    return -1;
}

dkv_status_t storage_adapter_put(storage_adapter_t *sa, const char *key, size_t key_len,
                                  const void *value, size_t value_len) {
    if (!sa || !key || !value) return DKV_ERR_INVALID;

    /* Check if key exists */
    int idx = find_entry(sa, key, key_len);
    if (idx >= 0) {
        /* Update existing entry */
        storage_entry_t *e = &sa->entries[idx];
        void *new_value = malloc(value_len);
        if (!new_value) return DKV_ERR_NOMEM;

        sa->data_size -= e->value_len;
        free(e->value);
        e->value = new_value;
        memcpy(e->value, value, value_len);
        e->value_len = value_len;
        sa->data_size += value_len;

        return DKV_OK;
    }

    /* Expand if needed */
    if (sa->entry_count >= sa->entry_capacity) {
        int new_cap = sa->entry_capacity * 2;
        if (new_cap > MAX_ENTRIES) return DKV_ERR_NOMEM;
        storage_entry_t *new_entries = realloc(sa->entries, new_cap * sizeof(storage_entry_t));
        if (!new_entries) return DKV_ERR_NOMEM;
        sa->entries = new_entries;
        sa->entry_capacity = new_cap;
    }

    /* Create new entry */
    storage_entry_t *e = &sa->entries[sa->entry_count];
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
    e->deleted = false;

    sa->entry_count++;
    sa->data_size += key_len + value_len;

    return DKV_OK;
}

dkv_status_t storage_adapter_get(storage_adapter_t *sa, const char *key, size_t key_len,
                                  void **value, size_t *value_len) {
    if (!sa || !key || !value || !value_len) return DKV_ERR_INVALID;

    int idx = find_entry(sa, key, key_len);
    if (idx < 0) return DKV_ERR_NOT_FOUND;

    storage_entry_t *e = &sa->entries[idx];
    *value = malloc(e->value_len);
    if (!*value) return DKV_ERR_NOMEM;

    memcpy(*value, e->value, e->value_len);
    *value_len = e->value_len;

    return DKV_OK;
}

dkv_status_t storage_adapter_delete(storage_adapter_t *sa, const char *key, size_t key_len) {
    if (!sa || !key) return DKV_ERR_INVALID;

    int idx = find_entry(sa, key, key_len);
    if (idx < 0) return DKV_ERR_NOT_FOUND;

    storage_entry_t *e = &sa->entries[idx];
    sa->data_size -= e->key_len + e->value_len;
    free(e->key);
    free(e->value);
    e->key = NULL;
    e->value = NULL;
    e->key_len = 0;
    e->value_len = 0;
    e->deleted = true;

    return DKV_OK;
}

bool storage_adapter_exists(storage_adapter_t *sa, const char *key, size_t key_len) {
    if (!sa || !key) return false;
    return find_entry(sa, key, key_len) >= 0;
}

dkv_status_t storage_adapter_snapshot(storage_adapter_t *sa, const char *path) {
    if (!sa || !path) return DKV_ERR_INVALID;
    /* Simplified: just return OK for now */
    return DKV_OK;
}

dkv_status_t storage_adapter_restore(storage_adapter_t *sa, const char *path) {
    if (!sa || !path) return DKV_ERR_INVALID;
    /* Simplified: just return OK for now */
    return DKV_OK;
}

void storage_adapter_get_stats(storage_adapter_t *sa, storage_stats_t *stats) {
    if (!sa || !stats) return;

    int count = 0;
    for (int i = 0; i < sa->entry_count; i++) {
        if (!sa->entries[i].deleted) count++;
    }

    stats->num_keys = count;
    stats->data_size = sa->data_size;
    stats->memtable_size = sa->config.memtable_size;
}

void storage_adapter_free_value(void *value) {
    free(value);
}
