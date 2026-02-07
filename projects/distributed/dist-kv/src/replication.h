/**
 * replication.h - Replication logic
 *
 * Combines Raft consensus with storage to provide replicated key-value operations.
 */

#ifndef DIST_KV_REPLICATION_H
#define DIST_KV_REPLICATION_H

#include "types.h"
#include "raft_group.h"
#include "storage_adapter.h"

/* Forward declaration */
typedef struct replication_group replication_group_t;

/* Replication group configuration */
typedef struct {
    partition_id_t partition_id;
    node_id_t node_id;
    char data_dir[256];
} replication_config_t;

/* Initialize config with defaults */
void replication_config_init(replication_config_t *config);

/* Create a new replication group */
replication_group_t *replication_create(const replication_config_t *config);

/* Destroy a replication group */
void replication_destroy(replication_group_t *rg);

/* Add a peer to the replication group */
dkv_status_t replication_add_peer(replication_group_t *rg, node_id_t peer_id);

/* Remove a peer from the replication group */
dkv_status_t replication_remove_peer(replication_group_t *rg, node_id_t peer_id);

/* Put a key-value pair (replicated) */
dkv_status_t replication_put(replication_group_t *rg, const char *key, size_t key_len,
                              const void *value, size_t value_len);

/* Get a value (linearizable read) */
dkv_status_t replication_get(replication_group_t *rg, const char *key, size_t key_len,
                              void **value, size_t *value_len,
                              consistency_level_t consistency);

/* Delete a key (replicated) */
dkv_status_t replication_delete(replication_group_t *rg, const char *key, size_t key_len);

/* Check if this node is the leader */
bool replication_is_leader(replication_group_t *rg);

/* Get the current leader */
node_id_t replication_get_leader(replication_group_t *rg);

/* Tick the replication state machine */
void replication_tick(replication_group_t *rg);

/* Trigger leader election */
void replication_trigger_election(replication_group_t *rg);

/* Get underlying storage adapter (for testing) */
storage_adapter_t *replication_get_storage(replication_group_t *rg);

/* Get underlying raft group (for testing) */
raft_group_t *replication_get_raft(replication_group_t *rg);

#endif /* DIST_KV_REPLICATION_H */
