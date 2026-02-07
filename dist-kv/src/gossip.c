/**
 * gossip.c - Gossip-based failure detection implementation
 */

#include "gossip.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_MEMBERS 64

struct gossip {
    gossip_config_t config;
    gossip_member_t members[MAX_MEMBERS];
    int member_count;
    gossip_callback_fn callback;
    void *callback_ctx;
    uint64_t last_tick;
};

void gossip_config_init(gossip_config_t *config) {
    if (!config) return;
    memset(config, 0, sizeof(*config));
    config->gossip_interval_ms = DKV_GOSSIP_INTERVAL;
    config->gossip_fanout = DKV_GOSSIP_FANOUT;
    config->failure_threshold = DKV_FAILURE_THRESHOLD;
    config->suspicion_timeout_ms = DKV_SUSPICION_TIMEOUT;
}

gossip_t *gossip_create(const gossip_config_t *config) {
    gossip_t *g = calloc(1, sizeof(gossip_t));
    if (!g) return NULL;

    if (config) {
        g->config = *config;
    } else {
        gossip_config_init(&g->config);
    }

    return g;
}

void gossip_destroy(gossip_t *g) {
    free(g);
}

void gossip_set_callback(gossip_t *g, gossip_callback_fn fn, void *ctx) {
    if (!g) return;
    g->callback = fn;
    g->callback_ctx = ctx;
}

static void notify_state_change(gossip_t *g, node_id_t node_id,
                                 member_state_t old_state, member_state_t new_state) {
    if (g->callback && old_state != new_state) {
        g->callback(g->callback_ctx, node_id, old_state, new_state);
    }
}

static gossip_member_t *find_member(gossip_t *g, node_id_t node_id) {
    for (int i = 0; i < g->member_count; i++) {
        if (g->members[i].node_id == node_id) {
            return &g->members[i];
        }
    }
    return NULL;
}

dkv_status_t gossip_add_member(gossip_t *g, node_id_t node_id, const node_addr_t *addr) {
    if (!g) return DKV_ERR_INVALID;
    if (g->member_count >= MAX_MEMBERS) return DKV_ERR_INVALID;

    /* Check for duplicate */
    if (find_member(g, node_id)) return DKV_ERR_EXISTS;

    gossip_member_t *m = &g->members[g->member_count++];
    m->node_id = node_id;
    if (addr) {
        m->addr = *addr;
    }
    m->state = MEMBER_STATE_ALIVE;
    m->incarnation = 0;
    m->last_seen = gossip_get_time_ms();
    m->missed_pings = 0;

    return DKV_OK;
}

dkv_status_t gossip_remove_member(gossip_t *g, node_id_t node_id) {
    if (!g) return DKV_ERR_INVALID;

    for (int i = 0; i < g->member_count; i++) {
        if (g->members[i].node_id == node_id) {
            memmove(&g->members[i], &g->members[i + 1],
                    (g->member_count - i - 1) * sizeof(gossip_member_t));
            g->member_count--;
            return DKV_OK;
        }
    }

    return DKV_ERR_NOT_FOUND;
}

int gossip_member_count(gossip_t *g) {
    return g ? g->member_count : 0;
}

member_state_t gossip_get_member_state(gossip_t *g, node_id_t node_id) {
    if (!g) return MEMBER_STATE_DEAD;

    gossip_member_t *m = find_member(g, node_id);
    return m ? m->state : MEMBER_STATE_DEAD;
}

int gossip_get_members(gossip_t *g, gossip_member_t *members, int max_members) {
    if (!g || !members || max_members <= 0) return 0;

    int count = g->member_count;
    if (count > max_members) count = max_members;

    memcpy(members, g->members, count * sizeof(gossip_member_t));
    return count;
}

int gossip_get_alive_members(gossip_t *g, node_id_t *nodes, int max_nodes) {
    if (!g || !nodes || max_nodes <= 0) return 0;

    int count = 0;
    for (int i = 0; i < g->member_count && count < max_nodes; i++) {
        if (g->members[i].state == MEMBER_STATE_ALIVE) {
            nodes[count++] = g->members[i].node_id;
        }
    }

    return count;
}

void gossip_record_ping_response(gossip_t *g, node_id_t node_id) {
    if (!g) return;

    gossip_member_t *m = find_member(g, node_id);
    if (!m) return;

    member_state_t old_state = m->state;
    m->state = MEMBER_STATE_ALIVE;
    m->missed_pings = 0;
    m->last_seen = gossip_get_time_ms();

    notify_state_change(g, node_id, old_state, MEMBER_STATE_ALIVE);
}

void gossip_record_ping_timeout(gossip_t *g, node_id_t node_id) {
    if (!g) return;

    gossip_member_t *m = find_member(g, node_id);
    if (!m) return;

    m->missed_pings++;

    if (m->state == MEMBER_STATE_ALIVE &&
        m->missed_pings >= g->config.failure_threshold) {
        member_state_t old_state = m->state;
        m->state = MEMBER_STATE_SUSPECT;
        m->last_seen = gossip_get_time_ms();  /* Start suspicion timer */
        notify_state_change(g, node_id, old_state, MEMBER_STATE_SUSPECT);
    }
}

void gossip_tick(gossip_t *g, uint64_t current_time_ms) {
    if (!g) return;

    /* Check for suspicion timeouts */
    for (int i = 0; i < g->member_count; i++) {
        gossip_member_t *m = &g->members[i];

        if (m->state == MEMBER_STATE_SUSPECT) {
            uint64_t elapsed = current_time_ms - m->last_seen;
            if (elapsed >= (uint64_t)g->config.suspicion_timeout_ms) {
                member_state_t old_state = m->state;
                m->state = MEMBER_STATE_DEAD;
                notify_state_change(g, m->node_id, old_state, MEMBER_STATE_DEAD);
            }
        }
    }

    g->last_tick = current_time_ms;
}

void gossip_mark_alive(gossip_t *g, node_id_t node_id) {
    gossip_record_ping_response(g, node_id);
}

uint64_t gossip_get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}
