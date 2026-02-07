/**
 * raft_group.h - Raft consensus group wrapper
 *
 * Wraps the Raft consensus implementation for use in the distributed KV store.
 * In a real implementation, this would integrate with the consensus project.
 */

#ifndef DIST_KV_RAFT_GROUP_H
#define DIST_KV_RAFT_GROUP_H

#include "types.h"
#include "param.h"

/* Forward declaration */
typedef struct raft_group raft_group_t;

/* Raft node role */
typedef enum {
    RAFT_ROLE_FOLLOWER = 0,
    RAFT_ROLE_CANDIDATE,
    RAFT_ROLE_LEADER,
} raft_role_t;

/* Raft group configuration */
typedef struct {
    partition_id_t partition_id;
    node_id_t node_id;
    int election_timeout_ms;
    int heartbeat_interval_ms;
} raft_group_config_t;

/* Log entry for Raft */
typedef struct {
    uint64_t index;
    uint64_t term;
    uint8_t *data;
    size_t data_len;
} raft_log_entry_t;

/* Callback for applying committed entries */
typedef dkv_status_t (*raft_apply_fn)(void *ctx, const raft_log_entry_t *entry);

/* Initialize config with defaults */
void raft_group_config_init(raft_group_config_t *config);

/* Create a new Raft group */
raft_group_t *raft_group_create(const raft_group_config_t *config);

/* Destroy a Raft group */
void raft_group_destroy(raft_group_t *rg);

/* Set the apply callback */
void raft_group_set_apply_callback(raft_group_t *rg, raft_apply_fn fn, void *ctx);

/* Add a peer to the group */
dkv_status_t raft_group_add_peer(raft_group_t *rg, node_id_t peer_id);

/* Remove a peer from the group */
dkv_status_t raft_group_remove_peer(raft_group_t *rg, node_id_t peer_id);

/* Get current role */
raft_role_t raft_group_get_role(raft_group_t *rg);

/* Get current leader (0 if unknown) */
node_id_t raft_group_get_leader(raft_group_t *rg);

/* Get current term */
uint64_t raft_group_get_term(raft_group_t *rg);

/* Get commit index */
uint64_t raft_group_get_commit_index(raft_group_t *rg);

/* Propose a new entry (leader only) */
dkv_status_t raft_group_propose(raft_group_t *rg, const void *data, size_t len,
                                 uint64_t *index);

/* Tick the Raft state machine (call periodically) */
void raft_group_tick(raft_group_t *rg);

/* Trigger an election (for testing) */
void raft_group_trigger_election(raft_group_t *rg);

/* Check if this node is the leader */
bool raft_group_is_leader(raft_group_t *rg);

#endif /* DIST_KV_RAFT_GROUP_H */
