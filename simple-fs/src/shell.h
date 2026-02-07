#ifndef SHELL_H
#define SHELL_H

#include "types.h"
#include "fs.h"

// Shell constants
#define MAXPATH 256
#define MAXLINE 512
#define MAXARGS 16

// Shell state
struct shell_state {
    char cwd[MAXPATH];        // Current working directory
    int running;              // Shell running flag
    struct superblock sb;     // Cached superblock
};

// Path resolution helpers
int resolve_path(const char *path, char *resolved);
int normalize_path(char *path);

// Type conversion helper
const char* type_to_string(uint16 type);

#endif // SHELL_H
