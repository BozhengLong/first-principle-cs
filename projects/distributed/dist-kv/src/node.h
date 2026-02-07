/**
 * node.h - Node lifecycle management
 *
 * Manages the lifecycle of a single node in the distributed KV store.
 */

#ifndef DIST_KV_NODE_H
#define DIST_KV_NODE_H

#include "types.h"
#include "param.h"

/* Forward declaration */
typedef struct dkv_node dkv_node_t;

/* Node configuration */
typedef struct {
    node_id_t node_id;           /* Unique node identifier */
    node_addr_t addr;            /* Node address */
    char data_dir[256];          /* Data directory path */
    int num_virtual_nodes;       /* Virtual nodes for hash ring */
    int replication_factor;      /* Replication factor */
} dkv_node_config_t;

/* Initialize config with defaults */
void dkv_node_config_init(dkv_node_config_t *config);

/* Validate configuration */
dkv_status_t dkv_node_config_validate(const dkv_node_config_t *config);

/* Create a new node */
dkv_node_t *dkv_node_create(const dkv_node_config_t *config);

/* Destroy a node */
void dkv_node_destroy(dkv_node_t *node);

/* Start the node */
dkv_status_t dkv_node_start(dkv_node_t *node);

/* Stop the node */
dkv_status_t dkv_node_stop(dkv_node_t *node);

/* Get node state */
node_state_t dkv_node_get_state(dkv_node_t *node);

/* Get node ID */
node_id_t dkv_node_get_id(dkv_node_t *node);

/* Get node address */
const node_addr_t *dkv_node_get_addr(dkv_node_t *node);

#endif /* DIST_KV_NODE_H */
