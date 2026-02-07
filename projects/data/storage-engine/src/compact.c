#include "compact.h"
#include "manifest.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

// Helper: decode varint
static size_t decode_varint(const uint8_t* buf, size_t len, uint64_t* value) {
    *value = 0;
    size_t i = 0;
    int shift = 0;
    while (i < len && i < 10) {
        uint64_t byte = buf[i++];
        *value |= (byte & 0x7F) << shift;
        if (!(byte & 0x80)) return i;
        shift += 7;
    }
    return 0;
}

// Helper: read all bytes
static ssize_t read_all(int fd, void* buf, size_t len) {
    uint8_t* p = buf;
    size_t remaining = len;
    while (remaining > 0) {
        ssize_t n = read(fd, p, remaining);
        if (n <= 0) return n == 0 ? (ssize_t)(len - remaining) : -1;
        p += n;
        remaining -= n;
    }
    return (ssize_t)len;
}

// SSTable iterator structure
struct sstable_iter {
    sstable_reader_t* reader;
    size_t current_block;
    uint8_t* block_data;
    size_t block_size;
    size_t pos;
    size_t data_end;
    char* current_key;
    size_t current_key_len;
    char* current_value;
    size_t current_value_len;
    bool current_deleted;
    bool valid;
};

// Create SSTable iterator
sstable_iter_t* sstable_iter_create(sstable_reader_t* reader) {
    if (!reader) return NULL;

    sstable_iter_t* iter = calloc(1, sizeof(sstable_iter_t));
    if (!iter) return NULL;

    iter->reader = reader;
    iter->current_block = 0;
    iter->block_data = NULL;
    iter->valid = false;

    return iter;
}

// Destroy SSTable iterator
void sstable_iter_destroy(sstable_iter_t* iter) {
    if (!iter) return;
    free(iter->block_data);
    free(iter->current_key);
    free(iter->current_value);
    free(iter);
}

// Helper: load a block into the iterator
static bool load_block(sstable_iter_t* iter, size_t block_idx) {
    if (block_idx >= iter->reader->index_count) {
        iter->valid = false;
        return false;
    }

    sstable_index_entry_t* entry = &iter->reader->index[block_idx];

    // Allocate/reallocate block buffer
    if (iter->block_data == NULL || iter->block_size < entry->size) {
        free(iter->block_data);
        iter->block_data = malloc(entry->size);
        if (!iter->block_data) {
            iter->valid = false;
            return false;
        }
    }
    iter->block_size = entry->size;

    // Read block from file
    if (lseek(iter->reader->fd, entry->offset, SEEK_SET) < 0) {
        iter->valid = false;
        return false;
    }
    if (read_all(iter->reader->fd, iter->block_data, entry->size) != (ssize_t)entry->size) {
        iter->valid = false;
        return false;
    }

    // Parse block trailer
    if (entry->size < 8) {
        iter->valid = false;
        return false;
    }

    uint32_t num_restarts;
    memcpy(&num_restarts, iter->block_data + entry->size - 8, 4);
    iter->data_end = entry->size - 8 - num_restarts * 4;
    iter->pos = 0;
    iter->current_block = block_idx;

    // Clear previous key for new block
    free(iter->current_key);
    iter->current_key = NULL;
    iter->current_key_len = 0;

    return true;
}

// Helper: parse next entry in current block
static bool parse_next_entry(sstable_iter_t* iter) {
    if (iter->pos >= iter->data_end) {
        return false;
    }

    uint64_t shared, unshared, val_len;
    size_t n;

    n = decode_varint(iter->block_data + iter->pos, iter->data_end - iter->pos, &shared);
    if (n == 0) return false;
    iter->pos += n;

    n = decode_varint(iter->block_data + iter->pos, iter->data_end - iter->pos, &unshared);
    if (n == 0) return false;
    iter->pos += n;

    n = decode_varint(iter->block_data + iter->pos, iter->data_end - iter->pos, &val_len);
    if (n == 0) return false;
    iter->pos += n;

    if (iter->pos >= iter->data_end) return false;
    iter->current_deleted = (iter->block_data[iter->pos++] != 0);

    if (iter->pos + unshared + val_len > iter->data_end) return false;

    // Reconstruct full key
    size_t full_key_len = shared + unshared;
    char* new_key = malloc(full_key_len);
    if (!new_key) return false;

    if (shared > 0 && iter->current_key) {
        memcpy(new_key, iter->current_key, shared);
    }
    memcpy(new_key + shared, iter->block_data + iter->pos, unshared);
    iter->pos += unshared;

    free(iter->current_key);
    iter->current_key = new_key;
    iter->current_key_len = full_key_len;

    // Copy value
    free(iter->current_value);
    if (val_len > 0) {
        iter->current_value = malloc(val_len);
        if (!iter->current_value) return false;
        memcpy(iter->current_value, iter->block_data + iter->pos, val_len);
        iter->current_value_len = val_len;
    } else {
        iter->current_value = NULL;
        iter->current_value_len = 0;
    }
    iter->pos += val_len;

    return true;
}

// Seek to first entry
void sstable_iter_seek_to_first(sstable_iter_t* iter) {
    if (!iter || iter->reader->index_count == 0) {
        if (iter) iter->valid = false;
        return;
    }

    if (!load_block(iter, 0)) {
        return;
    }

    iter->valid = parse_next_entry(iter);
}

// Check if iterator is valid
bool sstable_iter_valid(sstable_iter_t* iter) {
    return iter && iter->valid;
}

// Move to next entry
void sstable_iter_next(sstable_iter_t* iter) {
    if (!iter || !iter->valid) return;

    // Try to parse next entry in current block
    if (parse_next_entry(iter)) {
        return;
    }

    // Move to next block
    if (iter->current_block + 1 < iter->reader->index_count) {
        if (load_block(iter, iter->current_block + 1)) {
            iter->valid = parse_next_entry(iter);
            return;
        }
    }

    iter->valid = false;
}

// Get current key
const char* sstable_iter_key(sstable_iter_t* iter, size_t* len) {
    if (!iter || !iter->valid || !len) return NULL;
    *len = iter->current_key_len;
    return iter->current_key;
}

// Get current value
const char* sstable_iter_value(sstable_iter_t* iter, size_t* len) {
    if (!iter || !iter->valid || !len) return NULL;
    *len = iter->current_value_len;
    return iter->current_value;
}

// Check if current entry is deleted
bool sstable_iter_is_deleted(sstable_iter_t* iter) {
    return iter && iter->valid && iter->current_deleted;
}

// ============================================================
// Merge Iterator (min-heap based)
// ============================================================

typedef struct {
    sstable_iter_t** iters;
    size_t* heap;       // Indices into iters array
    size_t heap_size;
    size_t iter_count;
    compare_fn cmp;
} merge_iter_t;

// Helper: compare two iterators by their current key
static int merge_compare(merge_iter_t* mi, size_t a, size_t b) {
    sstable_iter_t* iter_a = mi->iters[a];
    sstable_iter_t* iter_b = mi->iters[b];

    size_t key_a_len, key_b_len;
    const char* key_a = sstable_iter_key(iter_a, &key_a_len);
    const char* key_b = sstable_iter_key(iter_b, &key_b_len);

    int cmp = mi->cmp(key_a, key_a_len, key_b, key_b_len);
    if (cmp != 0) return cmp;

    // Same key: prefer higher index (newer file in L0)
    return (a < b) ? 1 : -1;
}

// Helper: sift down in min-heap
static void heap_sift_down(merge_iter_t* mi, size_t idx) {
    while (true) {
        size_t smallest = idx;
        size_t left = 2 * idx + 1;
        size_t right = 2 * idx + 2;

        if (left < mi->heap_size &&
            merge_compare(mi, mi->heap[left], mi->heap[smallest]) < 0) {
            smallest = left;
        }
        if (right < mi->heap_size &&
            merge_compare(mi, mi->heap[right], mi->heap[smallest]) < 0) {
            smallest = right;
        }

        if (smallest == idx) break;

        size_t tmp = mi->heap[idx];
        mi->heap[idx] = mi->heap[smallest];
        mi->heap[smallest] = tmp;
        idx = smallest;
    }
}

// Create merge iterator
static merge_iter_t* merge_iter_create(sstable_iter_t** iters, size_t count, compare_fn cmp) {
    merge_iter_t* mi = calloc(1, sizeof(merge_iter_t));
    if (!mi) return NULL;

    mi->iters = iters;
    mi->iter_count = count;
    mi->cmp = cmp;
    mi->heap = malloc(count * sizeof(size_t));
    if (!mi->heap) {
        free(mi);
        return NULL;
    }

    // Initialize heap with valid iterators
    mi->heap_size = 0;
    for (size_t i = 0; i < count; i++) {
        if (sstable_iter_valid(iters[i])) {
            mi->heap[mi->heap_size++] = i;
        }
    }

    // Build min-heap
    for (size_t i = mi->heap_size / 2; i > 0; i--) {
        heap_sift_down(mi, i - 1);
    }
    if (mi->heap_size > 0) {
        heap_sift_down(mi, 0);
    }

    return mi;
}

// Destroy merge iterator
static void merge_iter_destroy(merge_iter_t* mi) {
    if (!mi) return;
    free(mi->heap);
    free(mi);
}

// Check if merge iterator is valid
static bool merge_iter_valid(merge_iter_t* mi) {
    return mi && mi->heap_size > 0;
}

// Get current entry from merge iterator
static void merge_iter_current(merge_iter_t* mi,
                               const char** key, size_t* key_len,
                               const char** value, size_t* value_len,
                               bool* deleted) {
    if (!merge_iter_valid(mi)) return;

    size_t top_idx = mi->heap[0];
    sstable_iter_t* iter = mi->iters[top_idx];

    *key = sstable_iter_key(iter, key_len);
    *value = sstable_iter_value(iter, value_len);
    *deleted = sstable_iter_is_deleted(iter);
}

// Advance merge iterator (skip duplicates)
static void merge_iter_next(merge_iter_t* mi) {
    if (!merge_iter_valid(mi)) return;

    // Get current key for duplicate detection
    size_t top_idx = mi->heap[0];
    sstable_iter_t* top_iter = mi->iters[top_idx];
    size_t cur_key_len;
    const char* cur_key = sstable_iter_key(top_iter, &cur_key_len);

    // Save current key for comparison
    char* saved_key = malloc(cur_key_len);
    if (!saved_key) return;
    memcpy(saved_key, cur_key, cur_key_len);
    size_t saved_key_len = cur_key_len;

    // Advance all iterators with the same key
    while (mi->heap_size > 0) {
        top_idx = mi->heap[0];
        top_iter = mi->iters[top_idx];

        size_t key_len;
        const char* key = sstable_iter_key(top_iter, &key_len);

        if (mi->cmp(key, key_len, saved_key, saved_key_len) != 0) {
            break;  // Different key, stop
        }

        // Advance this iterator
        sstable_iter_next(top_iter);

        if (sstable_iter_valid(top_iter)) {
            // Re-heapify
            heap_sift_down(mi, 0);
        } else {
            // Remove from heap
            mi->heap[0] = mi->heap[--mi->heap_size];
            if (mi->heap_size > 0) {
                heap_sift_down(mi, 0);
            }
        }
    }

    free(saved_key);
}

// ============================================================
// Compaction
// ============================================================

// Pick which level needs compaction
int compact_pick_level(level_manager_t* lm) {
    if (!lm) return -1;

    // Check L0 first (highest priority)
    if (level_needs_compaction(lm, 0)) {
        return 0;
    }

    // Check other levels
    for (int level = 1; level < MAX_LEVELS - 1; level++) {
        if (level_needs_compaction(lm, level)) {
            return level;
        }
    }

    return -1;  // No compaction needed
}

// Helper: get metadata for a file number
static sstable_meta_t* find_meta(level_manager_t* lm, int level, uint64_t file_num) {
    level_t* lvl = &lm->levels[level];
    for (size_t i = 0; i < lvl->file_count; i++) {
        if (lvl->files[i].file_number == file_num) {
            return &lvl->files[i];
        }
    }
    return NULL;
}

// Compact a level
status_t compact_level(level_manager_t* lm, int level) {
    if (!lm || level < 0 || level >= MAX_LEVELS - 1) {
        return STATUS_INVALID_ARG;
    }

    level_t* src_level = &lm->levels[level];
    int target_level = level + 1;

    if (src_level->file_count == 0) {
        return STATUS_OK;
    }

    // Collect input files from source level
    uint64_t* input_files = NULL;
    size_t input_count = 0;
    char* min_key = NULL;
    size_t min_key_len = 0;
    char* max_key = NULL;
    size_t max_key_len = 0;

    if (level == 0) {
        // L0: compact all files
        input_count = src_level->file_count;
        input_files = malloc(input_count * sizeof(uint64_t));
        if (!input_files) return STATUS_NO_MEMORY;

        for (size_t i = 0; i < input_count; i++) {
            input_files[i] = src_level->files[i].file_number;

            // Track overall key range
            sstable_meta_t* meta = &src_level->files[i];
            if (!min_key || lm->cmp(meta->min_key, meta->min_key_len,
                                    min_key, min_key_len) < 0) {
                min_key = meta->min_key;
                min_key_len = meta->min_key_len;
            }
            if (!max_key || lm->cmp(meta->max_key, meta->max_key_len,
                                    max_key, max_key_len) > 0) {
                max_key = meta->max_key;
                max_key_len = meta->max_key_len;
            }
        }
    } else {
        // L1+: pick first file
        input_count = 1;
        input_files = malloc(sizeof(uint64_t));
        if (!input_files) return STATUS_NO_MEMORY;
        input_files[0] = src_level->files[0].file_number;

        sstable_meta_t* meta = &src_level->files[0];
        min_key = meta->min_key;
        min_key_len = meta->min_key_len;
        max_key = meta->max_key;
        max_key_len = meta->max_key_len;
    }

    // Find overlapping files in target level
    uint64_t* target_files = NULL;
    size_t target_count = level_find_overlapping(lm, target_level,
                                                  min_key, min_key_len,
                                                  max_key, max_key_len,
                                                  &target_files);

    // Create iterators for all input files
    size_t total_iters = input_count + target_count;
    sstable_iter_t** iters = calloc(total_iters, sizeof(sstable_iter_t*));
    if (!iters) {
        free(input_files);
        free(target_files);
        return STATUS_NO_MEMORY;
    }

    size_t iter_idx = 0;

    // Add source level iterators
    for (size_t i = 0; i < input_count; i++) {
        sstable_meta_t* meta = find_meta(lm, level, input_files[i]);
        if (meta && meta->reader) {
            iters[iter_idx] = sstable_iter_create(meta->reader);
            if (iters[iter_idx]) {
                sstable_iter_seek_to_first(iters[iter_idx]);
                iter_idx++;
            }
        }
    }

    // Add target level iterators
    for (size_t i = 0; i < target_count; i++) {
        sstable_meta_t* meta = find_meta(lm, target_level, target_files[i]);
        if (meta && meta->reader) {
            iters[iter_idx] = sstable_iter_create(meta->reader);
            if (iters[iter_idx]) {
                sstable_iter_seek_to_first(iters[iter_idx]);
                iter_idx++;
            }
        }
    }

    // Create merge iterator
    merge_iter_t* merge = merge_iter_create(iters, iter_idx, lm->cmp);
    if (!merge) {
        for (size_t i = 0; i < iter_idx; i++) {
            sstable_iter_destroy(iters[i]);
        }
        free(iters);
        free(input_files);
        free(target_files);
        return STATUS_NO_MEMORY;
    }

    // Generate output file path
    uint64_t output_file_num = level_next_file_number(lm);
    char output_path[512];
    snprintf(output_path, sizeof(output_path), "%s/%06llu.sst",
             lm->db_path, (unsigned long long)output_file_num);

    // Count entries for bloom filter sizing
    size_t estimated_entries = 0;
    for (size_t i = 0; i < input_count; i++) {
        sstable_meta_t* meta = find_meta(lm, level, input_files[i]);
        if (meta && meta->reader) {
            estimated_entries += sstable_reader_num_entries(meta->reader);
        }
    }
    for (size_t i = 0; i < target_count; i++) {
        sstable_meta_t* meta = find_meta(lm, target_level, target_files[i]);
        if (meta && meta->reader) {
            estimated_entries += sstable_reader_num_entries(meta->reader);
        }
    }

    // Create output SSTable
    sstable_writer_t* writer = sstable_writer_create(output_path, estimated_entries, lm->cmp);
    if (!writer) {
        merge_iter_destroy(merge);
        for (size_t i = 0; i < iter_idx; i++) {
            sstable_iter_destroy(iters[i]);
        }
        free(iters);
        free(input_files);
        free(target_files);
        return STATUS_IO_ERROR;
    }

    // Merge and write entries
    bool is_bottommost = (target_level == MAX_LEVELS - 1);
    while (merge_iter_valid(merge)) {
        const char* key;
        const char* value;
        size_t key_len, value_len;
        bool deleted;

        merge_iter_current(merge, &key, &key_len, &value, &value_len, &deleted);

        // Skip tombstones at bottommost level
        if (!(deleted && is_bottommost)) {
            status_t status = sstable_writer_add(writer, key, key_len,
                                                  value, value_len, deleted);
            if (status != STATUS_OK) {
                sstable_writer_abort(writer);
                merge_iter_destroy(merge);
                for (size_t i = 0; i < iter_idx; i++) {
                    sstable_iter_destroy(iters[i]);
                }
                free(iters);
                free(input_files);
                free(target_files);
                return status;
            }
        }

        merge_iter_next(merge);
    }

    // Finish writing
    status_t status = sstable_writer_finish(writer);
    if (status != STATUS_OK) {
        merge_iter_destroy(merge);
        for (size_t i = 0; i < iter_idx; i++) {
            sstable_iter_destroy(iters[i]);
        }
        free(iters);
        free(input_files);
        free(target_files);
        return status;
    }

    // Cleanup iterators
    merge_iter_destroy(merge);
    for (size_t i = 0; i < iter_idx; i++) {
        sstable_iter_destroy(iters[i]);
    }
    free(iters);

    // Remove old files from level manager
    for (size_t i = 0; i < input_count; i++) {
        sstable_meta_t* meta = find_meta(lm, level, input_files[i]);
        if (meta) {
            char* old_path = strdup(meta->path);
            level_remove_sstable(lm, level, input_files[i]);
            if (old_path) {
                unlink(old_path);
                free(old_path);
            }
        }
    }
    for (size_t i = 0; i < target_count; i++) {
        sstable_meta_t* meta = find_meta(lm, target_level, target_files[i]);
        if (meta) {
            char* old_path = strdup(meta->path);
            level_remove_sstable(lm, target_level, target_files[i]);
            if (old_path) {
                unlink(old_path);
                free(old_path);
            }
        }
    }

    free(input_files);
    free(target_files);

    // Add new file to target level
    sstable_reader_t* new_reader = sstable_reader_open(output_path, lm->cmp);
    if (!new_reader) {
        return STATUS_IO_ERROR;
    }

    status = level_add_sstable(lm, target_level, output_file_num, output_path, new_reader);
    if (status != STATUS_OK) {
        sstable_reader_close(new_reader);
        return status;
    }

    return STATUS_OK;
}
