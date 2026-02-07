/**
 * coordinator.h - Request routing and partition coordination
 *
 * Routes requests to the appropriate partition and manages
 * cluster membership.
 */

#ifndef DIST_KV_COORDINATOR_H
#define DIST_KV_COORDINATOR_H

#include "types.h"
#include "param.h"
#include "hash_ring.h"
#include "partition.h"

/* Forward declaration */
typedef struct coordinator coordinator_t;

/* Coordinator configuration */
typedef struct {
    int num_partitions;
    int replication_factor;
    int num_virtual_nodes;
} coordinator_config_t;

/* Initialize config with defaults */
void coordinator_config_init(coordinator_config_t *config);

/* Create a new coordinator */
coordinator_t *coordinator_create(const coordinator_config_t *config);

/* Destroy a coordinator */
void coordinator_destroy(coordinator_t *c);

/* Add a node to the cluster */
dkv_status_t coordinator_add_node(coordinator_t *c, node_id_t node_id);

/* Remove a node from the cluster */
dkv_status_t coordinator_remove_node(coordinator_t *c, node_id_t node_id);

/* Get the number of nodes */
int coordinator_node_count(coordinator_t *c);

/* Get partition ID for a key */
partition_id_t coordinator_get_partition(coordinator_t *c, const char *key, size_t key_len);

/* Get the partition object */
partition_t *coordinator_get_partition_obj(coordinator_t *c, partition_id_t partition_id);

/* Get nodes responsible for a key */
int coordinator_get_nodes_for_key(coordinator_t *c, const char *key, size_t key_len,
                                   node_id_t *nodes, int max_nodes);

/* Get leader for a partition */
node_id_t coordinator_get_partition_leader(coordinator_t *c, partition_id_t partition_id);

/* Set leader for a partition */
dkv_status_t coordinator_set_partition_leader(coordinator_t *c, partition_id_t partition_id,
                                               node_id_t leader_id);

/* Rebalance partitions after node changes */
dkv_status_t coordinator_rebalance(coordinator_t *c);

/* Get number of partitions */
int coordinator_partition_count(coordinator_t *c);

/* Get partitions owned by a node */
int coordinator_get_node_partitions(coordinator_t *c, node_id_t node_id,
                                     partition_id_t *partitions, int max_partitions);

#endif /* DIST_KV_COORDINATOR_H */
