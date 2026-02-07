#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../src/skiplist.h"
#include "../src/memtable.h"
#include "../src/storage.h"

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

// ============================================================
// Skip List Tests
// ============================================================

TEST(skiplist_create_destroy) {
    skiplist_t* list = skiplist_create(NULL);
    ASSERT_NE(list, NULL);
    ASSERT_EQ(skiplist_count(list), 0);
    skiplist_destroy(list);
}

TEST(skiplist_put_get) {
    skiplist_t* list = skiplist_create(NULL);
    ASSERT_NE(list, NULL);

    // Insert key-value pairs
    ASSERT_EQ(skiplist_put(list, "key1", 4, "value1", 6), STATUS_OK);
    ASSERT_EQ(skiplist_put(list, "key2", 4, "value2", 6), STATUS_OK);
    ASSERT_EQ(skiplist_put(list, "key3", 4, "value3", 6), STATUS_OK);

    ASSERT_EQ(skiplist_count(list), 3);

    // Retrieve values
    char* val;
    size_t val_len;
    ASSERT_EQ(skiplist_get(list, "key1", 4, &val, &val_len), STATUS_OK);
    ASSERT_EQ(val_len, 6);
    ASSERT_STR_EQ(val, "value1", 6);

    ASSERT_EQ(skiplist_get(list, "key2", 4, &val, &val_len), STATUS_OK);
    ASSERT_STR_EQ(val, "value2", 6);

    // Non-existent key
    ASSERT_EQ(skiplist_get(list, "nokey", 5, &val, &val_len), STATUS_NOT_FOUND);

    skiplist_destroy(list);
}

TEST(skiplist_update) {
    skiplist_t* list = skiplist_create(NULL);

    ASSERT_EQ(skiplist_put(list, "key", 3, "old", 3), STATUS_OK);

    char* val;
    size_t val_len;
    ASSERT_EQ(skiplist_get(list, "key", 3, &val, &val_len), STATUS_OK);
    ASSERT_STR_EQ(val, "old", 3);

    // Update value
    ASSERT_EQ(skiplist_put(list, "key", 3, "newvalue", 8), STATUS_OK);
    ASSERT_EQ(skiplist_get(list, "key", 3, &val, &val_len), STATUS_OK);
    ASSERT_EQ(val_len, 8);
    ASSERT_STR_EQ(val, "newvalue", 8);

    // Count should still be 1
    ASSERT_EQ(skiplist_count(list), 1);

    skiplist_destroy(list);
}

TEST(skiplist_delete) {
    skiplist_t* list = skiplist_create(NULL);

    ASSERT_EQ(skiplist_put(list, "key", 3, "value", 5), STATUS_OK);
    ASSERT_EQ(skiplist_delete(list, "key", 3), STATUS_OK);

    char* val;
    size_t val_len;
    ASSERT_EQ(skiplist_get(list, "key", 3, &val, &val_len), STATUS_NOT_FOUND);

    // Key still exists as tombstone
    ASSERT(skiplist_contains(list, "key", 3));

    skiplist_destroy(list);
}

TEST(skiplist_iterator) {
    skiplist_t* list = skiplist_create(NULL);

    // Insert in random order
    ASSERT_EQ(skiplist_put(list, "c", 1, "3", 1), STATUS_OK);
    ASSERT_EQ(skiplist_put(list, "a", 1, "1", 1), STATUS_OK);
    ASSERT_EQ(skiplist_put(list, "b", 1, "2", 1), STATUS_OK);

    // Iterate - should be in sorted order
    skiplist_iter_t* iter = skiplist_iter_create(list);
    ASSERT_NE(iter, NULL);

    skiplist_iter_seek_to_first(iter);
    ASSERT(skiplist_iter_valid(iter));

    size_t key_len;
    const char* key = skiplist_iter_key(iter, &key_len);
    ASSERT_EQ(key_len, 1);
    ASSERT_EQ(key[0], 'a');

    skiplist_iter_next(iter);
    key = skiplist_iter_key(iter, &key_len);
    ASSERT_EQ(key[0], 'b');

    skiplist_iter_next(iter);
    key = skiplist_iter_key(iter, &key_len);
    ASSERT_EQ(key[0], 'c');

    skiplist_iter_next(iter);
    ASSERT(!skiplist_iter_valid(iter));

    skiplist_iter_destroy(iter);
    skiplist_destroy(list);
}

// ============================================================
// MemTable Tests
// ============================================================

TEST(memtable_basic) {
    memtable_t* mt = memtable_create(1024 * 1024, NULL);
    ASSERT_NE(mt, NULL);

    ASSERT_EQ(memtable_put(mt, "key", 3, "value", 5), STATUS_OK);

    char* val;
    size_t val_len;
    ASSERT_EQ(memtable_get(mt, "key", 3, &val, &val_len), STATUS_OK);
    ASSERT_EQ(val_len, 5);
    ASSERT_STR_EQ(val, "value", 5);

    memtable_destroy(mt);
}

TEST(memtable_should_flush) {
    // Create small memtable (but larger than initial overhead)
    memtable_t* mt = memtable_create(500, NULL);
    ASSERT_NE(mt, NULL);

    ASSERT(!memtable_should_flush(mt));

    // Fill it up
    char key[32], val[64];
    for (int i = 0; i < 20; i++) {
        snprintf(key, sizeof(key), "key%d", i);
        snprintf(val, sizeof(val), "value%d", i);
        memtable_put(mt, key, strlen(key), val, strlen(val));
    }

    ASSERT(memtable_should_flush(mt));

    memtable_destroy(mt);
}

// ============================================================
// Storage API Tests
// ============================================================

TEST(storage_open_close) {
    storage_t* db = storage_open(NULL, NULL);
    ASSERT_NE(db, NULL);
    storage_close(db);
}

TEST(storage_put_get) {
    storage_t* db = storage_open(NULL, NULL);
    ASSERT_NE(db, NULL);

    ASSERT_EQ(storage_put(db, "hello", 5, "world", 5), STATUS_OK);

    char* val;
    size_t val_len;
    ASSERT_EQ(storage_get(db, "hello", 5, &val, &val_len), STATUS_OK);
    ASSERT_EQ(val_len, 5);
    ASSERT_STR_EQ(val, "world", 5);

    storage_close(db);
}

TEST(storage_delete) {
    storage_t* db = storage_open(NULL, NULL);

    ASSERT_EQ(storage_put(db, "key", 3, "value", 5), STATUS_OK);
    ASSERT_EQ(storage_delete(db, "key", 3), STATUS_OK);

    char* val;
    size_t val_len;
    ASSERT_EQ(storage_get(db, "key", 3, &val, &val_len), STATUS_NOT_FOUND);

    storage_close(db);
}

TEST(storage_iterator) {
    storage_t* db = storage_open(NULL, NULL);

    // Insert in random order
    ASSERT_EQ(storage_put(db, "c", 1, "3", 1), STATUS_OK);
    ASSERT_EQ(storage_put(db, "a", 1, "1", 1), STATUS_OK);
    ASSERT_EQ(storage_put(db, "b", 1, "2", 1), STATUS_OK);

    // Iterate - should be in sorted order
    storage_iter_t* iter = storage_iter_create(db);
    ASSERT_NE(iter, NULL);

    storage_iter_seek_to_first(iter);
    ASSERT(storage_iter_valid(iter));

    size_t key_len;
    const char* key = storage_iter_key(iter, &key_len);
    ASSERT_EQ(key[0], 'a');

    storage_iter_next(iter);
    key = storage_iter_key(iter, &key_len);
    ASSERT_EQ(key[0], 'b');

    storage_iter_next(iter);
    key = storage_iter_key(iter, &key_len);
    ASSERT_EQ(key[0], 'c');

    storage_iter_next(iter);
    ASSERT(!storage_iter_valid(iter));

    storage_iter_destroy(iter);
    storage_close(db);
}

TEST(storage_iterator_skip_deleted) {
    storage_t* db = storage_open(NULL, NULL);

    ASSERT_EQ(storage_put(db, "a", 1, "1", 1), STATUS_OK);
    ASSERT_EQ(storage_put(db, "b", 1, "2", 1), STATUS_OK);
    ASSERT_EQ(storage_put(db, "c", 1, "3", 1), STATUS_OK);

    // Delete middle key
    ASSERT_EQ(storage_delete(db, "b", 1), STATUS_OK);

    // Iterator should skip deleted key
    storage_iter_t* iter = storage_iter_create(db);
    storage_iter_seek_to_first(iter);

    size_t key_len;
    const char* key = storage_iter_key(iter, &key_len);
    ASSERT_EQ(key[0], 'a');

    storage_iter_next(iter);
    key = storage_iter_key(iter, &key_len);
    ASSERT_EQ(key[0], 'c');  // 'b' is skipped

    storage_iter_destroy(iter);
    storage_close(db);
}

// ============================================================
// Main
// ============================================================

int main(void) {
    printf("Phase 1 Tests: Skip List, MemTable, Storage API\n");
    printf("================================================\n\n");

    printf("Skip List Tests:\n");
    RUN_TEST(skiplist_create_destroy);
    RUN_TEST(skiplist_put_get);
    RUN_TEST(skiplist_update);
    RUN_TEST(skiplist_delete);
    RUN_TEST(skiplist_iterator);

    printf("\nMemTable Tests:\n");
    RUN_TEST(memtable_basic);
    RUN_TEST(memtable_should_flush);

    printf("\nStorage API Tests:\n");
    RUN_TEST(storage_open_close);
    RUN_TEST(storage_put_get);
    RUN_TEST(storage_delete);
    RUN_TEST(storage_iterator);
    RUN_TEST(storage_iterator_skip_deleted);

    printf("\n================================================\n");
    printf("Results: %d passed, %d failed\n", tests_passed, tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
