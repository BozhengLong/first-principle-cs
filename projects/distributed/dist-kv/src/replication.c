/**
 * replication.c - Replication logic implementation
 */

#include "replication.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Operation types for log entries */
typedef enum {
    OP_PUT = 1,
    OP_DELETE,
} op_type_t;

/* Log entry format */
typedef struct {
    op_type_t op;
    uint32_t key_len;
    uint32_t value_len;
    /* Followed by key and value data */
} log_entry_header_t;

struct replication_group {
    replication_config_t config;
    raft_group_t *raft;
    storage_adapter_t *storage;
};

/* Apply callback for Raft */
static dkv_status_t apply_entry(void *ctx, const raft_log_entry_t *entry) {
    replication_group_t *rg = (replication_group_t *)ctx;
    if (!rg || !entry || !entry->data) return DKV_ERR_INVALID;

    log_entry_header_t *header = (log_entry_header_t *)entry->data;
    const char *key = (const char *)(entry->data + sizeof(log_entry_header_t));
    const void *value = key + header->key_len;

    switch (header->op) {
        case OP_PUT:
            return storage_adapter_put(rg->storage, key, header->key_len,
                                       value, header->value_len);
        case OP_DELETE:
            return storage_adapter_delete(rg->storage, key, header->key_len);
        default:
            return DKV_ERR_INVALID;
    }
}

void replication_config_init(replication_config_t *config) {
    if (!config) return;
    memset(config, 0, sizeof(*config));
    strncpy(config->data_dir, "/tmp/dkv", sizeof(config->data_dir) - 1);
}

replication_group_t *replication_create(const replication_config_t *config) {
    replication_group_t *rg = calloc(1, sizeof(replication_group_t));
    if (!rg) return NULL;

    if (config) {
        rg->config = *config;
    } else {
        replication_config_init(&rg->config);
    }

    /* Create Raft group */
    raft_group_config_t raft_config;
    raft_group_config_init(&raft_config);
    raft_config.partition_id = rg->config.partition_id;
    raft_config.node_id = rg->config.node_id;

    rg->raft = raft_group_create(&raft_config);
    if (!rg->raft) {
        free(rg);
        return NULL;
    }

    /* Create storage adapter */
    storage_adapter_config_t storage_config;
    storage_adapter_config_init(&storage_config);
    snprintf(storage_config.data_dir, sizeof(storage_config.data_dir),
             "%s/partition_%u", rg->config.data_dir, rg->config.partition_id);

    rg->storage = storage_adapter_create(&storage_config);
    if (!rg->storage) {
        raft_group_destroy(rg->raft);
        free(rg);
        return NULL;
    }

    /* Set apply callback */
    raft_group_set_apply_callback(rg->raft, apply_entry, rg);

    return rg;
}

void replication_destroy(replication_group_t *rg) {
    if (!rg) return;
    raft_group_destroy(rg->raft);
    storage_adapter_destroy(rg->storage);
    free(rg);
}

dkv_status_t replication_add_peer(replication_group_t *rg, node_id_t peer_id) {
    if (!rg) return DKV_ERR_INVALID;
    return raft_group_add_peer(rg->raft, peer_id);
}

dkv_status_t replication_remove_peer(replication_group_t *rg, node_id_t peer_id) {
    if (!rg) return DKV_ERR_INVALID;
    return raft_group_remove_peer(rg->raft, peer_id);
}

dkv_status_t replication_put(replication_group_t *rg, const char *key, size_t key_len,
                              const void *value, size_t value_len) {
    if (!rg || !key || !value) return DKV_ERR_INVALID;
    if (!raft_group_is_leader(rg->raft)) return DKV_ERR_NOT_LEADER;

    /* Create log entry */
    size_t entry_size = sizeof(log_entry_header_t) + key_len + value_len;
    uint8_t *entry_data = malloc(entry_size);
    if (!entry_data) return DKV_ERR_NOMEM;

    log_entry_header_t *header = (log_entry_header_t *)entry_data;
    header->op = OP_PUT;
    header->key_len = key_len;
    header->value_len = value_len;
    memcpy(entry_data + sizeof(log_entry_header_t), key, key_len);
    memcpy(entry_data + sizeof(log_entry_header_t) + key_len, value, value_len);

    /* Propose to Raft */
    uint64_t index;
    dkv_status_t status = raft_group_propose(rg->raft, entry_data, entry_size, &index);
    free(entry_data);

    return status;
}

dkv_status_t replication_get(replication_group_t *rg, const char *key, size_t key_len,
                              void **value, size_t *value_len,
                              consistency_level_t consistency) {
    if (!rg || !key || !value || !value_len) return DKV_ERR_INVALID;

    if (consistency == CONSISTENCY_LINEARIZABLE) {
        /* For linearizable reads, must be leader or forward to leader */
        if (!raft_group_is_leader(rg->raft)) {
            return DKV_ERR_NOT_LEADER;
        }
        /* In a real implementation, we would do a read index check here */
    }

    /* Read from local storage */
    return storage_adapter_get(rg->storage, key, key_len, value, value_len);
}

dkv_status_t replication_delete(replication_group_t *rg, const char *key, size_t key_len) {
    if (!rg || !key) return DKV_ERR_INVALID;
    if (!raft_group_is_leader(rg->raft)) return DKV_ERR_NOT_LEADER;

    /* Create log entry */
    size_t entry_size = sizeof(log_entry_header_t) + key_len;
    uint8_t *entry_data = malloc(entry_size);
    if (!entry_data) return DKV_ERR_NOMEM;

    log_entry_header_t *header = (log_entry_header_t *)entry_data;
    header->op = OP_DELETE;
    header->key_len = key_len;
    header->value_len = 0;
    memcpy(entry_data + sizeof(log_entry_header_t), key, key_len);

    /* Propose to Raft */
    uint64_t index;
    dkv_status_t status = raft_group_propose(rg->raft, entry_data, entry_size, &index);
    free(entry_data);

    return status;
}

bool replication_is_leader(replication_group_t *rg) {
    return rg && raft_group_is_leader(rg->raft);
}

node_id_t replication_get_leader(replication_group_t *rg) {
    return rg ? raft_group_get_leader(rg->raft) : 0;
}

void replication_tick(replication_group_t *rg) {
    if (!rg) return;
    raft_group_tick(rg->raft);
}

void replication_trigger_election(replication_group_t *rg) {
    if (!rg) return;
    raft_group_trigger_election(rg->raft);
}

storage_adapter_t *replication_get_storage(replication_group_t *rg) {
    return rg ? rg->storage : NULL;
}

raft_group_t *replication_get_raft(replication_group_t *rg) {
    return rg ? rg->raft : NULL;
}
