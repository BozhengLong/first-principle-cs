/**
 * coordinator.c - Coordinator implementation
 */

#include "coordinator.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct coordinator {
    coordinator_config_t config;
    hash_ring_t *ring;
    partition_t **partitions;
    int partition_count;
};

void coordinator_config_init(coordinator_config_t *config) {
    if (!config) return;
    config->num_partitions = DKV_DEFAULT_PARTITIONS;
    config->replication_factor = DKV_DEFAULT_REPLICATION;
    config->num_virtual_nodes = DKV_DEFAULT_VIRTUAL_NODES;
}

coordinator_t *coordinator_create(const coordinator_config_t *config) {
    coordinator_t *c = calloc(1, sizeof(coordinator_t));
    if (!c) return NULL;

    if (config) {
        c->config = *config;
    } else {
        coordinator_config_init(&c->config);
    }

    /* Validate config */
    if (c->config.num_partitions < DKV_MIN_PARTITIONS) {
        c->config.num_partitions = DKV_MIN_PARTITIONS;
    }
    if (c->config.num_partitions > DKV_MAX_PARTITIONS) {
        c->config.num_partitions = DKV_MAX_PARTITIONS;
    }

    /* Create hash ring */
    hash_ring_config_t ring_config = {
        .num_virtual_nodes = c->config.num_virtual_nodes,
        .replication_factor = c->config.replication_factor
    };
    c->ring = hash_ring_create(&ring_config);
    if (!c->ring) {
        free(c);
        return NULL;
    }

    /* Create partitions */
    c->partition_count = c->config.num_partitions;
    c->partitions = calloc(c->partition_count, sizeof(partition_t *));
    if (!c->partitions) {
        hash_ring_destroy(c->ring);
        free(c);
        return NULL;
    }

    for (int i = 0; i < c->partition_count; i++) {
        partition_config_t pconfig = {
            .partition_id = i,
            .replication_factor = c->config.replication_factor
        };
        c->partitions[i] = partition_create(&pconfig);
        if (!c->partitions[i]) {
            for (int j = 0; j < i; j++) {
                partition_destroy(c->partitions[j]);
            }
            free(c->partitions);
            hash_ring_destroy(c->ring);
            free(c);
            return NULL;
        }
    }

    return c;
}

void coordinator_destroy(coordinator_t *c) {
    if (!c) return;

    for (int i = 0; i < c->partition_count; i++) {
        partition_destroy(c->partitions[i]);
    }
    free(c->partitions);
    hash_ring_destroy(c->ring);
    free(c);
}

dkv_status_t coordinator_add_node(coordinator_t *c, node_id_t node_id) {
    if (!c) return DKV_ERR_INVALID;
    return hash_ring_add_node(c->ring, node_id);
}

dkv_status_t coordinator_remove_node(coordinator_t *c, node_id_t node_id) {
    if (!c) return DKV_ERR_INVALID;

    /* Remove node from all partitions */
    for (int i = 0; i < c->partition_count; i++) {
        partition_remove_replica(c->partitions[i], node_id);
    }

    return hash_ring_remove_node(c->ring, node_id);
}

int coordinator_node_count(coordinator_t *c) {
    return c ? hash_ring_node_count(c->ring) : 0;
}

partition_id_t coordinator_get_partition(coordinator_t *c, const char *key, size_t key_len) {
    if (!c || !key) return 0;

    /* Hash key to partition */
    uint64_t hash = hash_ring_hash_key(key, key_len);
    return hash % c->partition_count;
}

partition_t *coordinator_get_partition_obj(coordinator_t *c, partition_id_t partition_id) {
    if (!c || partition_id >= (partition_id_t)c->partition_count) return NULL;
    return c->partitions[partition_id];
}

int coordinator_get_nodes_for_key(coordinator_t *c, const char *key, size_t key_len,
                                   node_id_t *nodes, int max_nodes) {
    if (!c || !key || !nodes || max_nodes <= 0) return 0;
    return hash_ring_get_replicas(c->ring, key, key_len, nodes, max_nodes);
}

node_id_t coordinator_get_partition_leader(coordinator_t *c, partition_id_t partition_id) {
    partition_t *p = coordinator_get_partition_obj(c, partition_id);
    return p ? partition_get_leader(p) : 0;
}

dkv_status_t coordinator_set_partition_leader(coordinator_t *c, partition_id_t partition_id,
                                               node_id_t leader_id) {
    partition_t *p = coordinator_get_partition_obj(c, partition_id);
    if (!p) return DKV_ERR_INVALID;
    return partition_set_leader(p, leader_id);
}

dkv_status_t coordinator_rebalance(coordinator_t *c) {
    if (!c) return DKV_ERR_INVALID;
    if (hash_ring_node_count(c->ring) == 0) return DKV_OK;

    /* Assign partitions to nodes based on hash ring */
    for (int i = 0; i < c->partition_count; i++) {
        partition_t *p = c->partitions[i];

        /* Clear existing replicas */
        while (partition_replica_count(p) > 0) {
            replica_info_t replicas[1];
            partition_get_replicas(p, replicas, 1);
            partition_remove_replica(p, replicas[0].node_id);
        }

        /* Get nodes for this partition using partition ID as key */
        char key[32];
        int len = snprintf(key, sizeof(key), "partition_%d", i);
        node_id_t nodes[DKV_MAX_REPLICATION];
        int count = hash_ring_get_replicas(c->ring, key, len, nodes, c->config.replication_factor);

        /* Add replicas */
        for (int j = 0; j < count; j++) {
            partition_add_replica(p, nodes[j], false);
        }

        /* Set first node as leader */
        if (count > 0) {
            partition_set_leader(p, nodes[0]);
        }

        partition_set_state(p, PARTITION_STATE_ACTIVE);
    }

    return DKV_OK;
}

int coordinator_partition_count(coordinator_t *c) {
    return c ? c->partition_count : 0;
}

int coordinator_get_node_partitions(coordinator_t *c, node_id_t node_id,
                                     partition_id_t *partitions, int max_partitions) {
    if (!c || !partitions || max_partitions <= 0) return 0;

    int count = 0;
    for (int i = 0; i < c->partition_count && count < max_partitions; i++) {
        if (partition_has_replica(c->partitions[i], node_id)) {
            partitions[count++] = i;
        }
    }

    return count;
}
