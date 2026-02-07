/*
 * Phase 5 Tests: Block Cache and Benchmark
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#include "cache.h"
#include "storage.h"

#define TEST_DIR "test_phase5_db"

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

// ============================================================
// Test: Cache basic put/get
// ============================================================
static int test_cache_basic(void) {
    block_cache_t* cache = cache_create(1024);
    if (!cache) return 0;

    const char* key = "test_key";
    const uint8_t data[] = "test_data_value";
    size_t data_len = sizeof(data);

    cache_put(cache, key, strlen(key), data, data_len);

    size_t result_len = 0;
    uint8_t* result = cache_get(cache, key, strlen(key), &result_len);

    if (!result) {
        cache_destroy(cache);
        return 0;
    }

    if (result_len != data_len || memcmp(result, data, data_len) != 0) {
        free(result);
        cache_destroy(cache);
        return 0;
    }

    free(result);
    cache_destroy(cache);
    return 1;
}

// ============================================================
// Test: Cache LRU eviction
// ============================================================
static int test_cache_lru_eviction(void) {
    block_cache_t* cache = cache_create(200);
    if (!cache) return 0;

    uint8_t data[50];
    memset(data, 'A', sizeof(data));

    cache_put(cache, "key1", 4, data, 50);
    cache_put(cache, "key2", 4, data, 50);
    cache_put(cache, "key3", 4, data, 50);
    cache_put(cache, "key4", 4, data, 50);

    size_t len = 0;
    uint8_t* result = cache_get(cache, "key1", 4, &len);
    if (result) {
        free(result);
        cache_destroy(cache);
        return 0;
    }

    result = cache_get(cache, "key4", 4, &len);
    if (!result) {
        cache_destroy(cache);
        return 0;
    }
    free(result);

    cache_destroy(cache);
    return 1;
}

// ============================================================
// Test: Cache hit/miss statistics
// ============================================================
static int test_cache_hit_miss(void) {
    block_cache_t* cache = cache_create(1024);
    if (!cache) return 0;

    const uint8_t data[] = "test_data";
    cache_put(cache, "key1", 4, data, sizeof(data));

    size_t len = 0;
    uint8_t* result = cache_get(cache, "key1", 4, &len);
    free(result);

    result = cache_get(cache, "key2", 4, &len);

    double hit_rate = cache_hit_rate(cache);
    if (hit_rate < 0.49 || hit_rate > 0.51) {
        cache_destroy(cache);
        return 0;
    }

    cache_destroy(cache);
    return 1;
}

// ============================================================
// Test: Cache invalidate
// ============================================================
static int test_cache_invalidate(void) {
    block_cache_t* cache = cache_create(1024);
    if (!cache) return 0;

    const uint8_t data[] = "test_data";
    cache_put(cache, "key1", 4, data, sizeof(data));

    size_t len = 0;
    uint8_t* result = cache_get(cache, "key1", 4, &len);
    if (!result) {
        cache_destroy(cache);
        return 0;
    }
    free(result);

    cache_invalidate(cache, "key1", 4);

    result = cache_get(cache, "key1", 4, &len);
    if (result) {
        free(result);
        cache_destroy(cache);
        return 0;
    }

    cache_destroy(cache);
    return 1;
}

// ============================================================
// Test: Cache capacity limit
// ============================================================
static int test_cache_capacity(void) {
    block_cache_t* cache = cache_create(100);
    if (!cache) return 0;

    uint8_t large_data[200];
    memset(large_data, 'X', sizeof(large_data));

    cache_put(cache, "big", 3, large_data, sizeof(large_data));

    size_t len = 0;
    uint8_t* result = cache_get(cache, "big", 3, &len);
    if (result) {
        free(result);
        cache_destroy(cache);
        return 0;
    }

    uint8_t small_data[20];
    memset(small_data, 'Y', sizeof(small_data));
    cache_put(cache, "small", 5, small_data, sizeof(small_data));

    result = cache_get(cache, "small", 5, &len);
    if (!result) {
        cache_destroy(cache);
        return 0;
    }
    free(result);

    cache_destroy(cache);
    return 1;
}

// ============================================================
// Test: Cache clear
// ============================================================
static int test_cache_clear(void) {
    block_cache_t* cache = cache_create(1024);
    if (!cache) return 0;

    const uint8_t data[] = "test_data";
    cache_put(cache, "key1", 4, data, sizeof(data));
    cache_put(cache, "key2", 4, data, sizeof(data));
    cache_put(cache, "key3", 4, data, sizeof(data));

    cache_clear(cache);

    size_t len = 0;
    uint8_t* result = cache_get(cache, "key1", 4, &len);
    if (result) {
        free(result);
        cache_destroy(cache);
        return 0;
    }

    if (cache_usage(cache) != 0) {
        cache_destroy(cache);
        return 0;
    }

    cache_destroy(cache);
    return 1;
}

// ============================================================
// Test: Cache LRU access order
// ============================================================
static int test_cache_lru_access(void) {
    block_cache_t* cache = cache_create(150);
    if (!cache) return 0;

    uint8_t data[40];
    memset(data, 'A', sizeof(data));

    cache_put(cache, "key1", 4, data, 40);
    cache_put(cache, "key2", 4, data, 40);
    cache_put(cache, "key3", 4, data, 40);

    size_t len = 0;
    uint8_t* result = cache_get(cache, "key1", 4, &len);
    free(result);

    cache_put(cache, "key4", 4, data, 40);

    result = cache_get(cache, "key2", 4, &len);
    if (result) {
        free(result);
        cache_destroy(cache);
        return 0;
    }

    result = cache_get(cache, "key1", 4, &len);
    if (!result) {
        cache_destroy(cache);
        return 0;
    }
    free(result);

    cache_destroy(cache);
    return 1;
}

// ============================================================
// Test: Cache update existing key
// ============================================================
static int test_cache_update(void) {
    block_cache_t* cache = cache_create(1024);
    if (!cache) return 0;

    const uint8_t data1[] = "original_value";
    const uint8_t data2[] = "updated_value";

    cache_put(cache, "key1", 4, data1, sizeof(data1));
    cache_put(cache, "key1", 4, data2, sizeof(data2));

    size_t len = 0;
    uint8_t* result = cache_get(cache, "key1", 4, &len);
    if (!result) {
        cache_destroy(cache);
        return 0;
    }

    if (len != sizeof(data2) || memcmp(result, data2, len) != 0) {
        free(result);
        cache_destroy(cache);
        return 0;
    }

    free(result);
    cache_destroy(cache);
    return 1;
}

// ============================================================
// Main
// ============================================================
int main(void) {
    (void)remove_dir;  // Suppress unused warning

    printf("Phase 5 Tests: Block Cache and Benchmark\n");
    printf("=========================================\n\n");

    TEST(cache_basic);
    TEST(cache_lru_eviction);
    TEST(cache_hit_miss);
    TEST(cache_invalidate);
    TEST(cache_capacity);
    TEST(cache_clear);
    TEST(cache_lru_access);
    TEST(cache_update);

    printf("\n=========================================\n");
    printf("Results: %d/%d tests passed\n", tests_passed, tests_run);

    return tests_passed == tests_run ? 0 : 1;
}