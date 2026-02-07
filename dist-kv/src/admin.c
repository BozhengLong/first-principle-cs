/**
 * admin.c - Administrative operations implementation
 */

#include "admin.h"
#include <stdlib.h>
#include <string.h>

struct dkv_admin {
    coordinator_t *coordinator;
    gossip_t *gossip;
    failover_manager_t *failover;
};

dkv_admin_t *dkv_admin_create(coordinator_t *coordinator, gossip_t *gossip,
                               failover_manager_t *failover) {
    dkv_admin_t *admin = calloc(1, sizeof(dkv_admin_t));
    if (!admin) return NULL;

    admin->coordinator = coordinator;
    admin->gossip = gossip;
    admin->failover = failover;

    return admin;
}

void dkv_admin_destroy(dkv_admin_t *admin) {
    free(admin);
}

dkv_status_t dkv_admin_add_node(dkv_admin_t *admin, node_id_t node_id,
                                 const node_addr_t *addr) {
    if (!admin || !admin->coordinator) return DKV_ERR_INVALID;

    /* Add to coordinator */
    dkv_status_t status = coordinator_add_node(admin->coordinator, node_id);
    if (status != DKV_OK) return status;

    /* Add to gossip if available */
    if (admin->gossip) {
        gossip_add_member(admin->gossip, node_id, addr);
    }

    return DKV_OK;
}

dkv_status_t dkv_admin_remove_node(dkv_admin_t *admin, node_id_t node_id) {
    if (!admin || !admin->coordinator) return DKV_ERR_INVALID;

    /* Handle failover first */
    if (admin->failover) {
        failover_handle_node_failure(admin->failover, node_id);
    }

    /* Remove from gossip */
    if (admin->gossip) {
        gossip_remove_member(admin->gossip, node_id);
    }

    /* Remove from coordinator */
    return coordinator_remove_node(admin->coordinator, node_id);
}

dkv_status_t dkv_admin_rebalance(dkv_admin_t *admin) {
    if (!admin || !admin->coordinator) return DKV_ERR_INVALID;
    return coordinator_rebalance(admin->coordinator);
}

dkv_status_t dkv_admin_get_node_status(dkv_admin_t *admin, node_id_t node_id,
                                        node_status_t *status) {
    if (!admin || !status) return DKV_ERR_INVALID;

    memset(status, 0, sizeof(*status));
    status->node_id = node_id;

    /* Get state from gossip */
    if (admin->gossip) {
        status->state = gossip_get_member_state(admin->gossip, node_id);
    } else {
        status->state = MEMBER_STATE_ALIVE;
    }

    /* Count partitions */
    if (admin->coordinator) {
        partition_id_t partitions[1024];
        status->num_partitions = coordinator_get_node_partitions(
            admin->coordinator, node_id, partitions, 1024);

        /* Count leader partitions */
        for (int i = 0; i < status->num_partitions; i++) {
            if (coordinator_get_partition_leader(admin->coordinator, partitions[i]) == node_id) {
                status->num_leader_partitions++;
            }
        }
    }

    return DKV_OK;
}

int dkv_admin_get_all_node_status(dkv_admin_t *admin, node_status_t *statuses,
                                   int max_nodes) {
    if (!admin || !statuses || max_nodes <= 0) return 0;

    if (!admin->gossip) return 0;

    gossip_member_t members[64];
    int count = gossip_get_members(admin->gossip, members, 64);
    if (count > max_nodes) count = max_nodes;

    for (int i = 0; i < count; i++) {
        dkv_admin_get_node_status(admin, members[i].node_id, &statuses[i]);
        statuses[i].addr = members[i].addr;
    }

    return count;
}

int dkv_admin_node_count(dkv_admin_t *admin) {
    if (!admin || !admin->coordinator) return 0;
    return coordinator_node_count(admin->coordinator);
}

int dkv_admin_partition_count(dkv_admin_t *admin) {
    if (!admin || !admin->coordinator) return 0;
    return coordinator_partition_count(admin->coordinator);
}
