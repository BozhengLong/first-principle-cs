#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "types.h"
#include "param.h"
#include "fs.h"
#include "disk.h"

// Number of blocks to allocate for the file system
#define FSSIZE 1000

int nbitmap = FSSIZE/(BSIZE*8) + 1;
int ninodeblocks = FSSIZE / 3;
int nlog = LOGSIZE;
int nmeta;    // Number of meta blocks (boot, sb, nlog, inode, bitmap)
int nblocks;  // Number of data blocks

struct superblock sb;
uchar zeroes[BSIZE];
uint freeinode = 1;
uint freeblock;

void mkfs_balloc(int);
void wsect(uint, void*);
void winode(uint, struct dinode*);
void rinode(uint inum, struct dinode *ip);
void rsect(uint sec, void *buf);
uint ialloc(ushort type);
void iappend(uint inum, void *p, int n);

// Initialize superblock
void init_superblock(void) {
    sb.magic = FSMAGIC;
    sb.size = FSSIZE;
    sb.nblocks = nblocks;
    sb.ninodes = ninodeblocks * IPB;
    sb.nlog = nlog;
    sb.logstart = 2;
    sb.inodestart = 2 + nlog;
    sb.bmapstart = 2 + nlog + ninodeblocks;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: mkfs fs.img [files...]\n");
        exit(1);
    }

    // Calculate layout
    nmeta = 2 + nlog + ninodeblocks + nbitmap;
    nblocks = FSSIZE - nmeta;

    // Initialize superblock
    init_superblock();

    printf("nmeta %d (boot, super, log blocks %u inode blocks %u, bitmap blocks %u) blocks %d total %d\n",
           nmeta, nlog, ninodeblocks, nbitmap, nblocks, FSSIZE);

    // Initialize disk
    if (disk_init(argv[1], FSSIZE) < 0) {
        fprintf(stderr, "mkfs: failed to initialize disk\n");
        exit(1);
    }

    if (disk_open(argv[1]) < 0) {
        fprintf(stderr, "mkfs: failed to open disk\n");
        exit(1);
    }

    freeblock = nmeta;  // First free block

    // Write superblock
    memset(zeroes, 0, sizeof(zeroes));
    wsect(1, &sb);

    // Create root directory
    uint rootino = ialloc(T_DIR);
    assert(rootino == ROOTINO);

    struct dirent de;
    memset(&de, 0, sizeof(de));
    de.inum = rootino;
    strcpy(de.name, ".");
    iappend(rootino, &de, sizeof(de));

    memset(&de, 0, sizeof(de));
    de.inum = rootino;
    strcpy(de.name, "..");
    iappend(rootino, &de, sizeof(de));

    // Add files if provided
    for (int i = 2; i < argc; i++) {
        char *shortname = argv[i];
        // Find last component of path
        char *p = shortname;
        for (char *q = shortname; *q; q++) {
            if (*q == '/') {
                p = q + 1;
            }
        }

        if (strlen(p) >= DIRSIZ) {
            fprintf(stderr, "mkfs: filename too long: %s\n", p);
            continue;
        }

        uint inum = ialloc(T_FILE);

        memset(&de, 0, sizeof(de));
        de.inum = inum;
        strncpy(de.name, p, DIRSIZ);
        iappend(rootino, &de, sizeof(de));

        // Read file and write to inode
        FILE *f = fopen(argv[i], "rb");
        if (!f) {
            perror(argv[i]);
            continue;
        }

        char buf[BSIZE];
        int n;
        while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
            iappend(inum, buf, n);
        }
        fclose(f);
    }

    // Fix size of root inode dir
    struct dinode din;
    rinode(rootino, &din);
    uint off = din.size;
    off = ((off/BSIZE) + 1) * BSIZE;
    din.size = off;
    winode(rootino, &din);

    // Mark used blocks in bitmap
    mkfs_balloc(freeblock);

    disk_sync();
    disk_close();

    return 0;
}

void wsect(uint sec, void *buf) {
    if (disk_write(sec, buf) < 0) {
        fprintf(stderr, "mkfs: write error\n");
        exit(1);
    }
}

void rsect(uint sec, void *buf) {
    if (disk_read(sec, buf) < 0) {
        fprintf(stderr, "mkfs: read error\n");
        exit(1);
    }
}

void winode(uint inum, struct dinode *ip) {
    uchar buf[BSIZE];
    uint bn = IBLOCK(inum, sb);
    rsect(bn, buf);
    struct dinode *dip = ((struct dinode*)buf) + (inum % IPB);
    *dip = *ip;
    wsect(bn, buf);
}

void rinode(uint inum, struct dinode *ip) {
    uchar buf[BSIZE];
    uint bn = IBLOCK(inum, sb);
    rsect(bn, buf);
    struct dinode *dip = ((struct dinode*)buf) + (inum % IPB);
    *ip = *dip;
}

uint ialloc(ushort type) {
    uint inum = freeinode++;
    struct dinode din;

    memset(&din, 0, sizeof(din));
    din.type = type;
    din.nlink = 1;
    din.size = 0;
    winode(inum, &din);
    return inum;
}

void mkfs_balloc(int used) {
    uchar buf[BSIZE];
    int i;

    printf("mkfs_balloc: first %d blocks have been allocated\n", used);
    assert(used < BSIZE*8);
    memset(buf, 0, BSIZE);
    for (i = 0; i < used; i++) {
        buf[i/8] = buf[i/8] | (0x1 << (i%8));
    }
    printf("mkfs_balloc: write bitmap block at sector %d\n", sb.bmapstart);
    wsect(sb.bmapstart, buf);
}

#define min(a, b) ((a) < (b) ? (a) : (b))

void iappend(uint inum, void *xp, int n) {
    char *p = (char*)xp;
    uint fbn, off, n1;
    struct dinode din;
    uchar buf[BSIZE];
    uint indirect[NINDIRECT];
    uint x;

    rinode(inum, &din);
    off = din.size;
    while (n > 0) {
        fbn = off / BSIZE;
        assert(fbn < MAXFILE);
        if (fbn < NDIRECT) {
            if (din.addrs[fbn] == 0) {
                din.addrs[fbn] = freeblock++;
            }
            x = din.addrs[fbn];
        } else {
            if (din.addrs[NDIRECT] == 0) {
                din.addrs[NDIRECT] = freeblock++;
            }
            rsect(din.addrs[NDIRECT], (uchar*)indirect);
            if (indirect[fbn - NDIRECT] == 0) {
                indirect[fbn - NDIRECT] = freeblock++;
                wsect(din.addrs[NDIRECT], (uchar*)indirect);
            }
            x = indirect[fbn - NDIRECT];
        }
        n1 = min(n, (fbn + 1) * BSIZE - off);
        rsect(x, buf);
        memmove(buf + off - (fbn * BSIZE), p, n1);
        wsect(x, buf);
        n -= n1;
        off += n1;
        p += n1;
    }
    din.size = off;
    winode(inum, &din);
}
