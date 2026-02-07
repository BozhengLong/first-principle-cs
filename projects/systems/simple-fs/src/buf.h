#ifndef BUF_H
#define BUF_H

#include <pthread.h>
#include "types.h"
#include "param.h"

// Buffer flags
#define B_VALID  0x1  // Buffer has been read from disk
#define B_DIRTY  0x2  // Buffer needs to be written to disk

// Buffer structure
struct buf {
    int flags;                    // B_VALID, B_DIRTY
    uint dev;                     // Device number
    uint blockno;                 // Block number on disk
    int refcnt;                   // Reference count
    pthread_mutex_t lock;         // Per-buffer lock
    struct buf *prev;             // LRU list prev
    struct buf *next;             // LRU list next
    uchar data[BSIZE];            // Block data (4KB)
};

// Buffer cache interface
void            binit_cache(void);
struct buf*     bread(uint dev, uint blockno);
void            bwrite(struct buf *b);
void            brelse(struct buf *b);
void            bpin(struct buf *b);
void            bunpin(struct buf *b);

#endif // BUF_H
