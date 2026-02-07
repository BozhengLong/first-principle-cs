/**
 * network_sim.h - Network simulator for testing
 *
 * Simulates network conditions for integration testing.
 */

#ifndef DIST_KV_NETWORK_SIM_H
#define DIST_KV_NETWORK_SIM_H

#include "../../src/types.h"
#include <stdbool.h>

/* Forward declaration */
typedef struct network_sim network_sim_t;

/* Network condition */
typedef enum {
    NET_CONDITION_NORMAL = 0,
    NET_CONDITION_PARTITION,     /* Network partition */
    NET_CONDITION_SLOW,          /* High latency */
    NET_CONDITION_LOSSY,         /* Packet loss */
} net_condition_t;

/* Create a new network simulator */
network_sim_t *network_sim_create(int num_nodes);

/* Destroy a network simulator */
void network_sim_destroy(network_sim_t *ns);

/* Set network condition between two nodes */
void network_sim_set_condition(network_sim_t *ns, node_id_t from, node_id_t to,
                                net_condition_t condition);

/* Set network condition for a node (affects all connections) */
void network_sim_set_node_condition(network_sim_t *ns, node_id_t node,
                                     net_condition_t condition);

/* Create a network partition (split nodes into two groups) */
void network_sim_partition(network_sim_t *ns, node_id_t *group1, int group1_size,
                           node_id_t *group2, int group2_size);

/* Heal a network partition */
void network_sim_heal_partition(network_sim_t *ns);

/* Check if two nodes can communicate */
bool network_sim_can_communicate(network_sim_t *ns, node_id_t from, node_id_t to);

/* Get latency between two nodes (ms) */
int network_sim_get_latency(network_sim_t *ns, node_id_t from, node_id_t to);

/* Get packet loss rate between two nodes (0-100) */
int network_sim_get_loss_rate(network_sim_t *ns, node_id_t from, node_id_t to);

/* Simulate sending a message (returns true if delivered) */
bool network_sim_send(network_sim_t *ns, node_id_t from, node_id_t to);

#endif /* DIST_KV_NETWORK_SIM_H */
