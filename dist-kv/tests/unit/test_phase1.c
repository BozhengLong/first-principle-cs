/**
 * test_phase1.c - Phase 1 unit tests
 *
 * Tests for basic types, consistent hashing, and node lifecycle.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../../src/types.h"
#include "../../src/param.h"
#include "../../src/hash_ring.h"
#include "../../src/node.h"

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

/* Test 1: Hash ring create/destroy */
TEST(test_hash_ring_create_destroy) {
    hash_ring_t *ring = hash_ring_create(NULL);
    assert(ring != NULL);
    assert(hash_ring_node_count(ring) == 0);
    hash_ring_destroy(ring);

    /* With custom config */
    hash_ring_config_t config = {
        .num_virtual_nodes = 100,
        .replication_factor = 3
    };
    ring = hash_ring_create(&config);
    assert(ring != NULL);
    hash_ring_destroy(ring);
}

/* Test 2: Add/remove nodes */
TEST(test_hash_ring_add_remove_node) {
    hash_ring_t *ring = hash_ring_create(NULL);
    assert(ring != NULL);

    /* Add nodes */
    assert(hash_ring_add_node(ring, 1) == DKV_OK);
    assert(hash_ring_node_count(ring) == 1);
    assert(hash_ring_has_node(ring, 1) == true);

    assert(hash_ring_add_node(ring, 2) == DKV_OK);
    assert(hash_ring_node_count(ring) == 2);

    /* Duplicate add should fail */
    assert(hash_ring_add_node(ring, 1) == DKV_ERR_EXISTS);

    /* Remove node */
    assert(hash_ring_remove_node(ring, 1) == DKV_OK);
    assert(hash_ring_node_count(ring) == 1);
    assert(hash_ring_has_node(ring, 1) == false);

    /* Remove non-existent should fail */
    assert(hash_ring_remove_node(ring, 99) == DKV_ERR_NOT_FOUND);

    hash_ring_destroy(ring);
}

/* Test 3: Key routing */
TEST(test_hash_ring_key_routing) {
    hash_ring_t *ring = hash_ring_create(NULL);
    assert(ring != NULL);

    hash_ring_add_node(ring, 1);
    hash_ring_add_node(ring, 2);
    hash_ring_add_node(ring, 3);

    /* Same key should always route to same node */
    const char *key = "test_key";
    node_id_t node1 = hash_ring_get_node(ring, key, strlen(key));
    node_id_t node2 = hash_ring_get_node(ring, key, strlen(key));
    assert(node1 == node2);
    assert(node1 >= 1 && node1 <= 3);

    /* Different keys may route to different nodes */
    node_id_t nodes[100];
    int node_counts[4] = {0};
    for (int i = 0; i < 100; i++) {
        char buf[32];
        snprintf(buf, sizeof(buf), "key_%d", i);
        nodes[i] = hash_ring_get_node(ring, buf, strlen(buf));
        assert(nodes[i] >= 1 && nodes[i] <= 3);
        node_counts[nodes[i]]++;
    }

    /* Should have some distribution (not all to one node) */
    int non_zero = 0;
    for (int i = 1; i <= 3; i++) {
        if (node_counts[i] > 0) non_zero++;
    }
    assert(non_zero >= 2);  /* At least 2 nodes should have keys */

    hash_ring_destroy(ring);
}

/* Test 4: Replica selection */
TEST(test_hash_ring_replicas) {
    hash_ring_config_t config = {
        .num_virtual_nodes = 100,
        .replication_factor = 3
    };
    hash_ring_t *ring = hash_ring_create(&config);
    assert(ring != NULL);

    hash_ring_add_node(ring, 1);
    hash_ring_add_node(ring, 2);
    hash_ring_add_node(ring, 3);

    node_id_t replicas[3];
    int count = hash_ring_get_replicas(ring, "test", 4, replicas, 3);
    assert(count == 3);

    /* All replicas should be unique */
    assert(replicas[0] != replicas[1]);
    assert(replicas[1] != replicas[2]);
    assert(replicas[0] != replicas[2]);

    /* First replica should match get_node */
    node_id_t primary = hash_ring_get_node(ring, "test", 4);
    assert(replicas[0] == primary);

    hash_ring_destroy(ring);
}

/* Test 5: Distribution quality */
TEST(test_hash_ring_distribution) {
    hash_ring_config_t config = {
        .num_virtual_nodes = 150,
        .replication_factor = 3
    };
    hash_ring_t *ring = hash_ring_create(&config);
    assert(ring != NULL);

    /* Add 5 nodes */
    for (int i = 1; i <= 5; i++) {
        hash_ring_add_node(ring, i);
    }

    /* Count keys per node */
    int counts[6] = {0};
    int total_keys = 10000;
    for (int i = 0; i < total_keys; i++) {
        char buf[32];
        snprintf(buf, sizeof(buf), "key_%d", i);
        node_id_t node = hash_ring_get_node(ring, buf, strlen(buf));
        counts[node]++;
    }

    /* Check distribution - each node should have roughly 20% */
    double expected = total_keys / 5.0;
    for (int i = 1; i <= 5; i++) {
        double ratio = counts[i] / expected;
        /* Allow 30% deviation */
        assert(ratio > 0.7 && ratio < 1.3);
    }

    hash_ring_stats_t stats;
    hash_ring_get_stats(ring, &stats);
    assert(stats.node_count == 5);
    assert(stats.total_vnodes == 5 * 150);

    hash_ring_destroy(ring);
}

/* Test 6: Minimal movement on node changes */
TEST(test_hash_ring_minimal_movement) {
    hash_ring_t *ring = hash_ring_create(NULL);
    assert(ring != NULL);

    /* Start with 3 nodes */
    hash_ring_add_node(ring, 1);
    hash_ring_add_node(ring, 2);
    hash_ring_add_node(ring, 3);

    /* Record initial assignments */
    int num_keys = 1000;
    node_id_t *initial = malloc(num_keys * sizeof(node_id_t));
    for (int i = 0; i < num_keys; i++) {
        char buf[32];
        snprintf(buf, sizeof(buf), "key_%d", i);
        initial[i] = hash_ring_get_node(ring, buf, strlen(buf));
    }

    /* Add a 4th node */
    hash_ring_add_node(ring, 4);

    /* Count how many keys moved */
    int moved = 0;
    for (int i = 0; i < num_keys; i++) {
        char buf[32];
        snprintf(buf, sizeof(buf), "key_%d", i);
        node_id_t new_node = hash_ring_get_node(ring, buf, strlen(buf));
        if (new_node != initial[i]) {
            moved++;
        }
    }

    /* With consistent hashing, roughly 1/4 of keys should move */
    /* Allow some variance: 15-35% */
    double move_ratio = (double)moved / num_keys;
    assert(move_ratio > 0.15 && move_ratio < 0.35);

    free(initial);
    hash_ring_destroy(ring);
}

/* Test 7: Node create/destroy */
TEST(test_node_create_destroy) {
    dkv_node_config_t config;
    dkv_node_config_init(&config);
    config.node_id = 1;
    strncpy(config.data_dir, "/tmp/test_node", sizeof(config.data_dir) - 1);

    dkv_node_t *node = dkv_node_create(&config);
    assert(node != NULL);
    assert(dkv_node_get_id(node) == 1);
    assert(dkv_node_get_state(node) == NODE_STATE_INIT);

    dkv_node_destroy(node);

    /* NULL config should fail */
    node = dkv_node_create(NULL);
    assert(node == NULL);
}

/* Test 8: Node start/stop */
TEST(test_node_start_stop) {
    dkv_node_config_t config;
    dkv_node_config_init(&config);
    config.node_id = 1;
    strncpy(config.data_dir, "/tmp/test_node", sizeof(config.data_dir) - 1);

    dkv_node_t *node = dkv_node_create(&config);
    assert(node != NULL);

    /* Start node */
    assert(dkv_node_start(node) == DKV_OK);
    assert(dkv_node_get_state(node) == NODE_STATE_RUNNING);

    /* Double start should fail */
    assert(dkv_node_start(node) == DKV_ERR_INVALID);

    /* Stop node */
    assert(dkv_node_stop(node) == DKV_OK);
    assert(dkv_node_get_state(node) == NODE_STATE_STOPPED);

    /* Double stop should fail */
    assert(dkv_node_stop(node) == DKV_ERR_INVALID);

    /* Can restart after stop */
    assert(dkv_node_start(node) == DKV_OK);
    assert(dkv_node_get_state(node) == NODE_STATE_RUNNING);

    dkv_node_destroy(node);
}

/* Test 9: Config validation */
TEST(test_node_config_validation) {
    dkv_node_config_t config;

    /* Empty config should fail */
    memset(&config, 0, sizeof(config));
    assert(dkv_node_config_validate(&config) == DKV_ERR_INVALID);

    /* Missing node_id */
    dkv_node_config_init(&config);
    strncpy(config.data_dir, "/tmp/test", sizeof(config.data_dir) - 1);
    assert(dkv_node_config_validate(&config) == DKV_ERR_INVALID);

    /* Missing data_dir */
    dkv_node_config_init(&config);
    config.node_id = 1;
    config.data_dir[0] = '\0';
    assert(dkv_node_config_validate(&config) == DKV_ERR_INVALID);

    /* Invalid replication factor */
    dkv_node_config_init(&config);
    config.node_id = 1;
    strncpy(config.data_dir, "/tmp/test", sizeof(config.data_dir) - 1);
    config.replication_factor = 100;  /* Too high */
    assert(dkv_node_config_validate(&config) == DKV_ERR_INVALID);

    /* Valid config */
    dkv_node_config_init(&config);
    config.node_id = 1;
    strncpy(config.data_dir, "/tmp/test", sizeof(config.data_dir) - 1);
    assert(dkv_node_config_validate(&config) == DKV_OK);
}

/* Test 10: Hash function consistency */
TEST(test_hash_function_consistency) {
    /* Same input should always produce same hash */
    const char *key1 = "test_key";
    uint64_t h1 = hash_ring_hash_key(key1, strlen(key1));
    uint64_t h2 = hash_ring_hash_key(key1, strlen(key1));
    assert(h1 == h2);

    /* Different inputs should (usually) produce different hashes */
    const char *key2 = "test_key2";
    uint64_t h3 = hash_ring_hash_key(key2, strlen(key2));
    assert(h1 != h3);

    /* Empty key should work */
    uint64_t h4 = hash_ring_hash_key("", 0);
    assert(h4 != 0);  /* Should produce some hash */

    /* Long key should work */
    char long_key[1024];
    memset(long_key, 'x', sizeof(long_key));
    uint64_t h5 = hash_ring_hash_key(long_key, sizeof(long_key));
    assert(h5 != 0);

    /* Hash should be well-distributed (check high bits aren't always 0) */
    int high_bits_set = 0;
    for (int i = 0; i < 100; i++) {
        char buf[32];
        snprintf(buf, sizeof(buf), "key_%d", i);
        uint64_t h = hash_ring_hash_key(buf, strlen(buf));
        if (h & 0xFF00000000000000ULL) {
            high_bits_set++;
        }
    }
    assert(high_bits_set > 10);  /* At least some should have high bits set */
}

int main(void) {
    printf("Phase 1 Tests: Basic Types, Consistent Hashing, Node Lifecycle\n");
    printf("================================================================\n\n");

    RUN_TEST(test_hash_ring_create_destroy);
    RUN_TEST(test_hash_ring_add_remove_node);
    RUN_TEST(test_hash_ring_key_routing);
    RUN_TEST(test_hash_ring_replicas);
    RUN_TEST(test_hash_ring_distribution);
    RUN_TEST(test_hash_ring_minimal_movement);
    RUN_TEST(test_node_create_destroy);
    RUN_TEST(test_node_start_stop);
    RUN_TEST(test_node_config_validation);
    RUN_TEST(test_hash_function_consistency);

    printf("\n================================================================\n");
    printf("Results: %d/%d tests passed\n", tests_passed, tests_run);

    return (tests_passed == tests_run) ? 0 : 1;
}
