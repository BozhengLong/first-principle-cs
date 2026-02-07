#ifndef WAL_H
#define WAL_H

#include "types.h"
#include <stdbool.h>

// WAL record types
typedef enum {
    WAL_RECORD_PUT = 1,
    WAL_RECORD_DELETE = 2,
} wal_record_type_t;

// WAL structure
struct wal {
    int fd;              // File descriptor
    char* path;          // WAL file path
    size_t file_size;    // Current file size
    bool sync_writes;    // Whether to fsync after each write
};

// Lifecycle
wal_t* wal_open(const char* path, bool sync_writes);
void wal_close(wal_t* wal);

// Write operations
status_t wal_write_put(wal_t* wal, const char* key, size_t key_len,
                       const char* val, size_t val_len);
status_t wal_write_delete(wal_t* wal, const char* key, size_t key_len);
status_t wal_sync(wal_t* wal);

// Recovery callback type
typedef status_t (*wal_recover_fn)(void* ctx, wal_record_type_t type,
                                   const char* key, size_t key_len,
                                   const char* val, size_t val_len);

// Recovery operation
status_t wal_recover(const char* path, wal_recover_fn fn, void* ctx);

// Maintenance
status_t wal_truncate(wal_t* wal);

#endif // WAL_H
