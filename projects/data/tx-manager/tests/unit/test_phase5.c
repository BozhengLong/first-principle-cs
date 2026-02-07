/**
 * test_phase5.c - Phase 5 Tests: GC and MVCC Iterator
 *
 * Tests garbage collection and MVCC-aware range scans.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "tx_manager.h"
#include "gc.h"
#include "tx_iter.h"

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

/* Test 1: GC safe timestamp */
static int test_gc_safe_ts(void) {
    const char* path = "/tmp/tx_test_p5_1";
    setup_dir(path);

    tx_manager_t* tm = tx_manager_open(path, NULL);
    if (!tm) {
        cleanup_dir(path);
        return 0;
    }

    /* No active transactions - should return max */
    uint64_t safe = tx_gc_safe_ts(tm);
    int result = (safe == UINT64_MAX);

    /* Start a transaction */
    tx_t* tx = tx_begin(tm);
    uint64_t tx_start = tx->start_ts;

    /* Safe timestamp should be the transaction's start */
    safe = tx_gc_safe_ts(tm);
    result = result && (safe == tx_start);

    tx_commit(tm, tx);
    tx_manager_close(tm);
    cleanup_dir(path);
    return result;
}

/* Test 2: GC run */
static int test_gc_run(void) {
    const char* path = "/tmp/tx_test_p5_2";
    setup_dir(path);

    tx_manager_t* tm = tx_manager_open(path, NULL);
    if (!tm) {
        cleanup_dir(path);
        return 0;
    }

    /* Create multiple versions */
    for (int i = 0; i < 5; i++) {
        tx_t* tx = tx_begin(tm);
        char val[16];
        snprintf(val, sizeof(val), "v%d", i);
        tx_put(tm, tx, "key1", 4, val, strlen(val));
        tx_commit(tm, tx);
    }

    /* Run GC */
    tx_gc_stats_t stats;
    tx_status_t st = tx_gc_run(tm, &stats);

    tx_manager_close(tm);
    cleanup_dir(path);
    return st == TX_OK && stats.versions_scanned > 0;
}

/* Test 3: Iterator basic */
static int test_iter_basic(void) {
    const char* path = "/tmp/tx_test_p5_3";
    setup_dir(path);

    tx_manager_t* tm = tx_manager_open(path, NULL);
    if (!tm) {
        cleanup_dir(path);
        return 0;
    }

    /* Insert some data */
    tx_t* tx1 = tx_begin(tm);
    tx_put(tm, tx1, "a", 1, "1", 1);
    tx_put(tm, tx1, "b", 1, "2", 1);
    tx_put(tm, tx1, "c", 1, "3", 1);
    tx_commit(tm, tx1);

    /* Iterate */
    tx_t* tx2 = tx_begin(tm);
    tx_iter_t* iter = tx_iter_create(tm, tx2);
    if (!iter) {
        tx_commit(tm, tx2);
        tx_manager_close(tm);
        cleanup_dir(path);
        return 0;
    }

    int count = 0;
    tx_iter_seek_to_first(iter);
    while (tx_iter_valid(iter)) {
        count++;
        tx_iter_next(iter);
    }

    tx_iter_destroy(iter);
    tx_commit(tm, tx2);
    tx_manager_close(tm);
    cleanup_dir(path);
    return count == 3;
}

/* Test 4: Iterator sees correct version */
static int test_iter_visibility(void) {
    const char* path = "/tmp/tx_test_p5_4";
    setup_dir(path);

    tx_manager_t* tm = tx_manager_open(path, NULL);
    if (!tm) {
        cleanup_dir(path);
        return 0;
    }

    /* Insert v1 */
    tx_t* tx1 = tx_begin(tm);
    tx_put(tm, tx1, "key", 3, "v1", 2);
    tx_commit(tm, tx1);

    /* Start tx2 (will see v1) */
    tx_t* tx2 = tx_begin(tm);

    /* Insert v2 */
    tx_t* tx3 = tx_begin(tm);
    tx_put(tm, tx3, "key", 3, "v2", 2);
    tx_commit(tm, tx3);

    /* tx2's iterator should see v1 */
    tx_iter_t* iter = tx_iter_create(tm, tx2);
    tx_iter_seek_to_first(iter);

    int result = 0;
    if (tx_iter_valid(iter)) {
        size_t val_len;
        const char* val = tx_iter_value(iter, &val_len);
        result = (val_len == 2 && memcmp(val, "v1", 2) == 0);
    }

    tx_iter_destroy(iter);
    tx_commit(tm, tx2);
    tx_manager_close(tm);
    cleanup_dir(path);
    return result;
}

/* Test 5: Iterator skips tombstones */
static int test_iter_skips_tombstones(void) {
    const char* path = "/tmp/tx_test_p5_5";
    setup_dir(path);

    tx_manager_t* tm = tx_manager_open(path, NULL);
    if (!tm) {
        cleanup_dir(path);
        return 0;
    }

    /* Insert then delete */
    tx_t* tx1 = tx_begin(tm);
    tx_put(tm, tx1, "a", 1, "1", 1);
    tx_put(tm, tx1, "b", 1, "2", 1);
    tx_put(tm, tx1, "c", 1, "3", 1);
    tx_commit(tm, tx1);

    tx_t* tx2 = tx_begin(tm);
    tx_delete(tm, tx2, "b", 1);
    tx_commit(tm, tx2);

    /* Iterator should only see a and c */
    tx_t* tx3 = tx_begin(tm);
    tx_iter_t* iter = tx_iter_create(tm, tx3);

    int count = 0;
    tx_iter_seek_to_first(iter);
    while (tx_iter_valid(iter)) {
        count++;
        tx_iter_next(iter);
    }

    tx_iter_destroy(iter);
    tx_commit(tm, tx3);
    tx_manager_close(tm);
    cleanup_dir(path);
    return count == 2;
}

/* Test 6: Iterator seek */
static int test_iter_seek(void) {
    const char* path = "/tmp/tx_test_p5_6";
    setup_dir(path);

    tx_manager_t* tm = tx_manager_open(path, NULL);
    if (!tm) {
        cleanup_dir(path);
        return 0;
    }

    tx_t* tx1 = tx_begin(tm);
    tx_put(tm, tx1, "aaa", 3, "1", 1);
    tx_put(tm, tx1, "bbb", 3, "2", 1);
    tx_put(tm, tx1, "ccc", 3, "3", 1);
    tx_commit(tm, tx1);

    tx_t* tx2 = tx_begin(tm);
    tx_iter_t* iter = tx_iter_create(tm, tx2);

    /* Seek to bbb */
    tx_iter_seek(iter, "bbb", 3);

    int result = 0;
    if (tx_iter_valid(iter)) {
        size_t key_len;
        const char* key = tx_iter_key(iter, &key_len);
        result = (key_len == 3 && memcmp(key, "bbb", 3) == 0);
    }

    tx_iter_destroy(iter);
    tx_commit(tm, tx2);
    tx_manager_close(tm);
    cleanup_dir(path);
    return result;
}

int main(void) {
    printf("=== Phase 5: GC and MVCC Iterator ===\n");

    TEST(gc_safe_ts);
    TEST(gc_run);
    TEST(iter_basic);
    TEST(iter_visibility);
    TEST(iter_skips_tombstones);
    TEST(iter_seek);

    printf("\nResults: %d/%d tests passed\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
