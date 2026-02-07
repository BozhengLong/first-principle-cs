#ifndef SSTABLE_H
#define SSTABLE_H

#include "types.h"
#include "bloom.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// SSTable magic number
#define SSTABLE_MAGIC 0x535354424C455631ULL  // "SSTBLEV1"

// Maximum key size for footer
#define SSTABLE_MAX_KEY_SIZE 256

// Footer structure (fixed size for easy reading)
typedef struct {
    uint64_t index_offset;
    uint32_t index_size;
    uint64_t bloom_offset;
    uint32_t bloom_size;
    uint64_t num_entries;
    uint32_t min_key_len;
    char min_key[SSTABLE_MAX_KEY_SIZE];
    uint32_t max_key_len;
    char max_key[SSTABLE_MAX_KEY_SIZE];
    uint64_t magic;
    uint32_t crc32;
} sstable_footer_t;

// Index entry (points to a data block)
typedef struct {
    char* last_key;         // Last key in the block
    size_t last_key_len;
    uint64_t offset;        // Block offset in file
    uint32_t size;          // Block size
} sstable_index_entry_t;

// SSTable writer
struct sstable_writer {
    char* path;
    int fd;
    compare_fn cmp;

    // Current block buffer
    uint8_t* block_buf;
    size_t block_size;
    size_t block_offset;

    // Restart points within current block
    uint32_t* restarts;
    size_t restart_count;
    size_t restart_capacity;
    size_t entries_since_restart;

    // Previous key for prefix compression
    char* prev_key;
    size_t prev_key_len;

    // Index entries
    sstable_index_entry_t* index;
    size_t index_count;
    size_t index_capacity;

    // Bloom filter
    bloom_filter_t* bloom;

    // Statistics
    uint64_t num_entries;
    uint64_t file_offset;

    // Min/max keys
    char* min_key;
    size_t min_key_len;
    char* max_key;
    size_t max_key_len;
};

// SSTable reader
struct sstable_reader {
    char* path;
    int fd;
    compare_fn cmp;

    // Footer info
    sstable_footer_t footer;

    // Index (loaded into memory)
    sstable_index_entry_t* index;
    size_t index_count;

    // Bloom filter
    bloom_filter_t* bloom;
};

// Writer API
sstable_writer_t* sstable_writer_create(const char* path,
                                         size_t estimated_entries,
                                         compare_fn cmp);
status_t sstable_writer_add(sstable_writer_t* writer,
                            const char* key, size_t key_len,
                            const char* value, size_t value_len,
                            bool deleted);
status_t sstable_writer_finish(sstable_writer_t* writer);
void sstable_writer_abort(sstable_writer_t* writer);

// Reader API
sstable_reader_t* sstable_reader_open(const char* path, compare_fn cmp);
void sstable_reader_close(sstable_reader_t* reader);
status_t sstable_reader_get(sstable_reader_t* reader,
                            const char* key, size_t key_len,
                            char** value, size_t* value_len,
                            bool* deleted);

// Utility
const char* sstable_reader_min_key(sstable_reader_t* reader, size_t* len);
const char* sstable_reader_max_key(sstable_reader_t* reader, size_t* len);
uint64_t sstable_reader_num_entries(sstable_reader_t* reader);

#endif // SSTABLE_H
