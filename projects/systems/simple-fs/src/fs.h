#ifndef FS_H
#define FS_H

#include "types.h"
#include "param.h"

// Superblock structure
struct superblock {
    uint32 magic;        // Must be FSMAGIC
    uint32 size;         // Size of file system image (blocks)
    uint32 nblocks;      // Number of data blocks
    uint32 ninodes;      // Number of inodes
    uint32 nlog;         // Number of log blocks
    uint32 logstart;     // Block number of first log block
    uint32 inodestart;   // Block number of first inode block
    uint32 bmapstart;    // Block number of first free map block
};

// On-disk inode structure
struct dinode {
    uint16 type;         // File type (T_DIR, T_FILE, T_DEV)
    uint16 major;        // Major device number (T_DEV only)
    uint16 minor;        // Minor device number (T_DEV only)
    uint16 nlink;        // Number of links to inode in file system
    uint32 size;         // Size of file (bytes)
    uint32 addrs[NDIRECT+1];  // Data block addresses
};

// Directory entry structure
struct dirent {
    uint16 inum;         // Inode number
    char name[DIRSIZ];   // File name
};

// Inodes per block
#define IPB           (BSIZE / sizeof(struct dinode))

// Block containing inode i
#define IBLOCK(i, sb)     ((i) / IPB + sb.inodestart)

// Bitmap bits per block
#define BPB           (BSIZE*8)

// Block containing bit for block b
#define BBLOCK(b, sb) ((b)/BPB + sb.bmapstart)

#endif // FS_H
