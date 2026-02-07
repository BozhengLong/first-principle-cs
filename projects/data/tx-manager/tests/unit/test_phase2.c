/**
 * test_phase2.c - Phase 2 Tests: MVCC Read Path
 *
 * Tests snapshot isolation visibility rules.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "tx_manager.h"
#include "visibility.h"

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

/* Test 1: Read committed data from before snapshot */
static int test_read_committed_before_snapshot(void) {
    const char* path = "/tmp/tx_test_p2_1";
    setup_dir(path);

    tx_manager_t* tm = tx_manager_open(path, NULL);
    if (!tm) return 0;

    /* TX1: Write and commit */
    tx_t* tx1 = tx_begin(tm);
    tx_put(tm, tx1, "key1", 4, "value1", 6);
    tx_commit(tm, tx1);

    /* TX2: Should see TX1's data */
    tx_t* tx2 = tx_begin(tm);
    char* value = NULL;
    size_t value_len = 0;
    tx_status_t st = tx_get(tm, tx2, "key1", 4, &value, &value_len);
    int result = (st == TX_OK && value_len == 6 &&
                  memcmp(value, "value1", 6) == 0);
    free(value);

    tx_commit(tm, tx2);
    tx_manager_close(tm);
    cleanup_dir(path);
    return result;
}

/* Test 2: Cannot read uncommitted data */
static int test_cannot_read_uncommitted(void) {
    const char* path = "/tmp/tx_test_p2_2";
    setup_dir(path);

    tx_manager_t* tm = tx_manager_open(path, NULL);
    if (!tm) return 0;

    /* TX1: Write but don't commit yet */
    tx_t* tx1 = tx_begin(tm);
    tx_put(tm, tx1, "key1", 4, "value1", 6);
    /* tx1 still active */

    /* TX2: Should NOT see TX1's uncommitted data */
    tx_t* tx2 = tx_begin(tm);
    char* value = NULL;
    size_t value_len = 0;
    tx_status_t st = tx_get(tm, tx2, "key1", 4, &value, &value_len);
    int result = (st == TX_NOT_FOUND);
    free(value);

    tx_abort(tm, tx1);
    tx_commit(tm, tx2);
    tx_manager_close(tm);
    cleanup_dir(path);
    return result;
}

/* Test 3: Cannot read data committed after snapshot */
static int test_cannot_read_after_snapshot(void) {
    const char* path = "/tmp/tx_test_p2_3";
    setup_dir(path);

    tx_manager_t* tm = tx_manager_open(path, NULL);
    if (!tm) return 0;

    /* TX1: Start first (gets earlier snapshot) */
    tx_t* tx1 = tx_begin(tm);

    /* TX2: Write and commit */
    tx_t* tx2 = tx_begin(tm);
    tx_put(tm, tx2, "key1", 4, "value1", 6);
    tx_commit(tm, tx2);

    /* TX1: Should NOT see TX2's data (committed after TX1's snapshot) */
    char* value = NULL;
    size_t value_len = 0;
    tx_status_t st = tx_get(tm, tx1, "key1", 4, &value, &value_len);
    int result = (st == TX_NOT_FOUND);
    free(value);

    tx_commit(tm, tx1);
    tx_manager_close(tm);
    cleanup_dir(path);
    return result;
}

/* Test 4: Read your own writes */
static int test_read_your_writes(void) {
    const char* path = "/tmp/tx_test_p2_4";
    setup_dir(path);

    tx_manager_t* tm = tx_manager_open(path, NULL);
    if (!tm) return 0;

    tx_t* tx = tx_begin(tm);

    /* Write */
    tx_put(tm, tx, "key1", 4, "value1", 6);

    /* Read back (should see own write even though not committed) */
    char* value = NULL;
    size_t value_len = 0;
    tx_status_t st = tx_get(tm, tx, "key1", 4, &value, &value_len);
    int result = (st == TX_OK && value_len == 6 &&
                  memcmp(value, "value1", 6) == 0);
    free(value);

    tx_commit(tm, tx);
    tx_manager_close(tm);
    cleanup_dir(path);
    return result;
}

/* Test 5: Read your own deletes */
static int test_read_your_deletes(void) {
    const char* path = "/tmp/tx_test_p2_5";
    setup_dir(path);

    tx_manager_t* tm = tx_manager_open(path, NULL);
    if (!tm) return 0;

    /* First commit some data */
    tx_t* tx1 = tx_begin(tm);
    tx_put(tm, tx1, "key1", 4, "value1", 6);
    tx_commit(tm, tx1);

    /* TX2: Delete and verify not visible */
    tx_t* tx2 = tx_begin(tm);

    /* Should see the data first */
    char* value = NULL;
    size_t value_len = 0;
    tx_status_t st = tx_get(tm, tx2, "key1", 4, &value, &value_len);
    if (st != TX_OK) {
        tx_abort(tm, tx2);
        tx_manager_close(tm);
        return 0;
    }
    free(value);

    /* Delete it */
    tx_delete(tm, tx2, "key1", 4);

    /* Should NOT see it anymore */
    st = tx_get(tm, tx2, "key1", 4, &value, &value_len);
    int result = (st == TX_NOT_FOUND);
    free(value);

    tx_commit(tm, tx2);
    tx_manager_close(tm);
    cleanup_dir(path);
    return result;
}

/* Test 6: Multiple versions - read correct one */
static int test_multiple_versions(void) {
    const char* path = "/tmp/tx_test_p2_6";
    setup_dir(path);

    tx_manager_t* tm = tx_manager_open(path, NULL);
    if (!tm) return 0;

    /* TX1: Write version 1 */
    tx_t* tx1 = tx_begin(tm);
    tx_put(tm, tx1, "key1", 4, "v1", 2);
    tx_commit(tm, tx1);

    /* TX2: Start (should see v1) */
    tx_t* tx2 = tx_begin(tm);

    /* TX3: Write version 2 */
    tx_t* tx3 = tx_begin(tm);
    tx_put(tm, tx3, "key1", 4, "v2", 2);
    tx_commit(tm, tx3);

    /* TX2: Should still see v1 (snapshot isolation) */
    char* value = NULL;
    size_t value_len = 0;
    tx_status_t st = tx_get(tm, tx2, "key1", 4, &value, &value_len);
    int result = (st == TX_OK && value_len == 2 &&
                  memcmp(value, "v1", 2) == 0);
    free(value);

    /* TX4: Should see v2 */
    tx_t* tx4 = tx_begin(tm);
    st = tx_get(tm, tx4, "key1", 4, &value, &value_len);
    if (st != TX_OK || value_len != 2 || memcmp(value, "v2", 2) != 0) {
        result = 0;
    }
    free(value);

    tx_commit(tm, tx2);
    tx_commit(tm, tx4);
    tx_manager_close(tm);
    cleanup_dir(path);
    return result;
}

/* Test 7: Visibility helper function */
static int test_visibility_function(void) {
    const char* path = "/tmp/tx_test_p2_7";
    setup_dir(path);

    tx_manager_t* tm = tx_manager_open(path, NULL);
    if (!tm) return 0;

    tx_t* tx = tx_begin(tm);
    uint64_t snapshot = tx->start_ts;

    /* Version before snapshot should be visible */
    int result = is_version_visible(snapshot - 1, tx);

    /* Version at snapshot should be visible */
    result = result && is_version_visible(snapshot, tx);

    /* Version after snapshot should NOT be visible */
    result = result && !is_version_visible(snapshot + 1, tx);

    tx_commit(tm, tx);
    tx_manager_close(tm);
    cleanup_dir(path);
    return result;
}

/* Test 8: Overwrite in same transaction */
static int test_overwrite_same_tx(void) {
    const char* path = "/tmp/tx_test_p2_8";
    setup_dir(path);

    tx_manager_t* tm = tx_manager_open(path, NULL);
    if (!tm) return 0;

    tx_t* tx = tx_begin(tm);

    /* Write v1 */
    tx_put(tm, tx, "key1", 4, "v1", 2);

    /* Overwrite with v2 */
    tx_put(tm, tx, "key1", 4, "v2", 2);

    /* Should see v2 */
    char* value = NULL;
    size_t value_len = 0;
    tx_status_t st = tx_get(tm, tx, "key1", 4, &value, &value_len);
    int result = (st == TX_OK && value_len == 2 &&
                  memcmp(value, "v2", 2) == 0);
    free(value);

    tx_commit(tm, tx);
    tx_manager_close(tm);
    cleanup_dir(path);
    return result;
}

int main(void) {
    printf("=== Phase 2: MVCC Read Path ===\n");

    TEST(read_committed_before_snapshot);
    TEST(cannot_read_uncommitted);
    TEST(cannot_read_after_snapshot);
    TEST(read_your_writes);
    TEST(read_your_deletes);
    TEST(multiple_versions);
    TEST(visibility_function);
    TEST(overwrite_same_tx);

    printf("\nResults: %d/%d tests passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
