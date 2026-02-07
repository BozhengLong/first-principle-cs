#ifndef FILE_H
#define FILE_H

#include "types.h"
#include "inode.h"

// File operations
struct inode* create(char *path, uint16 type, uint16 major, uint16 minor);
int unlink(char *path);

#endif // FILE_H
