#ifndef DISK_H
#define DISK_H

#include "types.h"
#include "param.h"

// Disk operations
int disk_init(const char *path, uint nblocks);
int disk_open(const char *path);
void disk_close(void);
int disk_read(uint blockno, void *data);
int disk_write(uint blockno, const void *data);
void disk_sync(void);
uint disk_size(void);

#endif // DISK_H
