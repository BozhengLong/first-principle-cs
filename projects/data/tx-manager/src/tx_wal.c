/**
 * tx_wal.c - Transaction Write-Ahead Log Implementation
 */

#include "tx_wal.h"
#include "crc32.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define TX_WAL_FILENAME "tx.wal"

struct tx_wal {
    int fd;
    char* path;
};

tx_wal_t* tx_wal_open(const char* path) {
    if (!path) return NULL;

    tx_wal_t* wal = calloc(1, sizeof(tx_wal_t));
    if (!wal) return NULL;

    /* Build WAL file path */
    size_t path_len = strlen(path) + strlen(TX_WAL_FILENAME) + 2;
    wal->path = malloc(path_len);
    if (!wal->path) {
        free(wal);
        return NULL;
    }
    snprintf(wal->path, path_len, "%s/%s", path, TX_WAL_FILENAME);

    /* Open or create WAL file */
    wal->fd = open(wal->path, O_RDWR | O_CREAT | O_APPEND, 0644);
    if (wal->fd < 0) {
        free(wal->path);
        free(wal);
        return NULL;
    }

    return wal;
}

void tx_wal_close(tx_wal_t* wal) {
    if (!wal) return;
    if (wal->fd >= 0) close(wal->fd);
    free(wal->path);
    free(wal);
}

static tx_status_t write_record(tx_wal_t* wal, tx_wal_type_t type,
                                uint64_t tx_id, uint64_t timestamp,
                                const void* data, size_t data_len) {
    if (!wal || wal->fd < 0) return TX_INVALID_ARG;

    /* Build header */
    tx_wal_header_t header = {
        .type = type,
        .tx_id = tx_id,
        .timestamp = timestamp,
        .data_len = (uint32_t)data_len,
        .checksum = 0
    };

    /* Calculate checksum over header (without checksum field) and data */
    uint32_t crc = 0;
    crc = crc32_update(crc, &header.type, sizeof(header.type));
    crc = crc32_update(crc, &header.tx_id, sizeof(header.tx_id));
    crc = crc32_update(crc, &header.timestamp, sizeof(header.timestamp));
    crc = crc32_update(crc, &header.data_len, sizeof(header.data_len));
    if (data && data_len > 0) {
        crc = crc32_update(crc, data, data_len);
    }
    header.checksum = crc;

    /* Write header */
    if (write(wal->fd, &header, sizeof(header)) != sizeof(header)) {
        return TX_IO_ERROR;
    }

    /* Write data if present */
    if (data && data_len > 0) {
        if (write(wal->fd, data, data_len) != (ssize_t)data_len) {
            return TX_IO_ERROR;
        }
    }

    return TX_OK;
}

tx_status_t tx_wal_log_begin(tx_wal_t* wal, tx_t* tx) {
    if (!wal || !tx) return TX_INVALID_ARG;
    return write_record(wal, TX_WAL_BEGIN, tx->tx_id, tx->start_ts, NULL, 0);
}

tx_status_t tx_wal_log_commit(tx_wal_t* wal, tx_t* tx) {
    if (!wal || !tx) return TX_INVALID_ARG;
    return write_record(wal, TX_WAL_COMMIT, tx->tx_id, tx->commit_ts, NULL, 0);
}

tx_status_t tx_wal_log_abort(tx_wal_t* wal, tx_t* tx) {
    if (!wal || !tx) return TX_INVALID_ARG;
    return write_record(wal, TX_WAL_ABORT, tx->tx_id, 0, NULL, 0);
}

tx_status_t tx_wal_log_write(tx_wal_t* wal, tx_t* tx,
                             const char* key, size_t key_len,
                             const char* value, size_t value_len,
                             bool is_delete) {
    if (!wal || !tx || !key) return TX_INVALID_ARG;

    /* Data format: key_len (4) + key + value_len (4) + value + is_delete (1) */
    size_t data_len = 4 + key_len + 4 + value_len + 1;
    char* data = malloc(data_len);
    if (!data) return TX_NO_MEMORY;

    char* p = data;

    /* Write key_len and key */
    uint32_t klen = (uint32_t)key_len;
    memcpy(p, &klen, 4);
    p += 4;
    memcpy(p, key, key_len);
    p += key_len;

    /* Write value_len and value */
    uint32_t vlen = (uint32_t)value_len;
    memcpy(p, &vlen, 4);
    p += 4;
    if (value_len > 0) {
        memcpy(p, value, value_len);
        p += value_len;
    }

    /* Write is_delete flag */
    *p = is_delete ? 1 : 0;

    tx_status_t st = write_record(wal, TX_WAL_WRITE, tx->tx_id, 0, data, data_len);
    free(data);
    return st;
}

tx_status_t tx_wal_sync(tx_wal_t* wal) {
    if (!wal || wal->fd < 0) return TX_INVALID_ARG;
    if (fsync(wal->fd) != 0) return TX_IO_ERROR;
    return TX_OK;
}
