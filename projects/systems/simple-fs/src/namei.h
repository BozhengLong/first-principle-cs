#ifndef NAMEI_H
#define NAMEI_H

#include "inode.h"

// Path resolution
struct inode* namei(char *path);
struct inode* nameiparent(char *path, char *name);

#endif // NAMEI_H
