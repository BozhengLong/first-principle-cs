#ifndef DIR_H
#define DIR_H

#include "types.h"
#include "inode.h"

// Directory operations
struct inode* dirlookup(struct inode *dp, char *name, uint *poff);
int dirlink(struct inode *dp, char *name, uint inum);
int dirunlink(struct inode *dp, char *name);
int isdirempty(struct inode *dp);

#endif // DIR_H
