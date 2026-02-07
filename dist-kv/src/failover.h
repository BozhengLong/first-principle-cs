/**
 * failover.h - Automatic failover management
 *
 * Handles partition reassignment and leader election when nodes fail.
 */

#ifndef DIST_KV_FAILOVER_H
#define DIST_KV_FAILOVER_H

#include "types.h"
#include "coordinator.h"
#include "gossip.h"

/* Forward declaration */
typedef struct failover_manager failover_manager_t;

/* Failover configuration */
typedef struct {
    int min_replicas;            /* Minimum replicas before read-only */
    int rebalance_delay_ms;      /* Delay before rebalancing after failure */
} failover_config_t;

/* Initialize config with defaults */
void failover_config_init(failover_config_t *config);

/* Create a new failover manager */
failover_manager_t *failover_create(const failover_config_t *config,
                                     coordinator_t *coordinator,
                                     gossip_t *gossip);

/* Destroy a failover manager */
void failover_destroy(failover_manager_t *fm);

/* Handle node failure */
dkv_status_t failover_handle_node_failure(failover_manager_t *fm, node_id_t node_id);

/* Handle node recovery */
dkv_status_t failover_handle_node_recovery(failover_manager_t *fm, node_id_t node_id);

/* Check if a partition is available for writes */
bool failover_partition_writable(failover_manager_t *fm, partition_id_t partition_id);

/* Check if a partition is available for reads */
bool failover_partition_readable(failover_manager_t *fm, partition_id_t partition_id);

/* Get the number of available replicas for a partition */
int failover_get_available_replicas(failover_manager_t *fm, partition_id_t partition_id);

/* Trigger partition reassignment */
dkv_status_t failover_reassign_partitions(failover_manager_t *fm);

/* Trigger leader election for a partition */
dkv_status_t failover_elect_leader(failover_manager_t *fm, partition_id_t partition_id);

#endif /* DIST_KV_FAILOVER_H */
