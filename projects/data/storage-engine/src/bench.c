/*
 * Storage Engine Benchmark Tool
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#include "storage.h"
#include "cache.h"

#define BENCH_DIR "bench_db"
#define KEY_SIZE 16
#define VALUE_SIZE 100

// Get current time in microseconds
static uint64_t now_usec(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;
}

// Generate random key
static void random_key(char* buf, int index) {
    snprintf(buf, KEY_SIZE, "key%012d", index);
}

// Generate random value
static void random_value(char* buf, int index) {
    snprintf(buf, VALUE_SIZE, "value%012d_padding_to_make_it_longer_%d",
             index, index * 17);
}

// Remove directory recursively
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

// Simple LCG random number generator
static uint32_t rand_state = 12345;
static uint32_t fast_rand(void) {
    rand_state = rand_state * 1103515245 + 12345;
    return rand_state;
}

// Benchmark: Sequential Write
static void bench_seq_write(int count) {
    remove_dir(BENCH_DIR);

    storage_t* db = storage_open(BENCH_DIR, NULL);
    if (!db) {
        printf("Failed to open database\n");
        return;
    }

    char key[KEY_SIZE];
    char value[VALUE_SIZE];

    uint64_t start = now_usec();

    for (int i = 0; i < count; i++) {
        random_key(key, i);
        random_value(value, i);
        storage_put(db, key, strlen(key), value, strlen(value));
    }

    uint64_t elapsed = now_usec() - start;
    double ops_per_sec = (double)count / ((double)elapsed / 1000000.0);

    printf("Sequential Write: %d ops, %.0f ops/sec\n", count, ops_per_sec);

    storage_close(db);
    remove_dir(BENCH_DIR);
}

// Benchmark: Random Write
static void bench_rand_write(int count) {
    remove_dir(BENCH_DIR);

    storage_t* db = storage_open(BENCH_DIR, NULL);
    if (!db) {
        printf("Failed to open database\n");
        return;
    }

    char key[KEY_SIZE];
    char value[VALUE_SIZE];

    uint64_t start = now_usec();

    for (int i = 0; i < count; i++) {
        int idx = fast_rand() % count;
        random_key(key, idx);
        random_value(value, idx);
        storage_put(db, key, strlen(key), value, strlen(value));
    }

    uint64_t elapsed = now_usec() - start;
    double ops_per_sec = (double)count / ((double)elapsed / 1000000.0);

    printf("Random Write:     %d ops, %.0f ops/sec\n", count, ops_per_sec);

    storage_close(db);
    remove_dir(BENCH_DIR);
}

// Benchmark: Sequential Read
static void bench_seq_read(int count) {
    remove_dir(BENCH_DIR);

    storage_t* db = storage_open(BENCH_DIR, NULL);
    if (!db) {
        printf("Failed to open database\n");
        return;
    }

    char key[KEY_SIZE];
    char value[VALUE_SIZE];

    // First populate the database
    for (int i = 0; i < count; i++) {
        random_key(key, i);
        random_value(value, i);
        storage_put(db, key, strlen(key), value, strlen(value));
    }

    // Now benchmark reads
    uint64_t start = now_usec();

    for (int i = 0; i < count; i++) {
        random_key(key, i);
        char* val = NULL;
        size_t val_len = 0;
        storage_get(db, key, strlen(key), &val, &val_len);
        free(val);
    }

    uint64_t elapsed = now_usec() - start;
    double ops_per_sec = (double)count / ((double)elapsed / 1000000.0);

    printf("Sequential Read:  %d ops, %.0f ops/sec\n", count, ops_per_sec);

    storage_close(db);
    remove_dir(BENCH_DIR);
}

// Benchmark: Random Read
static void bench_rand_read(int count) {
    remove_dir(BENCH_DIR);

    storage_t* db = storage_open(BENCH_DIR, NULL);
    if (!db) {
        printf("Failed to open database\n");
        return;
    }

    char key[KEY_SIZE];
    char value[VALUE_SIZE];

    // First populate the database
    for (int i = 0; i < count; i++) {
        random_key(key, i);
        random_value(value, i);
        storage_put(db, key, strlen(key), value, strlen(value));
    }

    // Now benchmark random reads
    uint64_t start = now_usec();

    for (int i = 0; i < count; i++) {
        int idx = fast_rand() % count;
        random_key(key, idx);
        char* val = NULL;
        size_t val_len = 0;
        storage_get(db, key, strlen(key), &val, &val_len);
        free(val);
    }

    uint64_t elapsed = now_usec() - start;
    double ops_per_sec = (double)count / ((double)elapsed / 1000000.0);

    printf("Random Read:      %d ops, %.0f ops/sec\n", count, ops_per_sec);

    storage_close(db);
    remove_dir(BENCH_DIR);
}

// Benchmark: Mixed workload (50% read, 50% write)
static void bench_mixed(int count) {
    remove_dir(BENCH_DIR);

    storage_t* db = storage_open(BENCH_DIR, NULL);
    if (!db) {
        printf("Failed to open database\n");
        return;
    }

    char key[KEY_SIZE];
    char value[VALUE_SIZE];

    // Pre-populate with half the keys
    for (int i = 0; i < count / 2; i++) {
        random_key(key, i);
        random_value(value, i);
        storage_put(db, key, strlen(key), value, strlen(value));
    }

    // Mixed workload
    uint64_t start = now_usec();

    for (int i = 0; i < count; i++) {
        int idx = fast_rand() % count;
        random_key(key, idx);

        if (fast_rand() % 2 == 0) {
            // Write
            random_value(value, idx);
            storage_put(db, key, strlen(key), value, strlen(value));
        } else {
            // Read
            char* val = NULL;
            size_t val_len = 0;
            storage_get(db, key, strlen(key), &val, &val_len);
            free(val);
        }
    }

    uint64_t elapsed = now_usec() - start;
    double ops_per_sec = (double)count / ((double)elapsed / 1000000.0);

    printf("Mixed (50/50):    %d ops, %.0f ops/sec\n", count, ops_per_sec);

    storage_close(db);
    remove_dir(BENCH_DIR);
}

// Benchmark: Block Cache
static void bench_cache(int count) {
    block_cache_t* cache = cache_create(1024 * 1024);  // 1 MB cache
    if (!cache) {
        printf("Failed to create cache\n");
        return;
    }

    char key[KEY_SIZE];
    uint8_t data[VALUE_SIZE];

    // Populate cache
    for (int i = 0; i < count && i < 10000; i++) {
        random_key(key, i);
        memset(data, i & 0xFF, VALUE_SIZE);
        cache_put(cache, key, strlen(key), data, VALUE_SIZE);
    }

    // Benchmark cache hits
    uint64_t start = now_usec();

    for (int i = 0; i < count; i++) {
        int idx = fast_rand() % 10000;
        random_key(key, idx);
        size_t data_len = 0;
        uint8_t* result = cache_get(cache, key, strlen(key), &data_len);
        free(result);
    }

    uint64_t elapsed = now_usec() - start;
    double ops_per_sec = (double)count / ((double)elapsed / 1000000.0);

    printf("Cache Access:     %d ops, %.0f ops/sec (hit rate: %.1f%%)\n",
           count, ops_per_sec, cache_hit_rate(cache) * 100.0);

    cache_destroy(cache);
}

// Main
int main(int argc, char** argv) {
    int count = 10000;  // Default operation count

    if (argc > 1) {
        count = atoi(argv[1]);
        if (count <= 0) count = 10000;
    }

    printf("Storage Engine Benchmark\n");
    printf("========================\n");
    printf("Operations per test: %d\n\n", count);

    bench_seq_write(count);
    bench_rand_write(count);
    bench_seq_read(count);
    bench_rand_read(count);
    bench_mixed(count);
    bench_cache(count);

    printf("\nBenchmark complete.\n");

    return 0;
}
