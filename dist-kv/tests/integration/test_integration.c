/**
 * test_integration.c - Integration tests for dist-kv
 *
 * Tests the complete system with multiple components working together.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../../src/types.h"
#include "../../src/coordinator.h"
#include "../../src/replication.h"
#include "../../src/gossip.h"
#include "../../src/failover.h"
#include "../../src/client.h"
#include "../../src/admin.h"
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

/* Test 1: Basic cluster operations */
TEST(test_integration_basic_cluster) {
    /* Create coordinator */
    coordinator_config_t coord_config;
    coordinator_config_init(&coord_config);
    coord_config.num_partitions = 8;
    coord_config.replication_factor = 3;
    coordinator_t *c = coordinator_create(&coord_config);
    assert(c != NULL);

    /* Create gossip */
    gossip_t *g = gossip_create(NULL);
    assert(g != NULL);

    /* Create failover manager */
    failover_manager_t *fm = failover_create(NULL, c, g);
    assert(fm != NULL);

    /* Create admin */
    dkv_admin_t *admin = dkv_admin_create(c, g, fm);
    assert(admin != NULL);

    /* Add nodes */
    node_addr_t addr = { .host = "127.0.0.1", .port = 7001 };
    assert(dkv_admin_add_node(admin, 1, &addr) == DKV_OK);
    addr.port = 7002;
    assert(dkv_admin_add_node(admin, 2, &addr) == DKV_OK);
    addr.port = 7003;
    assert(dkv_admin_add_node(admin, 3, &addr) == DKV_OK);

    /* Rebalance */
    assert(dkv_admin_rebalance(admin) == DKV_OK);

    /* Verify cluster state */
    assert(dkv_admin_node_count(admin) == 3);
    assert(dkv_admin_partition_count(admin) == 8);

    /* Each partition should have 3 replicas */
    for (int i = 0; i < 8; i++) {
        partition_t *p = coordinator_get_partition_obj(c, i);
        assert(partition_replica_count(p) == 3);
        assert(partition_get_leader(p) != 0);
    }

    dkv_admin_destroy(admin);
    failover_destroy(fm);
    gossip_destroy(g);
    coordinator_destroy(c);
}

/* Test 2: Node failure handling */
TEST(test_integration_node_failure) {
    coordinator_config_t coord_config;
    coordinator_config_init(&coord_config);
    coord_config.num_partitions = 4;
    coord_config.replication_factor = 3;
    coordinator_t *c = coordinator_create(&coord_config);

    gossip_config_t gossip_config;
    gossip_config_init(&gossip_config);
    gossip_config.failure_threshold = 1;
    gossip_config.suspicion_timeout_ms = 10;
    gossip_t *g = gossip_create(&gossip_config);

    failover_manager_t *fm = failover_create(NULL, c, g);
    dkv_admin_t *admin = dkv_admin_create(c, g, fm);

    /* Setup cluster */
    dkv_admin_add_node(admin, 1, NULL);
    dkv_admin_add_node(admin, 2, NULL);
    dkv_admin_add_node(admin, 3, NULL);
    dkv_admin_rebalance(admin);

    /* Simulate node 1 failure */
    gossip_record_ping_timeout(g, 1);
    uint64_t now = gossip_get_time_ms();
    gossip_tick(g, now + 100);

    assert(gossip_get_member_state(g, 1) == MEMBER_STATE_DEAD);

    /* Handle failure */
    failover_handle_node_failure(fm, 1);

    /* Partitions should still be readable */
    for (int i = 0; i < 4; i++) {
        assert(failover_partition_readable(fm, i) == true);
    }

    dkv_admin_destroy(admin);
    failover_destroy(fm);
    gossip_destroy(g);
    coordinator_destroy(c);
}

/* Test 3: Network partition simulation */
TEST(test_integration_network_partition) {
    network_sim_t *ns = network_sim_create(5);
    assert(ns != NULL);

    /* Create partition: majority {1,2,3} vs minority {4,5} */
    node_id_t majority[] = {1, 2, 3};
    node_id_t minority[] = {4, 5};
    network_sim_partition(ns, majority, 3, minority, 2);

    /* Verify partition */
    assert(network_sim_can_communicate(ns, 1, 2) == true);
    assert(network_sim_can_communicate(ns, 1, 4) == false);
    assert(network_sim_can_communicate(ns, 4, 5) == true);

    /* Majority can still communicate */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (i != j) {
                assert(network_sim_can_communicate(ns, majority[i], majority[j]) == true);
            }
        }
    }

    network_sim_destroy(ns);
}

/* Test 4: Network partition heal */
TEST(test_integration_partition_heal) {
    network_sim_t *ns = network_sim_create(4);

    node_id_t group1[] = {1, 2};
    node_id_t group2[] = {3, 4};
    network_sim_partition(ns, group1, 2, group2, 2);

    /* Verify partition */
    assert(network_sim_can_communicate(ns, 1, 3) == false);

    /* Heal */
    network_sim_heal_partition(ns);

    /* All should communicate now */
    for (int i = 1; i <= 4; i++) {
        for (int j = 1; j <= 4; j++) {
            if (i != j) {
                assert(network_sim_can_communicate(ns, i, j) == true);
            }
        }
    }

    network_sim_destroy(ns);
}

/* Test 5: Linearizable consistency */
TEST(test_consistency_linearizable) {
    replication_config_t config;
    replication_config_init(&config);
    config.node_id = 1;
    replication_group_t *rg = replication_create(&config);

    /* Become leader */
    replication_tick(rg);
    assert(replication_is_leader(rg) == true);

    /* Write */
    assert(replication_put(rg, "key", 3, "value1", 6) == DKV_OK);

    /* Linearizable read should see the write */
    void *value;
    size_t value_len;
    assert(replication_get(rg, "key", 3, &value, &value_len,
                           CONSISTENCY_LINEARIZABLE) == DKV_OK);
    assert(value_len == 6);
    assert(memcmp(value, "value1", 6) == 0);
    free(value);

    /* Update */
    assert(replication_put(rg, "key", 3, "value2", 6) == DKV_OK);

    /* Should see updated value */
    assert(replication_get(rg, "key", 3, &value, &value_len,
                           CONSISTENCY_LINEARIZABLE) == DKV_OK);
    assert(memcmp(value, "value2", 6) == 0);
    free(value);

    replication_destroy(rg);
}

/* Test 6: Eventual consistency */
TEST(test_consistency_eventual) {
    replication_config_t config;
    replication_config_init(&config);
    config.node_id = 1;
    replication_group_t *rg = replication_create(&config);

    replication_tick(rg);
    replication_put(rg, "key", 3, "value", 5);

    /* Eventual read should work */
    void *value;
    size_t value_len;
    assert(replication_get(rg, "key", 3, &value, &value_len,
                           CONSISTENCY_EVENTUAL) == DKV_OK);
    assert(value_len == 5);
    free(value);

    replication_destroy(rg);
}

/* Test 7: Throughput benchmark (simplified) */
TEST(test_bench_throughput) {
    replication_config_t config;
    replication_config_init(&config);
    config.node_id = 1;
    replication_group_t *rg = replication_create(&config);
    replication_tick(rg);

    int num_ops = 1000;
    int successful = 0;

    for (int i = 0; i < num_ops; i++) {
        char key[32], value[64];
        snprintf(key, sizeof(key), "key_%d", i);
        snprintf(value, sizeof(value), "value_%d", i);

        if (replication_put(rg, key, strlen(key), value, strlen(value)) == DKV_OK) {
            successful++;
        }
    }

    /* Should have high success rate */
    assert(successful > num_ops * 0.99);

    replication_destroy(rg);
}

/* Test 8: Latency benchmark (simplified) */
TEST(test_bench_latency) {
    replication_config_t config;
    replication_config_init(&config);
    config.node_id = 1;
    replication_group_t *rg = replication_create(&config);
    replication_tick(rg);

    /* Write some data */
    replication_put(rg, "bench_key", 9, "bench_value", 11);

    /* Read multiple times */
    int num_reads = 100;
    int successful = 0;

    for (int i = 0; i < num_reads; i++) {
        void *value;
        size_t value_len;
        if (replication_get(rg, "bench_key", 9, &value, &value_len,
                            CONSISTENCY_EVENTUAL) == DKV_OK) {
            successful++;
            free(value);
        }
    }

    assert(successful == num_reads);

    replication_destroy(rg);
}

/* Test 9: Mixed workload benchmark */
TEST(test_bench_mixed_workload) {
    replication_config_t config;
    replication_config_init(&config);
    config.node_id = 1;
    replication_group_t *rg = replication_create(&config);
    replication_tick(rg);

    int num_ops = 500;
    int writes = 0, reads = 0;

    for (int i = 0; i < num_ops; i++) {
        char key[32];
        snprintf(key, sizeof(key), "mixed_%d", i % 100);

        if (i % 3 == 0) {
            /* Write */
            char value[64];
            snprintf(value, sizeof(value), "value_%d", i);
            if (replication_put(rg, key, strlen(key), value, strlen(value)) == DKV_OK) {
                writes++;
            }
        } else {
            /* Read */
            void *value;
            size_t value_len;
            dkv_status_t status = replication_get(rg, key, strlen(key), &value, &value_len,
                                                   CONSISTENCY_EVENTUAL);
            if (status == DKV_OK) {
                reads++;
                free(value);
            } else if (status == DKV_ERR_NOT_FOUND) {
                reads++;  /* Expected for keys not yet written */
            }
        }
    }

    /* Should have reasonable success */
    assert(writes > 100);
    assert(reads > 200);

    replication_destroy(rg);
}

/* Test 10: Chaos testing (random failures) */
TEST(test_chaos_random_failures) {
    coordinator_config_t coord_config;
    coordinator_config_init(&coord_config);
    coord_config.num_partitions = 4;
    coord_config.replication_factor = 3;
    coordinator_t *c = coordinator_create(&coord_config);

    gossip_config_t gossip_config;
    gossip_config_init(&gossip_config);
    gossip_config.failure_threshold = 1;
    gossip_config.suspicion_timeout_ms = 10;
    gossip_t *g = gossip_create(&gossip_config);

    failover_manager_t *fm = failover_create(NULL, c, g);

    /* Add 5 nodes */
    for (int i = 1; i <= 5; i++) {
        coordinator_add_node(c, i);
        gossip_add_member(g, i, NULL);
    }
    coordinator_rebalance(c);

    /* Simulate random failures and recoveries */
    for (int round = 0; round < 10; round++) {
        /* Fail a random node */
        node_id_t fail_node = (round % 5) + 1;
        gossip_record_ping_timeout(g, fail_node);
        uint64_t now = gossip_get_time_ms();
        gossip_tick(g, now + 100);
        failover_handle_node_failure(fm, fail_node);

        /* Recover it */
        gossip_mark_alive(g, fail_node);
        failover_handle_node_recovery(fm, fail_node);
    }

    /* System should still be functional */
    int alive = 0;
    for (int i = 1; i <= 5; i++) {
        if (gossip_get_member_state(g, i) == MEMBER_STATE_ALIVE) {
            alive++;
        }
    }
    assert(alive == 5);

    failover_destroy(fm);
    gossip_destroy(g);
    coordinator_destroy(c);
}

int main(void) {
    printf("Phase 6 Tests: Integration Tests and Benchmarks\n");
    printf("=================================================\n\n");

    RUN_TEST(test_integration_basic_cluster);
    RUN_TEST(test_integration_node_failure);
    RUN_TEST(test_integration_network_partition);
    RUN_TEST(test_integration_partition_heal);
    RUN_TEST(test_consistency_linearizable);
    RUN_TEST(test_consistency_eventual);
    RUN_TEST(test_bench_throughput);
    RUN_TEST(test_bench_latency);
    RUN_TEST(test_bench_mixed_workload);
    RUN_TEST(test_chaos_random_failures);

    printf("\n=================================================\n");
    printf("Results: %d/%d tests passed\n", tests_passed, tests_run);

    return (tests_passed == tests_run) ? 0 : 1;
}
