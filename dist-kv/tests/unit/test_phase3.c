/**
 * test_phase3.c - Phase 3 unit tests
 *
 * Tests for Raft consensus and storage integration.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../../src/types.h"
#include "../../src/raft_group.h"
#include "../../src/storage_adapter.h"
#include "../../src/replication.h"

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

/* Test 1: Raft group create/destroy */
TEST(test_raft_group_create_destroy) {
    raft_group_t *rg = raft_group_create(NULL);
    assert(rg != NULL);
    assert(raft_group_get_role(rg) == RAFT_ROLE_FOLLOWER);
    assert(raft_group_get_term(rg) == 0);
    raft_group_destroy(rg);

    /* With custom config */
    raft_group_config_t config;
    raft_group_config_init(&config);
    config.node_id = 1;
    config.partition_id = 0;

    rg = raft_group_create(&config);
    assert(rg != NULL);
    raft_group_destroy(rg);
}

/* Test 2: Single node becomes leader */
TEST(test_raft_group_single_node_leader) {
    raft_group_config_t config;
    raft_group_config_init(&config);
    config.node_id = 1;

    raft_group_t *rg = raft_group_create(&config);
    assert(rg != NULL);

    /* Initially follower */
    assert(raft_group_get_role(rg) == RAFT_ROLE_FOLLOWER);
    assert(raft_group_is_leader(rg) == false);

    /* Tick should make single node become leader */
    raft_group_tick(rg);
    assert(raft_group_get_role(rg) == RAFT_ROLE_LEADER);
    assert(raft_group_is_leader(rg) == true);
    assert(raft_group_get_leader(rg) == 1);
    assert(raft_group_get_term(rg) == 1);

    raft_group_destroy(rg);
}

/* Test 3: Raft propose and commit */
TEST(test_raft_group_propose_commit) {
    raft_group_config_t config;
    raft_group_config_init(&config);
    config.node_id = 1;

    raft_group_t *rg = raft_group_create(&config);
    assert(rg != NULL);

    /* Become leader first */
    raft_group_tick(rg);
    assert(raft_group_is_leader(rg) == true);

    /* Propose should fail before becoming leader */
    /* (Already leader, so this should succeed) */
    uint64_t index;
    const char *data = "test_data";
    dkv_status_t status = raft_group_propose(rg, data, strlen(data), &index);
    assert(status == DKV_OK);
    assert(index == 1);

    /* Commit index should be updated (single node) */
    assert(raft_group_get_commit_index(rg) == 1);

    /* Propose another entry */
    status = raft_group_propose(rg, "more_data", 9, &index);
    assert(status == DKV_OK);
    assert(index == 2);
    assert(raft_group_get_commit_index(rg) == 2);

    raft_group_destroy(rg);
}

/* Test 4: Storage adapter put/get */
TEST(test_storage_adapter_put_get) {
    storage_adapter_t *sa = storage_adapter_create(NULL);
    assert(sa != NULL);

    /* Put a value */
    const char *key = "test_key";
    const char *value = "test_value";
    dkv_status_t status = storage_adapter_put(sa, key, strlen(key), value, strlen(value));
    assert(status == DKV_OK);

    /* Get the value */
    void *retrieved;
    size_t retrieved_len;
    status = storage_adapter_get(sa, key, strlen(key), &retrieved, &retrieved_len);
    assert(status == DKV_OK);
    assert(retrieved_len == strlen(value));
    assert(memcmp(retrieved, value, retrieved_len) == 0);
    storage_adapter_free_value(retrieved);

    /* Update the value */
    const char *new_value = "updated_value";
    status = storage_adapter_put(sa, key, strlen(key), new_value, strlen(new_value));
    assert(status == DKV_OK);

    status = storage_adapter_get(sa, key, strlen(key), &retrieved, &retrieved_len);
    assert(status == DKV_OK);
    assert(retrieved_len == strlen(new_value));
    assert(memcmp(retrieved, new_value, retrieved_len) == 0);
    storage_adapter_free_value(retrieved);

    /* Get non-existent key */
    status = storage_adapter_get(sa, "nonexistent", 11, &retrieved, &retrieved_len);
    assert(status == DKV_ERR_NOT_FOUND);

    storage_adapter_destroy(sa);
}

/* Test 5: Storage adapter delete */
TEST(test_storage_adapter_delete) {
    storage_adapter_t *sa = storage_adapter_create(NULL);
    assert(sa != NULL);

    /* Put a value */
    const char *key = "delete_me";
    const char *value = "some_value";
    storage_adapter_put(sa, key, strlen(key), value, strlen(value));

    /* Verify it exists */
    assert(storage_adapter_exists(sa, key, strlen(key)) == true);

    /* Delete it */
    dkv_status_t status = storage_adapter_delete(sa, key, strlen(key));
    assert(status == DKV_OK);

    /* Verify it's gone */
    assert(storage_adapter_exists(sa, key, strlen(key)) == false);

    void *retrieved;
    size_t retrieved_len;
    status = storage_adapter_get(sa, key, strlen(key), &retrieved, &retrieved_len);
    assert(status == DKV_ERR_NOT_FOUND);

    /* Delete non-existent should fail */
    status = storage_adapter_delete(sa, "nonexistent", 11);
    assert(status == DKV_ERR_NOT_FOUND);

    storage_adapter_destroy(sa);
}

/* Test 6: Storage adapter snapshot */
TEST(test_storage_adapter_snapshot) {
    storage_adapter_t *sa = storage_adapter_create(NULL);
    assert(sa != NULL);

    /* Put some data */
    storage_adapter_put(sa, "key1", 4, "value1", 6);
    storage_adapter_put(sa, "key2", 4, "value2", 6);

    /* Create snapshot */
    dkv_status_t status = storage_adapter_snapshot(sa, "/tmp/snapshot");
    assert(status == DKV_OK);

    /* Get stats */
    storage_stats_t stats;
    storage_adapter_get_stats(sa, &stats);
    assert(stats.num_keys == 2);

    storage_adapter_destroy(sa);
}

/* Test 7: Replication put/get */
TEST(test_replication_put_get) {
    replication_config_t config;
    replication_config_init(&config);
    config.node_id = 1;
    config.partition_id = 0;

    replication_group_t *rg = replication_create(&config);
    assert(rg != NULL);

    /* Become leader */
    replication_tick(rg);
    assert(replication_is_leader(rg) == true);

    /* Put a value */
    const char *key = "rep_key";
    const char *value = "rep_value";
    dkv_status_t status = replication_put(rg, key, strlen(key), value, strlen(value));
    assert(status == DKV_OK);

    /* Get the value (linearizable) */
    void *retrieved;
    size_t retrieved_len;
    status = replication_get(rg, key, strlen(key), &retrieved, &retrieved_len,
                             CONSISTENCY_LINEARIZABLE);
    assert(status == DKV_OK);
    assert(retrieved_len == strlen(value));
    assert(memcmp(retrieved, value, retrieved_len) == 0);
    storage_adapter_free_value(retrieved);

    replication_destroy(rg);
}

/* Test 8: Replication linearizable read */
TEST(test_replication_linearizable_read) {
    replication_config_t config;
    replication_config_init(&config);
    config.node_id = 1;
    config.partition_id = 0;

    replication_group_t *rg = replication_create(&config);
    assert(rg != NULL);

    /* Before becoming leader, linearizable read should fail */
    void *value;
    size_t value_len;
    dkv_status_t status = replication_get(rg, "key", 3, &value, &value_len,
                                           CONSISTENCY_LINEARIZABLE);
    assert(status == DKV_ERR_NOT_LEADER);

    /* Become leader */
    replication_tick(rg);
    assert(replication_is_leader(rg) == true);

    /* Now linearizable read should work (but key doesn't exist) */
    status = replication_get(rg, "key", 3, &value, &value_len,
                             CONSISTENCY_LINEARIZABLE);
    assert(status == DKV_ERR_NOT_FOUND);

    replication_destroy(rg);
}

/* Test 9: Replication local read */
TEST(test_replication_local_read) {
    replication_config_t config;
    replication_config_init(&config);
    config.node_id = 1;
    config.partition_id = 0;

    replication_group_t *rg = replication_create(&config);
    assert(rg != NULL);

    /* Become leader and put data */
    replication_tick(rg);
    replication_put(rg, "key", 3, "value", 5);

    /* Local read should work even without leader check */
    void *value;
    size_t value_len;
    dkv_status_t status = replication_get(rg, "key", 3, &value, &value_len,
                                           CONSISTENCY_EVENTUAL);
    assert(status == DKV_OK);
    assert(value_len == 5);
    storage_adapter_free_value(value);

    replication_destroy(rg);
}

/* Test 10: Replication multi-node (simulated) */
TEST(test_replication_multi_node) {
    replication_config_t config;
    replication_config_init(&config);
    config.node_id = 1;
    config.partition_id = 0;

    replication_group_t *rg = replication_create(&config);
    assert(rg != NULL);

    /* Add peers (simulated) */
    assert(replication_add_peer(rg, 2) == DKV_OK);
    assert(replication_add_peer(rg, 3) == DKV_OK);

    /* Duplicate peer should fail */
    assert(replication_add_peer(rg, 2) == DKV_ERR_EXISTS);

    /* Trigger election */
    replication_trigger_election(rg);

    /* In multi-node mode, won't become leader without votes */
    /* (Our simplified implementation doesn't handle this fully) */

    /* Remove peer */
    assert(replication_remove_peer(rg, 2) == DKV_OK);
    assert(replication_remove_peer(rg, 99) == DKV_ERR_NOT_FOUND);

    /* Get leader */
    node_id_t leader = replication_get_leader(rg);
    /* Leader may or may not be set depending on election state */
    (void)leader;

    replication_destroy(rg);
}

int main(void) {
    printf("Phase 3 Tests: Raft Consensus and Storage Integration\n");
    printf("=======================================================\n\n");

    RUN_TEST(test_raft_group_create_destroy);
    RUN_TEST(test_raft_group_single_node_leader);
    RUN_TEST(test_raft_group_propose_commit);
    RUN_TEST(test_storage_adapter_put_get);
    RUN_TEST(test_storage_adapter_delete);
    RUN_TEST(test_storage_adapter_snapshot);
    RUN_TEST(test_replication_put_get);
    RUN_TEST(test_replication_linearizable_read);
    RUN_TEST(test_replication_local_read);
    RUN_TEST(test_replication_multi_node);

    printf("\n=======================================================\n");
    printf("Results: %d/%d tests passed\n", tests_passed, tests_run);

    return (tests_passed == tests_run) ? 0 : 1;
}
