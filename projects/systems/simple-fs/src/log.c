#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "types.h"
#include "param.h"
#include "fs.h"
#include "buf.h"
#include "disk.h"
#include "log.h"

// In-memory log state
static struct log {
    pthread_mutex_t lock;
    pthread_cond_t cond;        // For waiting on commit/space
    int start;                  // First log block (sb.logstart)
    int size;                   // Number of log blocks (sb.nlog)
    int outstanding;            // Number of active FS operations
    int committing;             // True if commit in progress
    int dev;                    // Device number
    struct logheader lh;        // In-memory copy of log header
} log;

static void recover_from_log(void);
static void commit(void);

// Read log header from disk into log.lh
static void read_head(void) {
    struct buf *bp = bread(log.dev, log.start);
    if (bp == NULL) {
        fprintf(stderr, "read_head: bread failed\n");
        return;
    }
    struct logheader *lh = (struct logheader*)bp->data;
    log.lh.n = lh->n;
    for (int i = 0; i < log.lh.n; i++) {
        log.lh.block[i] = lh->block[i];
    }
    brelse(bp);
}

// Write log.lh to disk
static void write_head(void) {
    struct buf *bp = bread(log.dev, log.start);
    if (bp == NULL) {
        fprintf(stderr, "write_head: bread failed\n");
        return;
    }
    struct logheader *lh = (struct logheader*)bp->data;
    lh->n = log.lh.n;
    for (int i = 0; i < log.lh.n; i++) {
        lh->block[i] = log.lh.block[i];
    }
    bwrite(bp);
    brelse(bp);
}

// Initialize logging system
void loginit(int dev, struct superblock *sb) {
    if (sizeof(struct logheader) > BSIZE) {
        fprintf(stderr, "loginit: log header too big\n");
        return;
    }

    pthread_mutex_init(&log.lock, NULL);
    pthread_cond_init(&log.cond, NULL);
    log.start = sb->logstart;
    log.size = sb->nlog;
    log.dev = dev;
    log.outstanding = 0;
    log.committing = 0;

    recover_from_log();
}

// Copy committed blocks from log to their home location
static void install_trans(void) {
    for (int tail = 0; tail < log.lh.n; tail++) {
        // Read block from log
        struct buf *lbuf = bread(log.dev, log.start + tail + 1);
        if (lbuf == NULL) continue;

        // Read destination block
        struct buf *dbuf = bread(log.dev, log.lh.block[tail]);
        if (dbuf == NULL) {
            brelse(lbuf);
            continue;
        }

        // Copy data from log to destination
        memcpy(dbuf->data, lbuf->data, BSIZE);
        bwrite(dbuf);
        bunpin(dbuf);  // Unpin after install
        brelse(lbuf);
        brelse(dbuf);
    }
}

// Recover from log if needed
static void recover_from_log(void) {
    read_head();
    if (log.lh.n > 0) {
        printf("log: recovering %d blocks\n", log.lh.n);
        install_trans();  // Replay committed transaction
        log.lh.n = 0;
        write_head();     // Clear the log
    }
}

// Start a file system operation
void begin_op(void) {
    pthread_mutex_lock(&log.lock);
    while (1) {
        if (log.committing) {
            // Wait for commit to finish
            pthread_cond_wait(&log.cond, &log.lock);
        } else if (log.lh.n + (log.outstanding + 1) * MAXOPBLOCKS > log.size - 1) {
            // Not enough log space for this operation
            pthread_cond_wait(&log.cond, &log.lock);
        } else {
            log.outstanding++;
            pthread_mutex_unlock(&log.lock);
            break;
        }
    }
}

// End a file system operation
void end_op(void) {
    int do_commit = 0;

    pthread_mutex_lock(&log.lock);
    log.outstanding--;
    if (log.committing) {
        fprintf(stderr, "end_op: committing\n");
    }
    if (log.outstanding == 0) {
        do_commit = 1;
        log.committing = 1;
    }
    pthread_mutex_unlock(&log.lock);

    if (do_commit) {
        commit();
        pthread_mutex_lock(&log.lock);
        log.committing = 0;
        pthread_cond_broadcast(&log.cond);
        pthread_mutex_unlock(&log.lock);
    }
}

// Copy modified blocks from cache to log
static void write_log(void) {
    for (int tail = 0; tail < log.lh.n; tail++) {
        // Read block from cache
        struct buf *from = bread(log.dev, log.lh.block[tail]);
        if (from == NULL) continue;

        // Read log block
        struct buf *to = bread(log.dev, log.start + tail + 1);
        if (to == NULL) {
            brelse(from);
            continue;
        }

        // Copy data to log
        memcpy(to->data, from->data, BSIZE);
        bwrite(to);
        brelse(from);
        brelse(to);
    }
}

// Commit the current transaction
static void commit(void) {
    if (log.lh.n > 0) {
        write_log();      // Write modified blocks to log
        write_head();     // Write header to disk (commit point)
        install_trans();  // Copy from log to home locations
        log.lh.n = 0;
        write_head();     // Erase transaction from log
    }
}

// Log a buffer write
// If called outside a transaction, falls back to direct bwrite for compatibility
void log_write(struct buf *b) {
    pthread_mutex_lock(&log.lock);

    // If not in a transaction, fall back to direct write
    if (log.outstanding < 1) {
        pthread_mutex_unlock(&log.lock);
        bwrite(b);
        return;
    }

    if (log.lh.n >= log.size - 1) {
        fprintf(stderr, "log_write: log full\n");
        pthread_mutex_unlock(&log.lock);
        return;
    }

    // Check if block is already in log
    int i;
    for (i = 0; i < log.lh.n; i++) {
        if (log.lh.block[i] == b->blockno)
            break;
    }

    // Add to log if not already present
    if (i == log.lh.n) {
        bpin(b);  // Pin buffer in cache
        log.lh.block[i] = b->blockno;
        log.lh.n++;
    }

    pthread_mutex_unlock(&log.lock);
}
