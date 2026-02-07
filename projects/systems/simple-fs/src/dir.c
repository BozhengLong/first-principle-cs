#include <stdio.h>
#include <string.h>
#include "types.h"
#include "param.h"
#include "fs.h"
#include "inode.h"
#include "dir.h"

// Look for a directory entry in a directory
// If found, set *poff to byte offset of entry
struct inode* dirlookup(struct inode *dp, char *name, uint *poff) {
    uint off, inum;
    struct dirent de;

    if (dp->type != T_DIR) {
        fprintf(stderr, "dirlookup: not a directory\n");
        return NULL;
    }

    for (off = 0; off < dp->size; off += sizeof(de)) {
        if (readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de)) {
            fprintf(stderr, "dirlookup: read error\n");
            return NULL;
        }
        if (de.inum == 0)
            continue;
        if (strncmp(name, de.name, DIRSIZ) == 0) {
            // Entry matches path element
            if (poff)
                *poff = off;
            inum = de.inum;
            return iget(dp->dev, inum);
        }
    }

    return NULL;
}

// Write a new directory entry (name, inum) into the directory dp
int dirlink(struct inode *dp, char *name, uint inum) {
    int off;
    struct dirent de;
    struct inode *ip;

    // Check that name is not present
    if ((ip = dirlookup(dp, name, 0)) != NULL) {
        iput(ip);
        return -1;
    }

    // Look for an empty dirent
    for (off = 0; off < dp->size; off += sizeof(de)) {
        if (readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de)) {
            fprintf(stderr, "dirlink: read error\n");
            return -1;
        }
        if (de.inum == 0)
            break;
    }

    strncpy(de.name, name, DIRSIZ);
    de.inum = inum;
    if (writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de)) {
        fprintf(stderr, "dirlink: write error\n");
        return -1;
    }

    return 0;
}

// Remove a directory entry
int dirunlink(struct inode *dp, char *name) {
    uint off;
    struct dirent de;
    struct inode *ip;

    if ((ip = dirlookup(dp, name, &off)) == NULL)
        return -1;

    // Clear the directory entry
    memset(&de, 0, sizeof(de));
    if (writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de)) {
        fprintf(stderr, "dirunlink: write error\n");
        iput(ip);
        return -1;
    }

    iput(ip);
    return 0;
}

// Is the directory dp empty except for "." and ".." ?
int isdirempty(struct inode *dp) {
    int off;
    struct dirent de;

    for (off = 2 * sizeof(de); off < dp->size; off += sizeof(de)) {
        if (readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de)) {
            fprintf(stderr, "isdirempty: read error\n");
            return 0;
        }
        if (de.inum != 0)
            return 0;
    }
    return 1;
}
