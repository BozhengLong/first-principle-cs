#include "wal.h"
#include "crc32.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

// WAL record header sizes
#define WAL_LENGTH_SIZE 4
#define WAL_CRC_SIZE 4
#define WAL_TYPE_SIZE 1
#define WAL_KEYLEN_SIZE 4
#define WAL_VALLEN_SIZE 4
#define WAL_HEADER_SIZE (WAL_LENGTH_SIZE + WAL_CRC_SIZE + WAL_TYPE_SIZE + \
                         WAL_KEYLEN_SIZE + WAL_VALLEN_SIZE)

// Helper: write all bytes to fd
static ssize_t write_all(int fd, const void* buf, size_t count) {
    const char* p = buf;
    size_t remaining = count;

    while (remaining > 0) {
        ssize_t written = write(fd, p, remaining);
        if (written < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        p += written;
        remaining -= written;
    }
    return count;
}

// Helper: read all bytes from fd
static ssize_t read_all(int fd, void* buf, size_t count) {
    char* p = buf;
    size_t remaining = count;

    while (remaining > 0) {
        ssize_t n = read(fd, p, remaining);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (n == 0) break;  // EOF
        p += n;
        remaining -= n;
    }
    return count - remaining;
}

// Open WAL file
wal_t* wal_open(const char* path, bool sync_writes) {
    if (!path) return NULL;

    wal_t* wal = malloc(sizeof(wal_t));
    if (!wal) return NULL;

    wal->path = strdup(path);
    if (!wal->path) {
        free(wal);
        return NULL;
    }

    // Open file for append (create if not exists)
    wal->fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (wal->fd < 0) {
        free(wal->path);
        free(wal);
        return NULL;
    }

    // Get current file size
    struct stat st;
    if (fstat(wal->fd, &st) == 0) {
        wal->file_size = st.st_size;
    } else {
        wal->file_size = 0;
    }

    wal->sync_writes = sync_writes;
    return wal;
}

// Close WAL file
void wal_close(wal_t* wal) {
    if (wal) {
        if (wal->fd >= 0) {
            fsync(wal->fd);
            close(wal->fd);
        }
        free(wal->path);
        free(wal);
    }
}

// Write a record to WAL
// Record format: length(4) | crc32(4) | type(1) | key_len(4) | key | val_len(4) | value
static status_t wal_write_record(wal_t* wal, wal_record_type_t type,
                                  const char* key, size_t key_len,
                                  const char* val, size_t val_len) {
    if (!wal || !key) return STATUS_INVALID_ARG;

    // Calculate record size (excluding length field)
    size_t record_size = WAL_CRC_SIZE + WAL_TYPE_SIZE + WAL_KEYLEN_SIZE +
                         key_len + WAL_VALLEN_SIZE + val_len;

    // Allocate buffer for entire record
    size_t total_size = WAL_LENGTH_SIZE + record_size;
    char* buf = malloc(total_size);
    if (!buf) return STATUS_NO_MEMORY;

    char* p = buf;

    // Write length (record_size, not including length field itself)
    uint32_t len32 = (uint32_t)record_size;
    memcpy(p, &len32, 4);
    p += 4;

    // Skip CRC for now (will fill in after)
    char* crc_pos = p;
    p += 4;

    // Write type
    *p++ = (char)type;

    // Write key_len and key
    uint32_t klen32 = (uint32_t)key_len;
    memcpy(p, &klen32, 4);
    p += 4;
    memcpy(p, key, key_len);
    p += key_len;

    // Write val_len and value
    uint32_t vlen32 = (uint32_t)val_len;
    memcpy(p, &vlen32, 4);
    p += 4;
    if (val_len > 0 && val) {
        memcpy(p, val, val_len);
    }

    // Calculate CRC32 over type + key_len + key + val_len + value
    uint32_t crc = crc32(crc_pos + 4, record_size - WAL_CRC_SIZE);
    memcpy(crc_pos, &crc, 4);

    // Write to file
    if (write_all(wal->fd, buf, total_size) != (ssize_t)total_size) {
        free(buf);
        return STATUS_IO_ERROR;
    }

    wal->file_size += total_size;
    free(buf);

    // Sync if configured
    if (wal->sync_writes) {
        if (fsync(wal->fd) != 0) {
            return STATUS_IO_ERROR;
        }
    }

    return STATUS_OK;
}

// Write PUT record
status_t wal_write_put(wal_t* wal, const char* key, size_t key_len,
                       const char* val, size_t val_len) {
    return wal_write_record(wal, WAL_RECORD_PUT, key, key_len, val, val_len);
}

// Write DELETE record
status_t wal_write_delete(wal_t* wal, const char* key, size_t key_len) {
    return wal_write_record(wal, WAL_RECORD_DELETE, key, key_len, NULL, 0);
}

// Sync WAL to disk
status_t wal_sync(wal_t* wal) {
    if (!wal) return STATUS_INVALID_ARG;
    if (fsync(wal->fd) != 0) {
        return STATUS_IO_ERROR;
    }
    return STATUS_OK;
}

// Recover WAL by replaying records
status_t wal_recover(const char* path, wal_recover_fn fn, void* ctx) {
    if (!path || !fn) return STATUS_INVALID_ARG;

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        if (errno == ENOENT) {
            return STATUS_OK;  // No WAL file, nothing to recover
        }
        return STATUS_IO_ERROR;
    }

    status_t result = STATUS_OK;
    char header[WAL_LENGTH_SIZE + WAL_CRC_SIZE];

    while (1) {
        // Read length and CRC
        ssize_t n = read_all(fd, header, WAL_LENGTH_SIZE);
        if (n == 0) break;  // EOF
        if (n != WAL_LENGTH_SIZE) {
            // Truncated record at end - ignore
            break;
        }

        uint32_t record_len;
        memcpy(&record_len, header, 4);

        // Sanity check on record length
        if (record_len < WAL_CRC_SIZE + WAL_TYPE_SIZE + WAL_KEYLEN_SIZE + WAL_VALLEN_SIZE) {
            result = STATUS_CORRUPTION;
            break;
        }

        // Read rest of record
        char* record = malloc(record_len);
        if (!record) {
            result = STATUS_NO_MEMORY;
            break;
        }

        n = read_all(fd, record, record_len);
        if (n != (ssize_t)record_len) {
            // Truncated record - ignore
            free(record);
            break;
        }

        // Verify CRC
        uint32_t stored_crc;
        memcpy(&stored_crc, record, 4);
        uint32_t computed_crc = crc32(record + 4, record_len - 4);

        if (stored_crc != computed_crc) {
            // Corrupted record - stop recovery
            free(record);
            result = STATUS_CORRUPTION;
            break;
        }

        // Parse record
        char* p = record + 4;  // Skip CRC
        wal_record_type_t type = (wal_record_type_t)*p++;

        uint32_t key_len;
        memcpy(&key_len, p, 4);
        p += 4;
        const char* key = p;
        p += key_len;

        uint32_t val_len;
        memcpy(&val_len, p, 4);
        p += 4;
        const char* val = (val_len > 0) ? p : NULL;

        // Call recovery callback
        result = fn(ctx, type, key, key_len, val, val_len);
        free(record);

        if (result != STATUS_OK) break;
    }

    close(fd);
    return result;
}

// Truncate WAL (clear all records)
status_t wal_truncate(wal_t* wal) {
    if (!wal) return STATUS_INVALID_ARG;

    if (ftruncate(wal->fd, 0) != 0) {
        return STATUS_IO_ERROR;
    }

    // Reset file position to beginning
    if (lseek(wal->fd, 0, SEEK_SET) != 0) {
        return STATUS_IO_ERROR;
    }

    wal->file_size = 0;
    return STATUS_OK;
}
