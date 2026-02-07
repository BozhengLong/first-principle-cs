/**
 * test_phase1.c - Phase 1 Tests: Basic Transaction Framework
 *
 * Tests transaction lifecycle, basic put/get, commit, and abort.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>

#include "tx_manager.h"

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

/* Test 1: Transaction manager open/close */
static int test_manager_lifecycle(void) {
    const char* path = "/tmp/tx_test_1";
    setup_dir(path);

    tx_manager_t* tm = tx_manager_open(path, NULL);
    if (!tm) return 0;

    tx_manager_close(tm);
    cleanup_dir(path);
    return 1;
}

/* Test 2: Transaction begin/abort */
static int test_tx_begin_abort(void) {
    const char* path = "/tmp/tx_test_2";
    setup_dir(path);

    tx_manager_t* tm = tx_manager_open(path, NULL);
    if (!tm) return 0;

    tx_t* tx = tx_begin(tm);
    if (!tx) {
        tx_manager_close(tm);
        return 0;
    }

    if (tx->state != TX_STATE_ACTIVE) {
        tx_abort(tm, tx);
        tx_manager_close(tm);
        return 0;
    }

    tx_status_t st = tx_abort(tm, tx);
    tx_manager_close(tm);
    cleanup_dir(path);
    return st == TX_OK;
}

/* Test 3: Transaction begin/commit (empty) */
static int test_tx_begin_commit_empty(void) {
    const char* path = "/tmp/tx_test_3";
    setup_dir(path);

    tx_manager_t* tm = tx_manager_open(path, NULL);
    if (!tm) return 0;

    tx_t* tx = tx_begin(tm);
    if (!tx) {
        tx_manager_close(tm);
        return 0;
    }

    tx_status_t st = tx_commit(tm, tx);
    tx_manager_close(tm);
    cleanup_dir(path);
    return st == TX_OK;
}

/* Test 4: Basic put and get within same transaction */
static int test_put_get_same_tx(void) {
    const char* path = "/tmp/tx_test_4";
    setup_dir(path);

    tx_manager_t* tm = tx_manager_open(path, NULL);
    if (!tm) return 0;

    tx_t* tx = tx_begin(tm);
    if (!tx) {
        tx_manager_close(tm);
        return 0;
    }

    /* Put a value */
    tx_status_t st = tx_put(tm, tx, "key1", 4, "value1", 6);
    if (st != TX_OK) {
        tx_abort(tm, tx);
        tx_manager_close(tm);
        return 0;
    }

    /* Get it back (read-your-writes) */
    char* value = NULL;
    size_t value_len = 0;
    st = tx_get(tm, tx, "key1", 4, &value, &value_len);
    if (st != TX_OK || value_len != 6 || memcmp(value, "value1", 6) != 0) {
        free(value);
        tx_abort(tm, tx);
        tx_manager_close(tm);
        return 0;
    }
    free(value);

    tx_commit(tm, tx);
    tx_manager_close(tm);
    cleanup_dir(path);
    return 1;
}

/* Test 5: Commit persists data for next transaction */
static int test_commit_persists(void) {
    const char* path = "/tmp/tx_test_5";
    setup_dir(path);

    tx_manager_t* tm = tx_manager_open(path, NULL);
    if (!tm) return 0;

    /* Transaction 1: write data */
    tx_t* tx1 = tx_begin(tm);
    if (!tx1) {
        tx_manager_close(tm);
        return 0;
    }

    tx_put(tm, tx1, "key1", 4, "value1", 6);
    tx_commit(tm, tx1);

    /* Transaction 2: read data */
    tx_t* tx2 = tx_begin(tm);
    if (!tx2) {
        tx_manager_close(tm);
        return 0;
    }

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

/* Test 6: Abort discards writes */
static int test_abort_discards(void) {
    const char* path = "/tmp/tx_test_6";
    setup_dir(path);

    tx_manager_t* tm = tx_manager_open(path, NULL);
    if (!tm) return 0;

    /* Transaction 1: write and abort */
    tx_t* tx1 = tx_begin(tm);
    if (!tx1) {
        tx_manager_close(tm);
        return 0;
    }

    tx_put(tm, tx1, "key1", 4, "value1", 6);
    tx_abort(tm, tx1);

    /* Transaction 2: should not see the data */
    tx_t* tx2 = tx_begin(tm);
    if (!tx2) {
        tx_manager_close(tm);
        return 0;
    }

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

/* Test 7: Delete within transaction */
static int test_delete_in_tx(void) {
    const char* path = "/tmp/tx_test_7";
    setup_dir(path);

    tx_manager_t* tm = tx_manager_open(path, NULL);
    if (!tm) return 0;

    tx_t* tx = tx_begin(tm);
    if (!tx) {
        tx_manager_close(tm);
        return 0;
    }

    /* Put then delete */
    tx_put(tm, tx, "key1", 4, "value1", 6);
    tx_delete(tm, tx, "key1", 4);

    /* Should not find it */
    char* value = NULL;
    size_t value_len = 0;
    tx_status_t st = tx_get(tm, tx, "key1", 4, &value, &value_len);
    int result = (st == TX_NOT_FOUND);
    free(value);

    tx_commit(tm, tx);
    tx_manager_close(tm);
    cleanup_dir(path);
    return result;
}

/* Test 8: Multiple keys in one transaction */
static int test_multiple_keys(void) {
    const char* path = "/tmp/tx_test_8";
    setup_dir(path);

    tx_manager_t* tm = tx_manager_open(path, NULL);
    if (!tm) return 0;

    tx_t* tx = tx_begin(tm);
    if (!tx) {
        tx_manager_close(tm);
        return 0;
    }

    tx_put(tm, tx, "key1", 4, "value1", 6);
    tx_put(tm, tx, "key2", 4, "value2", 6);
    tx_put(tm, tx, "key3", 4, "value3", 6);

    /* Verify all keys */
    char* value = NULL;
    size_t value_len = 0;
    int result = 1;

    if (tx_get(tm, tx, "key1", 4, &value, &value_len) != TX_OK ||
        memcmp(value, "value1", 6) != 0) result = 0;
    free(value);

    if (tx_get(tm, tx, "key2", 4, &value, &value_len) != TX_OK ||
        memcmp(value, "value2", 6) != 0) result = 0;
    free(value);

    if (tx_get(tm, tx, "key3", 4, &value, &value_len) != TX_OK ||
        memcmp(value, "value3", 6) != 0) result = 0;
    free(value);

    tx_commit(tm, tx);
    tx_manager_close(tm);
    cleanup_dir(path);
    return result;
}

int main(void) {
    printf("=== Phase 1: Basic Transaction Framework ===\n");

    TEST(manager_lifecycle);
    TEST(tx_begin_abort);
    TEST(tx_begin_commit_empty);
    TEST(put_get_same_tx);
    TEST(commit_persists);
    TEST(abort_discards);
    TEST(delete_in_tx);
    TEST(multiple_keys);

    printf("\nResults: %d/%d tests passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
