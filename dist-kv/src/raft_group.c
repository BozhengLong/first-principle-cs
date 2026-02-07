/**
 * raft_group.c - Raft consensus group implementation
 *
 * Simplified Raft implementation for the distributed KV store.
 * In production, this would integrate with the full consensus project.
 */

#include "raft_group.h"
#include <stdlib.h>
#include <string.h>

#define MAX_PEERS 16
#define MAX_LOG_ENTRIES 1024

struct raft_group {
    raft_group_config_t config;
    raft_role_t role;
    uint64_t current_term;
    node_id_t voted_for;
    node_id_t leader_id;

    /* Peers */
    node_id_t peers[MAX_PEERS];
    int peer_count;

    /* Log */
    raft_log_entry_t *log;
    int log_capacity;
    int log_count;
    uint64_t commit_index;
    uint64_t last_applied;

    /* Apply callback */
    raft_apply_fn apply_fn;
    void *apply_ctx;

    /* Timing */
    int ticks_since_heartbeat;
};

void raft_group_config_init(raft_group_config_t *config) {
    if (!config) return;
    memset(config, 0, sizeof(*config));
    config->election_timeout_ms = DKV_DEFAULT_ELECTION_TIMEOUT;
    config->heartbeat_interval_ms = DKV_DEFAULT_HEARTBEAT_INTERVAL;
}

raft_group_t *raft_group_create(const raft_group_config_t *config) {
    raft_group_t *rg = calloc(1, sizeof(raft_group_t));
    if (!rg) return NULL;

    if (config) {
        rg->config = *config;
    } else {
        raft_group_config_init(&rg->config);
    }

    rg->role = RAFT_ROLE_FOLLOWER;
    rg->current_term = 0;
    rg->voted_for = 0;
    rg->leader_id = 0;

    rg->log_capacity = 64;
    rg->log = calloc(rg->log_capacity, sizeof(raft_log_entry_t));
    if (!rg->log) {
        free(rg);
        return NULL;
    }

    return rg;
}

void raft_group_destroy(raft_group_t *rg) {
    if (!rg) return;

    /* Free log entries */
    for (int i = 0; i < rg->log_count; i++) {
        free(rg->log[i].data);
    }
    free(rg->log);
    free(rg);
}

void raft_group_set_apply_callback(raft_group_t *rg, raft_apply_fn fn, void *ctx) {
    if (!rg) return;
    rg->apply_fn = fn;
    rg->apply_ctx = ctx;
}

dkv_status_t raft_group_add_peer(raft_group_t *rg, node_id_t peer_id) {
    if (!rg) return DKV_ERR_INVALID;
    if (rg->peer_count >= MAX_PEERS) return DKV_ERR_INVALID;

    /* Check for duplicate */
    for (int i = 0; i < rg->peer_count; i++) {
        if (rg->peers[i] == peer_id) return DKV_ERR_EXISTS;
    }

    rg->peers[rg->peer_count++] = peer_id;
    return DKV_OK;
}

dkv_status_t raft_group_remove_peer(raft_group_t *rg, node_id_t peer_id) {
    if (!rg) return DKV_ERR_INVALID;

    for (int i = 0; i < rg->peer_count; i++) {
        if (rg->peers[i] == peer_id) {
            memmove(&rg->peers[i], &rg->peers[i + 1],
                    (rg->peer_count - i - 1) * sizeof(node_id_t));
            rg->peer_count--;
            return DKV_OK;
        }
    }

    return DKV_ERR_NOT_FOUND;
}

raft_role_t raft_group_get_role(raft_group_t *rg) {
    return rg ? rg->role : RAFT_ROLE_FOLLOWER;
}

node_id_t raft_group_get_leader(raft_group_t *rg) {
    return rg ? rg->leader_id : 0;
}

uint64_t raft_group_get_term(raft_group_t *rg) {
    return rg ? rg->current_term : 0;
}

uint64_t raft_group_get_commit_index(raft_group_t *rg) {
    return rg ? rg->commit_index : 0;
}

bool raft_group_is_leader(raft_group_t *rg) {
    return rg && rg->role == RAFT_ROLE_LEADER;
}

dkv_status_t raft_group_propose(raft_group_t *rg, const void *data, size_t len,
                                 uint64_t *index) {
    if (!rg || !data) return DKV_ERR_INVALID;
    if (rg->role != RAFT_ROLE_LEADER) return DKV_ERR_NOT_LEADER;

    /* Expand log if needed */
    if (rg->log_count >= rg->log_capacity) {
        int new_cap = rg->log_capacity * 2;
        raft_log_entry_t *new_log = realloc(rg->log, new_cap * sizeof(raft_log_entry_t));
        if (!new_log) return DKV_ERR_NOMEM;
        rg->log = new_log;
        rg->log_capacity = new_cap;
    }

    /* Create new entry */
    raft_log_entry_t *entry = &rg->log[rg->log_count];
    entry->index = rg->log_count + 1;
    entry->term = rg->current_term;
    entry->data = malloc(len);
    if (!entry->data) return DKV_ERR_NOMEM;
    memcpy(entry->data, data, len);
    entry->data_len = len;

    rg->log_count++;

    if (index) *index = entry->index;

    /* In single-node mode, commit immediately */
    if (rg->peer_count == 0) {
        rg->commit_index = entry->index;

        /* Apply committed entries */
        while (rg->last_applied < rg->commit_index) {
            rg->last_applied++;
            if (rg->apply_fn) {
                raft_log_entry_t *e = &rg->log[rg->last_applied - 1];
                rg->apply_fn(rg->apply_ctx, e);
            }
        }
    }

    return DKV_OK;
}

void raft_group_tick(raft_group_t *rg) {
    if (!rg) return;

    rg->ticks_since_heartbeat++;

    /* Simplified: in single-node mode, become leader immediately */
    if (rg->peer_count == 0 && rg->role == RAFT_ROLE_FOLLOWER) {
        rg->role = RAFT_ROLE_LEADER;
        rg->leader_id = rg->config.node_id;
        rg->current_term++;
    }
}

void raft_group_trigger_election(raft_group_t *rg) {
    if (!rg) return;

    rg->current_term++;
    rg->role = RAFT_ROLE_CANDIDATE;
    rg->voted_for = rg->config.node_id;

    /* In single-node mode, win immediately */
    if (rg->peer_count == 0) {
        rg->role = RAFT_ROLE_LEADER;
        rg->leader_id = rg->config.node_id;
    }
}
