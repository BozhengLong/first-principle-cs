/**
 * recovery.c - Crash Recovery Implementation
 */

#include "recovery.h"
#include "tx_wal.h"
#include "crc32.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define TX_WAL_FILENAME "tx.wal"
#define MAX_TRACKED_TXS 1024

typedef struct {
    uint64_t tx_id;
    bool committed;
    bool aborted;
} tx_track_t;

static tx_track_t* find_or_add_tx(tx_track_t* tracked, size_t* count,
                                  size_t max, uint64_t tx_id) {
    for (size_t i = 0; i < *count; i++) {
        if (tracked[i].tx_id == tx_id) return &tracked[i];
    }
    if (*count < max) {
        tracked[*count].tx_id = tx_id;
        tracked[*count].committed = false;
        tracked[*count].aborted = false;
        return &tracked[(*count)++];
    }
    return NULL;
}

tx_status_t tx_recover(tx_manager_t* tm, tx_recovery_result_t* result) {
    if (!tm) return TX_INVALID_ARG;

    /* Initialize result */
    tx_recovery_result_t res = {0};

    /* Build WAL path */
    size_t path_len = strlen(tm->path) + strlen(TX_WAL_FILENAME) + 2;
    char* wal_path = malloc(path_len);
    if (!wal_path) return TX_NO_MEMORY;
    snprintf(wal_path, path_len, "%s/%s", tm->path, TX_WAL_FILENAME);

    /* Open WAL for reading */
    int fd = open(wal_path, O_RDONLY);
    free(wal_path);

    if (fd < 0) {
        /* No WAL file - nothing to recover */
        if (result) *result = res;
        return TX_OK;
    }

    /* Track transactions */
    tx_track_t* tracked = calloc(MAX_TRACKED_TXS, sizeof(tx_track_t));
    if (!tracked) {
        close(fd);
        return TX_NO_MEMORY;
    }
    size_t tracked_count = 0;

    /* Read and process WAL records */
    tx_wal_header_t header;
    while (read(fd, &header, sizeof(header)) == sizeof(header)) {
        /* Verify checksum */
        uint32_t saved_crc = header.checksum;
        uint32_t crc = 0;
        crc = crc32_update(crc, &header.type, sizeof(header.type));
        crc = crc32_update(crc, &header.tx_id, sizeof(header.tx_id));
        crc = crc32_update(crc, &header.timestamp, sizeof(header.timestamp));
        crc = crc32_update(crc, &header.data_len, sizeof(header.data_len));

        /* Read data if present */
        char* data = NULL;
        if (header.data_len > 0) {
            data = malloc(header.data_len);
            if (!data || read(fd, data, header.data_len) != (ssize_t)header.data_len) {
                free(data);
                break;  /* Truncated record */
            }
            crc = crc32_update(crc, data, header.data_len);
        }

        if (crc != saved_crc) {
            free(data);
            break;  /* Corrupted record */
        }
        free(data);

        /* Track max IDs */
        if (header.tx_id > res.max_tx_id) {
            res.max_tx_id = header.tx_id;
        }
        if (header.timestamp > res.max_ts) {
            res.max_ts = header.timestamp;
        }

        /* Process record */
        tx_track_t* tx = find_or_add_tx(tracked, &tracked_count,
                                        MAX_TRACKED_TXS, header.tx_id);
        if (!tx) continue;

        switch (header.type) {
            case TX_WAL_BEGIN:
                break;
            case TX_WAL_WRITE:
                break;
            case TX_WAL_COMMIT:
                tx->committed = true;
                res.committed_count++;
                break;
            case TX_WAL_ABORT:
                tx->aborted = true;
                res.aborted_count++;
                break;
        }
    }

    close(fd);
    free(tracked);

    /* Update transaction manager state */
    pthread_mutex_lock(&tm->mutex);
    if (res.max_tx_id >= tm->next_tx_id) {
        tm->next_tx_id = res.max_tx_id + 1;
    }
    if (res.max_ts >= tm->next_ts) {
        tm->next_ts = res.max_ts + 1;
    }
    pthread_mutex_unlock(&tm->mutex);

    if (result) *result = res;
    return TX_OK;
}
