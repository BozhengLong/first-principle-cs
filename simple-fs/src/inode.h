#ifndef INODE_H
#define INODE_H

#include <pthread.h>
#include "types.h"
#include "param.h"
#include "fs.h"

#define NINODE 50  // Maximum in-memory inodes

// In-memory inode structure
struct inode {
    uint dev;           // Device number
    uint inum;          // Inode number
    int ref;            // Reference count
    int valid;          // Has inode been read from disk?
    pthread_mutex_t lock;  // Protects fields below

    // Copy of disk inode
    uint16 type;
    uint16 major;
    uint16 minor;
    uint16 nlink;
    uint32 size;
    uint32 addrs[NDIRECT+1];
};

// Inode operations
void iinit(struct superblock *sb);
uint ialloc(uint dev, uint16 type);
void ifree(uint dev, uint inum);
struct inode* iget(uint dev, uint inum);
struct inode* idup(struct inode *ip);
void ilock(struct inode *ip);
void iunlock(struct inode *ip);
void iput(struct inode *ip);
void iupdate(struct inode *ip);
void itrunc(struct inode *ip);
int readi(struct inode *ip, char *dst, uint off, uint n);
int writei(struct inode *ip, char *src, uint off, uint n);
uint bmap(struct inode *ip, uint bn);

#endif // INODE_H
