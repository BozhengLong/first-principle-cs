#include <stdio.h>
#include <string.h>
#include "types.h"
#include "param.h"
#include "fs.h"
#include "inode.h"
#include "dir.h"
#include "namei.h"
#include "file.h"
#include "log.h"

// Create the path new as a file, directory, or device
// Returns locked inode on success, NULL on failure
// Note: Caller must call end_op() after done with the inode
struct inode* create(char *path, uint16 type, uint16 major, uint16 minor) {
    struct inode *ip, *dp;
    char name[DIRSIZ];

    begin_op();

    if ((dp = nameiparent(path, name)) == NULL) {
        end_op();
        return NULL;
    }
    ilock(dp);

    if ((ip = dirlookup(dp, name, 0)) != NULL) {
        iunlock(dp);
        iput(dp);
        ilock(ip);
        if (type == T_FILE && ip->type == T_FILE) {
            // File exists, return it (caller will call end_op)
            return ip;
        }
        iunlock(ip);
        iput(ip);
        end_op();
        return NULL;
    }

    if ((ip = iget(0, ialloc(0, type))) == NULL) {
        iunlock(dp);
        iput(dp);
        end_op();
        return NULL;
    }

    ilock(ip);
    ip->major = major;
    ip->minor = minor;
    ip->nlink = 1;
    iupdate(ip);

    if (type == T_DIR) {  // Create . and .. entries
        dp->nlink++;  // for ".."
        iupdate(dp);
        // No ip->nlink++ for ".": avoid cyclic ref count
        if (dirlink(ip, ".", ip->inum) < 0 || dirlink(ip, "..", dp->inum) < 0) {
            fprintf(stderr, "create: dirlink failed\n");
        }
    }

    if (dirlink(dp, name, ip->inum) < 0) {
        fprintf(stderr, "create: dirlink failed\n");
    }

    iunlock(dp);
    iput(dp);

    // Note: Caller must call end_op() after done with the inode
    return ip;
}

// Remove a file or directory
int unlink(char *path) {
    struct inode *ip, *dp;
    struct dirent de;
    char name[DIRSIZ];
    uint off;

    begin_op();

    if ((dp = nameiparent(path, name)) == NULL) {
        end_op();
        return -1;
    }

    ilock(dp);

    // Cannot unlink "." or ".."
    if (strncmp(name, ".", DIRSIZ) == 0 || strncmp(name, "..", DIRSIZ) == 0) {
        iunlock(dp);
        iput(dp);
        end_op();
        return -1;
    }

    if ((ip = dirlookup(dp, name, &off)) == NULL) {
        iunlock(dp);
        iput(dp);
        end_op();
        return -1;
    }
    ilock(ip);

    if (ip->nlink < 1) {
        fprintf(stderr, "unlink: nlink < 1\n");
    }

    if (ip->type == T_DIR && !isdirempty(ip)) {
        iunlock(ip);
        iput(ip);
        iunlock(dp);
        iput(dp);
        end_op();
        return -1;
    }

    memset(&de, 0, sizeof(de));
    if (writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de)) {
        fprintf(stderr, "unlink: writei failed\n");
    }
    if (ip->type == T_DIR) {
        dp->nlink--;
        iupdate(dp);
    }
    iunlock(dp);
    iput(dp);

    ip->nlink--;
    iupdate(ip);
    iunlock(ip);
    iput(ip);

    end_op();
    return 0;
}
