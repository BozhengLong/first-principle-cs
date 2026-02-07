#include "storage.h"
#include "compact.h"
#include "manifest.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>

// WAL recovery callback
static status_t recover_callback(void* ctx, wal_record_type_t type,
                                  const char* key, size_t key_len,
                                  const char* val, size_t val_len) {
    memtable_t* mt = (memtable_t*)ctx;

    if (type == WAL_RECORD_PUT) {
        return memtable_put(mt, key, key_len, val, val_len);
    } else if (type == WAL_RECORD_DELETE) {
        return memtable_delete(mt, key, key_len);
    }
    return STATUS_CORRUPTION;
}

// Helper: create directory if it doesn't exist
static int ensure_directory(const char* path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        return S_ISDIR(st.st_mode) ? 0 : -1;
    }
    if (errno == ENOENT) {
        return mkdir(path, 0755);
    }
    return -1;
}

// Open storage engine
storage_t* storage_open(const char* path, storage_opts_t* opts) {
    storage_t* db = malloc(sizeof(storage_t));
    if (!db) return NULL;

    // Copy path
    if (path) {
        db->path = strdup(path);
        if (!db->path) {
            free(db);
            return NULL;
        }
    } else {
        db->path = NULL;
    }

    // Copy options or use defaults
    if (opts) {
        db->opts = *opts;
    } else {
        storage_opts_t defaults = STORAGE_OPTS_DEFAULT;
        db->opts = defaults;
    }

    // Create memtable
    db->memtable = memtable_create(db->opts.memtable_size, db->opts.comparator);
    if (!db->memtable) {
        free(db->path);
        free(db);
        return NULL;
    }

    // Initialize WAL
    db->wal = NULL;

    // Initialize level manager
    db->levels = level_manager_create(path, db->opts.comparator);
    if (!db->levels) {
        memtable_destroy(db->memtable);
        free(db->path);
        free(db);
        return NULL;
    }

    // If path is provided, set up persistence
    if (path) {
        // Ensure directory exists
        if (ensure_directory(path) != 0) {
            level_manager_destroy(db->levels);
            memtable_destroy(db->memtable);
            free(db->path);
            free(db);
            return NULL;
        }

        // Build WAL path
        size_t wal_path_len = strlen(path) + 16;
        char* wal_path = malloc(wal_path_len);
        if (!wal_path) {
            level_manager_destroy(db->levels);
            memtable_destroy(db->memtable);
            free(db->path);
            free(db);
            return NULL;
        }
        snprintf(wal_path, wal_path_len, "%s/wal.log", path);

        // Recover from WAL if it exists
        status_t status = wal_recover(wal_path, recover_callback, db->memtable);
        if (status != STATUS_OK && status != STATUS_NOT_FOUND) {
            free(wal_path);
            level_manager_destroy(db->levels);
            memtable_destroy(db->memtable);
            free(db->path);
            free(db);
            return NULL;
        }

        // Open WAL for writing
        db->wal = wal_open(wal_path, db->opts.sync_writes);
        free(wal_path);

        if (!db->wal) {
            level_manager_destroy(db->levels);
            memtable_destroy(db->memtable);
            free(db->path);
            free(db);
            return NULL;
        }

        // Recover level structure from manifest
        if (manifest_recover(path, db->levels) != STATUS_OK) {
            wal_close(db->wal);
            level_manager_destroy(db->levels);
            memtable_destroy(db->memtable);
            free(db->path);
            free(db);
            return NULL;
        }
    }

    return db;
}

// Close storage engine
void storage_close(storage_t* db) {
    if (db) {
        level_manager_destroy(db->levels);
        if (db->wal) {
            wal_close(db->wal);
        }
        memtable_destroy(db->memtable);
        free(db->path);
        free(db);
    }
}

// Put a key-value pair
status_t storage_put(storage_t* db, const char* key, size_t key_len,
                     const char* val, size_t val_len) {
    if (!db) return STATUS_INVALID_ARG;

    // Write to WAL first (if enabled)
    if (db->wal) {
        status_t status = wal_write_put(db->wal, key, key_len, val, val_len);
        if (status != STATUS_OK) return status;
    }

    // Then update memtable
    return memtable_put(db->memtable, key, key_len, val, val_len);
}

// Get value for a key
status_t storage_get(storage_t* db, const char* key, size_t key_len,
                     char** val, size_t* val_len) {
    if (!db) return STATUS_INVALID_ARG;

    // First check memtable
    char* mt_val = NULL;
    size_t mt_val_len = 0;
    status_t status = memtable_get(db->memtable, key, key_len, &mt_val, &mt_val_len);
    if (status == STATUS_OK) {
        // Memtable returns internal pointer, make a copy
        *val = malloc(mt_val_len);
        if (!*val) return STATUS_NO_MEMORY;
        memcpy(*val, mt_val, mt_val_len);
        *val_len = mt_val_len;
        return STATUS_OK;
    }
    if (status != STATUS_NOT_FOUND) {
        return status;
    }

    // Search levels using level manager
    if (db->levels) {
        bool deleted = false;
        status = level_get(db->levels, key, key_len, val, val_len, &deleted);
        if (status == STATUS_OK) {
            if (deleted) {
                free(*val);
                *val = NULL;
                *val_len = 0;
                return STATUS_NOT_FOUND;
            }
            return STATUS_OK;
        }
        if (status != STATUS_NOT_FOUND) {
            return status;
        }
    }

    return STATUS_NOT_FOUND;
}

// Delete a key
status_t storage_delete(storage_t* db, const char* key, size_t key_len) {
    if (!db) return STATUS_INVALID_ARG;

    // Write to WAL first (if enabled)
    if (db->wal) {
        status_t status = wal_write_delete(db->wal, key, key_len);
        if (status != STATUS_OK) return status;
    }

    // Then update memtable
    return memtable_delete(db->memtable, key, key_len);
}

// Create iterator
storage_iter_t* storage_iter_create(storage_t* db) {
    if (!db) return NULL;

    storage_iter_t* iter = malloc(sizeof(storage_iter_t));
    if (!iter) return NULL;

    iter->db = db;
    iter->mt_iter = memtable_iter_create(db->memtable);
    if (!iter->mt_iter) {
        free(iter);
        return NULL;
    }

    return iter;
}

// Destroy iterator
void storage_iter_destroy(storage_iter_t* iter) {
    if (iter) {
        memtable_iter_destroy(iter->mt_iter);
        free(iter);
    }
}

// Seek to first entry
void storage_iter_seek_to_first(storage_iter_t* iter) {
    if (iter) {
        memtable_iter_seek_to_first(iter->mt_iter);
        // Skip deleted entries
        while (memtable_iter_valid(iter->mt_iter) &&
               memtable_iter_is_deleted(iter->mt_iter)) {
            memtable_iter_next(iter->mt_iter);
        }
    }
}

// Seek to key
void storage_iter_seek(storage_iter_t* iter, const char* key, size_t key_len) {
    if (iter) {
        memtable_iter_seek(iter->mt_iter, key, key_len);
        // Skip deleted entries
        while (memtable_iter_valid(iter->mt_iter) &&
               memtable_iter_is_deleted(iter->mt_iter)) {
            memtable_iter_next(iter->mt_iter);
        }
    }
}

// Check if iterator is valid
bool storage_iter_valid(storage_iter_t* iter) {
    return iter && memtable_iter_valid(iter->mt_iter);
}

// Move to next entry
void storage_iter_next(storage_iter_t* iter) {
    if (iter) {
        memtable_iter_next(iter->mt_iter);
        // Skip deleted entries
        while (memtable_iter_valid(iter->mt_iter) &&
               memtable_iter_is_deleted(iter->mt_iter)) {
            memtable_iter_next(iter->mt_iter);
        }
    }
}

// Get current key
const char* storage_iter_key(storage_iter_t* iter, size_t* key_len) {
    return iter ? memtable_iter_key(iter->mt_iter, key_len) : NULL;
}

// Get current value
const char* storage_iter_value(storage_iter_t* iter, size_t* val_len) {
    return iter ? memtable_iter_value(iter->mt_iter, val_len) : NULL;
}

// Compact: trigger compaction if needed
status_t storage_compact(storage_t* db) {
    if (!db || !db->levels) return STATUS_INVALID_ARG;

    int level = compact_pick_level(db->levels);
    if (level >= 0) {
        return compact_level(db->levels, level);
    }
    return STATUS_OK;
}

// Flush memtable to SSTable
status_t storage_flush(storage_t* db) {
    if (!db || !db->path || !db->levels) return STATUS_INVALID_ARG;

    // Check if memtable has data
    size_t count = memtable_count(db->memtable);
    if (count == 0) return STATUS_OK;

    // Get next file number from level manager
    uint64_t file_num = level_next_file_number(db->levels);

    // Generate SSTable filename
    size_t path_len = strlen(db->path) + 32;
    char* sst_path = malloc(path_len);
    if (!sst_path) return STATUS_NO_MEMORY;
    snprintf(sst_path, path_len, "%s/%06llu.sst", db->path, (unsigned long long)file_num);

    // Create SSTable writer
    sstable_writer_t* writer = sstable_writer_create(sst_path, count, db->opts.comparator);
    if (!writer) {
        free(sst_path);
        return STATUS_IO_ERROR;
    }

    // Iterate memtable and write all entries (including tombstones)
    memtable_iter_t* iter = memtable_iter_create(db->memtable);
    if (!iter) {
        sstable_writer_abort(writer);
        free(sst_path);
        return STATUS_NO_MEMORY;
    }

    memtable_iter_seek_to_first(iter);
    while (memtable_iter_valid(iter)) {
        size_t key_len, val_len;
        const char* key = memtable_iter_key(iter, &key_len);
        const char* val = memtable_iter_value(iter, &val_len);
        bool deleted = memtable_iter_is_deleted(iter);

        status_t status = sstable_writer_add(writer, key, key_len, val, val_len, deleted);
        if (status != STATUS_OK) {
            memtable_iter_destroy(iter);
            sstable_writer_abort(writer);
            free(sst_path);
            return status;
        }

        memtable_iter_next(iter);
    }
    memtable_iter_destroy(iter);

    // Finish writing SSTable
    status_t status = sstable_writer_finish(writer);
    if (status != STATUS_OK) {
        free(sst_path);
        return status;
    }

    // Open the new SSTable for reading
    sstable_reader_t* reader = sstable_reader_open(sst_path, db->opts.comparator);
    if (!reader) {
        free(sst_path);
        return STATUS_IO_ERROR;
    }

    // Add to L0 in level manager
    status = level_add_sstable(db->levels, 0, file_num, sst_path, reader);
    free(sst_path);
    if (status != STATUS_OK) {
        sstable_reader_close(reader);
        return status;
    }

    // Log to manifest
    manifest_log_add_file(db->path, 0, file_num);
    manifest_log_next_file_num(db->path, level_next_file_number(db->levels));

    // Clear memtable
    memtable_destroy(db->memtable);
    db->memtable = memtable_create(db->opts.memtable_size, db->opts.comparator);
    if (!db->memtable) {
        return STATUS_NO_MEMORY;
    }

    // Reset WAL
    if (db->wal) {
        wal_close(db->wal);
        size_t wal_path_len = strlen(db->path) + 16;
        char* wal_path = malloc(wal_path_len);
        if (!wal_path) return STATUS_NO_MEMORY;
        snprintf(wal_path, wal_path_len, "%s/wal.log", db->path);
        db->wal = wal_open(wal_path, db->opts.sync_writes);
        free(wal_path);
        if (!db->wal) return STATUS_IO_ERROR;
    }

    // Check if compaction is needed
    if (level_needs_compaction(db->levels, 0)) {
        storage_compact(db);
    }

    return STATUS_OK;
}

// Get count
size_t storage_count(storage_t* db) {
    return db ? memtable_count(db->memtable) : 0;
}

// Get memory usage
size_t storage_memory_usage(storage_t* db) {
    return db ? memtable_memory_usage(db->memtable) : 0;
}