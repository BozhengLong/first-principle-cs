#ifndef STORAGE_LEVEL_H
#define STORAGE_LEVEL_H

#include "types.h"
#include "param.h"
#include "sstable.h"

// SSTable metadata for level management
typedef struct {
    uint64_t file_number;
    char* path;
    sstable_reader_t* reader;
    char* min_key;
    size_t min_key_len;
    char* max_key;
    size_t max_key_len;
    uint64_t file_size;
} sstable_meta_t;

// Level structure
typedef struct {
    int level_num;
    sstable_meta_t* files;
    size_t file_count;
    size_t file_capacity;
    uint64_t total_bytes;
} level_t;

// Level manager structure
struct level_manager {
    char* db_path;
    compare_fn cmp;
    level_t levels[MAX_LEVELS];
    uint64_t next_file_number;
};

// Lifecycle
level_manager_t* level_manager_create(const char* db_path, compare_fn cmp);
void level_manager_destroy(level_manager_t* lm);

// SSTable management
status_t level_add_sstable(level_manager_t* lm, int level, uint64_t file_num,
                           const char* path, sstable_reader_t* reader);
status_t level_remove_sstable(level_manager_t* lm, int level, uint64_t file_num);

// Query
status_t level_get(level_manager_t* lm, const char* key, size_t key_len,
                   char** value, size_t* value_len, bool* deleted);

// Compaction helpers
bool level_needs_compaction(level_manager_t* lm, int level);
size_t level_find_overlapping(level_manager_t* lm, int level,
                              const char* min_key, size_t min_key_len,
                              const char* max_key, size_t max_key_len,
                              uint64_t** file_nums);
uint64_t level_max_bytes_for_level(int level);

// Accessors
size_t level_file_count(level_manager_t* lm, int level);
uint64_t level_next_file_number(level_manager_t* lm);
void level_set_next_file_number(level_manager_t* lm, uint64_t num);

#endif // STORAGE_LEVEL_H
