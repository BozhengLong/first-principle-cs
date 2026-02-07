#include "level.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// Create level manager
level_manager_t* level_manager_create(const char* db_path, compare_fn cmp) {
    level_manager_t* lm = calloc(1, sizeof(level_manager_t));
    if (!lm) return NULL;

    if (db_path) {
        lm->db_path = strdup(db_path);
        if (!lm->db_path) {
            free(lm);
            return NULL;
        }
    }

    lm->cmp = cmp ? cmp : default_compare;
    lm->next_file_number = 1;

    // Initialize all levels
    for (int i = 0; i < MAX_LEVELS; i++) {
        lm->levels[i].level_num = i;
        lm->levels[i].files = NULL;
        lm->levels[i].file_count = 0;
        lm->levels[i].file_capacity = 0;
        lm->levels[i].total_bytes = 0;
    }

    return lm;
}

// Destroy level manager
void level_manager_destroy(level_manager_t* lm) {
    if (!lm) return;

    for (int i = 0; i < MAX_LEVELS; i++) {
        level_t* level = &lm->levels[i];
        for (size_t j = 0; j < level->file_count; j++) {
            sstable_meta_t* meta = &level->files[j];
            if (meta->reader) {
                sstable_reader_close(meta->reader);
            }
            free(meta->path);
            free(meta->min_key);
            free(meta->max_key);
        }
        free(level->files);
    }

    free(lm->db_path);
    free(lm);
}

// Helper: get file size
static uint64_t get_file_size(const char* path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        return (uint64_t)st.st_size;
    }
    return 0;
}

// Helper: find insertion position for L1+ (sorted by min_key)
static size_t find_insert_pos(level_manager_t* lm, level_t* level,
                              const char* min_key, size_t min_key_len) {
    size_t left = 0, right = level->file_count;
    while (left < right) {
        size_t mid = left + (right - left) / 2;
        int cmp = lm->cmp(level->files[mid].min_key, level->files[mid].min_key_len,
                         min_key, min_key_len);
        if (cmp < 0) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    return left;
}

// Add SSTable to a level
status_t level_add_sstable(level_manager_t* lm, int level, uint64_t file_num,
                           const char* path, sstable_reader_t* reader) {
    if (!lm || level < 0 || level >= MAX_LEVELS || !path || !reader) {
        return STATUS_INVALID_ARG;
    }

    level_t* lvl = &lm->levels[level];

    // Ensure capacity
    if (lvl->file_count >= lvl->file_capacity) {
        size_t new_cap = lvl->file_capacity * 2;
        if (new_cap == 0) new_cap = 4;
        sstable_meta_t* new_files = realloc(lvl->files, new_cap * sizeof(sstable_meta_t));
        if (!new_files) return STATUS_NO_MEMORY;
        lvl->files = new_files;
        lvl->file_capacity = new_cap;
    }

    // Get min/max keys from reader
    size_t min_len, max_len;
    const char* min_key = sstable_reader_min_key(reader, &min_len);
    const char* max_key = sstable_reader_max_key(reader, &max_len);

    // Create metadata
    sstable_meta_t meta = {0};
    meta.file_number = file_num;
    meta.path = strdup(path);
    meta.reader = reader;
    meta.file_size = get_file_size(path);

    if (min_key && min_len > 0) {
        meta.min_key = malloc(min_len);
        if (meta.min_key) {
            memcpy(meta.min_key, min_key, min_len);
            meta.min_key_len = min_len;
        }
    }
    if (max_key && max_len > 0) {
        meta.max_key = malloc(max_len);
        if (meta.max_key) {
            memcpy(meta.max_key, max_key, max_len);
            meta.max_key_len = max_len;
        }
    }

    if (!meta.path) {
        free(meta.min_key);
        free(meta.max_key);
        return STATUS_NO_MEMORY;
    }

    // For L0, append at end (newest last)
    // For L1+, insert in sorted order by min_key
    if (level == 0) {
        lvl->files[lvl->file_count++] = meta;
    } else {
        size_t pos = find_insert_pos(lm, lvl, meta.min_key, meta.min_key_len);
        // Shift elements to make room
        memmove(&lvl->files[pos + 1], &lvl->files[pos],
                (lvl->file_count - pos) * sizeof(sstable_meta_t));
        lvl->files[pos] = meta;
        lvl->file_count++;
    }

    lvl->total_bytes += meta.file_size;

    // Update next file number if needed
    if (file_num >= lm->next_file_number) {
        lm->next_file_number = file_num + 1;
    }

    return STATUS_OK;
}

// Remove SSTable from a level
status_t level_remove_sstable(level_manager_t* lm, int level, uint64_t file_num) {
    if (!lm || level < 0 || level >= MAX_LEVELS) {
        return STATUS_INVALID_ARG;
    }

    level_t* lvl = &lm->levels[level];

    for (size_t i = 0; i < lvl->file_count; i++) {
        if (lvl->files[i].file_number == file_num) {
            sstable_meta_t* meta = &lvl->files[i];

            // Update total bytes
            lvl->total_bytes -= meta->file_size;

            // Close reader and free memory
            if (meta->reader) {
                sstable_reader_close(meta->reader);
            }
            free(meta->path);
            free(meta->min_key);
            free(meta->max_key);

            // Shift remaining elements
            memmove(&lvl->files[i], &lvl->files[i + 1],
                    (lvl->file_count - i - 1) * sizeof(sstable_meta_t));
            lvl->file_count--;

            return STATUS_OK;
        }
    }

    return STATUS_NOT_FOUND;
}

// Helper: check if key is in range [min, max]
static bool key_in_range(compare_fn cmp,
                         const char* key, size_t key_len,
                         const char* min_key, size_t min_key_len,
                         const char* max_key, size_t max_key_len) {
    if (min_key && cmp(key, key_len, min_key, min_key_len) < 0) return false;
    if (max_key && cmp(key, key_len, max_key, max_key_len) > 0) return false;
    return true;
}

// Query: search all levels for a key
status_t level_get(level_manager_t* lm, const char* key, size_t key_len,
                   char** value, size_t* value_len, bool* deleted) {
    if (!lm || !key || !value || !value_len || !deleted) {
        return STATUS_INVALID_ARG;
    }

    *value = NULL;
    *value_len = 0;
    *deleted = false;

    // Search L0 first (all files, newest to oldest)
    level_t* l0 = &lm->levels[0];
    for (size_t i = l0->file_count; i > 0; i--) {
        sstable_meta_t* meta = &l0->files[i - 1];

        // Check key range
        if (!key_in_range(lm->cmp, key, key_len,
                          meta->min_key, meta->min_key_len,
                          meta->max_key, meta->max_key_len)) {
            continue;
        }

        status_t status = sstable_reader_get(meta->reader, key, key_len,
                                             value, value_len, deleted);
        if (status == STATUS_OK) {
            return STATUS_OK;
        }
        if (status != STATUS_NOT_FOUND) {
            return status;
        }
    }

    // Search L1+ (binary search to find the right file)
    for (int level = 1; level < MAX_LEVELS; level++) {
        level_t* lvl = &lm->levels[level];
        if (lvl->file_count == 0) continue;

        // Binary search for the file that might contain the key
        size_t left = 0, right = lvl->file_count;
        while (left < right) {
            size_t mid = left + (right - left) / 2;
            // Compare with max_key of file at mid
            int cmp = lm->cmp(lvl->files[mid].max_key, lvl->files[mid].max_key_len,
                             key, key_len);
            if (cmp < 0) {
                left = mid + 1;
            } else {
                right = mid;
            }
        }

        if (left < lvl->file_count) {
            sstable_meta_t* meta = &lvl->files[left];

            // Verify key is in range
            if (key_in_range(lm->cmp, key, key_len,
                            meta->min_key, meta->min_key_len,
                            meta->max_key, meta->max_key_len)) {
                status_t status = sstable_reader_get(meta->reader, key, key_len,
                                                     value, value_len, deleted);
                if (status == STATUS_OK) {
                    return STATUS_OK;
                }
                if (status != STATUS_NOT_FOUND) {
                    return status;
                }
            }
        }
    }

    return STATUS_NOT_FOUND;
}

// Calculate max bytes for a level
uint64_t level_max_bytes_for_level(int level) {
    if (level == 0) {
        // L0 is controlled by file count, not bytes
        return UINT64_MAX;
    }
    uint64_t result = L1_MAX_BYTES;
    for (int i = 1; i < level; i++) {
        result *= LEVEL_SIZE_MULTIPLIER;
    }
    return result;
}

// Check if a level needs compaction
bool level_needs_compaction(level_manager_t* lm, int level) {
    if (!lm || level < 0 || level >= MAX_LEVELS - 1) {
        return false;
    }

    level_t* lvl = &lm->levels[level];

    if (level == 0) {
        // L0 triggers on file count
        return lvl->file_count >= L0_COMPACTION_TRIGGER;
    } else {
        // L1+ triggers on total bytes
        return lvl->total_bytes > level_max_bytes_for_level(level);
    }
}

// Helper: check if two key ranges overlap
static bool ranges_overlap(compare_fn cmp,
                           const char* min1, size_t min1_len,
                           const char* max1, size_t max1_len,
                           const char* min2, size_t min2_len,
                           const char* max2, size_t max2_len) {
    // Ranges overlap if: min1 <= max2 AND min2 <= max1
    if (max1 && min2 && cmp(max1, max1_len, min2, min2_len) < 0) return false;
    if (max2 && min1 && cmp(max2, max2_len, min1, min1_len) < 0) return false;
    return true;
}

// Find files in a level that overlap with a key range
size_t level_find_overlapping(level_manager_t* lm, int level,
                              const char* min_key, size_t min_key_len,
                              const char* max_key, size_t max_key_len,
                              uint64_t** file_nums) {
    if (!lm || level < 0 || level >= MAX_LEVELS || !file_nums) {
        return 0;
    }

    level_t* lvl = &lm->levels[level];
    if (lvl->file_count == 0) {
        *file_nums = NULL;
        return 0;
    }

    // Allocate array for results
    uint64_t* result = malloc(lvl->file_count * sizeof(uint64_t));
    if (!result) {
        *file_nums = NULL;
        return 0;
    }

    size_t count = 0;

    if (level == 0) {
        // L0: check all files (they can overlap)
        for (size_t i = 0; i < lvl->file_count; i++) {
            sstable_meta_t* meta = &lvl->files[i];
            if (ranges_overlap(lm->cmp,
                              min_key, min_key_len, max_key, max_key_len,
                              meta->min_key, meta->min_key_len,
                              meta->max_key, meta->max_key_len)) {
                result[count++] = meta->file_number;
            }
        }
    } else {
        // L1+: files are sorted and non-overlapping
        // Binary search for first file that might overlap
        size_t left = 0, right = lvl->file_count;
        while (left < right) {
            size_t mid = left + (right - left) / 2;
            if (lm->cmp(lvl->files[mid].max_key, lvl->files[mid].max_key_len,
                       min_key, min_key_len) < 0) {
                left = mid + 1;
            } else {
                right = mid;
            }
        }

        // Collect all overlapping files
        for (size_t i = left; i < lvl->file_count; i++) {
            sstable_meta_t* meta = &lvl->files[i];
            if (lm->cmp(meta->min_key, meta->min_key_len,
                       max_key, max_key_len) > 0) {
                break;  // No more overlapping files
            }
            result[count++] = meta->file_number;
        }
    }

    if (count == 0) {
        free(result);
        *file_nums = NULL;
        return 0;
    }

    *file_nums = result;
    return count;
}

// Accessors
size_t level_file_count(level_manager_t* lm, int level) {
    if (!lm || level < 0 || level >= MAX_LEVELS) return 0;
    return lm->levels[level].file_count;
}

uint64_t level_next_file_number(level_manager_t* lm) {
    return lm ? lm->next_file_number : 0;
}

void level_set_next_file_number(level_manager_t* lm, uint64_t num) {
    if (lm) lm->next_file_number = num;
}
