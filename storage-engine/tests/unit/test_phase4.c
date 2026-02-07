/*
 * Phase 4 Tests: Multi-level LSM and Compaction
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#include "storage.h"
#include "level.h"
#include "compact.h"
#include "manifest.h"

#define TEST_DIR "test_phase4_db"

// Test counters
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

// Helper: remove directory recursively
static void remove_dir(const char* path) {
    DIR* dir = opendir(path);
    if (!dir) return;

    struct dirent* entry;
    char filepath[512];
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);
        unlink(filepath);
    }
    closedir(dir);
    rmdir(path);
}

// Helper: create test SSTable
static sstable_reader_t* create_test_sstable(const char* path, const char* prefix,
                                              int start, int count) {
    sstable_writer_t* writer = sstable_writer_create(path, count, NULL);
    if (!writer) return NULL;

    char key[64], value[64];
    for (int i = start; i < start + count; i++) {
        snprintf(key, sizeof(key), "%s%04d", prefix, i);
        snprintf(value, sizeof(value), "value%04d", i);
        sstable_writer_add(writer, key, strlen(key), value, strlen(value), false);
    }

    sstable_writer_finish(writer);
    return sstable_reader_open(path, NULL);
}

// ============================================================
// Test: Level manager basic operations
// ============================================================
static int test_level_basic(void) {
    remove_dir(TEST_DIR);
    mkdir(TEST_DIR, 0755);

    level_manager_t* lm = level_manager_create(TEST_DIR, NULL);
    if (!lm) return 0;

    // Create and add an SSTable
    char path[256];
    snprintf(path, sizeof(path), "%s/000001.sst", TEST_DIR);
    sstable_reader_t* reader = create_test_sstable(path, "key", 0, 100);
    if (!reader) {
        level_manager_destroy(lm);
        return 0;
    }

    status_t status = level_add_sstable(lm, 0, 1, path, reader);
    if (status != STATUS_OK) {
        level_manager_destroy(lm);
        return 0;
    }

    // Verify file count
    if (level_file_count(lm, 0) != 1) {
        level_manager_destroy(lm);
        return 0;
    }

    // Query for a key
    char* value = NULL;
    size_t value_len = 0;
    bool deleted = false;
    status = level_get(lm, "key0050", 7, &value, &value_len, &deleted);
    if (status != STATUS_OK || deleted) {
        level_manager_destroy(lm);
        return 0;
    }

    if (strcmp(value, "value0050") != 0) {
        free(value);
        level_manager_destroy(lm);
        return 0;
    }
    free(value);

    level_manager_destroy(lm);
    remove_dir(TEST_DIR);
    return 1;
}

// ============================================================
// Test: L0 allows overlapping files
// ============================================================
static int test_level_l0_overlap(void) {
    remove_dir(TEST_DIR);
    mkdir(TEST_DIR, 0755);

    level_manager_t* lm = level_manager_create(TEST_DIR, NULL);
    if (!lm) return 0;

    // Add two overlapping SSTables to L0
    char path1[256], path2[256];
    snprintf(path1, sizeof(path1), "%s/000001.sst", TEST_DIR);
    snprintf(path2, sizeof(path2), "%s/000002.sst", TEST_DIR);

    sstable_reader_t* r1 = create_test_sstable(path1, "key", 0, 100);
    sstable_reader_t* r2 = create_test_sstable(path2, "key", 50, 100);  // Overlaps with r1

    if (!r1 || !r2) {
        if (r1) sstable_reader_close(r1);
        if (r2) sstable_reader_close(r2);
        level_manager_destroy(lm);
        return 0;
    }

    level_add_sstable(lm, 0, 1, path1, r1);
    level_add_sstable(lm, 0, 2, path2, r2);

    if (level_file_count(lm, 0) != 2) {
        level_manager_destroy(lm);
        return 0;
    }

    // Query should return value from newer file (r2)
    char* value = NULL;
    size_t value_len = 0;
    bool deleted = false;
    status_t status = level_get(lm, "key0075", 7, &value, &value_len, &deleted);
    if (status != STATUS_OK) {
        level_manager_destroy(lm);
        return 0;
    }

    // key0075 exists in both files, should get from newer (r2)
    if (strcmp(value, "value0075") != 0) {
        free(value);
        level_manager_destroy(lm);
        return 0;
    }
    free(value);

    level_manager_destroy(lm);
    remove_dir(TEST_DIR);
    return 1;
}

// ============================================================
// Test: L1+ files are sorted by min_key
// ============================================================
static int test_level_l1_sorted(void) {
    remove_dir(TEST_DIR);
    mkdir(TEST_DIR, 0755);

    level_manager_t* lm = level_manager_create(TEST_DIR, NULL);
    if (!lm) return 0;

    // Add non-overlapping SSTables to L1 in random order
    char path1[256], path2[256], path3[256];
    snprintf(path1, sizeof(path1), "%s/000001.sst", TEST_DIR);
    snprintf(path2, sizeof(path2), "%s/000002.sst", TEST_DIR);
    snprintf(path3, sizeof(path3), "%s/000003.sst", TEST_DIR);

    // Create files with non-overlapping ranges
    sstable_reader_t* r1 = create_test_sstable(path1, "c", 0, 10);
    sstable_reader_t* r2 = create_test_sstable(path2, "a", 0, 10);
    sstable_reader_t* r3 = create_test_sstable(path3, "b", 0, 10);

    if (!r1 || !r2 || !r3) {
        if (r1) sstable_reader_close(r1);
        if (r2) sstable_reader_close(r2);
        if (r3) sstable_reader_close(r3);
        level_manager_destroy(lm);
        return 0;
    }

    // Add in non-sorted order
    level_add_sstable(lm, 1, 1, path1, r1);
    level_add_sstable(lm, 1, 2, path2, r2);
    level_add_sstable(lm, 1, 3, path3, r3);

    if (level_file_count(lm, 1) != 3) {
        level_manager_destroy(lm);
        return 0;
    }

    // Query each range
    char* value = NULL;
    size_t value_len = 0;
    bool deleted = false;

    status_t status = level_get(lm, "a0005", 5, &value, &value_len, &deleted);
    if (status != STATUS_OK) {
        level_manager_destroy(lm);
        return 0;
    }
    free(value);

    status = level_get(lm, "b0005", 5, &value, &value_len, &deleted);
    if (status != STATUS_OK) {
        level_manager_destroy(lm);
        return 0;
    }
    free(value);

    status = level_get(lm, "c0005", 5, &value, &value_len, &deleted);
    if (status != STATUS_OK) {
        level_manager_destroy(lm);
        return 0;
    }
    free(value);

    level_manager_destroy(lm);
    remove_dir(TEST_DIR);
    return 1;
}

// ============================================================
// Test: Level query across multiple levels
// ============================================================
static int test_level_query(void) {
    remove_dir(TEST_DIR);
    mkdir(TEST_DIR, 0755);

    level_manager_t* lm = level_manager_create(TEST_DIR, NULL);
    if (!lm) return 0;

    // Add files to different levels
    char path0[256], path1[256];
    snprintf(path0, sizeof(path0), "%s/000001.sst", TEST_DIR);
    snprintf(path1, sizeof(path1), "%s/000002.sst", TEST_DIR);

    sstable_reader_t* r0 = create_test_sstable(path0, "key", 0, 50);
    sstable_reader_t* r1 = create_test_sstable(path1, "key", 25, 50);

    if (!r0 || !r1) {
        if (r0) sstable_reader_close(r0);
        if (r1) sstable_reader_close(r1);
        level_manager_destroy(lm);
        return 0;
    }

    level_add_sstable(lm, 0, 1, path0, r0);  // L0: key0000-key0049
    level_add_sstable(lm, 1, 2, path1, r1);  // L1: key0025-key0074

    // Query key only in L0
    char* value = NULL;
    size_t value_len = 0;
    bool deleted = false;
    status_t status = level_get(lm, "key0010", 7, &value, &value_len, &deleted);
    if (status != STATUS_OK) {
        level_manager_destroy(lm);
        return 0;
    }
    free(value);

    // Query key only in L1
    status = level_get(lm, "key0060", 7, &value, &value_len, &deleted);
    if (status != STATUS_OK) {
        level_manager_destroy(lm);
        return 0;
    }
    free(value);

    // Query key in both (should return L0 value)
    status = level_get(lm, "key0030", 7, &value, &value_len, &deleted);
    if (status != STATUS_OK) {
        level_manager_destroy(lm);
        return 0;
    }
    free(value);

    level_manager_destroy(lm);
    remove_dir(TEST_DIR);
    return 1;
}

// ============================================================
// Test: SSTable iterator
// ============================================================
static int test_sstable_iterator(void) {
    remove_dir(TEST_DIR);
    mkdir(TEST_DIR, 0755);

    char path[256];
    snprintf(path, sizeof(path), "%s/000001.sst", TEST_DIR);

    sstable_reader_t* reader = create_test_sstable(path, "key", 0, 100);
    if (!reader) return 0;

    sstable_iter_t* iter = sstable_iter_create(reader);
    if (!iter) {
        sstable_reader_close(reader);
        return 0;
    }

    sstable_iter_seek_to_first(iter);

    int count = 0;
    while (sstable_iter_valid(iter)) {
        size_t key_len, value_len;
        const char* key = sstable_iter_key(iter, &key_len);
        const char* value = sstable_iter_value(iter, &value_len);

        if (!key || !value) {
            sstable_iter_destroy(iter);
            sstable_reader_close(reader);
            return 0;
        }

        count++;
        sstable_iter_next(iter);
    }

    sstable_iter_destroy(iter);
    sstable_reader_close(reader);

    if (count != 100) return 0;

    remove_dir(TEST_DIR);
    return 1;
}

// ============================================================
// Test: Compaction trigger check
// ============================================================
static int test_compaction_trigger(void) {
    remove_dir(TEST_DIR);
    mkdir(TEST_DIR, 0755);

    level_manager_t* lm = level_manager_create(TEST_DIR, NULL);
    if (!lm) return 0;

    // Initially no compaction needed
    if (level_needs_compaction(lm, 0)) {
        level_manager_destroy(lm);
        return 0;
    }

    // Add L0_COMPACTION_TRIGGER files to L0
    for (int i = 0; i < L0_COMPACTION_TRIGGER; i++) {
        char path[256];
        snprintf(path, sizeof(path), "%s/%06d.sst", TEST_DIR, i + 1);
        sstable_reader_t* reader = create_test_sstable(path, "key", i * 10, 10);
        if (!reader) {
            level_manager_destroy(lm);
            return 0;
        }
        level_add_sstable(lm, 0, i + 1, path, reader);
    }

    // Now compaction should be needed
    if (!level_needs_compaction(lm, 0)) {
        level_manager_destroy(lm);
        return 0;
    }

    level_manager_destroy(lm);
    remove_dir(TEST_DIR);
    return 1;
}

// ============================================================
// Test: Find overlapping files
// ============================================================
static int test_find_overlapping(void) {
    remove_dir(TEST_DIR);
    mkdir(TEST_DIR, 0755);

    level_manager_t* lm = level_manager_create(TEST_DIR, NULL);
    if (!lm) return 0;

    // Add non-overlapping files to L1
    char path1[256], path2[256], path3[256];
    snprintf(path1, sizeof(path1), "%s/000001.sst", TEST_DIR);
    snprintf(path2, sizeof(path2), "%s/000002.sst", TEST_DIR);
    snprintf(path3, sizeof(path3), "%s/000003.sst", TEST_DIR);

    sstable_reader_t* r1 = create_test_sstable(path1, "a", 0, 10);
    sstable_reader_t* r2 = create_test_sstable(path2, "b", 0, 10);
    sstable_reader_t* r3 = create_test_sstable(path3, "c", 0, 10);

    if (!r1 || !r2 || !r3) {
        if (r1) sstable_reader_close(r1);
        if (r2) sstable_reader_close(r2);
        if (r3) sstable_reader_close(r3);
        level_manager_destroy(lm);
        return 0;
    }

    level_add_sstable(lm, 1, 1, path1, r1);
    level_add_sstable(lm, 1, 2, path2, r2);
    level_add_sstable(lm, 1, 3, path3, r3);

    // Find files overlapping with "b" range
    uint64_t* overlapping = NULL;
    size_t count = level_find_overlapping(lm, 1, "b0000", 5, "b0009", 5, &overlapping);

    if (count != 1) {
        free(overlapping);
        level_manager_destroy(lm);
        return 0;
    }

    free(overlapping);
    level_manager_destroy(lm);
    remove_dir(TEST_DIR);
    return 1;
}

// ============================================================
// Test: Storage integration with levels
// ============================================================
static int test_storage_with_levels(void) {
    remove_dir(TEST_DIR);

    storage_t* db = storage_open(TEST_DIR, NULL);
    if (!db) return 0;

    // Write some data
    for (int i = 0; i < 100; i++) {
        char key[32], value[32];
        snprintf(key, sizeof(key), "key%04d", i);
        snprintf(value, sizeof(value), "value%04d", i);
        status_t status = storage_put(db, key, strlen(key), value, strlen(value));
        if (status != STATUS_OK) {
            storage_close(db);
            return 0;
        }
    }

    // Flush to L0
    status_t status = storage_flush(db);
    if (status != STATUS_OK) {
        storage_close(db);
        return 0;
    }

    // Verify data can be read
    char* value = NULL;
    size_t value_len = 0;
    status = storage_get(db, "key0050", 7, &value, &value_len);
    if (status != STATUS_OK) {
        storage_close(db);
        return 0;
    }

    if (strcmp(value, "value0050") != 0) {
        free(value);
        storage_close(db);
        return 0;
    }
    free(value);

    storage_close(db);
    remove_dir(TEST_DIR);
    return 1;
}

// ============================================================
// Test: Manifest recovery
// ============================================================
static int test_manifest_recovery(void) {
    remove_dir(TEST_DIR);
    mkdir(TEST_DIR, 0755);

    // Create manifest and log some operations
    manifest_create(TEST_DIR);

    // Create an SSTable
    char path[256];
    snprintf(path, sizeof(path), "%s/000001.sst", TEST_DIR);
    sstable_reader_t* reader = create_test_sstable(path, "key", 0, 50);
    if (!reader) return 0;
    sstable_reader_close(reader);

    // Log the file addition
    manifest_log_add_file(TEST_DIR, 0, 1);
    manifest_log_next_file_num(TEST_DIR, 2);

    // Create a new level manager and recover
    level_manager_t* lm = level_manager_create(TEST_DIR, NULL);
    if (!lm) return 0;

    status_t status = manifest_recover(TEST_DIR, lm);
    if (status != STATUS_OK) {
        level_manager_destroy(lm);
        return 0;
    }

    // Verify the file was recovered
    if (level_file_count(lm, 0) != 1) {
        level_manager_destroy(lm);
        return 0;
    }

    if (level_next_file_number(lm) != 2) {
        level_manager_destroy(lm);
        return 0;
    }

    level_manager_destroy(lm);
    remove_dir(TEST_DIR);
    return 1;
}

// ============================================================
// Main
// ============================================================
int main(void) {
    printf("Phase 4 Tests: Multi-level LSM and Compaction\n");
    printf("==============================================\n\n");

    TEST(level_basic);
    TEST(level_l0_overlap);
    TEST(level_l1_sorted);
    TEST(level_query);
    TEST(sstable_iterator);
    TEST(compaction_trigger);
    TEST(find_overlapping);
    TEST(storage_with_levels);
    TEST(manifest_recovery);

    printf("\n==============================================\n");
    printf("Results: %d/%d tests passed\n", tests_passed, tests_run);

    return tests_passed == tests_run ? 0 : 1;
}
