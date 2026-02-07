#include <stdio.h>
#include <string.h>
#include "types.h"
#include "param.h"
#include "fs.h"
#include "inode.h"
#include "dir.h"
#include "namei.h"

// Copy the next path element from path into name
// Return a pointer to the element following the copied one
// The returned path has no leading slashes,
// so the caller can check *path=='\0' to see if the name is the last one
// If no name to remove, return 0
//
// Examples:
//   skipelem("a/bb/c", name) = "bb/c", setting name = "a"
//   skipelem("///a//bb", name) = "bb", setting name = "a"
//   skipelem("a", name) = "", setting name = "a"
//   skipelem("", name) = skipelem("////", name) = 0
//
static char* skipelem(char *path, char *name) {
    char *s;
    int len;

    while (*path == '/')
        path++;
    if (*path == 0)
        return NULL;
    s = path;
    while (*path != '/' && *path != 0)
        path++;
    len = path - s;
    if (len >= DIRSIZ)
        memcpy(name, s, DIRSIZ);
    else {
        memcpy(name, s, len);
        name[len] = 0;
    }
    while (*path == '/')
        path++;
    return path;
}

// Look up and return the inode for a path name
// If parent != 0, return the inode for the parent and copy the final
// path element into name, which must have room for DIRSIZ bytes
// Must be called inside a transaction since it calls iput()
static struct inode* namex(char *path, int nameiparent, char *name) {
    struct inode *ip, *next;

    if (*path == '/')
        ip = iget(0, ROOTINO);
    else
        return NULL;  // Only support absolute paths for now

    while ((path = skipelem(path, name)) != NULL) {
        ilock(ip);
        if (ip->type != T_DIR) {
            iunlock(ip);
            iput(ip);
            return NULL;
        }
        if (nameiparent && *path == '\0') {
            // Stop one level early
            iunlock(ip);
            return ip;
        }
        if ((next = dirlookup(ip, name, 0)) == NULL) {
            iunlock(ip);
            iput(ip);
            return NULL;
        }
        iunlock(ip);
        iput(ip);
        ip = next;
    }
    if (nameiparent) {
        iput(ip);
        return NULL;
    }
    return ip;
}

struct inode* namei(char *path) {
    char name[DIRSIZ];
    return namex(path, 0, name);
}

struct inode* nameiparent(char *path, char *name) {
    return namex(path, 1, name);
}
