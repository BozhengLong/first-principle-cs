#ifndef BLOCK_H
#define BLOCK_H

#include "types.h"
#include "fs.h"

// Block allocator operations
void binit(struct superblock *sb);
uint balloc(void);
void bfree(uint blockno);
void bzeroblk(uint blockno);

#endif // BLOCK_H
