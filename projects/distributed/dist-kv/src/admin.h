/**
 * admin.h - Administrative operations
 *
 * Provides cluster management operations.
 */

#ifndef DIST_KV_ADMIN_H
#define DIST_KV_ADMIN_H

#include "types.h"
#include "coordinator.h"
#include "gossip.h"
#include "failover.h"

/* Forward declaration */
typedef struct dkv_admin dkv_admin_t;

/* Node status */
typedef struct {
    node_id_t node_id;
    node_addr_t addr;
    member_state_t state;
    int num_partitions;
    int num_leader_partitions;
} node_status_t;

/* Create admin interface */
dkv_admin_t *dkv_admin_create(coordinator_t *coordinator, gossip_t *gossip,
                               failover_manager_t *failover);

/* Destroy admin interface */
void dkv_admin_destroy(dkv_admin_t *admin);

/* Add a node to the cluster */
dkv_status_t dkv_admin_add_node(dkv_admin_t *admin, node_id_t node_id,
                                 const node_addr_t *addr);

/* Remove a node from the cluster */
dkv_status_t dkv_admin_remove_node(dkv_admin_t *admin, node_id_t node_id);

/* Trigger cluster rebalance */
dkv_status_t dkv_admin_rebalance(dkv_admin_t *admin);

/* Get node status */
dkv_status_t dkv_admin_get_node_status(dkv_admin_t *admin, node_id_t node_id,
                                        node_status_t *status);

/* Get all node statuses */
int dkv_admin_get_all_node_status(dkv_admin_t *admin, node_status_t *statuses,
                                   int max_nodes);

/* Get number of nodes */
int dkv_admin_node_count(dkv_admin_t *admin);

/* Get number of partitions */
int dkv_admin_partition_count(dkv_admin_t *admin);

#endif /* DIST_KV_ADMIN_H */
