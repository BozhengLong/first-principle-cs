/**
 * network_sim.c - Network simulator implementation
 */

#include "network_sim.h"
#include <stdlib.h>
#include <string.h>

#define MAX_NODES 32

typedef struct {
    net_condition_t condition;
    int latency_ms;
    int loss_rate;
} link_state_t;

struct network_sim {
    int num_nodes;
    link_state_t links[MAX_NODES][MAX_NODES];
    bool partitioned;
    node_id_t partition_group1[MAX_NODES];
    int group1_size;
    node_id_t partition_group2[MAX_NODES];
    int group2_size;
};

network_sim_t *network_sim_create(int num_nodes) {
    if (num_nodes > MAX_NODES) return NULL;

    network_sim_t *ns = calloc(1, sizeof(network_sim_t));
    if (!ns) return NULL;

    ns->num_nodes = num_nodes;

    /* Initialize all links as normal */
    for (int i = 0; i < num_nodes; i++) {
        for (int j = 0; j < num_nodes; j++) {
            ns->links[i][j].condition = NET_CONDITION_NORMAL;
            ns->links[i][j].latency_ms = 1;
            ns->links[i][j].loss_rate = 0;
        }
    }

    return ns;
}

void network_sim_destroy(network_sim_t *ns) {
    free(ns);
}

static int node_to_index(node_id_t node) {
    /* Assume node IDs are 1-based */
    return (int)(node - 1);
}

void network_sim_set_condition(network_sim_t *ns, node_id_t from, node_id_t to,
                                net_condition_t condition) {
    if (!ns) return;

    int fi = node_to_index(from);
    int ti = node_to_index(to);

    if (fi < 0 || fi >= ns->num_nodes || ti < 0 || ti >= ns->num_nodes) return;

    ns->links[fi][ti].condition = condition;

    switch (condition) {
        case NET_CONDITION_NORMAL:
            ns->links[fi][ti].latency_ms = 1;
            ns->links[fi][ti].loss_rate = 0;
            break;
        case NET_CONDITION_PARTITION:
            ns->links[fi][ti].latency_ms = 0;
            ns->links[fi][ti].loss_rate = 100;
            break;
        case NET_CONDITION_SLOW:
            ns->links[fi][ti].latency_ms = 500;
            ns->links[fi][ti].loss_rate = 0;
            break;
        case NET_CONDITION_LOSSY:
            ns->links[fi][ti].latency_ms = 10;
            ns->links[fi][ti].loss_rate = 30;
            break;
    }
}

void network_sim_set_node_condition(network_sim_t *ns, node_id_t node,
                                     net_condition_t condition) {
    if (!ns) return;

    for (int i = 1; i <= ns->num_nodes; i++) {
        if ((node_id_t)i != node) {
            network_sim_set_condition(ns, node, i, condition);
            network_sim_set_condition(ns, i, node, condition);
        }
    }
}

static bool in_group(node_id_t node, node_id_t *group, int size) {
    for (int i = 0; i < size; i++) {
        if (group[i] == node) return true;
    }
    return false;
}

void network_sim_partition(network_sim_t *ns, node_id_t *group1, int group1_size,
                           node_id_t *group2, int group2_size) {
    if (!ns) return;

    ns->partitioned = true;
    ns->group1_size = group1_size;
    ns->group2_size = group2_size;
    memcpy(ns->partition_group1, group1, group1_size * sizeof(node_id_t));
    memcpy(ns->partition_group2, group2, group2_size * sizeof(node_id_t));

    /* Set partition condition between groups */
    for (int i = 0; i < group1_size; i++) {
        for (int j = 0; j < group2_size; j++) {
            network_sim_set_condition(ns, group1[i], group2[j], NET_CONDITION_PARTITION);
            network_sim_set_condition(ns, group2[j], group1[i], NET_CONDITION_PARTITION);
        }
    }
}

void network_sim_heal_partition(network_sim_t *ns) {
    if (!ns || !ns->partitioned) return;

    /* Restore normal condition between groups */
    for (int i = 0; i < ns->group1_size; i++) {
        for (int j = 0; j < ns->group2_size; j++) {
            network_sim_set_condition(ns, ns->partition_group1[i],
                                      ns->partition_group2[j], NET_CONDITION_NORMAL);
            network_sim_set_condition(ns, ns->partition_group2[j],
                                      ns->partition_group1[i], NET_CONDITION_NORMAL);
        }
    }

    ns->partitioned = false;
}

bool network_sim_can_communicate(network_sim_t *ns, node_id_t from, node_id_t to) {
    if (!ns) return false;

    int fi = node_to_index(from);
    int ti = node_to_index(to);

    if (fi < 0 || fi >= ns->num_nodes || ti < 0 || ti >= ns->num_nodes) return false;

    return ns->links[fi][ti].condition != NET_CONDITION_PARTITION;
}

int network_sim_get_latency(network_sim_t *ns, node_id_t from, node_id_t to) {
    if (!ns) return 0;

    int fi = node_to_index(from);
    int ti = node_to_index(to);

    if (fi < 0 || fi >= ns->num_nodes || ti < 0 || ti >= ns->num_nodes) return 0;

    return ns->links[fi][ti].latency_ms;
}

int network_sim_get_loss_rate(network_sim_t *ns, node_id_t from, node_id_t to) {
    if (!ns) return 100;

    int fi = node_to_index(from);
    int ti = node_to_index(to);

    if (fi < 0 || fi >= ns->num_nodes || ti < 0 || ti >= ns->num_nodes) return 100;

    return ns->links[fi][ti].loss_rate;
}

bool network_sim_send(network_sim_t *ns, node_id_t from, node_id_t to) {
    if (!ns) return false;

    int loss_rate = network_sim_get_loss_rate(ns, from, to);
    if (loss_rate >= 100) return false;
    if (loss_rate <= 0) return true;

    /* Simulate packet loss */
    int r = rand() % 100;
    return r >= loss_rate;
}
