/**
 * test_phase4.c - Phase 4 unit tests
 *
 * Tests for fault detection and automatic failover.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../../src/types.h"
#include "../../src/gossip.h"
#include "../../src/failover.h"
#include "../../src/coordinator.h"
#include "../../src/partition.h"
#include "../integration/network_sim.h"

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) static void name(void)
#define RUN_TEST(name) do { \
    printf("  Running %s... ", #name); \
    fflush(stdout); \
    tests_run++; \
    name(); \
    tests_passed++; \
    printf("PASSED\n"); \
} while(0)

/* Test 1: Gossip create/destroy */
TEST(test_gossip_create_destroy) {
    gossip_t *g = gossip_create(NULL);
    assert(g != NULL);
    assert(gossip_member_count(g) == 0);
    gossip_destroy(g);

    /* With custom config */
    gossip_config_t config;
    gossip_config_init(&config);
    config.local_node_id = 1;
    config.failure_threshold = 5;

    g = gossip_create(&config);
    assert(g != NULL);
    gossip_destroy(g);
}

/* Test 2: Gossip add/remove member */
TEST(test_gossip_add_remove_member) {
    gossip_t *g = gossip_create(NULL);
    assert(g != NULL);

    node_addr_t addr = { .host = "127.0.0.1", .port = 7001 };

    /* Add members */
    assert(gossip_add_member(g, 1, &addr) == DKV_OK);
    assert(gossip_add_member(g, 2, &addr) == DKV_OK);
    assert(gossip_member_count(g) == 2);

    /* Duplicate should fail */
    assert(gossip_add_member(g, 1, &addr) == DKV_ERR_EXISTS);

    /* Check state */
    assert(gossip_get_member_state(g, 1) == MEMBER_STATE_ALIVE);
    assert(gossip_get_member_state(g, 99) == MEMBER_STATE_DEAD);

    /* Remove member */
    assert(gossip_remove_member(g, 1) == DKV_OK);
    assert(gossip_member_count(g) == 1);

    /* Remove non-existent should fail */
    assert(gossip_remove_member(g, 99) == DKV_ERR_NOT_FOUND);

    gossip_destroy(g);
}

/* Test 3: Gossip failure detection */
TEST(test_gossip_failure_detection) {
    gossip_config_t config;
    gossip_config_init(&config);
    config.failure_threshold = 3;
    config.suspicion_timeout_ms = 100;

    gossip_t *g = gossip_create(&config);
    assert(g != NULL);

    gossip_add_member(g, 1, NULL);
    assert(gossip_get_member_state(g, 1) == MEMBER_STATE_ALIVE);

    /* Simulate missed pings */
    gossip_record_ping_timeout(g, 1);
    assert(gossip_get_member_state(g, 1) == MEMBER_STATE_ALIVE);

    gossip_record_ping_timeout(g, 1);
    assert(gossip_get_member_state(g, 1) == MEMBER_STATE_ALIVE);

    gossip_record_ping_timeout(g, 1);
    assert(gossip_get_member_state(g, 1) == MEMBER_STATE_SUSPECT);

    /* Wait for suspicion timeout */
    uint64_t now = gossip_get_time_ms();
    gossip_tick(g, now + 150);
    assert(gossip_get_member_state(g, 1) == MEMBER_STATE_DEAD);

    gossip_destroy(g);
}

/* Test 4: Gossip recovery detection */
TEST(test_gossip_recovery_detection) {
    gossip_config_t config;
    gossip_config_init(&config);
    config.failure_threshold = 2;

    gossip_t *g = gossip_create(&config);
    assert(g != NULL);

    gossip_add_member(g, 1, NULL);

    /* Make it suspect */
    gossip_record_ping_timeout(g, 1);
    gossip_record_ping_timeout(g, 1);
    assert(gossip_get_member_state(g, 1) == MEMBER_STATE_SUSPECT);

    /* Recovery via ping response */
    gossip_record_ping_response(g, 1);
    assert(gossip_get_member_state(g, 1) == MEMBER_STATE_ALIVE);

    /* Make it suspect again */
    gossip_record_ping_timeout(g, 1);
    gossip_record_ping_timeout(g, 1);
    assert(gossip_get_member_state(g, 1) == MEMBER_STATE_SUSPECT);

    /* Recovery via mark_alive */
    gossip_mark_alive(g, 1);
    assert(gossip_get_member_state(g, 1) == MEMBER_STATE_ALIVE);

    gossip_destroy(g);
}

/* Test 5: Gossip propagation (get members) */
TEST(test_gossip_propagation) {
    gossip_t *g = gossip_create(NULL);
    assert(g != NULL);

    /* Add several members */
    for (int i = 1; i <= 5; i++) {
        gossip_add_member(g, i, NULL);
    }

    /* Get all members */
    gossip_member_t members[10];
    int count = gossip_get_members(g, members, 10);
    assert(count == 5);

    /* Get alive members */
    node_id_t alive[10];
    count = gossip_get_alive_members(g, alive, 10);
    assert(count == 5);

    /* Make one suspect */
    gossip_config_t config;
    gossip_config_init(&config);
    /* Need to recreate with config to set threshold */
    gossip_destroy(g);

    config.failure_threshold = 1;
    g = gossip_create(&config);
    for (int i = 1; i <= 5; i++) {
        gossip_add_member(g, i, NULL);
    }

    gossip_record_ping_timeout(g, 3);
    count = gossip_get_alive_members(g, alive, 10);
    assert(count == 4);  /* Node 3 is now suspect */

    gossip_destroy(g);
}

/* Test 6: Failover partition reassignment */
TEST(test_failover_partition_reassignment) {
    coordinator_config_t coord_config;
    coordinator_config_init(&coord_config);
    coord_config.num_partitions = 4;
    coord_config.replication_factor = 3;

    coordinator_t *c = coordinator_create(&coord_config);
    assert(c != NULL);

    /* Add nodes and rebalance */
    coordinator_add_node(c, 1);
    coordinator_add_node(c, 2);
    coordinator_add_node(c, 3);
    coordinator_rebalance(c);

    /* Create gossip */
    gossip_t *g = gossip_create(NULL);
    gossip_add_member(g, 1, NULL);
    gossip_add_member(g, 2, NULL);
    gossip_add_member(g, 3, NULL);

    /* Create failover manager */
    failover_manager_t *fm = failover_create(NULL, c, g);
    assert(fm != NULL);

    /* All partitions should be writable */
    for (int i = 0; i < 4; i++) {
        assert(failover_partition_readable(fm, i) == true);
    }

    /* Simulate node failure */
    gossip_config_t gconfig;
    gossip_config_init(&gconfig);
    gconfig.failure_threshold = 1;
    gossip_destroy(g);
    g = gossip_create(&gconfig);
    gossip_add_member(g, 1, NULL);
    gossip_add_member(g, 2, NULL);
    gossip_add_member(g, 3, NULL);

    failover_destroy(fm);
    fm = failover_create(NULL, c, g);

    /* Mark node 1 as dead */
    gossip_record_ping_timeout(g, 1);
    uint64_t now = gossip_get_time_ms();
    gossip_tick(g, now + 10000);

    /* Handle failure */
    failover_handle_node_failure(fm, 1);

    /* Partitions should still be readable (2 replicas left) */
    for (int i = 0; i < 4; i++) {
        assert(failover_partition_readable(fm, i) == true);
    }

    failover_destroy(fm);
    gossip_destroy(g);
    coordinator_destroy(c);
}

/* Test 7: Failover leader election */
TEST(test_failover_leader_election) {
    coordinator_config_t coord_config;
    coordinator_config_init(&coord_config);
    coord_config.num_partitions = 2;
    coord_config.replication_factor = 3;

    coordinator_t *c = coordinator_create(&coord_config);
    coordinator_add_node(c, 1);
    coordinator_add_node(c, 2);
    coordinator_add_node(c, 3);
    coordinator_rebalance(c);

    gossip_t *g = gossip_create(NULL);
    gossip_add_member(g, 1, NULL);
    gossip_add_member(g, 2, NULL);
    gossip_add_member(g, 3, NULL);

    failover_manager_t *fm = failover_create(NULL, c, g);

    /* Get current leader for partition 0 */
    partition_t *p = coordinator_get_partition_obj(c, 0);
    node_id_t old_leader = partition_get_leader(p);
    assert(old_leader != 0);

    /* Elect new leader */
    dkv_status_t status = failover_elect_leader(fm, 0);
    assert(status == DKV_OK);

    /* Should have a leader */
    node_id_t new_leader = partition_get_leader(p);
    assert(new_leader != 0);

    failover_destroy(fm);
    gossip_destroy(g);
    coordinator_destroy(c);
}

/* Test 8: Network partition handling */
TEST(test_network_partition_handling) {
    network_sim_t *ns = network_sim_create(5);
    assert(ns != NULL);

    /* Initially all can communicate */
    assert(network_sim_can_communicate(ns, 1, 2) == true);
    assert(network_sim_can_communicate(ns, 1, 5) == true);

    /* Create partition: {1,2} vs {3,4,5} */
    node_id_t group1[] = {1, 2};
    node_id_t group2[] = {3, 4, 5};
    network_sim_partition(ns, group1, 2, group2, 3);

    /* Within group should work */
    assert(network_sim_can_communicate(ns, 1, 2) == true);
    assert(network_sim_can_communicate(ns, 3, 4) == true);

    /* Across groups should fail */
    assert(network_sim_can_communicate(ns, 1, 3) == false);
    assert(network_sim_can_communicate(ns, 2, 5) == false);

    network_sim_destroy(ns);
}

/* Test 9: Network partition heal */
TEST(test_network_partition_heal) {
    network_sim_t *ns = network_sim_create(4);
    assert(ns != NULL);

    /* Create partition */
    node_id_t group1[] = {1, 2};
    node_id_t group2[] = {3, 4};
    network_sim_partition(ns, group1, 2, group2, 2);

    assert(network_sim_can_communicate(ns, 1, 3) == false);

    /* Heal partition */
    network_sim_heal_partition(ns);

    assert(network_sim_can_communicate(ns, 1, 3) == true);
    assert(network_sim_can_communicate(ns, 2, 4) == true);

    network_sim_destroy(ns);
}

/* Test 10: Minority partition becomes read-only */
TEST(test_minority_partition_readonly) {
    coordinator_config_t coord_config;
    coordinator_config_init(&coord_config);
    coord_config.num_partitions = 2;
    coord_config.replication_factor = 3;

    coordinator_t *c = coordinator_create(&coord_config);
    coordinator_add_node(c, 1);
    coordinator_add_node(c, 2);
    coordinator_add_node(c, 3);
    coordinator_rebalance(c);

    gossip_config_t gconfig;
    gossip_config_init(&gconfig);
    gconfig.failure_threshold = 1;
    gconfig.suspicion_timeout_ms = 10;

    gossip_t *g = gossip_create(&gconfig);
    gossip_add_member(g, 1, NULL);
    gossip_add_member(g, 2, NULL);
    gossip_add_member(g, 3, NULL);

    failover_config_t fconfig;
    failover_config_init(&fconfig);
    fconfig.min_replicas = 2;

    failover_manager_t *fm = failover_create(&fconfig, c, g);

    /* Initially writable */
    assert(failover_partition_writable(fm, 0) == true);

    /* Fail 2 nodes (majority) */
    gossip_record_ping_timeout(g, 1);
    gossip_record_ping_timeout(g, 2);
    uint64_t now = gossip_get_time_ms();
    gossip_tick(g, now + 100);

    failover_handle_node_failure(fm, 1);
    failover_handle_node_failure(fm, 2);

    /* Should be read-only now (only 1 replica available) */
    partition_t *p = coordinator_get_partition_obj(c, 0);
    assert(partition_get_state(p) == PARTITION_STATE_READONLY);

    /* But still readable */
    assert(failover_partition_readable(fm, 0) == true);

    failover_destroy(fm);
    gossip_destroy(g);
    coordinator_destroy(c);
}

int main(void) {
    printf("Phase 4 Tests: Fault Detection and Automatic Failover\n");
    printf("=======================================================\n\n");

    RUN_TEST(test_gossip_create_destroy);
    RUN_TEST(test_gossip_add_remove_member);
    RUN_TEST(test_gossip_failure_detection);
    RUN_TEST(test_gossip_recovery_detection);
    RUN_TEST(test_gossip_propagation);
    RUN_TEST(test_failover_partition_reassignment);
    RUN_TEST(test_failover_leader_election);
    RUN_TEST(test_network_partition_handling);
    RUN_TEST(test_network_partition_heal);
    RUN_TEST(test_minority_partition_readonly);

    printf("\n=======================================================\n");
    printf("Results: %d/%d tests passed\n", tests_passed, tests_run);

    return (tests_passed == tests_run) ? 0 : 1;
}
