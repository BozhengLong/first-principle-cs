/**
 * partition.h - Partition management
 *
 * Manages partitions (shards) of the key space.
 */

#ifndef DIST_KV_PARTITION_H
#define DIST_KV_PARTITION_H

#include "types.h"
#include "param.h"

/* Forward declaration */
typedef struct partition partition_t;

/* Partition configuration */
typedef struct {
    partition_id_t partition_id;
    int replication_factor;
} partition_config_t;

/* Replica information */
typedef struct {
    node_id_t node_id;
    bool is_leader;
    bool is_local;
} replica_info_t;

/* Create a new partition */
partition_t *partition_create(const partition_config_t *config);

/* Destroy a partition */
void partition_destroy(partition_t *p);

/* Get partition ID */
partition_id_t partition_get_id(partition_t *p);

/* Get partition state */
partition_state_t partition_get_state(partition_t *p);

/* Set partition state */
void partition_set_state(partition_t *p, partition_state_t state);

/* Add a replica to the partition */
dkv_status_t partition_add_replica(partition_t *p, node_id_t node_id, bool is_local);

/* Remove a replica from the partition */
dkv_status_t partition_remove_replica(partition_t *p, node_id_t node_id);

/* Get replica count */
int partition_replica_count(partition_t *p);

/* Get all replicas */
int partition_get_replicas(partition_t *p, replica_info_t *replicas, int max_replicas);

/* Set leader for the partition */
dkv_status_t partition_set_leader(partition_t *p, node_id_t leader_id);

/* Get current leader */
node_id_t partition_get_leader(partition_t *p);

/* Check if a node is a replica */
bool partition_has_replica(partition_t *p, node_id_t node_id);

#endif /* DIST_KV_PARTITION_H */
