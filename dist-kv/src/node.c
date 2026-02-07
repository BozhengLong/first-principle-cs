/**
 * node.c - Node lifecycle implementation
 */

#include "node.h"
#include <stdlib.h>
#include <string.h>

struct dkv_node {
    dkv_node_config_t config;
    node_state_t state;
};

void dkv_node_config_init(dkv_node_config_t *config) {
    if (!config) return;
    memset(config, 0, sizeof(*config));
    config->num_virtual_nodes = DKV_DEFAULT_VIRTUAL_NODES;
    config->replication_factor = DKV_DEFAULT_REPLICATION;
    config->addr.port = 7000;
    strncpy(config->addr.host, "127.0.0.1", sizeof(config->addr.host) - 1);
}

dkv_status_t dkv_node_config_validate(const dkv_node_config_t *config) {
    if (!config) return DKV_ERR_INVALID;
    if (config->node_id == 0) return DKV_ERR_INVALID;
    if (config->addr.port == 0) return DKV_ERR_INVALID;
    if (config->data_dir[0] == '\0') return DKV_ERR_INVALID;
    if (config->num_virtual_nodes < 1) return DKV_ERR_INVALID;
    if (config->replication_factor < DKV_MIN_REPLICATION ||
        config->replication_factor > DKV_MAX_REPLICATION) {
        return DKV_ERR_INVALID;
    }
    return DKV_OK;
}

dkv_node_t *dkv_node_create(const dkv_node_config_t *config) {
    if (!config) return NULL;
    if (dkv_node_config_validate(config) != DKV_OK) return NULL;

    dkv_node_t *node = calloc(1, sizeof(dkv_node_t));
    if (!node) return NULL;

    node->config = *config;
    node->state = NODE_STATE_INIT;

    return node;
}

void dkv_node_destroy(dkv_node_t *node) {
    if (!node) return;
    if (node->state == NODE_STATE_RUNNING) {
        dkv_node_stop(node);
    }
    free(node);
}

dkv_status_t dkv_node_start(dkv_node_t *node) {
    if (!node) return DKV_ERR_INVALID;

    if (node->state != NODE_STATE_INIT &&
        node->state != NODE_STATE_STOPPED) {
        return DKV_ERR_INVALID;
    }

    node->state = NODE_STATE_STARTING;
    /* In a real implementation, this would:
     * - Initialize storage engine
     * - Start RPC server
     * - Join cluster
     */
    node->state = NODE_STATE_RUNNING;

    return DKV_OK;
}

dkv_status_t dkv_node_stop(dkv_node_t *node) {
    if (!node) return DKV_ERR_INVALID;

    if (node->state != NODE_STATE_RUNNING) {
        return DKV_ERR_INVALID;
    }

    node->state = NODE_STATE_STOPPING;
    /* In a real implementation, this would:
     * - Stop accepting new requests
     * - Drain pending requests
     * - Leave cluster gracefully
     * - Shutdown storage engine
     */
    node->state = NODE_STATE_STOPPED;

    return DKV_OK;
}

node_state_t dkv_node_get_state(dkv_node_t *node) {
    return node ? node->state : NODE_STATE_INIT;
}

node_id_t dkv_node_get_id(dkv_node_t *node) {
    return node ? node->config.node_id : 0;
}

const node_addr_t *dkv_node_get_addr(dkv_node_t *node) {
    return node ? &node->config.addr : NULL;
}
