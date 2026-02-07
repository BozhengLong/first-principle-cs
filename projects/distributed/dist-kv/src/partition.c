/**
 * partition.c - Partition management implementation
 */

#include "partition.h"
#include <stdlib.h>
#include <string.h>

#define MAX_REPLICAS 16

struct partition {
    partition_config_t config;
    partition_state_t state;
    replica_info_t replicas[MAX_REPLICAS];
    int replica_count;
    node_id_t leader_id;
};

partition_t *partition_create(const partition_config_t *config) {
    if (!config) return NULL;

    partition_t *p = calloc(1, sizeof(partition_t));
    if (!p) return NULL;

    p->config = *config;
    p->state = PARTITION_STATE_INIT;
    p->leader_id = 0;

    return p;
}

void partition_destroy(partition_t *p) {
    free(p);
}

partition_id_t partition_get_id(partition_t *p) {
    return p ? p->config.partition_id : 0;
}

partition_state_t partition_get_state(partition_t *p) {
    return p ? p->state : PARTITION_STATE_INIT;
}

void partition_set_state(partition_t *p, partition_state_t state) {
    if (p) p->state = state;
}

dkv_status_t partition_add_replica(partition_t *p, node_id_t node_id, bool is_local) {
    if (!p) return DKV_ERR_INVALID;
    if (p->replica_count >= MAX_REPLICAS) return DKV_ERR_INVALID;

    /* Check for duplicate */
    for (int i = 0; i < p->replica_count; i++) {
        if (p->replicas[i].node_id == node_id) {
            return DKV_ERR_EXISTS;
        }
    }

    replica_info_t *r = &p->replicas[p->replica_count++];
    r->node_id = node_id;
    r->is_leader = false;
    r->is_local = is_local;

    return DKV_OK;
}

dkv_status_t partition_remove_replica(partition_t *p, node_id_t node_id) {
    if (!p) return DKV_ERR_INVALID;

    for (int i = 0; i < p->replica_count; i++) {
        if (p->replicas[i].node_id == node_id) {
            /* Shift remaining replicas */
            memmove(&p->replicas[i], &p->replicas[i + 1],
                    (p->replica_count - i - 1) * sizeof(replica_info_t));
            p->replica_count--;

            /* Clear leader if removed */
            if (p->leader_id == node_id) {
                p->leader_id = 0;
            }
            return DKV_OK;
        }
    }

    return DKV_ERR_NOT_FOUND;
}

int partition_replica_count(partition_t *p) {
    return p ? p->replica_count : 0;
}

int partition_get_replicas(partition_t *p, replica_info_t *replicas, int max_replicas) {
    if (!p || !replicas || max_replicas <= 0) return 0;

    int count = p->replica_count;
    if (count > max_replicas) count = max_replicas;

    memcpy(replicas, p->replicas, count * sizeof(replica_info_t));
    return count;
}

dkv_status_t partition_set_leader(partition_t *p, node_id_t leader_id) {
    if (!p) return DKV_ERR_INVALID;

    /* Verify leader is a replica */
    bool found = false;
    for (int i = 0; i < p->replica_count; i++) {
        if (p->replicas[i].node_id == leader_id) {
            p->replicas[i].is_leader = true;
            found = true;
        } else {
            p->replicas[i].is_leader = false;
        }
    }

    if (!found && leader_id != 0) {
        return DKV_ERR_NOT_FOUND;
    }

    p->leader_id = leader_id;
    return DKV_OK;
}

node_id_t partition_get_leader(partition_t *p) {
    return p ? p->leader_id : 0;
}

bool partition_has_replica(partition_t *p, node_id_t node_id) {
    if (!p) return false;

    for (int i = 0; i < p->replica_count; i++) {
        if (p->replicas[i].node_id == node_id) {
            return true;
        }
    }
    return false;
}
