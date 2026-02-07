#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "types.h"
#include "param.h"
#include "buf.h"
#include "disk.h"

// Buffer cache structure
static struct {
    pthread_mutex_t lock;         // Protects the cache structure
    struct buf buf[NBUF];         // Fixed-size buffer pool
    struct buf head;              // LRU list head (sentinel)
} bcache;

// Initialize buffer cache
void binit_cache(void) {
    struct buf *b;

    // Initialize cache lock
    pthread_mutex_init(&bcache.lock, NULL);

    // Initialize LRU list as circular doubly-linked list
    bcache.head.prev = &bcache.head;
    bcache.head.next = &bcache.head;

    // Initialize all buffers and add to LRU list
    for (b = bcache.buf; b < bcache.buf + NBUF; b++) {
        b->refcnt = 0;
        b->flags = 0;
        b->dev = 0;
        b->blockno = 0;
        pthread_mutex_init(&b->lock, NULL);

        // Add to front of LRU list
        b->next = bcache.head.next;
        b->prev = &bcache.head;
        bcache.head.next->prev = b;
        bcache.head.next = b;
    }
}

// Internal: Get buffer for block (find in cache or allocate)
static struct buf* bget(uint dev, uint blockno) {
    struct buf *b;

    pthread_mutex_lock(&bcache.lock);

    // Is the block already cached?
    for (b = bcache.head.next; b != &bcache.head; b = b->next) {
        if (b->dev == dev && b->blockno == blockno) {
            b->refcnt++;
            pthread_mutex_unlock(&bcache.lock);
            pthread_mutex_lock(&b->lock);
            return b;
        }
    }

    // Not cached. Find LRU unused buffer to recycle.
    for (b = bcache.head.prev; b != &bcache.head; b = b->prev) {
        if (b->refcnt == 0) {
            // Write back dirty buffer before recycling
            if (b->flags & B_DIRTY) {
                disk_write(b->blockno, b->data);
            }

            // Recycle buffer
            b->dev = dev;
            b->blockno = blockno;
            b->flags = 0;  // Clear valid and dirty flags
            b->refcnt = 1;

            // Move to front of LRU list
            b->prev->next = b->next;
            b->next->prev = b->prev;
            b->next = bcache.head.next;
            b->prev = &bcache.head;
            bcache.head.next->prev = b;
            bcache.head.next = b;

            pthread_mutex_unlock(&bcache.lock);
            pthread_mutex_lock(&b->lock);
            return b;
        }
    }

    // No buffers available
    fprintf(stderr, "bget: no buffers available\n");
    pthread_mutex_unlock(&bcache.lock);
    return NULL;
}

// Read block into buffer
struct buf* bread(uint dev, uint blockno) {
    struct buf *b;

    b = bget(dev, blockno);
    if (b == NULL) {
        return NULL;
    }

    // Read from disk if not valid
    if (!(b->flags & B_VALID)) {
        if (disk_read(blockno, b->data) < 0) {
            fprintf(stderr, "bread: disk_read failed\n");
            pthread_mutex_unlock(&b->lock);
            // Decrement refcnt since we're failing
            pthread_mutex_lock(&bcache.lock);
            b->refcnt--;
            pthread_mutex_unlock(&bcache.lock);
            return NULL;
        }
        b->flags |= B_VALID;
    }

    return b;
}

// Write buffer to disk (write-through policy)
void bwrite(struct buf *b) {
    if (disk_write(b->blockno, b->data) < 0) {
        fprintf(stderr, "bwrite: disk_write failed\n");
        return;
    }
    b->flags &= ~B_DIRTY;  // Clear dirty flag
}

// Release buffer
void brelse(struct buf *b) {
    pthread_mutex_unlock(&b->lock);

    pthread_mutex_lock(&bcache.lock);
    b->refcnt--;

    if (b->refcnt == 0) {
        // Move to front of LRU list (most recently used)
        b->prev->next = b->next;
        b->next->prev = b->prev;
        b->next = bcache.head.next;
        b->prev = &bcache.head;
        bcache.head.next->prev = b;
        bcache.head.next = b;
    }

    pthread_mutex_unlock(&bcache.lock);
}

// Pin buffer in cache (prevent eviction during transaction)
void bpin(struct buf *b) {
    pthread_mutex_lock(&bcache.lock);
    b->refcnt++;
    pthread_mutex_unlock(&bcache.lock);
}

// Unpin buffer (allow eviction)
void bunpin(struct buf *b) {
    pthread_mutex_lock(&bcache.lock);
    b->refcnt--;
    pthread_mutex_unlock(&bcache.lock);
}
