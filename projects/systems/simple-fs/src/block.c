#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "block.h"
#include "disk.h"
#include "buf.h"
#include "log.h"

static struct superblock sb;

// Initialize block allocator
void binit(struct superblock *superblock) {
    memcpy(&sb, superblock, sizeof(sb));
}

// Allocate a zeroed disk block
uint balloc(void) {
    struct buf *bp;

    for (uint b = 0; b < sb.size; b += BPB) {
        bp = bread(0, BBLOCK(b, sb));
        if (bp == NULL) {
            fprintf(stderr, "balloc: bread failed\n");
            return 0;
        }

        for (uint bi = 0; bi < BPB && b + bi < sb.size; bi++) {
            uint m = 1 << (bi % 8);
            if ((bp->data[bi/8] & m) == 0) {  // Is block free?
                bp->data[bi/8] |= m;  // Mark block in use
                log_write(bp);
                brelse(bp);
                bzeroblk(b + bi);
                return b + bi;
            }
        }
        brelse(bp);
    }

    fprintf(stderr, "balloc: out of blocks\n");
    return 0;
}

// Free a disk block
void bfree(uint blockno) {
    assert(blockno > 0 && blockno < sb.size);

    struct buf *bp;
    bp = bread(0, BBLOCK(blockno, sb));
    if (bp == NULL) {
        fprintf(stderr, "bfree: bread failed\n");
        return;
    }

    uint bi = blockno % BPB;
    uint m = 1 << (bi % 8);

    if ((bp->data[bi/8] & m) == 0) {
        fprintf(stderr, "bfree: freeing free block %d\n", blockno);
        brelse(bp);
        return;
    }

    bp->data[bi/8] &= ~m;  // Mark block as free
    log_write(bp);
    brelse(bp);
}

// Zero a block
void bzeroblk(uint blockno) {
    struct buf *bp;
    bp = bread(0, blockno);
    if (bp == NULL) {
        fprintf(stderr, "bzeroblk: bread failed\n");
        return;
    }
    memset(bp->data, 0, BSIZE);
    log_write(bp);
    brelse(bp);
}
