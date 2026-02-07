/**
 * test_phase5.c - Phase 5 unit tests
 *
 * Tests for client API and cluster management.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../../src/types.h"
#include "../../src/client.h"
#include "../../src/admin.h"
#include "../../src/iterator.h"
#include "../../src/coordinator.h"
#include "../../src/replication.h"
#include "../../src/gossip.h"
#include "../../src/failover.h"

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

/* Helper to create a test cluster */
typedef struct {
    coordinator_t *coordinator;
    gossip_t *gossip;
    failover_manager_t *failover;
    replication_group_t *rg;
} test_cluster_t;

static test_cluster_t *create_test_cluster(void) {
    test_cluster_t *tc = calloc(1, sizeof(test_cluster_t));
    if (!tc) return NULL;

    coordinator_config_t coord_config;
    coordinator_config_init(&coord_config);
    coord_config.num_partitions = 4;
    tc->coordinator = coordinator_create(&coord_config);

    tc->gossip = gossip_create(NULL);

    tc->failover = failover_create(NULL, tc->coordinator, tc->gossip);

    /* Add nodes */
    coordinator_add_node(tc->coordinator, 1);
    coordinator_add_node(tc->coordinator, 2);
    coordinator_add_node(tc->coordinator, 3);
    coordinator_rebalance(tc->coordinator);

    gossip_add_member(tc->gossip, 1, NULL);
    gossip_add_member(tc->gossip, 2, NULL);
    gossip_add_member(tc->gossip, 3, NULL);

    /* Create replication group */
    replication_config_t rep_config;
    replication_config_init(&rep_config);
    rep_config.node_id = 1;
    tc->rg = replication_create(&rep_config);
    replication_tick(tc->rg);  /* Become leader */

    return tc;
}

static void destroy_test_cluster(test_cluster_t *tc) {
    if (!tc) return;
    replication_destroy(tc->rg);
    failover_destroy(tc->failover);
    gossip_destroy(tc->gossip);
    coordinator_destroy(tc->coordinator);
    free(tc);
}

/* Test 1: Client create/destroy */
TEST(test_client_create_destroy) {
    test_cluster_t *tc = create_test_cluster();
    assert(tc != NULL);

    dkv_client_t *client = dkv_client_create(NULL, tc->coordinator, tc->failover);
    assert(client != NULL);
    dkv_client_destroy(client);

    /* With custom config */
    dkv_client_config_t config;
    dkv_client_config_init(&config);
    config.timeout_ms = 10000;
    config.max_retries = 5;

    client = dkv_client_create(&config, tc->coordinator, tc->failover);
    assert(client != NULL);
    dkv_client_destroy(client);

    destroy_test_cluster(tc);
}

/* Test 2: Client put/get */
TEST(test_client_put_get) {
    test_cluster_t *tc = create_test_cluster();
    dkv_client_t *client = dkv_client_create(NULL, tc->coordinator, tc->failover);

    /* Set replication group for all partitions */
    for (int i = 0; i < 4; i++) {
        dkv_client_set_replication(client, i, tc->rg);
    }

    /* Put a value */
    const char *key = "test_key";
    const char *value = "test_value";
    dkv_status_t status = dkv_client_put(client, key, strlen(key), value, strlen(value));
    assert(status == DKV_OK);

    /* Get the value */
    void *retrieved;
    size_t retrieved_len;
    status = dkv_client_get(client, key, strlen(key), &retrieved, &retrieved_len);
    assert(status == DKV_OK);
    assert(retrieved_len == strlen(value));
    assert(memcmp(retrieved, value, retrieved_len) == 0);
    dkv_client_free_value(retrieved);

    /* Get non-existent key */
    status = dkv_client_get(client, "nonexistent", 11, &retrieved, &retrieved_len);
    assert(status == DKV_ERR_NOT_FOUND);

    dkv_client_destroy(client);
    destroy_test_cluster(tc);
}

/* Test 3: Client delete */
TEST(test_client_delete) {
    test_cluster_t *tc = create_test_cluster();
    dkv_client_t *client = dkv_client_create(NULL, tc->coordinator, tc->failover);

    for (int i = 0; i < 4; i++) {
        dkv_client_set_replication(client, i, tc->rg);
    }

    /* Put and delete */
    dkv_client_put(client, "key", 3, "value", 5);

    void *value;
    size_t value_len;
    assert(dkv_client_get(client, "key", 3, &value, &value_len) == DKV_OK);
    dkv_client_free_value(value);

    assert(dkv_client_delete(client, "key", 3) == DKV_OK);
    assert(dkv_client_get(client, "key", 3, &value, &value_len) == DKV_ERR_NOT_FOUND);

    dkv_client_destroy(client);
    destroy_test_cluster(tc);
}

/* Test 4: Client batch put */
TEST(test_client_batch_put) {
    test_cluster_t *tc = create_test_cluster();
    dkv_client_t *client = dkv_client_create(NULL, tc->coordinator, tc->failover);

    for (int i = 0; i < 4; i++) {
        dkv_client_set_replication(client, i, tc->rg);
    }

    /* Batch put */
    kv_pair_t pairs[3] = {
        { .key = "key1", .key_len = 4, .value = "value1", .value_len = 6 },
        { .key = "key2", .key_len = 4, .value = "value2", .value_len = 6 },
        { .key = "key3", .key_len = 4, .value = "value3", .value_len = 6 },
    };

    dkv_status_t status = dkv_client_batch_put(client, pairs, 3);
    assert(status == DKV_OK);

    /* Verify all keys */
    void *value;
    size_t value_len;
    for (int i = 0; i < 3; i++) {
        status = dkv_client_get(client, pairs[i].key, pairs[i].key_len, &value, &value_len);
        assert(status == DKV_OK);
        assert(value_len == pairs[i].value_len);
        dkv_client_free_value(value);
    }

    dkv_client_destroy(client);
    destroy_test_cluster(tc);
}

/* Test 5: Client timeout handling (simulated) */
TEST(test_client_timeout_handling) {
    test_cluster_t *tc = create_test_cluster();

    dkv_client_config_t config;
    dkv_client_config_init(&config);
    config.max_retries = 2;

    dkv_client_t *client = dkv_client_create(&config, tc->coordinator, tc->failover);

    for (int i = 0; i < 4; i++) {
        dkv_client_set_replication(client, i, tc->rg);
    }

    /* Normal operation should work */
    dkv_status_t status = dkv_client_put(client, "key", 3, "value", 5);
    assert(status == DKV_OK);

    dkv_client_destroy(client);
    destroy_test_cluster(tc);
}

/* Test 6: Client leader redirect */
TEST(test_client_leader_redirect) {
    test_cluster_t *tc = create_test_cluster();
    dkv_client_t *client = dkv_client_create(NULL, tc->coordinator, tc->failover);

    for (int i = 0; i < 4; i++) {
        dkv_client_set_replication(client, i, tc->rg);
    }

    /* Get leader for a key */
    node_id_t leader = dkv_client_get_leader_for_key(client, "test", 4);
    /* Leader should be set after rebalance */
    assert(leader != 0);

    dkv_client_destroy(client);
    destroy_test_cluster(tc);
}

/* Test 7: Admin add/remove node */
TEST(test_admin_add_remove_node) {
    coordinator_t *c = coordinator_create(NULL);
    gossip_t *g = gossip_create(NULL);
    failover_manager_t *fm = failover_create(NULL, c, g);

    dkv_admin_t *admin = dkv_admin_create(c, g, fm);
    assert(admin != NULL);

    /* Add nodes */
    node_addr_t addr = { .host = "127.0.0.1", .port = 7001 };
    assert(dkv_admin_add_node(admin, 1, &addr) == DKV_OK);
    assert(dkv_admin_add_node(admin, 2, &addr) == DKV_OK);
    assert(dkv_admin_node_count(admin) == 2);

    /* Duplicate should fail */
    assert(dkv_admin_add_node(admin, 1, &addr) == DKV_ERR_EXISTS);

    /* Remove node */
    assert(dkv_admin_remove_node(admin, 1) == DKV_OK);
    assert(dkv_admin_node_count(admin) == 1);

    dkv_admin_destroy(admin);
    failover_destroy(fm);
    gossip_destroy(g);
    coordinator_destroy(c);
}

/* Test 8: Admin rebalance */
TEST(test_admin_rebalance) {
    coordinator_config_t coord_config;
    coordinator_config_init(&coord_config);
    coord_config.num_partitions = 8;

    coordinator_t *c = coordinator_create(&coord_config);
    gossip_t *g = gossip_create(NULL);
    failover_manager_t *fm = failover_create(NULL, c, g);
    dkv_admin_t *admin = dkv_admin_create(c, g, fm);

    /* Add nodes */
    dkv_admin_add_node(admin, 1, NULL);
    dkv_admin_add_node(admin, 2, NULL);
    dkv_admin_add_node(admin, 3, NULL);

    /* Rebalance */
    assert(dkv_admin_rebalance(admin) == DKV_OK);

    /* Check partitions are assigned */
    assert(dkv_admin_partition_count(admin) == 8);

    /* Get node status */
    node_status_t status;
    assert(dkv_admin_get_node_status(admin, 1, &status) == DKV_OK);
    assert(status.node_id == 1);
    assert(status.num_partitions > 0);

    dkv_admin_destroy(admin);
    failover_destroy(fm);
    gossip_destroy(g);
    coordinator_destroy(c);
}

/* Test 9: Iterator range scan */
TEST(test_iterator_range_scan) {
    storage_adapter_t *storage = storage_adapter_create(NULL);
    assert(storage != NULL);

    /* Create iterator with range */
    iterator_options_t opts = {
        .start_key = "b",
        .start_key_len = 1,
        .end_key = "d",
        .end_key_len = 1,
        .limit = 0,
        .reverse = false
    };

    dkv_iterator_t *iter = dkv_iterator_create(storage, &opts);
    assert(iter != NULL);

    /* Add entries manually for testing */
    dkv_iterator_add_entry(iter, "a", 1, "val_a", 5);  /* Before range */
    dkv_iterator_add_entry(iter, "b", 1, "val_b", 5);  /* In range */
    dkv_iterator_add_entry(iter, "c", 1, "val_c", 5);  /* In range */
    dkv_iterator_add_entry(iter, "d", 1, "val_d", 5);  /* After range */
    dkv_iterator_add_entry(iter, "e", 1, "val_e", 5);  /* After range */

    dkv_iterator_sort(iter);

    /* Should have 2 entries (b and c) */
    int count = 0;
    while (dkv_iterator_valid(iter)) {
        size_t key_len;
        const char *key = dkv_iterator_key(iter, &key_len);
        assert(key != NULL);
        assert(key[0] >= 'b' && key[0] < 'd');
        dkv_iterator_next(iter);
        count++;
    }
    assert(count == 2);

    dkv_iterator_destroy(iter);
    storage_adapter_destroy(storage);
}

/* Test 10: Client cluster info */
TEST(test_client_cluster_info) {
    test_cluster_t *tc = create_test_cluster();
    dkv_client_t *client = dkv_client_create(NULL, tc->coordinator, tc->failover);

    cluster_info_t info;
    dkv_status_t status = dkv_client_get_cluster_info(client, &info);
    assert(status == DKV_OK);
    assert(info.num_nodes == 3);
    assert(info.num_partitions == 4);

    dkv_client_free_cluster_info(&info);
    dkv_client_destroy(client);
    destroy_test_cluster(tc);
}

int main(void) {
    printf("Phase 5 Tests: Client API and Cluster Management\n");
    printf("==================================================\n\n");

    RUN_TEST(test_client_create_destroy);
    RUN_TEST(test_client_put_get);
    RUN_TEST(test_client_delete);
    RUN_TEST(test_client_batch_put);
    RUN_TEST(test_client_timeout_handling);
    RUN_TEST(test_client_leader_redirect);
    RUN_TEST(test_admin_add_remove_node);
    RUN_TEST(test_admin_rebalance);
    RUN_TEST(test_iterator_range_scan);
    RUN_TEST(test_client_cluster_info);

    printf("\n==================================================\n");
    printf("Results: %d/%d tests passed\n", tests_passed, tests_run);

    return (tests_passed == tests_run) ? 0 : 1;
}
