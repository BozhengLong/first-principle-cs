/**
 * hash_ring.h - Consistent hashing with virtual nodes
 *
 * Implements a hash ring for distributing keys across nodes with
 * minimal data movement when nodes are added or removed.
 */

#ifndef DIST_KV_HASH_RING_H
#define DIST_KV_HASH_RING_H

#include "types.h"
#include "param.h"

/* Forward declaration */
typedef struct hash_ring hash_ring_t;

/* Virtual node on the ring */
typedef struct {
    uint64_t hash;           /* Position on the ring */
    node_id_t node_id;       /* Physical node this belongs to */
    int vnode_index;         /* Virtual node index (0 to num_vnodes-1) */
} vnode_t;

/* Hash ring configuration */
typedef struct {
    int num_virtual_nodes;   /* Virtual nodes per physical node */
    int replication_factor;  /* Number of replicas for each key */
} hash_ring_config_t;

/* Create a new hash ring */
hash_ring_t *hash_ring_create(const hash_ring_config_t *config);

/* Destroy a hash ring */
void hash_ring_destroy(hash_ring_t *ring);

/* Add a node to the ring */
dkv_status_t hash_ring_add_node(hash_ring_t *ring, node_id_t node_id);

/* Remove a node from the ring */
dkv_status_t hash_ring_remove_node(hash_ring_t *ring, node_id_t node_id);

/* Check if a node exists in the ring */
bool hash_ring_has_node(hash_ring_t *ring, node_id_t node_id);

/* Get the number of physical nodes */
int hash_ring_node_count(hash_ring_t *ring);

/* Get the primary node for a key */
node_id_t hash_ring_get_node(hash_ring_t *ring, const char *key, size_t key_len);

/**
 * Get replica nodes for a key
 * Returns the number of nodes written to the nodes array
 * nodes array must have space for at least replication_factor entries
 */
int hash_ring_get_replicas(hash_ring_t *ring, const char *key, size_t key_len,
                           node_id_t *nodes, int max_nodes);

/* Hash a key to a 64-bit value (exposed for testing) */
uint64_t hash_ring_hash_key(const char *key, size_t key_len);

/* Get distribution statistics */
typedef struct {
    int total_vnodes;        /* Total virtual nodes on ring */
    int node_count;          /* Number of physical nodes */
    double avg_vnodes;       /* Average vnodes per node */
    double std_dev;          /* Standard deviation of distribution */
} hash_ring_stats_t;

void hash_ring_get_stats(hash_ring_t *ring, hash_ring_stats_t *stats);

#endif /* DIST_KV_HASH_RING_H */
