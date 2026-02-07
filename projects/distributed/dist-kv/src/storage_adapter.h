/**
 * storage_adapter.h - Storage engine adapter
 *
 * Wraps the LSM-tree storage engine for use in the distributed KV store.
 * In a real implementation, this would integrate with the storage-engine project.
 */

#ifndef DIST_KV_STORAGE_ADAPTER_H
#define DIST_KV_STORAGE_ADAPTER_H

#include "types.h"

/* Forward declaration */
typedef struct storage_adapter storage_adapter_t;

/* Storage adapter configuration */
typedef struct {
    char data_dir[256];
    size_t memtable_size;
    size_t block_size;
} storage_adapter_config_t;

/* Initialize config with defaults */
void storage_adapter_config_init(storage_adapter_config_t *config);

/* Create a new storage adapter */
storage_adapter_t *storage_adapter_create(const storage_adapter_config_t *config);

/* Destroy a storage adapter */
void storage_adapter_destroy(storage_adapter_t *sa);

/* Put a key-value pair */
dkv_status_t storage_adapter_put(storage_adapter_t *sa, const char *key, size_t key_len,
                                  const void *value, size_t value_len);

/* Get a value by key */
dkv_status_t storage_adapter_get(storage_adapter_t *sa, const char *key, size_t key_len,
                                  void **value, size_t *value_len);

/* Delete a key */
dkv_status_t storage_adapter_delete(storage_adapter_t *sa, const char *key, size_t key_len);

/* Check if a key exists */
bool storage_adapter_exists(storage_adapter_t *sa, const char *key, size_t key_len);

/* Create a snapshot */
dkv_status_t storage_adapter_snapshot(storage_adapter_t *sa, const char *path);

/* Restore from a snapshot */
dkv_status_t storage_adapter_restore(storage_adapter_t *sa, const char *path);

/* Get storage statistics */
typedef struct {
    size_t num_keys;
    size_t data_size;
    size_t memtable_size;
} storage_stats_t;

void storage_adapter_get_stats(storage_adapter_t *sa, storage_stats_t *stats);

/* Free a value returned by storage_adapter_get */
void storage_adapter_free_value(void *value);

#endif /* DIST_KV_STORAGE_ADAPTER_H */
