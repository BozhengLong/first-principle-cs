/**
 * client.c - Client API implementation
 */

#include "client.h"
#include "param.h"
#include <stdlib.h>
#include <string.h>

#define MAX_PARTITIONS 1024

struct dkv_client {
    dkv_client_config_t config;
    coordinator_t *coordinator;
    failover_manager_t *failover;
    replication_group_t *replication_groups[MAX_PARTITIONS];
};

void dkv_client_config_init(dkv_client_config_t *config) {
    if (!config) return;
    config->timeout_ms = DKV_DEFAULT_RPC_TIMEOUT;
    config->max_retries = DKV_CLIENT_MAX_RETRIES;
    config->retry_delay_ms = DKV_CLIENT_RETRY_DELAY;
    config->default_consistency = CONSISTENCY_LINEARIZABLE;
}

dkv_client_t *dkv_client_create(const dkv_client_config_t *config,
                                 coordinator_t *coordinator,
                                 failover_manager_t *failover) {
    dkv_client_t *client = calloc(1, sizeof(dkv_client_t));
    if (!client) return NULL;

    if (config) {
        client->config = *config;
    } else {
        dkv_client_config_init(&client->config);
    }

    client->coordinator = coordinator;
    client->failover = failover;

    return client;
}

void dkv_client_destroy(dkv_client_t *client) {
    free(client);
}

void dkv_client_set_replication(dkv_client_t *client, partition_id_t partition_id,
                                 replication_group_t *rg) {
    if (!client || partition_id >= MAX_PARTITIONS) return;
    client->replication_groups[partition_id] = rg;
}

static replication_group_t *get_replication_group(dkv_client_t *client,
                                                   const char *key, size_t key_len) {
    if (!client || !client->coordinator) return NULL;

    partition_id_t pid = coordinator_get_partition(client->coordinator, key, key_len);
    return client->replication_groups[pid];
}

dkv_status_t dkv_client_put(dkv_client_t *client, const char *key, size_t key_len,
                             const void *value, size_t value_len) {
    if (!client || !key || !value) return DKV_ERR_INVALID;

    /* Check if partition is writable */
    if (client->failover) {
        partition_id_t pid = coordinator_get_partition(client->coordinator, key, key_len);
        if (!failover_partition_writable(client->failover, pid)) {
            return DKV_ERR_READONLY;
        }
    }

    replication_group_t *rg = get_replication_group(client, key, key_len);
    if (!rg) return DKV_ERR_INVALID;

    /* Retry logic */
    dkv_status_t status = DKV_ERR_INTERNAL;
    for (int retry = 0; retry <= client->config.max_retries; retry++) {
        status = replication_put(rg, key, key_len, value, value_len);
        if (status == DKV_OK) break;
        if (status != DKV_ERR_TIMEOUT && status != DKV_ERR_NOT_LEADER) break;
    }

    return status;
}

dkv_status_t dkv_client_get(dkv_client_t *client, const char *key, size_t key_len,
                             void **value, size_t *value_len) {
    return dkv_client_get_with_consistency(client, key, key_len, value, value_len,
                                            client->config.default_consistency);
}

dkv_status_t dkv_client_get_with_consistency(dkv_client_t *client,
                                              const char *key, size_t key_len,
                                              void **value, size_t *value_len,
                                              consistency_level_t consistency) {
    if (!client || !key || !value || !value_len) return DKV_ERR_INVALID;

    /* Check if partition is readable */
    if (client->failover) {
        partition_id_t pid = coordinator_get_partition(client->coordinator, key, key_len);
        if (!failover_partition_readable(client->failover, pid)) {
            return DKV_ERR_PARTITION;
        }
    }

    replication_group_t *rg = get_replication_group(client, key, key_len);
    if (!rg) return DKV_ERR_INVALID;

    /* Retry logic */
    dkv_status_t status = DKV_ERR_INTERNAL;
    for (int retry = 0; retry <= client->config.max_retries; retry++) {
        status = replication_get(rg, key, key_len, value, value_len, consistency);
        if (status == DKV_OK || status == DKV_ERR_NOT_FOUND) break;
        if (status != DKV_ERR_TIMEOUT && status != DKV_ERR_NOT_LEADER) break;
    }

    return status;
}

dkv_status_t dkv_client_delete(dkv_client_t *client, const char *key, size_t key_len) {
    if (!client || !key) return DKV_ERR_INVALID;

    /* Check if partition is writable */
    if (client->failover) {
        partition_id_t pid = coordinator_get_partition(client->coordinator, key, key_len);
        if (!failover_partition_writable(client->failover, pid)) {
            return DKV_ERR_READONLY;
        }
    }

    replication_group_t *rg = get_replication_group(client, key, key_len);
    if (!rg) return DKV_ERR_INVALID;

    return replication_delete(rg, key, key_len);
}

dkv_status_t dkv_client_batch_put(dkv_client_t *client, const kv_pair_t *pairs, int count) {
    if (!client || !pairs || count <= 0) return DKV_ERR_INVALID;

    for (int i = 0; i < count; i++) {
        dkv_status_t status = dkv_client_put(client, pairs[i].key, pairs[i].key_len,
                                              pairs[i].value, pairs[i].value_len);
        if (status != DKV_OK) return status;
    }

    return DKV_OK;
}

dkv_status_t dkv_client_get_cluster_info(dkv_client_t *client, cluster_info_t *info) {
    if (!client || !info || !client->coordinator) return DKV_ERR_INVALID;

    memset(info, 0, sizeof(*info));
    info->num_nodes = coordinator_node_count(client->coordinator);
    info->num_partitions = coordinator_partition_count(client->coordinator);
    info->replication_factor = 3;  /* TODO: get from coordinator */

    return DKV_OK;
}

void dkv_client_free_cluster_info(cluster_info_t *info) {
    if (!info) return;
    free(info->node_ids);
    info->node_ids = NULL;
    info->node_count = 0;
}

void dkv_client_free_value(void *value) {
    free(value);
}

node_id_t dkv_client_get_leader_for_key(dkv_client_t *client, const char *key, size_t key_len) {
    if (!client || !client->coordinator) return 0;

    partition_id_t pid = coordinator_get_partition(client->coordinator, key, key_len);
    return coordinator_get_partition_leader(client->coordinator, pid);
}
