#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>
#include "../../src/crc32.h"
#include "../../src/wal.h"
#include "../../src/storage.h"

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) static void test_##name(void)
#define RUN_TEST(name) do { \
    printf("  Running %s... ", #name); \
    test_##name(); \
    printf("PASSED\n"); \
    tests_passed++; \
} while(0)

#define ASSERT(cond) do { \
    if (!(cond)) { \
        printf("FAILED at %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        tests_failed++; \
        return; \
    } \
} while(0)

#define ASSERT_EQ(a, b) ASSERT((a) == (b))
#define ASSERT_NE(a, b) ASSERT((a) != (b))
#define ASSERT_STR_EQ(a, b, len) ASSERT(memcmp(a, b, len) == 0)

// Helper: remove directory recursively
static void remove_dir(const char* path) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", path);
    system(cmd);
}

// ============================================================
// CRC32 Tests
// ============================================================

TEST(crc32_basic) {
    // Test with known values
    const char* data = "hello";
    uint32_t crc = crc32(data, 5);
    ASSERT_NE(crc, 0);

    // Same data should produce same CRC
    uint32_t crc2 = crc32(data, 5);
    ASSERT_EQ(crc, crc2);

    // Different data should produce different CRC
    const char* data2 = "world";
    uint32_t crc3 = crc32(data2, 5);
    ASSERT_NE(crc, crc3);
}

TEST(crc32_incremental) {
    // Full CRC
    const char* data = "hello world";
    uint32_t full_crc = crc32(data, 11);

    // Incremental CRC
    uint32_t inc_crc = crc32_update(0, "hello ", 6);
    inc_crc = crc32_update(inc_crc, "world", 5);

    ASSERT_EQ(full_crc, inc_crc);
}

// ============================================================
// WAL Tests
// ============================================================

TEST(wal_open_close) {
    const char* path = "test_wal_open.wal";
    unlink(path);

    wal_t* wal = wal_open(path, false);
    ASSERT_NE(wal, NULL);

    wal_close(wal);
    unlink(path);
}

TEST(wal_write_put) {
    const char* path = "test_wal_put.wal";
    unlink(path);

    wal_t* wal = wal_open(path, false);
    ASSERT_NE(wal, NULL);

    ASSERT_EQ(wal_write_put(wal, "key1", 4, "value1", 6), STATUS_OK);
    ASSERT_EQ(wal_write_put(wal, "key2", 4, "value2", 6), STATUS_OK);

    wal_close(wal);
    unlink(path);
}

TEST(wal_write_delete) {
    const char* path = "test_wal_delete.wal";
    unlink(path);

    wal_t* wal = wal_open(path, false);
    ASSERT_NE(wal, NULL);

    ASSERT_EQ(wal_write_put(wal, "key1", 4, "value1", 6), STATUS_OK);
    ASSERT_EQ(wal_write_delete(wal, "key1", 4), STATUS_OK);

    wal_close(wal);
    unlink(path);
}

// Recovery context for testing
typedef struct {
    int put_count;
    int delete_count;
    char last_key[64];
    char last_val[64];
} recover_ctx_t;

static status_t test_recover_fn(void* ctx, wal_record_type_t type,
                                 const char* key, size_t key_len,
                                 const char* val, size_t val_len) {
    recover_ctx_t* rctx = (recover_ctx_t*)ctx;

    if (type == WAL_RECORD_PUT) {
        rctx->put_count++;
        memcpy(rctx->last_key, key, key_len);
        rctx->last_key[key_len] = '\0';
        memcpy(rctx->last_val, val, val_len);
        rctx->last_val[val_len] = '\0';
    } else if (type == WAL_RECORD_DELETE) {
        rctx->delete_count++;
        memcpy(rctx->last_key, key, key_len);
        rctx->last_key[key_len] = '\0';
    }
    return STATUS_OK;
}

TEST(wal_recover) {
    const char* path = "test_wal_recover.wal";
    unlink(path);

    // Write some records
    wal_t* wal = wal_open(path, false);
    ASSERT_NE(wal, NULL);

    ASSERT_EQ(wal_write_put(wal, "key1", 4, "value1", 6), STATUS_OK);
    ASSERT_EQ(wal_write_put(wal, "key2", 4, "value2", 6), STATUS_OK);
    ASSERT_EQ(wal_write_delete(wal, "key1", 4), STATUS_OK);

    wal_close(wal);

    // Recover and verify
    recover_ctx_t ctx = {0};
    ASSERT_EQ(wal_recover(path, test_recover_fn, &ctx), STATUS_OK);

    ASSERT_EQ(ctx.put_count, 2);
    ASSERT_EQ(ctx.delete_count, 1);
    ASSERT_STR_EQ(ctx.last_key, "key1", 4);

    unlink(path);
}

// ============================================================
// Storage Persistence Tests
// ============================================================

TEST(storage_persistence) {
    const char* db_path = "test_storage_persist";
    remove_dir(db_path);

    // Open, write, close
    storage_t* db = storage_open(db_path, NULL);
    ASSERT_NE(db, NULL);

    ASSERT_EQ(storage_put(db, "key1", 4, "value1", 6), STATUS_OK);
    ASSERT_EQ(storage_put(db, "key2", 4, "value2", 6), STATUS_OK);

    storage_close(db);

    // Reopen and verify data persisted
    db = storage_open(db_path, NULL);
    ASSERT_NE(db, NULL);

    char* val;
    size_t val_len;
    ASSERT_EQ(storage_get(db, "key1", 4, &val, &val_len), STATUS_OK);
    ASSERT_EQ(val_len, 6);
    ASSERT_STR_EQ(val, "value1", 6);

    ASSERT_EQ(storage_get(db, "key2", 4, &val, &val_len), STATUS_OK);
    ASSERT_STR_EQ(val, "value2", 6);

    storage_close(db);
    remove_dir(db_path);
}

TEST(storage_crash_recovery) {
    const char* db_path = "test_storage_crash";
    remove_dir(db_path);

    // Open, write some data
    storage_t* db = storage_open(db_path, NULL);
    ASSERT_NE(db, NULL);

    ASSERT_EQ(storage_put(db, "key1", 4, "value1", 6), STATUS_OK);
    ASSERT_EQ(storage_put(db, "key2", 4, "value2", 6), STATUS_OK);
    ASSERT_EQ(storage_delete(db, "key1", 4), STATUS_OK);
    ASSERT_EQ(storage_put(db, "key3", 4, "value3", 6), STATUS_OK);

    storage_close(db);

    // Reopen (simulates recovery after crash)
    db = storage_open(db_path, NULL);
    ASSERT_NE(db, NULL);

    char* val;
    size_t val_len;

    // key1 should be deleted
    ASSERT_EQ(storage_get(db, "key1", 4, &val, &val_len), STATUS_NOT_FOUND);

    // key2 and key3 should exist
    ASSERT_EQ(storage_get(db, "key2", 4, &val, &val_len), STATUS_OK);
    ASSERT_STR_EQ(val, "value2", 6);

    ASSERT_EQ(storage_get(db, "key3", 4, &val, &val_len), STATUS_OK);
    ASSERT_STR_EQ(val, "value3", 6);

    storage_close(db);
    remove_dir(db_path);
}

// ============================================================
// Main
// ============================================================

int main(void) {
    printf("Phase 2 Tests: CRC32, WAL, Storage Persistence\n");
    printf("================================================\n\n");

    printf("CRC32 Tests:\n");
    RUN_TEST(crc32_basic);
    RUN_TEST(crc32_incremental);

    printf("\nWAL Tests:\n");
    RUN_TEST(wal_open_close);
    RUN_TEST(wal_write_put);
    RUN_TEST(wal_write_delete);
    RUN_TEST(wal_recover);

    printf("\nStorage Persistence Tests:\n");
    RUN_TEST(storage_persistence);
    RUN_TEST(storage_crash_recovery);

    printf("\n================================================\n");
    printf("Results: %d passed, %d failed\n", tests_passed, tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
