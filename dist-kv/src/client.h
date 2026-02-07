/**
 * client.h - Client API for distributed key-value store
 *
 * Provides a simple interface for applications to interact with the cluster.
 */

#ifndef DIST_KV_CLIENT_H
#define DIST_KV_CLIENT_H

#include "types.h"
#include "coordinator.h"
#include "replication.h"
#include "failover.h"

/* Forward declaration */
typedef struct dkv_client dkv_client_t;

/* Client configuration */
typedef struct {
    int timeout_ms;              /* Request timeout */
    int max_retries;             /* Maximum retry attempts */
    int retry_delay_ms;          /* Delay between retries */
    consistency_level_t default_consistency;
} dkv_client_config_t;

/* Cluster information */
typedef struct {
    int num_nodes;
    int num_partitions;
    int replication_factor;
    node_id_t *node_ids;
    int node_count;
} cluster_info_t;

/* Initialize config with defaults */
void dkv_client_config_init(dkv_client_config_t *config);

/* Create a new client */
dkv_client_t *dkv_client_create(const dkv_client_config_t *config,
                                 coordinator_t *coordinator,
                                 failover_manager_t *failover);

/* Destroy a client */
void dkv_client_destroy(dkv_client_t *client);

/* Set the replication group for a partition (for testing) */
void dkv_client_set_replication(dkv_client_t *client, partition_id_t partition_id,
                                 replication_group_t *rg);

/* Put a key-value pair */
dkv_status_t dkv_client_put(dkv_client_t *client, const char *key, size_t key_len,
                             const void *value, size_t value_len);

/* Get a value by key */
dkv_status_t dkv_client_get(dkv_client_t *client, const char *key, size_t key_len,
                             void **value, size_t *value_len);

/* Get with specific consistency level */
dkv_status_t dkv_client_get_with_consistency(dkv_client_t *client,
                                              const char *key, size_t key_len,
                                              void **value, size_t *value_len,
                                              consistency_level_t consistency);

/* Delete a key */
dkv_status_t dkv_client_delete(dkv_client_t *client, const char *key, size_t key_len);

/* Batch put multiple key-value pairs */
dkv_status_t dkv_client_batch_put(dkv_client_t *client, const kv_pair_t *pairs, int count);

/* Get cluster information */
dkv_status_t dkv_client_get_cluster_info(dkv_client_t *client, cluster_info_t *info);

/* Free cluster info */
void dkv_client_free_cluster_info(cluster_info_t *info);

/* Free a value returned by dkv_client_get */
void dkv_client_free_value(void *value);

/* Get the leader for a key */
node_id_t dkv_client_get_leader_for_key(dkv_client_t *client, const char *key, size_t key_len);

#endif /* DIST_KV_CLIENT_H */
