#ifndef LOG_H
#define LOG_H

#include "types.h"
#include "param.h"
#include "fs.h"
#include "buf.h"

// Maximum blocks a single operation can write
#define MAXOPBLOCKS  10

// On-disk log header structure (stored at sb.logstart)
struct logheader {
    int n;                      // Number of logged blocks (0 = no transaction)
    uint block[LOGSIZE - 1];    // Destination block numbers
};

// Initialize logging system and perform recovery if needed
void loginit(int dev, struct superblock *sb);

// Start a file system operation
void begin_op(void);

// End a file system operation, commit if needed
void end_op(void);

// Log a buffer write (call instead of bwrite for logged operations)
void log_write(struct buf *b);

#endif // LOG_H
