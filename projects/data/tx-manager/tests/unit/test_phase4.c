/**
 * test_phase4.c - Phase 4 Tests: Crash Recovery
 *
 * Tests WAL logging and crash recovery.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "tx_manager.h"
#include "tx_wal.h"
#include "recovery.h"

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

/* Test 1: WAL open/close */
static int test_wal_lifecycle(void) {
    const char* path = "/tmp/tx_test_p4_1";
    setup_dir(path);

    tx_wal_t* wal = tx_wal_open(path);
    if (!wal) {
        cleanup_dir(path);
        return 0;
    }

    tx_wal_close(wal);
    cleanup_dir(path);
    return 1;
}

/* Test 2: WAL log begin/commit */
static int test_wal_log_commit(void) {
    const char* path = "/tmp/tx_test_p4_2";
    setup_dir(path);

    tx_wal_t* wal = tx_wal_open(path);
    if (!wal) {
        cleanup_dir(path);
        return 0;
    }

    /* Create a fake transaction */
    tx_t* tx = tx_create(1, 1);
    if (!tx) {
        tx_wal_close(wal);
        cleanup_dir(path);
        return 0;
    }

    tx_status_t st = tx_wal_log_begin(wal, tx);
    if (st != TX_OK) {
        tx_destroy(tx);
        tx_wal_close(wal);
        cleanup_dir(path);
        return 0;
    }

    tx->commit_ts = 2;
    st = tx_wal_log_commit(wal, tx);

    tx_destroy(tx);
    tx_wal_close(wal);
    cleanup_dir(path);
    return st == TX_OK;
}

/* Test 3: Recovery with no WAL */
static int test_recovery_no_wal(void) {
    const char* path = "/tmp/tx_test_p4_3";
    setup_dir(path);

    tx_manager_t* tm = tx_manager_open(path, NULL);
    if (!tm) {
        cleanup_dir(path);
        return 0;
    }

    tx_recovery_result_t result;
    tx_status_t st = tx_recover(tm, &result);

    tx_manager_close(tm);
    cleanup_dir(path);
    return st == TX_OK && result.committed_count == 0;
}

/* Test 4: Recovery restores tx_id counter */
static int test_recovery_restores_counters(void) {
    const char* path = "/tmp/tx_test_p4_4";
    setup_dir(path);

    /* First session: create transactions */
    tx_manager_t* tm1 = tx_manager_open(path, NULL);
    if (!tm1) {
        cleanup_dir(path);
        return 0;
    }

    /* Create and commit several transactions */
    for (int i = 0; i < 5; i++) {
        tx_t* tx = tx_begin(tm1);
        tx_put(tm1, tx, "key", 3, "val", 3);
        tx_commit(tm1, tx);
    }

    uint64_t last_tx_id = tm1->next_tx_id;
    tx_manager_close(tm1);

    /* Second session: recover */
    tx_manager_t* tm2 = tx_manager_open(path, NULL);
    if (!tm2) {
        cleanup_dir(path);
        return 0;
    }

    tx_recovery_result_t result;
    tx_recover(tm2, &result);

    /* New transaction should have higher ID */
    tx_t* tx = tx_begin(tm2);
    int success = (tx->tx_id >= last_tx_id);

    tx_commit(tm2, tx);
    tx_manager_close(tm2);
    cleanup_dir(path);
    return success;
}

/* Test 5: Committed data survives restart */
static int test_committed_survives_restart(void) {
    const char* path = "/tmp/tx_test_p4_5";
    setup_dir(path);

    /* First session: write data */
    tx_manager_t* tm1 = tx_manager_open(path, NULL);
    if (!tm1) {
        cleanup_dir(path);
        return 0;
    }

    tx_t* tx1 = tx_begin(tm1);
    tx_put(tm1, tx1, "key1", 4, "value1", 6);
    tx_commit(tm1, tx1);
    tx_manager_close(tm1);

    /* Second session: read data */
    tx_manager_t* tm2 = tx_manager_open(path, NULL);
    if (!tm2) {
        cleanup_dir(path);
        return 0;
    }

    tx_recover(tm2, NULL);

    tx_t* tx2 = tx_begin(tm2);
    char* value = NULL;
    size_t value_len = 0;
    tx_status_t st = tx_get(tm2, tx2, "key1", 4, &value, &value_len);
    int result = (st == TX_OK && value_len == 6 &&
                  memcmp(value, "value1", 6) == 0);
    free(value);

    tx_commit(tm2, tx2);
    tx_manager_close(tm2);
    cleanup_dir(path);
    return result;
}

/* Test 6: Multiple transactions survive restart */
static int test_multiple_tx_survive(void) {
    const char* path = "/tmp/tx_test_p4_6";
    setup_dir(path);

    /* First session */
    tx_manager_t* tm1 = tx_manager_open(path, NULL);
    if (!tm1) {
        cleanup_dir(path);
        return 0;
    }

    tx_t* tx1 = tx_begin(tm1);
    tx_put(tm1, tx1, "k1", 2, "v1", 2);
    tx_commit(tm1, tx1);

    tx_t* tx2 = tx_begin(tm1);
    tx_put(tm1, tx2, "k2", 2, "v2", 2);
    tx_commit(tm1, tx2);

    tx_t* tx3 = tx_begin(tm1);
    tx_put(tm1, tx3, "k3", 2, "v3", 2);
    tx_commit(tm1, tx3);

    tx_manager_close(tm1);

    /* Second session */
    tx_manager_t* tm2 = tx_manager_open(path, NULL);
    if (!tm2) {
        cleanup_dir(path);
        return 0;
    }

    tx_recover(tm2, NULL);

    tx_t* tx = tx_begin(tm2);
    char* value = NULL;
    size_t value_len = 0;
    int result = 1;

    if (tx_get(tm2, tx, "k1", 2, &value, &value_len) != TX_OK ||
        memcmp(value, "v1", 2) != 0) result = 0;
    free(value);

    if (tx_get(tm2, tx, "k2", 2, &value, &value_len) != TX_OK ||
        memcmp(value, "v2", 2) != 0) result = 0;
    free(value);

    if (tx_get(tm2, tx, "k3", 2, &value, &value_len) != TX_OK ||
        memcmp(value, "v3", 2) != 0) result = 0;
    free(value);

    tx_commit(tm2, tx);
    tx_manager_close(tm2);
    cleanup_dir(path);
    return result;
}

int main(void) {
    printf("=== Phase 4: Crash Recovery ===\n");

    TEST(wal_lifecycle);
    TEST(wal_log_commit);
    TEST(recovery_no_wal);
    TEST(recovery_restores_counters);
    TEST(committed_survives_restart);
    TEST(multiple_tx_survive);

    printf("\nResults: %d/%d tests passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
