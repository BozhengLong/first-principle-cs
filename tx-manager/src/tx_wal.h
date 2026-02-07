/**
 * tx_wal.h - Transaction Write-Ahead Log
 *
 * Provides durability for transactions through WAL logging.
 */

#ifndef TX_WAL_H
#define TX_WAL_H

#include "types.h"
#include "tx.h"

/**
 * WAL record types
 */
typedef enum {
    TX_WAL_BEGIN = 1,
    TX_WAL_WRITE = 2,
    TX_WAL_COMMIT = 3,
    TX_WAL_ABORT = 4,
} tx_wal_type_t;

/**
 * WAL record header
 */
typedef struct {
    uint32_t type;        /* tx_wal_type_t */
    uint64_t tx_id;       /* Transaction ID */
    uint64_t timestamp;   /* Timestamp (start_ts or commit_ts) */
    uint32_t data_len;    /* Length of following data */
    uint32_t checksum;    /* CRC32 of header + data */
} tx_wal_header_t;

/**
 * Transaction WAL handle
 */
typedef struct tx_wal tx_wal_t;

/**
 * Open transaction WAL
 */
tx_wal_t* tx_wal_open(const char* path);

/**
 * Close transaction WAL
 */
void tx_wal_close(tx_wal_t* wal);

/**
 * Log transaction begin
 */
tx_status_t tx_wal_log_begin(tx_wal_t* wal, tx_t* tx);

/**
 * Log a write operation
 */
tx_status_t tx_wal_log_write(tx_wal_t* wal, tx_t* tx,
                             const char* key, size_t key_len,
                             const char* value, size_t value_len,
                             bool is_delete);

/**
 * Log transaction commit
 */
tx_status_t tx_wal_log_commit(tx_wal_t* wal, tx_t* tx);

/**
 * Log transaction abort
 */
tx_status_t tx_wal_log_abort(tx_wal_t* wal, tx_t* tx);

/**
 * Sync WAL to disk
 */
tx_status_t tx_wal_sync(tx_wal_t* wal);

#endif /* TX_WAL_H */
