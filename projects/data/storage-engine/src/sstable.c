#include "sstable.h"
#include "param.h"
#include "crc32.h"
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stddef.h>

// Helper: write all bytes to fd
static ssize_t write_all(int fd, const void* buf, size_t len) {
    const uint8_t* p = buf;
    size_t remaining = len;
    while (remaining > 0) {
        ssize_t n = write(fd, p, remaining);
        if (n <= 0) return -1;
        p += n;
        remaining -= n;
    }
    return (ssize_t)len;
}

// Helper: read all bytes from fd
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

// Helper: compute shared prefix length
static size_t shared_prefix_len(const char* a, size_t a_len,
                                 const char* b, size_t b_len) {
    size_t min_len = a_len < b_len ? a_len : b_len;
    size_t i = 0;
    while (i < min_len && a[i] == b[i]) i++;
    return i;
}

// Helper: encode varint (returns bytes written)
static size_t encode_varint(uint8_t* buf, uint64_t value) {
    size_t i = 0;
    while (value >= 0x80) {
        buf[i++] = (uint8_t)(value | 0x80);
        value >>= 7;
    }
    buf[i++] = (uint8_t)value;
    return i;
}

// Helper: decode varint (returns bytes read, 0 on error)
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
    return 0;  // Error: varint too long or truncated
}

// ============================================================
// SSTable Writer
// ============================================================

sstable_writer_t* sstable_writer_create(const char* path,
                                         size_t estimated_entries,
                                         compare_fn cmp) {
    if (!path) return NULL;

    sstable_writer_t* w = calloc(1, sizeof(sstable_writer_t));
    if (!w) return NULL;

    w->path = strdup(path);
    if (!w->path) {
        free(w);
        return NULL;
    }

    w->fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (w->fd < 0) {
        free(w->path);
        free(w);
        return NULL;
    }

    w->cmp = cmp ? cmp : default_compare;
    w->block_size = SSTABLE_BLOCK_SIZE;
    w->block_buf = malloc(w->block_size * 2);  // Extra space for overflow
    if (!w->block_buf) {
        close(w->fd);
        free(w->path);
        free(w);
        return NULL;
    }
    w->block_offset = 0;

    // Initialize restart points
    w->restart_capacity = 64;
    w->restarts = malloc(w->restart_capacity * sizeof(uint32_t));
    if (!w->restarts) {
        free(w->block_buf);
        close(w->fd);
        free(w->path);
        free(w);
        return NULL;
    }
    w->restart_count = 0;
    w->entries_since_restart = SSTABLE_RESTART_INTERVAL;  // Force first restart

    // Initialize index
    w->index_capacity = 64;
    w->index = malloc(w->index_capacity * sizeof(sstable_index_entry_t));
    if (!w->index) {
        free(w->restarts);
        free(w->block_buf);
        close(w->fd);
        free(w->path);
        free(w);
        return NULL;
    }
    w->index_count = 0;

    // Create bloom filter
    w->bloom = bloom_create(estimated_entries > 0 ? estimated_entries : 1000);
    if (!w->bloom) {
        free(w->index);
        free(w->restarts);
        free(w->block_buf);
        close(w->fd);
        free(w->path);
        free(w);
        return NULL;
    }

    w->num_entries = 0;
    w->file_offset = 0;
    w->prev_key = NULL;
    w->prev_key_len = 0;
    w->min_key = NULL;
    w->max_key = NULL;

    return w;
}

// Helper: flush current block to file
static status_t flush_block(sstable_writer_t* w, const char* last_key, size_t last_key_len) {
    if (w->block_offset == 0) return STATUS_OK;

    // Write restart offsets at end of block
    for (size_t i = 0; i < w->restart_count; i++) {
        uint32_t offset = w->restarts[i];
        memcpy(w->block_buf + w->block_offset, &offset, 4);
        w->block_offset += 4;
    }

    // Write number of restarts
    uint32_t num_restarts = (uint32_t)w->restart_count;
    memcpy(w->block_buf + w->block_offset, &num_restarts, 4);
    w->block_offset += 4;

    // Write CRC32
    uint32_t block_crc = crc32(w->block_buf, w->block_offset);
    memcpy(w->block_buf + w->block_offset, &block_crc, 4);
    w->block_offset += 4;

    // Write block to file
    if (write_all(w->fd, w->block_buf, w->block_offset) < 0) {
        return STATUS_IO_ERROR;
    }

    // Add index entry
    if (w->index_count >= w->index_capacity) {
        size_t new_cap = w->index_capacity * 2;
        sstable_index_entry_t* new_idx = realloc(w->index, new_cap * sizeof(sstable_index_entry_t));
        if (!new_idx) return STATUS_NO_MEMORY;
        w->index = new_idx;
        w->index_capacity = new_cap;
    }

    sstable_index_entry_t* entry = &w->index[w->index_count++];
    entry->last_key = malloc(last_key_len);
    if (!entry->last_key) return STATUS_NO_MEMORY;
    memcpy(entry->last_key, last_key, last_key_len);
    entry->last_key_len = last_key_len;
    entry->offset = w->file_offset;
    entry->size = (uint32_t)w->block_offset;

    w->file_offset += w->block_offset;

    // Reset block state
    w->block_offset = 0;
    w->restart_count = 0;
    w->entries_since_restart = SSTABLE_RESTART_INTERVAL;
    free(w->prev_key);
    w->prev_key = NULL;
    w->prev_key_len = 0;

    return STATUS_OK;
}

// Add entry to SSTable (must be called in sorted order)
status_t sstable_writer_add(sstable_writer_t* w,
                            const char* key, size_t key_len,
                            const char* value, size_t value_len,
                            bool deleted) {
    if (!w || !key) return STATUS_INVALID_ARG;

    // Add to bloom filter
    bloom_add(w->bloom, key, key_len);

    // Track min/max keys
    if (!w->min_key) {
        w->min_key = malloc(key_len);
        if (!w->min_key) return STATUS_NO_MEMORY;
        memcpy(w->min_key, key, key_len);
        w->min_key_len = key_len;
    }
    // Always update max key (since keys come in sorted order)
    free(w->max_key);
    w->max_key = malloc(key_len);
    if (!w->max_key) return STATUS_NO_MEMORY;
    memcpy(w->max_key, key, key_len);
    w->max_key_len = key_len;

    // Check if we need a restart point
    bool is_restart = (w->entries_since_restart >= SSTABLE_RESTART_INTERVAL);

    // Calculate prefix compression
    size_t shared = 0;
    if (!is_restart && w->prev_key) {
        shared = shared_prefix_len(w->prev_key, w->prev_key_len, key, key_len);
    }
    size_t unshared = key_len - shared;

    // Encode entry: shared | unshared | value_len | deleted | key_delta | value
    uint8_t entry_buf[32];  // For varints
    size_t entry_len = 0;
    entry_len += encode_varint(entry_buf + entry_len, shared);
    entry_len += encode_varint(entry_buf + entry_len, unshared);
    entry_len += encode_varint(entry_buf + entry_len, value_len);
    entry_buf[entry_len++] = deleted ? 1 : 0;

    size_t total_entry_size = entry_len + unshared + value_len;

    // Check if block is full (leave room for restarts + crc)
    size_t overhead = (w->restart_count + 1) * 4 + 8;  // restarts + num_restarts + crc
    if (w->block_offset + total_entry_size + overhead > w->block_size && w->block_offset > 0) {
        // Flush current block
        status_t status = flush_block(w, w->prev_key, w->prev_key_len);
        if (status != STATUS_OK) return status;
        is_restart = true;
        shared = 0;
        unshared = key_len;
        // Re-encode without prefix compression
        entry_len = 0;
        entry_len += encode_varint(entry_buf + entry_len, 0);
        entry_len += encode_varint(entry_buf + entry_len, key_len);
        entry_len += encode_varint(entry_buf + entry_len, value_len);
        entry_buf[entry_len++] = deleted ? 1 : 0;
    }

    // Record restart point if needed
    if (is_restart) {
        if (w->restart_count >= w->restart_capacity) {
            size_t new_cap = w->restart_capacity * 2;
            uint32_t* new_restarts = realloc(w->restarts, new_cap * sizeof(uint32_t));
            if (!new_restarts) return STATUS_NO_MEMORY;
            w->restarts = new_restarts;
            w->restart_capacity = new_cap;
        }
        w->restarts[w->restart_count++] = (uint32_t)w->block_offset;
        w->entries_since_restart = 0;
    }

    // Write entry to block buffer
    memcpy(w->block_buf + w->block_offset, entry_buf, entry_len);
    w->block_offset += entry_len;
    memcpy(w->block_buf + w->block_offset, key + shared, unshared);
    w->block_offset += unshared;
    if (value && value_len > 0) {
        memcpy(w->block_buf + w->block_offset, value, value_len);
        w->block_offset += value_len;
    }

    // Update previous key
    free(w->prev_key);
    w->prev_key = malloc(key_len);
    if (!w->prev_key) return STATUS_NO_MEMORY;
    memcpy(w->prev_key, key, key_len);
    w->prev_key_len = key_len;

    w->num_entries++;
    w->entries_since_restart++;

    return STATUS_OK;
}

// Finish writing SSTable
status_t sstable_writer_finish(sstable_writer_t* w) {
    if (!w) return STATUS_INVALID_ARG;

    // Flush remaining data block
    if (w->block_offset > 0 && w->prev_key) {
        status_t status = flush_block(w, w->prev_key, w->prev_key_len);
        if (status != STATUS_OK) return status;
    }

    // Write index block
    uint64_t index_offset = w->file_offset;
    for (size_t i = 0; i < w->index_count; i++) {
        sstable_index_entry_t* entry = &w->index[i];
        uint8_t buf[32];
        size_t len = 0;
        len += encode_varint(buf + len, entry->last_key_len);
        if (write_all(w->fd, buf, len) < 0) return STATUS_IO_ERROR;
        if (write_all(w->fd, entry->last_key, entry->last_key_len) < 0) return STATUS_IO_ERROR;
        if (write_all(w->fd, &entry->offset, 8) < 0) return STATUS_IO_ERROR;
        if (write_all(w->fd, &entry->size, 4) < 0) return STATUS_IO_ERROR;
        w->file_offset += len + entry->last_key_len + 12;
    }
    uint32_t index_size = (uint32_t)(w->file_offset - index_offset);

    // Write bloom filter
    uint64_t bloom_offset = w->file_offset;
    size_t bloom_size = bloom_serialized_size(w->bloom);
    uint8_t* bloom_buf = malloc(bloom_size);
    if (!bloom_buf) return STATUS_NO_MEMORY;
    bloom_serialize(w->bloom, bloom_buf, bloom_size);
    if (write_all(w->fd, bloom_buf, bloom_size) < 0) {
        free(bloom_buf);
        return STATUS_IO_ERROR;
    }
    free(bloom_buf);
    w->file_offset += bloom_size;

    // Write footer
    sstable_footer_t footer = {0};
    footer.index_offset = index_offset;
    footer.index_size = index_size;
    footer.bloom_offset = bloom_offset;
    footer.bloom_size = (uint32_t)bloom_size;
    footer.num_entries = w->num_entries;

    if (w->min_key && w->min_key_len <= SSTABLE_MAX_KEY_SIZE) {
        footer.min_key_len = (uint32_t)w->min_key_len;
        memcpy(footer.min_key, w->min_key, w->min_key_len);
    }
    if (w->max_key && w->max_key_len <= SSTABLE_MAX_KEY_SIZE) {
        footer.max_key_len = (uint32_t)w->max_key_len;
        memcpy(footer.max_key, w->max_key, w->max_key_len);
    }

    footer.magic = SSTABLE_MAGIC;
    footer.crc32 = crc32(&footer, offsetof(sstable_footer_t, crc32));

    if (write_all(w->fd, &footer, sizeof(footer)) < 0) return STATUS_IO_ERROR;

    // Cleanup
    close(w->fd);
    w->fd = -1;

    // Free resources
    for (size_t i = 0; i < w->index_count; i++) {
        free(w->index[i].last_key);
    }
    free(w->index);
    free(w->restarts);
    free(w->block_buf);
    free(w->prev_key);
    free(w->min_key);
    free(w->max_key);
    bloom_destroy(w->bloom);
    free(w->path);
    free(w);

    return STATUS_OK;
}

// Abort writing (cleanup without finishing)
void sstable_writer_abort(sstable_writer_t* w) {
    if (!w) return;

    if (w->fd >= 0) {
        close(w->fd);
        unlink(w->path);
    }

    for (size_t i = 0; i < w->index_count; i++) {
        free(w->index[i].last_key);
    }
    free(w->index);
    free(w->restarts);
    free(w->block_buf);
    free(w->prev_key);
    free(w->min_key);
    free(w->max_key);
    bloom_destroy(w->bloom);
    free(w->path);
    free(w);
}

// ============================================================
// SSTable Reader
// ============================================================

sstable_reader_t* sstable_reader_open(const char* path, compare_fn cmp) {
    if (!path) return NULL;

    sstable_reader_t* r = calloc(1, sizeof(sstable_reader_t));
    if (!r) return NULL;

    r->path = strdup(path);
    if (!r->path) {
        free(r);
        return NULL;
    }

    r->fd = open(path, O_RDONLY);
    if (r->fd < 0) {
        free(r->path);
        free(r);
        return NULL;
    }

    r->cmp = cmp ? cmp : default_compare;

    // Get file size
    struct stat st;
    if (fstat(r->fd, &st) < 0) {
        close(r->fd);
        free(r->path);
        free(r);
        return NULL;
    }

    // Read footer
    if ((size_t)st.st_size < sizeof(sstable_footer_t)) {
        close(r->fd);
        free(r->path);
        free(r);
        return NULL;
    }

    if (lseek(r->fd, st.st_size - sizeof(sstable_footer_t), SEEK_SET) < 0) {
        close(r->fd);
        free(r->path);
        free(r);
        return NULL;
    }

    if (read_all(r->fd, &r->footer, sizeof(sstable_footer_t)) != sizeof(sstable_footer_t)) {
        close(r->fd);
        free(r->path);
        free(r);
        return NULL;
    }

    // Verify magic and CRC
    if (r->footer.magic != SSTABLE_MAGIC) {
        close(r->fd);
        free(r->path);
        free(r);
        return NULL;
    }

    uint32_t expected_crc = crc32(&r->footer, offsetof(sstable_footer_t, crc32));
    if (r->footer.crc32 != expected_crc) {
        close(r->fd);
        free(r->path);
        free(r);
        return NULL;
    }

    // Read bloom filter
    if (lseek(r->fd, r->footer.bloom_offset, SEEK_SET) < 0) {
        close(r->fd);
        free(r->path);
        free(r);
        return NULL;
    }

    uint8_t* bloom_buf = malloc(r->footer.bloom_size);
    if (!bloom_buf) {
        close(r->fd);
        free(r->path);
        free(r);
        return NULL;
    }

    if (read_all(r->fd, bloom_buf, r->footer.bloom_size) != (ssize_t)r->footer.bloom_size) {
        free(bloom_buf);
        close(r->fd);
        free(r->path);
        free(r);
        return NULL;
    }

    r->bloom = bloom_deserialize(bloom_buf, r->footer.bloom_size);
    free(bloom_buf);
    if (!r->bloom) {
        close(r->fd);
        free(r->path);
        free(r);
        return NULL;
    }

    // Read index
    if (lseek(r->fd, r->footer.index_offset, SEEK_SET) < 0) {
        bloom_destroy(r->bloom);
        close(r->fd);
        free(r->path);
        free(r);
        return NULL;
    }

    // Count index entries by parsing
    uint8_t* index_buf = malloc(r->footer.index_size);
    if (!index_buf) {
        bloom_destroy(r->bloom);
        close(r->fd);
        free(r->path);
        free(r);
        return NULL;
    }

    if (read_all(r->fd, index_buf, r->footer.index_size) != (ssize_t)r->footer.index_size) {
        free(index_buf);
        bloom_destroy(r->bloom);
        close(r->fd);
        free(r->path);
        free(r);
        return NULL;
    }

    // Parse index entries
    size_t capacity = 16;
    r->index = malloc(capacity * sizeof(sstable_index_entry_t));
    if (!r->index) {
        free(index_buf);
        bloom_destroy(r->bloom);
        close(r->fd);
        free(r->path);
        free(r);
        return NULL;
    }
    r->index_count = 0;

    size_t pos = 0;
    while (pos < r->footer.index_size) {
        if (r->index_count >= capacity) {
            capacity *= 2;
            sstable_index_entry_t* new_idx = realloc(r->index, capacity * sizeof(sstable_index_entry_t));
            if (!new_idx) {
                for (size_t i = 0; i < r->index_count; i++) free(r->index[i].last_key);
                free(r->index);
                free(index_buf);
                bloom_destroy(r->bloom);
                close(r->fd);
                free(r->path);
                free(r);
                return NULL;
            }
            r->index = new_idx;
        }

        uint64_t key_len;
        size_t n = decode_varint(index_buf + pos, r->footer.index_size - pos, &key_len);
        if (n == 0) break;
        pos += n;

        if (pos + key_len + 12 > r->footer.index_size) break;

        sstable_index_entry_t* entry = &r->index[r->index_count++];
        entry->last_key = malloc(key_len);
        if (!entry->last_key) break;
        memcpy(entry->last_key, index_buf + pos, key_len);
        entry->last_key_len = key_len;
        pos += key_len;

        memcpy(&entry->offset, index_buf + pos, 8);
        pos += 8;
        memcpy(&entry->size, index_buf + pos, 4);
        pos += 4;
    }

    free(index_buf);
    return r;
}

void sstable_reader_close(sstable_reader_t* r) {
    if (!r) return;

    for (size_t i = 0; i < r->index_count; i++) {
        free(r->index[i].last_key);
    }
    free(r->index);
    bloom_destroy(r->bloom);
    close(r->fd);
    free(r->path);
    free(r);
}

// Helper: search for key in a data block
static status_t search_block(sstable_reader_t* r, uint8_t* block, size_t block_size,
                              const char* key, size_t key_len,
                              char** value, size_t* value_len, bool* deleted) {
    // Read trailer: num_restarts (4B) + crc32 (4B)
    if (block_size < 8) return STATUS_CORRUPTION;

    uint32_t num_restarts;
    memcpy(&num_restarts, block + block_size - 8, 4);

    uint32_t stored_crc;
    memcpy(&stored_crc, block + block_size - 4, 4);

    // Verify CRC (excluding the CRC itself)
    uint32_t computed_crc = crc32(block, block_size - 4);
    if (computed_crc != stored_crc) return STATUS_CORRUPTION;

    // Calculate data end (before restart offsets)
    size_t restarts_start = block_size - 8 - num_restarts * 4;

    // Binary search restart points to find starting position
    uint32_t* restarts = (uint32_t*)(block + restarts_start);
    size_t left = 0, right = num_restarts;

    // Build key at each restart point and binary search
    while (left < right) {
        size_t mid = left + (right - left) / 2;
        uint32_t restart_offset = restarts[mid];

        // Decode key at restart point (shared=0)
        size_t pos = restart_offset;
        uint64_t shared, unshared, val_len;
        size_t n = decode_varint(block + pos, restarts_start - pos, &shared);
        if (n == 0 || shared != 0) return STATUS_CORRUPTION;
        pos += n;
        n = decode_varint(block + pos, restarts_start - pos, &unshared);
        if (n == 0) return STATUS_CORRUPTION;
        pos += n;
        n = decode_varint(block + pos, restarts_start - pos, &val_len);
        if (n == 0) return STATUS_CORRUPTION;
        pos += n;
        pos++;  // Skip deleted flag

        if (pos + unshared > restarts_start) return STATUS_CORRUPTION;

        int cmp = r->cmp((const char*)(block + pos), unshared, key, key_len);
        if (cmp < 0) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }

    // Start from the restart point before or at our target
    size_t start_restart = (left > 0) ? left - 1 : 0;
    size_t pos = restarts[start_restart];

    // Linear scan from restart point
    char* current_key = NULL;
    size_t current_key_len = 0;

    while (pos < restarts_start) {
        uint64_t shared, unshared, val_len;
        size_t n = decode_varint(block + pos, restarts_start - pos, &shared);
        if (n == 0) break;
        pos += n;
        n = decode_varint(block + pos, restarts_start - pos, &unshared);
        if (n == 0) break;
        pos += n;
        n = decode_varint(block + pos, restarts_start - pos, &val_len);
        if (n == 0) break;
        pos += n;

        uint8_t is_deleted = block[pos++];

        if (pos + unshared + val_len > restarts_start) break;

        // Reconstruct full key
        size_t full_key_len = shared + unshared;
        char* full_key = malloc(full_key_len);
        if (!full_key) {
            free(current_key);
            return STATUS_NO_MEMORY;
        }

        if (shared > 0 && current_key) {
            memcpy(full_key, current_key, shared);
        }
        memcpy(full_key + shared, block + pos, unshared);
        pos += unshared;

        free(current_key);
        current_key = full_key;
        current_key_len = full_key_len;

        int cmp = r->cmp(current_key, current_key_len, key, key_len);
        if (cmp == 0) {
            // Found it
            *deleted = (is_deleted != 0);
            if (!*deleted && val_len > 0) {
                *value = malloc(val_len);
                if (!*value) {
                    free(current_key);
                    return STATUS_NO_MEMORY;
                }
                memcpy(*value, block + pos, val_len);
                *value_len = val_len;
            } else {
                *value = NULL;
                *value_len = 0;
            }
            free(current_key);
            return STATUS_OK;
        } else if (cmp > 0) {
            // Passed the target key
            free(current_key);
            return STATUS_NOT_FOUND;
        }

        pos += val_len;
    }

    free(current_key);
    return STATUS_NOT_FOUND;
}

// Get value for key from SSTable
status_t sstable_reader_get(sstable_reader_t* r,
                            const char* key, size_t key_len,
                            char** value, size_t* value_len,
                            bool* deleted) {
    if (!r || !key || !value || !value_len || !deleted) return STATUS_INVALID_ARG;

    *value = NULL;
    *value_len = 0;
    *deleted = false;

    // Check bloom filter first
    if (!bloom_may_contain(r->bloom, key, key_len)) {
        return STATUS_NOT_FOUND;
    }

    // Binary search index to find candidate block
    size_t left = 0, right = r->index_count;
    while (left < right) {
        size_t mid = left + (right - left) / 2;
        int cmp = r->cmp(r->index[mid].last_key, r->index[mid].last_key_len, key, key_len);
        if (cmp < 0) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }

    if (left >= r->index_count) {
        return STATUS_NOT_FOUND;
    }

    // Read and search the candidate block
    sstable_index_entry_t* entry = &r->index[left];
    uint8_t* block = malloc(entry->size);
    if (!block) return STATUS_NO_MEMORY;

    if (lseek(r->fd, entry->offset, SEEK_SET) < 0) {
        free(block);
        return STATUS_IO_ERROR;
    }

    if (read_all(r->fd, block, entry->size) != (ssize_t)entry->size) {
        free(block);
        return STATUS_IO_ERROR;
    }

    status_t status = search_block(r, block, entry->size, key, key_len, value, value_len, deleted);
    free(block);
    return status;
}

// Utility functions
const char* sstable_reader_min_key(sstable_reader_t* r, size_t* len) {
    if (!r || !len) return NULL;
    *len = r->footer.min_key_len;
    return r->footer.min_key;
}

const char* sstable_reader_max_key(sstable_reader_t* r, size_t* len) {
    if (!r || !len) return NULL;
    *len = r->footer.max_key_len;
    return r->footer.max_key;
}

uint64_t sstable_reader_num_entries(sstable_reader_t* r) {
    return r ? r->footer.num_entries : 0;
}
