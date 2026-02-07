#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>
#include "../../src/bloom.h"
#include "../../src/sstable.h"
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
// Bloom Filter Tests
// ============================================================

TEST(bloom_basic) {
    bloom_filter_t* bf = bloom_create(100);
    ASSERT_NE(bf, NULL);

    // Add some keys
    bloom_add(bf, "key1", 4);
    bloom_add(bf, "key2", 4);
    bloom_add(bf, "key3", 4);

    // Check they may be present
    ASSERT(bloom_may_contain(bf, "key1", 4));
    ASSERT(bloom_may_contain(bf, "key2", 4));
    ASSERT(bloom_may_contain(bf, "key3", 4));

    bloom_destroy(bf);
}

TEST(bloom_false_positive) {
    // Create bloom filter for 1000 keys
    bloom_filter_t* bf = bloom_create(1000);
    ASSERT_NE(bf, NULL);

    // Add 1000 keys
    char key[32];
    for (int i = 0; i < 1000; i++) {
        snprintf(key, sizeof(key), "key%d", i);
        bloom_add(bf, key, strlen(key));
    }

    // All added keys should be found
    for (int i = 0; i < 1000; i++) {
        snprintf(key, sizeof(key), "key%d", i);
        ASSERT(bloom_may_contain(bf, key, strlen(key)));
    }

    // Count false positives for keys not added
    int false_positives = 0;
    for (int i = 1000; i < 2000; i++) {
        snprintf(key, sizeof(key), "key%d", i);
        if (bloom_may_contain(bf, key, strlen(key))) {
            false_positives++;
        }
    }

    // With 10 bits/key and 7 hash functions, expected FP rate is ~1%
    // Allow up to 5% for test stability
    ASSERT(false_positives < 50);

    bloom_destroy(bf);
}

TEST(bloom_serialize) {
    bloom_filter_t* bf = bloom_create(100);
    ASSERT_NE(bf, NULL);

    bloom_add(bf, "key1", 4);
    bloom_add(bf, "key2", 4);
    bloom_add(bf, "key3", 4);

    // Serialize
    size_t size = bloom_serialized_size(bf);
    uint8_t* buf = malloc(size);
    ASSERT_NE(buf, NULL);
    ASSERT_EQ(bloom_serialize(bf, buf, size), STATUS_OK);

    // Deserialize
    bloom_filter_t* bf2 = bloom_deserialize(buf, size);
    ASSERT_NE(bf2, NULL);

    // Check same keys are found
    ASSERT(bloom_may_contain(bf2, "key1", 4));
    ASSERT(bloom_may_contain(bf2, "key2", 4));
    ASSERT(bloom_may_contain(bf2, "key3", 4));

    free(buf);
    bloom_destroy(bf);
    bloom_destroy(bf2);
}

// ============================================================
// SSTable Tests
// ============================================================

TEST(sstable_write_read) {
    const char* path = "test_sstable.sst";
    unlink(path);

    // Write SSTable
    sstable_writer_t* writer = sstable_writer_create(path, 10, NULL);
    ASSERT_NE(writer, NULL);

    ASSERT_EQ(sstable_writer_add(writer, "key1", 4, "value1", 6, false), STATUS_OK);
    ASSERT_EQ(sstable_writer_add(writer, "key2", 4, "value2", 6, false), STATUS_OK);
    ASSERT_EQ(sstable_writer_add(writer, "key3", 4, "value3", 6, false), STATUS_OK);

    ASSERT_EQ(sstable_writer_finish(writer), STATUS_OK);

    // Read SSTable
    sstable_reader_t* reader = sstable_reader_open(path, NULL);
    ASSERT_NE(reader, NULL);

    char* value;
    size_t value_len;
    bool deleted;

    ASSERT_EQ(sstable_reader_get(reader, "key1", 4, &value, &value_len, &deleted), STATUS_OK);
    ASSERT_EQ(deleted, false);
    ASSERT_EQ(value_len, 6);
    ASSERT_STR_EQ(value, "value1", 6);
    free(value);

    ASSERT_EQ(sstable_reader_get(reader, "key2", 4, &value, &value_len, &deleted), STATUS_OK);
    ASSERT_STR_EQ(value, "value2", 6);
    free(value);

    ASSERT_EQ(sstable_reader_get(reader, "key3", 4, &value, &value_len, &deleted), STATUS_OK);
    ASSERT_STR_EQ(value, "value3", 6);
    free(value);

    sstable_reader_close(reader);
    unlink(path);
}

TEST(sstable_many_entries) {
    const char* path = "test_sstable_many.sst";
    unlink(path);

    // Write many entries (enough to span multiple blocks)
    sstable_writer_t* writer = sstable_writer_create(path, 500, NULL);
    ASSERT_NE(writer, NULL);

    char key[32], value[128];
    for (int i = 0; i < 500; i++) {
        snprintf(key, sizeof(key), "key%05d", i);
        snprintf(value, sizeof(value), "value%05d_padding_to_make_it_longer", i);
        ASSERT_EQ(sstable_writer_add(writer, key, strlen(key), value, strlen(value), false), STATUS_OK);
    }

    ASSERT_EQ(sstable_writer_finish(writer), STATUS_OK);

    // Read and verify
    sstable_reader_t* reader = sstable_reader_open(path, NULL);
    ASSERT_NE(reader, NULL);

    ASSERT_EQ(sstable_reader_num_entries(reader), 500);

    char* val;
    size_t val_len;
    bool deleted;

    // Check first, middle, and last entries
    ASSERT_EQ(sstable_reader_get(reader, "key00000", 8, &val, &val_len, &deleted), STATUS_OK);
    free(val);

    ASSERT_EQ(sstable_reader_get(reader, "key00250", 8, &val, &val_len, &deleted), STATUS_OK);
    free(val);

    ASSERT_EQ(sstable_reader_get(reader, "key00499", 8, &val, &val_len, &deleted), STATUS_OK);
    free(val);

    sstable_reader_close(reader);
    unlink(path);
}

TEST(sstable_tombstones) {
    const char* path = "test_sstable_tomb.sst";
    unlink(path);

    sstable_writer_t* writer = sstable_writer_create(path, 10, NULL);
    ASSERT_NE(writer, NULL);

    ASSERT_EQ(sstable_writer_add(writer, "key1", 4, "value1", 6, false), STATUS_OK);
    ASSERT_EQ(sstable_writer_add(writer, "key2", 4, NULL, 0, true), STATUS_OK);  // Tombstone
    ASSERT_EQ(sstable_writer_add(writer, "key3", 4, "value3", 6, false), STATUS_OK);

    ASSERT_EQ(sstable_writer_finish(writer), STATUS_OK);

    sstable_reader_t* reader = sstable_reader_open(path, NULL);
    ASSERT_NE(reader, NULL);

    char* value;
    size_t value_len;
    bool deleted;

    // key1 should exist
    ASSERT_EQ(sstable_reader_get(reader, "key1", 4, &value, &value_len, &deleted), STATUS_OK);
    ASSERT_EQ(deleted, false);
    free(value);

    // key2 should be marked as deleted
    ASSERT_EQ(sstable_reader_get(reader, "key2", 4, &value, &value_len, &deleted), STATUS_OK);
    ASSERT_EQ(deleted, true);

    // key3 should exist
    ASSERT_EQ(sstable_reader_get(reader, "key3", 4, &value, &value_len, &deleted), STATUS_OK);
    ASSERT_EQ(deleted, false);
    free(value);

    sstable_reader_close(reader);
    unlink(path);
}

TEST(sstable_not_found) {
    const char* path = "test_sstable_nf.sst";
    unlink(path);

    sstable_writer_t* writer = sstable_writer_create(path, 10, NULL);
    ASSERT_NE(writer, NULL);

    ASSERT_EQ(sstable_writer_add(writer, "key1", 4, "value1", 6, false), STATUS_OK);
    ASSERT_EQ(sstable_writer_add(writer, "key3", 4, "value3", 6, false), STATUS_OK);

    ASSERT_EQ(sstable_writer_finish(writer), STATUS_OK);

    sstable_reader_t* reader = sstable_reader_open(path, NULL);
    ASSERT_NE(reader, NULL);

    char* value;
    size_t value_len;
    bool deleted;

    // key2 doesn't exist
    ASSERT_EQ(sstable_reader_get(reader, "key2", 4, &value, &value_len, &deleted), STATUS_NOT_FOUND);

    // key0 doesn't exist (before first key)
    ASSERT_EQ(sstable_reader_get(reader, "key0", 4, &value, &value_len, &deleted), STATUS_NOT_FOUND);

    // key4 doesn't exist (after last key)
    ASSERT_EQ(sstable_reader_get(reader, "key4", 4, &value, &value_len, &deleted), STATUS_NOT_FOUND);

    sstable_reader_close(reader);
    unlink(path);
}

// ============================================================
// Storage Integration Tests
// ============================================================

TEST(storage_flush) {
    const char* db_path = "test_storage_flush";
    remove_dir(db_path);

    storage_t* db = storage_open(db_path, NULL);
    ASSERT_NE(db, NULL);

    // Add some data
    ASSERT_EQ(storage_put(db, "key1", 4, "value1", 6), STATUS_OK);
    ASSERT_EQ(storage_put(db, "key2", 4, "value2", 6), STATUS_OK);
    ASSERT_EQ(storage_put(db, "key3", 4, "value3", 6), STATUS_OK);

    // Flush to SSTable
    ASSERT_EQ(storage_flush(db), STATUS_OK);

    // Data should still be accessible
    char* val;
    size_t val_len;
    ASSERT_EQ(storage_get(db, "key1", 4, &val, &val_len), STATUS_OK);
    ASSERT_STR_EQ(val, "value1", 6);
    free(val);

    storage_close(db);
    remove_dir(db_path);
}

TEST(storage_query_after_flush) {
    const char* db_path = "test_storage_query";
    remove_dir(db_path);

    storage_t* db = storage_open(db_path, NULL);
    ASSERT_NE(db, NULL);

    // Add data and flush
    ASSERT_EQ(storage_put(db, "key1", 4, "value1", 6), STATUS_OK);
    ASSERT_EQ(storage_put(db, "key2", 4, "value2", 6), STATUS_OK);
    ASSERT_EQ(storage_flush(db), STATUS_OK);

    // Add more data to memtable
    ASSERT_EQ(storage_put(db, "key3", 4, "value3", 6), STATUS_OK);
    ASSERT_EQ(storage_put(db, "key1", 4, "updated1", 8), STATUS_OK);  // Update key1

    // Query should find key1 in memtable (updated value)
    char* val;
    size_t val_len;
    ASSERT_EQ(storage_get(db, "key1", 4, &val, &val_len), STATUS_OK);
    ASSERT_EQ(val_len, 8);
    ASSERT_STR_EQ(val, "updated1", 8);
    free(val);

    // Query should find key2 in SSTable
    ASSERT_EQ(storage_get(db, "key2", 4, &val, &val_len), STATUS_OK);
    ASSERT_STR_EQ(val, "value2", 6);
    free(val);

    // Query should find key3 in memtable
    ASSERT_EQ(storage_get(db, "key3", 4, &val, &val_len), STATUS_OK);
    ASSERT_STR_EQ(val, "value3", 6);
    free(val);

    storage_close(db);
    remove_dir(db_path);
}

TEST(storage_multiple_flushes) {
    const char* db_path = "test_storage_multi";
    remove_dir(db_path);

    storage_t* db = storage_open(db_path, NULL);
    ASSERT_NE(db, NULL);

    // First batch
    ASSERT_EQ(storage_put(db, "key1", 4, "value1", 6), STATUS_OK);
    ASSERT_EQ(storage_put(db, "key2", 4, "value2", 6), STATUS_OK);
    ASSERT_EQ(storage_flush(db), STATUS_OK);

    // Second batch
    ASSERT_EQ(storage_put(db, "key3", 4, "value3", 6), STATUS_OK);
    ASSERT_EQ(storage_put(db, "key4", 4, "value4", 6), STATUS_OK);
    ASSERT_EQ(storage_flush(db), STATUS_OK);

    // Third batch with delete
    ASSERT_EQ(storage_put(db, "key5", 4, "value5", 6), STATUS_OK);
    ASSERT_EQ(storage_delete(db, "key1", 4), STATUS_OK);
    ASSERT_EQ(storage_flush(db), STATUS_OK);

    // Verify all data
    char* val;
    size_t val_len;

    // key1 should be deleted (tombstone in newest SSTable)
    ASSERT_EQ(storage_get(db, "key1", 4, &val, &val_len), STATUS_NOT_FOUND);

    // key2-5 should exist
    ASSERT_EQ(storage_get(db, "key2", 4, &val, &val_len), STATUS_OK);
    free(val);
    ASSERT_EQ(storage_get(db, "key3", 4, &val, &val_len), STATUS_OK);
    free(val);
    ASSERT_EQ(storage_get(db, "key4", 4, &val, &val_len), STATUS_OK);
    free(val);
    ASSERT_EQ(storage_get(db, "key5", 4, &val, &val_len), STATUS_OK);
    free(val);

    storage_close(db);
    remove_dir(db_path);
}

// ============================================================
// Main
// ============================================================

int main(void) {
    printf("Phase 3 Tests: Bloom Filter, SSTable, Storage Flush\n");
    printf("====================================================\n\n");

    printf("Bloom Filter Tests:\n");
    RUN_TEST(bloom_basic);
    RUN_TEST(bloom_false_positive);
    RUN_TEST(bloom_serialize);

    printf("\nSSTable Tests:\n");
    RUN_TEST(sstable_write_read);
    RUN_TEST(sstable_many_entries);
    RUN_TEST(sstable_tombstones);
    RUN_TEST(sstable_not_found);

    printf("\nStorage Integration Tests:\n");
    RUN_TEST(storage_flush);
    RUN_TEST(storage_query_after_flush);
    RUN_TEST(storage_multiple_flushes);

    printf("\n====================================================\n");
    printf("Results: %d passed, %d failed\n", tests_passed, tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
