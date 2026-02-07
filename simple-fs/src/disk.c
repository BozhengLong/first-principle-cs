#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include "disk.h"

static int disk_fd = -1;
static uint disk_nblocks = 0;

// Initialize a new disk image
int disk_init(const char *path, uint nblocks) {
    int fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("disk_init: open");
        return -1;
    }

    // Allocate space for the disk
    char zero[BSIZE];
    memset(zero, 0, BSIZE);

    for (uint i = 0; i < nblocks; i++) {
        if (write(fd, zero, BSIZE) != BSIZE) {
            perror("disk_init: write");
            close(fd);
            return -1;
        }
    }

    close(fd);
    return 0;
}

// Open an existing disk image
int disk_open(const char *path) {
    disk_fd = open(path, O_RDWR);
    if (disk_fd < 0) {
        perror("disk_open: open");
        return -1;
    }

    // Get disk size
    off_t size = lseek(disk_fd, 0, SEEK_END);
    if (size < 0) {
        perror("disk_open: lseek");
        close(disk_fd);
        disk_fd = -1;
        return -1;
    }

    disk_nblocks = size / BSIZE;
    return 0;
}

// Close the disk
void disk_close(void) {
    if (disk_fd >= 0) {
        close(disk_fd);
        disk_fd = -1;
        disk_nblocks = 0;
    }
}

// Read a block from disk
int disk_read(uint blockno, void *data) {
    assert(disk_fd >= 0);
    assert(blockno < disk_nblocks);

    if (lseek(disk_fd, blockno * BSIZE, SEEK_SET) < 0) {
        perror("disk_read: lseek");
        return -1;
    }

    if (read(disk_fd, data, BSIZE) != BSIZE) {
        perror("disk_read: read");
        return -1;
    }

    return 0;
}

// Write a block to disk
int disk_write(uint blockno, const void *data) {
    assert(disk_fd >= 0);
    assert(blockno < disk_nblocks);

    if (lseek(disk_fd, blockno * BSIZE, SEEK_SET) < 0) {
        perror("disk_write: lseek");
        return -1;
    }

    if (write(disk_fd, data, BSIZE) != BSIZE) {
        perror("disk_write: write");
        return -1;
    }

    return 0;
}

// Sync disk to ensure all writes are persisted
void disk_sync(void) {
    if (disk_fd >= 0) {
        fsync(disk_fd);
    }
}

// Get disk size in blocks
uint disk_size(void) {
    return disk_nblocks;
}
