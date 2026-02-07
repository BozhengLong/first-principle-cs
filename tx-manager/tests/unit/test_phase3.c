/**
 * test_phase3.c - Phase 3 Tests: MVCC Write Path
 *
 * Tests write-write conflict detection and first-committer-wins.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "tx_manager.h"
#include "conflict.h"

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { \
    printf("  Testing %s... ", #name); \
    fflush(stdout); \
    tests_run++; \
    if (test_##name()) { \
        printf("PASSED\n"); \
        tests_passed++; \
    } else { \
        printf("FAILED\n"); \
    } \
} while(0)

static void cleanup_dir(const char* path) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", path);
    system(cmd);
}

static void setup_dir(const char* path) {
    cleanup_dir(path);
    mkdir(path, 0755);
}

/* Test 1: No conflict on different keys */
static int test_no_conflict_different_keys(void) {
    const char* path = "/tmp/tx_test_p3_1";
    setup_dir(path);

    tx_manager_t* tm = tx_manager_open(path, NULL);
    if (!tm) return 0;

    /* TX1 and TX2 start concurrently */
    tx_t* tx1 = tx_begin(tm);
    tx_t* tx2 = tx_begin(tm);

    /* TX1 writes key1, TX2 writes key2 */
    tx_put(tm, tx1, "key1", 4, "value1", 6);
    tx_put(tm, tx2, "key2", 4, "value2", 6);

    /* Both should commit successfully */
    tx_status_t st1 = tx_commit(tm, tx1);
    tx_status_t st2 = tx_commit(tm, tx2);

    tx_manager_close(tm);
    cleanup_dir(path);
    return st1 == TX_OK && st2 == TX_OK;
}

/* Test 2: First committer wins on same key */
static int test_first_committer_wins(void) {
    const char* path = "/tmp/tx_test_p3_2";
    setup_dir(path);

    tx_manager_t* tm = tx_manager_open(path, NULL);
    if (!tm) return 0;

    /* TX1 and TX2 start concurrently */
    tx_t* tx1 = tx_begin(tm);
    tx_t* tx2 = tx_begin(tm);

    /* Both write to same key */
    tx_put(tm, tx1, "key1", 4, "value1", 6);
    tx_put(tm, tx2, "key1", 4, "value2", 6);

    /* TX1 commits first - should succeed */
    tx_status_t st1 = tx_commit(tm, tx1);

    /* TX2 commits second - should fail with conflict */
    tx_status_t st2 = tx_commit(tm, tx2);

    tx_manager_close(tm);
    cleanup_dir(path);
    return st1 == TX_OK && st2 == TX_CONFLICT;
}

/* Test 3: Aborted transaction's writes not visible */
static int test_aborted_writes_invisible(void) {
    const char* path = "/tmp/tx_test_p3_3";
    setup_dir(path);

    tx_manager_t* tm = tx_manager_open(path, NULL);
    if (!tm) return 0;

    /* TX1 writes and aborts */
    tx_t* tx1 = tx_begin(tm);
    tx_put(tm, tx1, "key1", 4, "value1", 6);
    tx_abort(tm, tx1);

    /* TX2 should not see TX1's write */
    tx_t* tx2 = tx_begin(tm);
    char* value = NULL;
    size_t value_len = 0;
    tx_status_t st = tx_get(tm, tx2, "key1", 4, &value, &value_len);
    int result = (st == TX_NOT_FOUND);
    free(value);

    tx_commit(tm, tx2);
    tx_manager_close(tm);
    cleanup_dir(path);
    return result;
}

/* Test 4: Conflict on update after read */
static int test_conflict_update_after_read(void) {
    const char* path = "/tmp/tx_test_p3_4";
    setup_dir(path);

    tx_manager_t* tm = tx_manager_open(path, NULL);
    if (!tm) return 0;

    /* Initial data */
    tx_t* tx0 = tx_begin(tm);
    tx_put(tm, tx0, "key1", 4, "v0", 2);
    tx_commit(tm, tx0);

    /* TX1 reads and plans to update */
    tx_t* tx1 = tx_begin(tm);
    char* value = NULL;
    size_t value_len = 0;
    tx_get(tm, tx1, "key1", 4, &value, &value_len);
    free(value);

    /* TX2 updates and commits first */
    tx_t* tx2 = tx_begin(tm);
    tx_put(tm, tx2, "key1", 4, "v2", 2);
    tx_commit(tm, tx2);

    /* TX1 tries to update - should conflict */
    tx_put(tm, tx1, "key1", 4, "v1", 2);
    tx_status_t st = tx_commit(tm, tx1);

    tx_manager_close(tm);
    cleanup_dir(path);
    return st == TX_CONFLICT;
}

/* Test 5: No conflict if no write overlap */
static int test_no_conflict_read_only(void) {
    const char* path = "/tmp/tx_test_p3_5";
    setup_dir(path);

    tx_manager_t* tm = tx_manager_open(path, NULL);
    if (!tm) return 0;

    /* Initial data */
    tx_t* tx0 = tx_begin(tm);
    tx_put(tm, tx0, "key1", 4, "v0", 2);
    tx_commit(tm, tx0);

    /* TX1 only reads */
    tx_t* tx1 = tx_begin(tm);
    char* value = NULL;
    size_t value_len = 0;
    tx_get(tm, tx1, "key1", 4, &value, &value_len);
    free(value);

    /* TX2 updates and commits */
    tx_t* tx2 = tx_begin(tm);
    tx_put(tm, tx2, "key1", 4, "v2", 2);
    tx_commit(tm, tx2);

    /* TX1 commits (read-only) - should succeed */
    tx_status_t st = tx_commit(tm, tx1);

    tx_manager_close(tm);
    cleanup_dir(path);
    return st == TX_OK;
}

/* Test 6: Multiple keys, partial conflict */
static int test_partial_conflict(void) {
    const char* path = "/tmp/tx_test_p3_6";
    setup_dir(path);

    tx_manager_t* tm = tx_manager_open(path, NULL);
    if (!tm) return 0;

    /* TX1 and TX2 start */
    tx_t* tx1 = tx_begin(tm);
    tx_t* tx2 = tx_begin(tm);

    /* TX1 writes key1 and key2 */
    tx_put(tm, tx1, "key1", 4, "v1a", 3);
    tx_put(tm, tx1, "key2", 4, "v2a", 3);

    /* TX2 writes key2 and key3 (overlaps on key2) */
    tx_put(tm, tx2, "key2", 4, "v2b", 3);
    tx_put(tm, tx2, "key3", 4, "v3b", 3);

    /* TX1 commits first */
    tx_status_t st1 = tx_commit(tm, tx1);

    /* TX2 should conflict on key2 */
    tx_status_t st2 = tx_commit(tm, tx2);

    tx_manager_close(tm);
    cleanup_dir(path);
    return st1 == TX_OK && st2 == TX_CONFLICT;
}

/* Test 7: Verify committed value after conflict */
static int test_verify_after_conflict(void) {
    const char* path = "/tmp/tx_test_p3_7";
    setup_dir(path);

    tx_manager_t* tm = tx_manager_open(path, NULL);
    if (!tm) return 0;

    /* TX1 and TX2 start */
    tx_t* tx1 = tx_begin(tm);
    tx_t* tx2 = tx_begin(tm);

    tx_put(tm, tx1, "key1", 4, "winner", 6);
    tx_put(tm, tx2, "key1", 4, "loser", 5);

    tx_commit(tm, tx1);  /* Winner */
    tx_commit(tm, tx2);  /* Loser (conflict) */

    /* TX3 should see winner's value */
    tx_t* tx3 = tx_begin(tm);
    char* value = NULL;
    size_t value_len = 0;
    tx_status_t st = tx_get(tm, tx3, "key1", 4, &value, &value_len);
    int result = (st == TX_OK && value_len == 6 &&
                  memcmp(value, "winner", 6) == 0);
    free(value);

    tx_commit(tm, tx3);
    tx_manager_close(tm);
    cleanup_dir(path);
    return result;
}

/* Test 8: Delete causes conflict */
static int test_delete_conflict(void) {
    const char* path = "/tmp/tx_test_p3_8";
    setup_dir(path);

    tx_manager_t* tm = tx_manager_open(path, NULL);
    if (!tm) return 0;

    /* Initial data */
    tx_t* tx0 = tx_begin(tm);
    tx_put(tm, tx0, "key1", 4, "v0", 2);
    tx_commit(tm, tx0);

    /* TX1 and TX2 start */
    tx_t* tx1 = tx_begin(tm);
    tx_t* tx2 = tx_begin(tm);

    /* TX1 deletes */
    tx_delete(tm, tx1, "key1", 4);

    /* TX2 updates */
    tx_put(tm, tx2, "key1", 4, "v2", 2);

    /* TX1 commits first */
    tx_status_t st1 = tx_commit(tm, tx1);

    /* TX2 should conflict */
    tx_status_t st2 = tx_commit(tm, tx2);

    tx_manager_close(tm);
    cleanup_dir(path);
    return st1 == TX_OK && st2 == TX_CONFLICT;
}

int main(void) {
    printf("=== Phase 3: MVCC Write Path ===\n");

    TEST(no_conflict_different_keys);
    TEST(first_committer_wins);
    TEST(aborted_writes_invisible);
    TEST(conflict_update_after_read);
    TEST(no_conflict_read_only);
    TEST(partial_conflict);
    TEST(verify_after_conflict);
    TEST(delete_conflict);

    printf("\nResults: %d/%d tests passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
