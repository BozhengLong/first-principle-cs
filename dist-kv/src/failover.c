/**
 * failover.c - Automatic failover implementation
 */

#include "failover.h"
#include <stdlib.h>
#include <string.h>

struct failover_manager {
    failover_config_t config;
    coordinator_t *coordinator;
    gossip_t *gossip;
};

void failover_config_init(failover_config_t *config) {
    if (!config) return;
    config->min_replicas = 1;
    config->rebalance_delay_ms = 5000;
}

failover_manager_t *failover_create(const failover_config_t *config,
                                     coordinator_t *coordinator,
                                     gossip_t *gossip) {
    failover_manager_t *fm = calloc(1, sizeof(failover_manager_t));
    if (!fm) return NULL;

    if (config) {
        fm->config = *config;
    } else {
        failover_config_init(&fm->config);
    }

    fm->coordinator = coordinator;
    fm->gossip = gossip;

    return fm;
}

void failover_destroy(failover_manager_t *fm) {
    free(fm);
}

dkv_status_t failover_handle_node_failure(failover_manager_t *fm, node_id_t node_id) {
    if (!fm || !fm->coordinator) return DKV_ERR_INVALID;

    /* Mark partitions with this node as needing attention */
    int num_partitions = coordinator_partition_count(fm->coordinator);

    for (int i = 0; i < num_partitions; i++) {
        partition_t *p = coordinator_get_partition_obj(fm->coordinator, i);
        if (!p) continue;

        if (partition_has_replica(p, node_id)) {
            /* If this was the leader, need to elect new leader */
            if (partition_get_leader(p) == node_id) {
                failover_elect_leader(fm, i);
            }

            /* Check if partition is still viable */
            int available = failover_get_available_replicas(fm, i);
            if (available < fm->config.min_replicas) {
                partition_set_state(p, PARTITION_STATE_READONLY);
            }
        }
    }

    return DKV_OK;
}

dkv_status_t failover_handle_node_recovery(failover_manager_t *fm, node_id_t node_id) {
    if (!fm || !fm->coordinator) return DKV_ERR_INVALID;

    /* Re-enable partitions that this node is part of */
    int num_partitions = coordinator_partition_count(fm->coordinator);

    for (int i = 0; i < num_partitions; i++) {
        partition_t *p = coordinator_get_partition_obj(fm->coordinator, i);
        if (!p) continue;

        if (partition_has_replica(p, node_id)) {
            /* Check if partition can be made active again */
            int available = failover_get_available_replicas(fm, i);
            if (available >= fm->config.min_replicas &&
                partition_get_state(p) == PARTITION_STATE_READONLY) {
                partition_set_state(p, PARTITION_STATE_ACTIVE);
            }

            /* If no leader, elect one */
            if (partition_get_leader(p) == 0) {
                failover_elect_leader(fm, i);
            }
        }
    }

    return DKV_OK;
}

bool failover_partition_writable(failover_manager_t *fm, partition_id_t partition_id) {
    if (!fm || !fm->coordinator) return false;

    partition_t *p = coordinator_get_partition_obj(fm->coordinator, partition_id);
    if (!p) return false;

    partition_state_t state = partition_get_state(p);
    if (state != PARTITION_STATE_ACTIVE) return false;

    /* Need a leader and quorum */
    if (partition_get_leader(p) == 0) return false;

    int available = failover_get_available_replicas(fm, partition_id);
    int total = partition_replica_count(p);

    /* Need majority for writes */
    return available > total / 2;
}

bool failover_partition_readable(failover_manager_t *fm, partition_id_t partition_id) {
    if (!fm || !fm->coordinator) return false;

    partition_t *p = coordinator_get_partition_obj(fm->coordinator, partition_id);
    if (!p) return false;

    partition_state_t state = partition_get_state(p);
    if (state == PARTITION_STATE_OFFLINE) return false;

    /* Can read from any available replica */
    return failover_get_available_replicas(fm, partition_id) > 0;
}

int failover_get_available_replicas(failover_manager_t *fm, partition_id_t partition_id) {
    if (!fm || !fm->coordinator) return 0;

    partition_t *p = coordinator_get_partition_obj(fm->coordinator, partition_id);
    if (!p) return 0;

    replica_info_t replicas[16];
    int count = partition_get_replicas(p, replicas, 16);

    int available = 0;
    for (int i = 0; i < count; i++) {
        /* Check if node is alive via gossip */
        if (fm->gossip) {
            member_state_t state = gossip_get_member_state(fm->gossip, replicas[i].node_id);
            if (state == MEMBER_STATE_ALIVE) {
                available++;
            }
        } else {
            /* No gossip, assume all are available */
            available++;
        }
    }

    return available;
}

dkv_status_t failover_reassign_partitions(failover_manager_t *fm) {
    if (!fm || !fm->coordinator) return DKV_ERR_INVALID;
    return coordinator_rebalance(fm->coordinator);
}

dkv_status_t failover_elect_leader(failover_manager_t *fm, partition_id_t partition_id) {
    if (!fm || !fm->coordinator) return DKV_ERR_INVALID;

    partition_t *p = coordinator_get_partition_obj(fm->coordinator, partition_id);
    if (!p) return DKV_ERR_INVALID;

    replica_info_t replicas[16];
    int count = partition_get_replicas(p, replicas, 16);

    /* Find first alive replica to be leader */
    for (int i = 0; i < count; i++) {
        member_state_t state = MEMBER_STATE_ALIVE;
        if (fm->gossip) {
            state = gossip_get_member_state(fm->gossip, replicas[i].node_id);
        }

        if (state == MEMBER_STATE_ALIVE) {
            return partition_set_leader(p, replicas[i].node_id);
        }
    }

    /* No alive replicas, clear leader */
    partition_set_leader(p, 0);
    return DKV_ERR_NO_QUORUM;
}
