/**
 * gossip.h - Gossip-based failure detection (SWIM protocol)
 *
 * Implements failure detection using the SWIM protocol with
 * suspicion mechanism for reduced false positives.
 */

#ifndef DIST_KV_GOSSIP_H
#define DIST_KV_GOSSIP_H

#include "types.h"
#include "param.h"

/* Forward declaration */
typedef struct gossip gossip_t;

/* Member state */
typedef enum {
    MEMBER_STATE_ALIVE = 0,
    MEMBER_STATE_SUSPECT,
    MEMBER_STATE_DEAD,
} member_state_t;

/* Member information */
typedef struct {
    node_id_t node_id;
    node_addr_t addr;
    member_state_t state;
    uint64_t incarnation;        /* Incarnation number for conflict resolution */
    uint64_t last_seen;          /* Timestamp of last activity */
    int missed_pings;            /* Number of missed ping responses */
} gossip_member_t;

/* Gossip configuration */
typedef struct {
    node_id_t local_node_id;
    int gossip_interval_ms;      /* How often to gossip */
    int gossip_fanout;           /* Number of nodes to gossip to */
    int failure_threshold;       /* Missed pings before suspicion */
    int suspicion_timeout_ms;    /* Time in suspicion before dead */
} gossip_config_t;

/* Callback for member state changes */
typedef void (*gossip_callback_fn)(void *ctx, node_id_t node_id, member_state_t old_state,
                                    member_state_t new_state);

/* Initialize config with defaults */
void gossip_config_init(gossip_config_t *config);

/* Create a new gossip instance */
gossip_t *gossip_create(const gossip_config_t *config);

/* Destroy a gossip instance */
void gossip_destroy(gossip_t *g);

/* Set callback for member state changes */
void gossip_set_callback(gossip_t *g, gossip_callback_fn fn, void *ctx);

/* Add a member to the gossip group */
dkv_status_t gossip_add_member(gossip_t *g, node_id_t node_id, const node_addr_t *addr);

/* Remove a member from the gossip group */
dkv_status_t gossip_remove_member(gossip_t *g, node_id_t node_id);

/* Get member count */
int gossip_member_count(gossip_t *g);

/* Get member state */
member_state_t gossip_get_member_state(gossip_t *g, node_id_t node_id);

/* Get all members */
int gossip_get_members(gossip_t *g, gossip_member_t *members, int max_members);

/* Get alive members only */
int gossip_get_alive_members(gossip_t *g, node_id_t *nodes, int max_nodes);

/* Record a ping response from a member */
void gossip_record_ping_response(gossip_t *g, node_id_t node_id);

/* Record a ping timeout from a member */
void gossip_record_ping_timeout(gossip_t *g, node_id_t node_id);

/* Tick the gossip state machine (call periodically) */
void gossip_tick(gossip_t *g, uint64_t current_time_ms);

/* Mark a member as alive (e.g., received message from them) */
void gossip_mark_alive(gossip_t *g, node_id_t node_id);

/* Get current time in milliseconds (for testing) */
uint64_t gossip_get_time_ms(void);

#endif /* DIST_KV_GOSSIP_H */
