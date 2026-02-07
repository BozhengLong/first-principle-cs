/**
 * test_phase2.c - Phase 2 unit tests
 *
 * Tests for partition management and request routing.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../../src/types.h"
#include "../../src/param.h"
#include "../../src/partition.h"
#include "../../src/coordinator.h"
#include "../../src/rpc.h"

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

/* Test 1: Partition create/destroy */
TEST(test_partition_create_destroy) {
    partition_config_t config = {
        .partition_id = 1,
        .replication_factor = 3
    };

    partition_t *p = partition_create(&config);
    assert(p != NULL);
    assert(partition_get_id(p) == 1);
    assert(partition_get_state(p) == PARTITION_STATE_INIT);
    assert(partition_replica_count(p) == 0);

    partition_destroy(p);

    /* NULL config should fail */
    p = partition_create(NULL);
    assert(p == NULL);
}

/* Test 2: Partition replica management */
TEST(test_partition_replica_management) {
    partition_config_t config = { .partition_id = 0, .replication_factor = 3 };
    partition_t *p = partition_create(&config);
    assert(p != NULL);

    /* Add replicas */
    assert(partition_add_replica(p, 1, true) == DKV_OK);
    assert(partition_add_replica(p, 2, false) == DKV_OK);
    assert(partition_add_replica(p, 3, false) == DKV_OK);
    assert(partition_replica_count(p) == 3);

    /* Duplicate should fail */
    assert(partition_add_replica(p, 1, false) == DKV_ERR_EXISTS);

    /* Check has_replica */
    assert(partition_has_replica(p, 1) == true);
    assert(partition_has_replica(p, 99) == false);

    /* Get replicas */
    replica_info_t replicas[3];
    int count = partition_get_replicas(p, replicas, 3);
    assert(count == 3);
    assert(replicas[0].node_id == 1);
    assert(replicas[0].is_local == true);

    /* Remove replica */
    assert(partition_remove_replica(p, 2) == DKV_OK);
    assert(partition_replica_count(p) == 2);
    assert(partition_has_replica(p, 2) == false);

    /* Remove non-existent should fail */
    assert(partition_remove_replica(p, 99) == DKV_ERR_NOT_FOUND);

    partition_destroy(p);
}

/* Test 3: Key to partition mapping */
TEST(test_key_to_partition_mapping) {
    coordinator_config_t config;
    coordinator_config_init(&config);
    config.num_partitions = 16;

    coordinator_t *c = coordinator_create(&config);
    assert(c != NULL);

    /* Same key should always map to same partition */
    partition_id_t p1 = coordinator_get_partition(c, "test_key", 8);
    partition_id_t p2 = coordinator_get_partition(c, "test_key", 8);
    assert(p1 == p2);
    assert(p1 < 16);

    /* Different keys should distribute across partitions */
    int partition_counts[16] = {0};
    for (int i = 0; i < 1000; i++) {
        char buf[32];
        snprintf(buf, sizeof(buf), "key_%d", i);
        partition_id_t pid = coordinator_get_partition(c, buf, strlen(buf));
        assert(pid < 16);
        partition_counts[pid]++;
    }

    /* Check distribution - each partition should have some keys */
    int non_empty = 0;
    for (int i = 0; i < 16; i++) {
        if (partition_counts[i] > 0) non_empty++;
    }
    assert(non_empty >= 12);  /* At least 75% of partitions should have keys */

    coordinator_destroy(c);
}

/* Test 4: Coordinator create/destroy */
TEST(test_coordinator_create_destroy) {
    coordinator_t *c = coordinator_create(NULL);
    assert(c != NULL);
    assert(coordinator_partition_count(c) == DKV_DEFAULT_PARTITIONS);
    assert(coordinator_node_count(c) == 0);
    coordinator_destroy(c);

    /* With custom config */
    coordinator_config_t config = {
        .num_partitions = 32,
        .replication_factor = 3,
        .num_virtual_nodes = 100
    };
    c = coordinator_create(&config);
    assert(c != NULL);
    assert(coordinator_partition_count(c) == 32);
    coordinator_destroy(c);
}

/* Test 5: Coordinator partition routing */
TEST(test_coordinator_partition_routing) {
    coordinator_config_t config;
    coordinator_config_init(&config);
    config.num_partitions = 8;

    coordinator_t *c = coordinator_create(&config);
    assert(c != NULL);

    /* Add nodes */
    coordinator_add_node(c, 1);
    coordinator_add_node(c, 2);
    coordinator_add_node(c, 3);

    /* Rebalance to assign partitions */
    assert(coordinator_rebalance(c) == DKV_OK);

    /* Get nodes for a key */
    node_id_t nodes[3];
    int count = coordinator_get_nodes_for_key(c, "test", 4, nodes, 3);
    assert(count == 3);

    /* Get partition for key */
    partition_id_t pid = coordinator_get_partition(c, "test", 4);
    partition_t *p = coordinator_get_partition_obj(c, pid);
    assert(p != NULL);
    assert(partition_get_id(p) == pid);

    coordinator_destroy(c);
}

/* Test 6: Coordinator add/remove node */
TEST(test_coordinator_add_remove_node) {
    coordinator_t *c = coordinator_create(NULL);
    assert(c != NULL);

    /* Add nodes */
    assert(coordinator_add_node(c, 1) == DKV_OK);
    assert(coordinator_add_node(c, 2) == DKV_OK);
    assert(coordinator_node_count(c) == 2);

    /* Duplicate should fail */
    assert(coordinator_add_node(c, 1) == DKV_ERR_EXISTS);

    /* Remove node */
    assert(coordinator_remove_node(c, 1) == DKV_OK);
    assert(coordinator_node_count(c) == 1);

    /* Remove non-existent should fail */
    assert(coordinator_remove_node(c, 99) == DKV_ERR_NOT_FOUND);

    coordinator_destroy(c);
}

/* Test 7: Coordinator rebalance */
TEST(test_coordinator_rebalance) {
    coordinator_config_t config;
    coordinator_config_init(&config);
    config.num_partitions = 8;
    config.replication_factor = 3;

    coordinator_t *c = coordinator_create(&config);
    assert(c != NULL);

    /* Add 3 nodes */
    coordinator_add_node(c, 1);
    coordinator_add_node(c, 2);
    coordinator_add_node(c, 3);

    /* Rebalance */
    assert(coordinator_rebalance(c) == DKV_OK);

    /* Each partition should have 3 replicas */
    for (int i = 0; i < 8; i++) {
        partition_t *p = coordinator_get_partition_obj(c, i);
        assert(p != NULL);
        assert(partition_replica_count(p) == 3);
        assert(partition_get_leader(p) != 0);
        assert(partition_get_state(p) == PARTITION_STATE_ACTIVE);
    }

    /* Each node should have some partitions */
    partition_id_t parts[8];
    for (node_id_t n = 1; n <= 3; n++) {
        int count = coordinator_get_node_partitions(c, n, parts, 8);
        assert(count > 0);
    }

    coordinator_destroy(c);
}

/* Test 8: Partition leader tracking */
TEST(test_partition_leader_tracking) {
    partition_config_t config = { .partition_id = 0, .replication_factor = 3 };
    partition_t *p = partition_create(&config);
    assert(p != NULL);

    /* Add replicas */
    partition_add_replica(p, 1, false);
    partition_add_replica(p, 2, false);
    partition_add_replica(p, 3, false);

    /* Initially no leader */
    assert(partition_get_leader(p) == 0);

    /* Set leader */
    assert(partition_set_leader(p, 2) == DKV_OK);
    assert(partition_get_leader(p) == 2);

    /* Verify leader flag in replicas */
    replica_info_t replicas[3];
    partition_get_replicas(p, replicas, 3);
    for (int i = 0; i < 3; i++) {
        if (replicas[i].node_id == 2) {
            assert(replicas[i].is_leader == true);
        } else {
            assert(replicas[i].is_leader == false);
        }
    }

    /* Change leader */
    assert(partition_set_leader(p, 3) == DKV_OK);
    assert(partition_get_leader(p) == 3);

    /* Set non-replica as leader should fail */
    assert(partition_set_leader(p, 99) == DKV_ERR_NOT_FOUND);

    /* Remove leader should clear leader */
    partition_remove_replica(p, 3);
    assert(partition_get_leader(p) == 0);

    partition_destroy(p);
}

/* Test 9: RPC message serialization */
TEST(test_rpc_message_serialization) {
    rpc_header_t header = {
        .magic = RPC_MAGIC,
        .version = RPC_VERSION,
        .type = RPC_PUT,
        .payload_len = 100,
        .sender_id = 12345,
        .request_id = 67890,
        .partition_id = 5
    };

    uint8_t buf[RPC_HEADER_SIZE];
    rpc_header_serialize(&header, buf);

    rpc_header_t decoded;
    assert(rpc_header_deserialize(buf, &decoded) == true);

    assert(decoded.magic == RPC_MAGIC);
    assert(decoded.version == RPC_VERSION);
    assert(decoded.type == RPC_PUT);
    assert(decoded.payload_len == 100);
    assert(decoded.sender_id == 12345);
    assert(decoded.request_id == 67890);
    assert(decoded.partition_id == 5);

    /* Invalid magic should fail */
    buf[0] = 0xFF;
    assert(rpc_header_deserialize(buf, &decoded) == false);
}

/* Test 10: Coordinator multi-partition operations */
TEST(test_coordinator_multi_partition) {
    coordinator_config_t config;
    coordinator_config_init(&config);
    config.num_partitions = 16;
    config.replication_factor = 3;

    coordinator_t *c = coordinator_create(&config);
    assert(c != NULL);

    /* Add 5 nodes */
    for (node_id_t i = 1; i <= 5; i++) {
        coordinator_add_node(c, i);
    }
    coordinator_rebalance(c);

    /* Verify all partitions are assigned */
    for (int i = 0; i < 16; i++) {
        partition_t *p = coordinator_get_partition_obj(c, i);
        assert(p != NULL);
        assert(partition_replica_count(p) == 3);
    }

    /* Remove a node and rebalance */
    coordinator_remove_node(c, 3);
    coordinator_rebalance(c);

    /* Partitions should still have 3 replicas (from remaining 4 nodes) */
    for (int i = 0; i < 16; i++) {
        partition_t *p = coordinator_get_partition_obj(c, i);
        assert(partition_replica_count(p) == 3);
        /* Node 3 should not be in any partition */
        assert(partition_has_replica(p, 3) == false);
    }

    coordinator_destroy(c);
}

int main(void) {
    printf("Phase 2 Tests: Partition Management and Request Routing\n");
    printf("=========================================================\n\n");

    RUN_TEST(test_partition_create_destroy);
    RUN_TEST(test_partition_replica_management);
    RUN_TEST(test_key_to_partition_mapping);
    RUN_TEST(test_coordinator_create_destroy);
    RUN_TEST(test_coordinator_partition_routing);
    RUN_TEST(test_coordinator_add_remove_node);
    RUN_TEST(test_coordinator_rebalance);
    RUN_TEST(test_partition_leader_tracking);
    RUN_TEST(test_rpc_message_serialization);
    RUN_TEST(test_coordinator_multi_partition);

    printf("\n=========================================================\n");
    printf("Results: %d/%d tests passed\n", tests_passed, tests_run);

    return (tests_passed == tests_run) ? 0 : 1;
}
