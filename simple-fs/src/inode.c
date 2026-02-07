#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "types.h"
#include "param.h"
#include "fs.h"
#include "inode.h"
#include "disk.h"
#include "block.h"
#include "buf.h"
#include "log.h"

// Global inode table
static struct {
    pthread_mutex_t lock;
    struct inode inode[NINODE];
} icache;

// Global superblock
static struct superblock sb;

// Initialize inode table
void iinit(struct superblock *superblock) {
    int i;

    // Copy superblock
    sb = *superblock;

    // Initialize cache lock
    pthread_mutex_init(&icache.lock, NULL);

    // Initialize all inodes
    for (i = 0; i < NINODE; i++) {
        pthread_mutex_init(&icache.inode[i].lock, NULL);
        icache.inode[i].ref = 0;
        icache.inode[i].valid = 0;
    }
}

// Find the inode with number inum on device dev
// and return the in-memory copy. Does not lock
// the inode and does not read it from disk.
struct inode* iget(uint dev, uint inum) {
    struct inode *ip, *empty;

    pthread_mutex_lock(&icache.lock);

    // Is the inode already cached?
    empty = NULL;
    for (ip = &icache.inode[0]; ip < &icache.inode[NINODE]; ip++) {
        if (ip->ref > 0 && ip->dev == dev && ip->inum == inum) {
            ip->ref++;
            pthread_mutex_unlock(&icache.lock);
            return ip;
        }
        if (empty == NULL && ip->ref == 0)  // Remember empty slot
            empty = ip;
    }

    // Recycle an inode cache entry
    if (empty == NULL) {
        fprintf(stderr, "iget: no inodes\n");
        pthread_mutex_unlock(&icache.lock);
        return NULL;
    }

    ip = empty;
    ip->dev = dev;
    ip->inum = inum;
    ip->ref = 1;
    ip->valid = 0;
    pthread_mutex_unlock(&icache.lock);

    return ip;
}

// Increment reference count for ip
struct inode* idup(struct inode *ip) {
    pthread_mutex_lock(&icache.lock);
    ip->ref++;
    pthread_mutex_unlock(&icache.lock);
    return ip;
}

// Lock the given inode
// Reads the inode from disk if necessary
void ilock(struct inode *ip) {
    struct buf *bp;
    struct dinode *dip;

    if (ip == NULL || ip->ref < 1) {
        fprintf(stderr, "ilock: invalid inode\n");
        return;
    }

    pthread_mutex_lock(&ip->lock);

    if (ip->valid == 0) {
        // Read inode from disk using buffer cache
        bp = bread(ip->dev, IBLOCK(ip->inum, sb));
        if (bp == NULL) {
            fprintf(stderr, "ilock: bread failed\n");
            return;
        }
        dip = (struct dinode*)bp->data + ip->inum % IPB;

        ip->type = dip->type;
        ip->major = dip->major;
        ip->minor = dip->minor;
        ip->nlink = dip->nlink;
        ip->size = dip->size;
        memcpy(ip->addrs, dip->addrs, sizeof(ip->addrs));
        ip->valid = 1;

        brelse(bp);

        if (ip->type == 0) {
            fprintf(stderr, "ilock: no type\n");
        }
    }
}

// Unlock the given inode
void iunlock(struct inode *ip) {
    if (ip == NULL || !pthread_mutex_trylock(&ip->lock)) {
        fprintf(stderr, "iunlock: not locked\n");
        return;
    }
    pthread_mutex_unlock(&ip->lock);
    pthread_mutex_unlock(&ip->lock);
}

// Drop a reference to an in-memory inode
// If that was the last reference, the inode cache entry can be recycled
// If that was the last reference and the inode has no links
// to it, free the inode (and its content) on disk
void iput(struct inode *ip) {
    pthread_mutex_lock(&icache.lock);

    if (ip->ref == 1 && ip->valid && ip->nlink == 0) {
        // inode has no links and no other references: truncate and free

        pthread_mutex_unlock(&icache.lock);
        pthread_mutex_lock(&ip->lock);

        itrunc(ip);
        ip->type = 0;
        iupdate(ip);
        ip->valid = 0;

        pthread_mutex_unlock(&ip->lock);
        pthread_mutex_lock(&icache.lock);
    }

    ip->ref--;
    pthread_mutex_unlock(&icache.lock);
}

// Copy a modified in-memory inode to disk
void iupdate(struct inode *ip) {
    struct buf *bp;
    struct dinode *dip;

    bp = bread(ip->dev, IBLOCK(ip->inum, sb));
    if (bp == NULL) {
        fprintf(stderr, "iupdate: bread failed\n");
        return;
    }
    dip = (struct dinode*)bp->data + ip->inum % IPB;
    dip->type = ip->type;
    dip->major = ip->major;
    dip->minor = ip->minor;
    dip->nlink = ip->nlink;
    dip->size = ip->size;
    memcpy(dip->addrs, ip->addrs, sizeof(ip->addrs));
    log_write(bp);
    brelse(bp);
}

// Allocate a new inode with the given type on device dev
// Returns inode number, or 0 if no free inodes
uint ialloc(uint dev, uint16 type) {
    uint inum;
    struct buf *bp;
    struct dinode *dip;

    for (inum = 1; inum < sb.ninodes; inum++) {
        bp = bread(dev, IBLOCK(inum, sb));
        if (bp == NULL) continue;
        dip = (struct dinode*)bp->data + inum % IPB;
        if (dip->type == 0) {  // Free inode
            memset(dip, 0, sizeof(*dip));
            dip->type = type;
            log_write(bp);
            brelse(bp);
            return inum;
        }
        brelse(bp);
    }

    fprintf(stderr, "ialloc: no inodes\n");
    return 0;
}

// Free the given inode on disk
void ifree(uint dev, uint inum) {
    struct buf *bp;
    struct dinode *dip;

    bp = bread(dev, IBLOCK(inum, sb));
    if (bp == NULL) {
        fprintf(stderr, "ifree: bread failed\n");
        return;
    }
    dip = (struct dinode*)bp->data + inum % IPB;
    memset(dip, 0, sizeof(*dip));
    log_write(bp);
    brelse(bp);
}

// Inode content
//
// The content (data) associated with each inode is stored
// in blocks on the disk. The first NDIRECT block numbers
// are listed in ip->addrs[]. The next NINDIRECT blocks are
// listed in block ip->addrs[NDIRECT].

// Return the disk block address of the nth block in inode ip
// If there is no such block, bmap allocates one
uint bmap(struct inode *ip, uint bn) {
    uint addr, *a;
    struct buf *bp;

    if (bn < NDIRECT) {
        if ((addr = ip->addrs[bn]) == 0)
            ip->addrs[bn] = addr = balloc();
        return addr;
    }
    bn -= NDIRECT;

    if (bn < NINDIRECT) {
        // Load indirect block, allocating if necessary
        if ((addr = ip->addrs[NDIRECT]) == 0)
            ip->addrs[NDIRECT] = addr = balloc();
        bp = bread(ip->dev, addr);
        if (bp == NULL) return 0;
        a = (uint*)bp->data;
        if ((addr = a[bn]) == 0) {
            a[bn] = addr = balloc();
            log_write(bp);
        }
        brelse(bp);
        return addr;
    }

    fprintf(stderr, "bmap: out of range\n");
    return 0;
}

// Truncate inode (discard contents)
// Called when the inode has no links to it (no directory entries referring to it)
// and has no in-memory reference to it (is not an open file or current directory)
void itrunc(struct inode *ip) {
    int i;
    uint j;
    struct buf *bp;
    uint *a;

    for (i = 0; i < NDIRECT; i++) {
        if (ip->addrs[i]) {
            bfree(ip->addrs[i]);
            ip->addrs[i] = 0;
        }
    }

    if (ip->addrs[NDIRECT]) {
        bp = bread(ip->dev, ip->addrs[NDIRECT]);
        if (bp != NULL) {
            a = (uint*)bp->data;
            for (j = 0; j < NINDIRECT; j++) {
                if (a[j])
                    bfree(a[j]);
            }
            brelse(bp);
        }
        bfree(ip->addrs[NDIRECT]);
        ip->addrs[NDIRECT] = 0;
    }

    ip->size = 0;
    iupdate(ip);
}

// Read data from inode
// Returns number of bytes read, or -1 on error
int readi(struct inode *ip, char *dst, uint off, uint n) {
    uint tot, m;
    struct buf *bp;

    if (ip->type == T_DEV) {
        fprintf(stderr, "readi: device not supported\n");
        return -1;
    }

    if (off > ip->size || off + n < off)
        return -1;
    if (off + n > ip->size)
        n = ip->size - off;

    for (tot = 0; tot < n; tot += m, off += m, dst += m) {
        uint bn = bmap(ip, off / BSIZE);
        if (bn == 0)
            return -1;
        bp = bread(ip->dev, bn);
        if (bp == NULL)
            return -1;
        m = BSIZE - off % BSIZE;
        if (n - tot < m)
            m = n - tot;
        memcpy(dst, bp->data + off % BSIZE, m);
        brelse(bp);
    }
    return n;
}

// Write data to inode
// Returns number of bytes written, or -1 on error
int writei(struct inode *ip, char *src, uint off, uint n) {
    uint tot, m;
    struct buf *bp;

    if (ip->type == T_DEV) {
        fprintf(stderr, "writei: device not supported\n");
        return -1;
    }

    if (off > ip->size || off + n < off)
        return -1;
    if (off + n > MAXFILE * BSIZE)
        return -1;

    for (tot = 0; tot < n; tot += m, off += m, src += m) {
        uint bn = bmap(ip, off / BSIZE);
        if (bn == 0)
            return -1;
        bp = bread(ip->dev, bn);
        if (bp == NULL)
            return -1;
        m = BSIZE - off % BSIZE;
        if (n - tot < m)
            m = n - tot;
        memcpy(bp->data + off % BSIZE, src, m);
        log_write(bp);
        brelse(bp);
    }

    if (n > 0 && off > ip->size) {
        ip->size = off;
        iupdate(ip);
    }
    return n;
}
