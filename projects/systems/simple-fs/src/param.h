#ifndef PARAM_H
#define PARAM_H

// File system parameters
#define BSIZE       4096                // Block size (4KB)
#define NDIRECT     12                  // Number of direct blocks in inode
#define NINDIRECT   (BSIZE / sizeof(uint32))  // Number of indirect blocks
#define MAXFILE     (NDIRECT + NINDIRECT)     // Max file size in blocks
#define DIRSIZ      14                  // Max directory name length
#define LOGSIZE     30                  // Max log size in blocks
#define NBUF        30                  // Size of buffer cache
#define ROOTINO     1                   // Root inode number
#define FSMAGIC     0x53465346          // "SFSF" - Simple File System

// Inode types
#define T_DIR       1                   // Directory
#define T_FILE      2                   // File
#define T_DEV       3                   // Device

#endif // PARAM_H
