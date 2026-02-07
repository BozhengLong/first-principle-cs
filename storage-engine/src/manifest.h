#ifndef STORAGE_MANIFEST_H
#define STORAGE_MANIFEST_H

#include "types.h"
#include "level.h"

// Manifest record types
typedef enum {
    MANIFEST_ADD_FILE = 1,
    MANIFEST_REMOVE_FILE = 2,
    MANIFEST_NEXT_FILE_NUM = 3,
} manifest_record_type_t;

// Manifest operations
status_t manifest_create(const char* db_path);
status_t manifest_log_add_file(const char* db_path, int level, uint64_t file_num);
status_t manifest_log_remove_file(const char* db_path, int level, uint64_t file_num);
status_t manifest_log_next_file_num(const char* db_path, uint64_t next_num);
status_t manifest_recover(const char* db_path, level_manager_t* lm);

#endif // STORAGE_MANIFEST_H
