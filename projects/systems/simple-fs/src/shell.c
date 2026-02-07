#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "shell.h"
#include "types.h"
#include "param.h"
#include "fs.h"
#include "disk.h"
#include "block.h"
#include "buf.h"
#include "inode.h"
#include "dir.h"
#include "namei.h"
#include "file.h"
#include "log.h"

// Global shell state
static struct shell_state shell;

// Forward declarations
static int cmd_pwd(int argc, char **argv);
static int cmd_cd(int argc, char **argv);
static int cmd_ls(int argc, char **argv);
static int cmd_mkdir(int argc, char **argv);
static int cmd_touch(int argc, char **argv);
static int cmd_rm(int argc, char **argv);
static int cmd_cat(int argc, char **argv);
static int cmd_write(int argc, char **argv);
static int cmd_append(int argc, char **argv);
static int cmd_stat(int argc, char **argv);
static int cmd_help(int argc, char **argv);
static int cmd_exit(int argc, char **argv);

// Command table
struct command {
    const char *name;
    const char *usage;
    const char *description;
    int (*handler)(int argc, char **argv);
};

static struct command commands[] = {
    {"ls",     "ls [path]",           "List directory contents",    cmd_ls},
    {"mkdir",  "mkdir <path>",        "Create directory",           cmd_mkdir},
    {"touch",  "touch <path>",        "Create empty file",          cmd_touch},
    {"rm",     "rm <path>",           "Delete file/directory",      cmd_rm},
    {"cat",    "cat <path>",          "Display file contents",      cmd_cat},
    {"write",  "write <path> <text>", "Write text to file",         cmd_write},
    {"append", "append <path> <text>","Append text to file",        cmd_append},
    {"stat",   "stat <path>",         "Show file/directory info",   cmd_stat},
    {"pwd",    "pwd",                 "Print working directory",    cmd_pwd},
    {"cd",     "cd <path>",           "Change directory",           cmd_cd},
    {"help",   "help",                "Show available commands",    cmd_help},
    {"exit",   "exit",                "Exit shell",                 cmd_exit},
    {NULL, NULL, NULL, NULL}
};

// Normalize path by handling . and ..
int normalize_path(char *path) {
    char *parts[MAXPATH / 2];
    int nparts = 0;
    char temp[MAXPATH];
    strncpy(temp, path, MAXPATH - 1);
    temp[MAXPATH - 1] = '\0';

    // Tokenize by /
    char *token = strtok(temp, "/");
    while (token != NULL) {
        if (strcmp(token, ".") == 0) {
            // Skip current directory
        } else if (strcmp(token, "..") == 0) {
            // Go up one level
            if (nparts > 0) {
                nparts--;
            }
        } else if (strlen(token) > 0) {
            parts[nparts++] = token;
        }
        token = strtok(NULL, "/");
    }

    // Rebuild path
    path[0] = '/';
    path[1] = '\0';
    for (int i = 0; i < nparts; i++) {
        if (i > 0) {
            strncat(path, "/", MAXPATH - strlen(path) - 1);
        }
        strncat(path, parts[i], MAXPATH - strlen(path) - 1);
    }

    return 0;
}

// Resolve relative path to absolute path
int resolve_path(const char *path, char *resolved) {
    if (path[0] == '/') {
        strncpy(resolved, path, MAXPATH - 1);
        resolved[MAXPATH - 1] = '\0';
    } else {
        if (strcmp(shell.cwd, "/") == 0) {
            snprintf(resolved, MAXPATH, "/%s", path);
        } else {
            snprintf(resolved, MAXPATH, "%s/%s", shell.cwd, path);
        }
    }
    return normalize_path(resolved);
}

// Convert inode type to string
const char* type_to_string(uint16 type) {
    switch (type) {
        case T_DIR:  return "DIR";
        case T_FILE: return "FILE";
        case T_DEV:  return "DEV";
        default:     return "???";
    }
}

// Parse command line into arguments
static int parse_line(char *line, char **argv) {
    int argc = 0;
    char *p = line;

    while (*p && argc < MAXARGS - 1) {
        // Skip whitespace
        while (*p && isspace(*p)) p++;
        if (!*p) break;

        // Start of argument
        argv[argc++] = p;

        // Find end of argument
        while (*p && !isspace(*p)) p++;
        if (*p) *p++ = '\0';
    }
    argv[argc] = NULL;
    return argc;
}

// Execute a command
static int execute_command(int argc, char **argv) {
    for (struct command *cmd = commands; cmd->name != NULL; cmd++) {
        if (strcmp(argv[0], cmd->name) == 0) {
            return cmd->handler(argc, argv);
        }
    }
    printf("Unknown command: %s\n", argv[0]);
    printf("Type 'help' for available commands.\n");
    return -1;
}

// Command: pwd - print working directory
static int cmd_pwd(int argc, char **argv) {
    (void)argc; (void)argv;
    printf("%s\n", shell.cwd);
    return 0;
}

// Command: cd - change directory
static int cmd_cd(int argc, char **argv) {
    if (argc < 2) {
        // cd with no args goes to root
        strcpy(shell.cwd, "/");
        return 0;
    }

    char resolved[MAXPATH];
    resolve_path(argv[1], resolved);

    // Verify the path exists and is a directory
    struct inode *ip = namei(resolved);
    if (ip == NULL) {
        printf("cd: %s: No such directory\n", argv[1]);
        return -1;
    }

    ilock(ip);
    if (ip->type != T_DIR) {
        printf("cd: %s: Not a directory\n", argv[1]);
        iunlock(ip);
        iput(ip);
        return -1;
    }
    iunlock(ip);
    iput(ip);

    strncpy(shell.cwd, resolved, MAXPATH - 1);
    shell.cwd[MAXPATH - 1] = '\0';
    return 0;
}

// Command: ls - list directory contents
static int cmd_ls(int argc, char **argv) {
    char resolved[MAXPATH];
    if (argc < 2) {
        strcpy(resolved, shell.cwd);
    } else {
        resolve_path(argv[1], resolved);
    }

    struct inode *dp = namei(resolved);
    if (dp == NULL) {
        printf("ls: %s: No such file or directory\n", argc < 2 ? "." : argv[1]);
        return -1;
    }

    ilock(dp);
    if (dp->type != T_DIR) {
        // It's a file, just print its info
        printf("%s  %s  %u bytes\n", type_to_string(dp->type),
               argc < 2 ? "." : argv[1], dp->size);
        iunlock(dp);
        iput(dp);
        return 0;
    }

    // Read directory entries
    struct dirent de;
    for (uint off = 0; off < dp->size; off += sizeof(de)) {
        if (readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de)) {
            break;
        }
        if (de.inum == 0) continue;

        // Get inode for this entry to show type and size
        // Skip locking if it's the same inode (. entry)
        if (de.inum == dp->inum) {
            printf("%s  %-14s  %u bytes\n", type_to_string(dp->type), de.name, dp->size);
        } else {
            struct inode *ip = iget(0, de.inum);
            ilock(ip);
            printf("%s  %-14s  %u bytes\n", type_to_string(ip->type), de.name, ip->size);
            iunlock(ip);
            iput(ip);
        }
    }

    iunlock(dp);
    iput(dp);
    return 0;
}

// Command: mkdir - create directory
static int cmd_mkdir(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: mkdir <path>\n");
        return -1;
    }

    char resolved[MAXPATH];
    resolve_path(argv[1], resolved);

    struct inode *ip = create(resolved, T_DIR, 0, 0);
    if (ip == NULL) {
        printf("mkdir: %s: Cannot create directory\n", argv[1]);
        return -1;
    }
    iunlock(ip);
    iput(ip);
    end_op();
    return 0;
}

// Command: touch - create empty file
static int cmd_touch(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: touch <path>\n");
        return -1;
    }

    char resolved[MAXPATH];
    resolve_path(argv[1], resolved);

    // Check if file already exists
    struct inode *ip = namei(resolved);
    if (ip != NULL) {
        // File exists, just return success
        iput(ip);
        return 0;
    }

    ip = create(resolved, T_FILE, 0, 0);
    if (ip == NULL) {
        printf("touch: %s: Cannot create file\n", argv[1]);
        return -1;
    }
    iunlock(ip);
    iput(ip);
    end_op();
    return 0;
}

// Command: rm - remove file or directory
static int cmd_rm(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: rm <path>\n");
        return -1;
    }

    char resolved[MAXPATH];
    resolve_path(argv[1], resolved);

    // Cannot remove root
    if (strcmp(resolved, "/") == 0) {
        printf("rm: cannot remove root directory\n");
        return -1;
    }

    if (unlink(resolved) < 0) {
        printf("rm: %s: Cannot remove\n", argv[1]);
        return -1;
    }
    return 0;
}

// Command: cat - display file contents
static int cmd_cat(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: cat <path>\n");
        return -1;
    }

    char resolved[MAXPATH];
    resolve_path(argv[1], resolved);

    struct inode *ip = namei(resolved);
    if (ip == NULL) {
        printf("cat: %s: No such file\n", argv[1]);
        return -1;
    }

    ilock(ip);
    if (ip->type != T_FILE) {
        printf("cat: %s: Not a file\n", argv[1]);
        iunlock(ip);
        iput(ip);
        return -1;
    }

    // Read and print file contents
    char buf[512];
    uint off = 0;
    int n;
    while ((n = readi(ip, buf, off, sizeof(buf) - 1)) > 0) {
        buf[n] = '\0';
        printf("%s", buf);
        off += n;
    }
    if (ip->size > 0 && buf[0] != '\0') {
        // Add newline if file doesn't end with one
        printf("\n");
    }

    iunlock(ip);
    iput(ip);
    return 0;
}

// Command: write - write text to file (overwrite)
static int cmd_write(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: write <path> <text>\n");
        return -1;
    }

    char resolved[MAXPATH];
    resolve_path(argv[1], resolved);

    // Build text from remaining arguments
    char text[MAXLINE];
    text[0] = '\0';
    for (int i = 2; i < argc; i++) {
        if (i > 2) strncat(text, " ", MAXLINE - strlen(text) - 1);
        strncat(text, argv[i], MAXLINE - strlen(text) - 1);
    }

    struct inode *ip = namei(resolved);
    if (ip == NULL) {
        // Create the file
        ip = create(resolved, T_FILE, 0, 0);
        if (ip == NULL) {
            printf("write: %s: Cannot create file\n", argv[1]);
            return -1;
        }
    } else {
        begin_op();
        ilock(ip);
    }

    // Truncate and write
    itrunc(ip);
    int n = writei(ip, text, 0, strlen(text));
    if (n != (int)strlen(text)) {
        printf("write: error writing to file\n");
        iunlock(ip);
        iput(ip);
        end_op();
        return -1;
    }

    iunlock(ip);
    iput(ip);
    end_op();
    return 0;
}

// Command: append - append text to file
static int cmd_append(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: append <path> <text>\n");
        return -1;
    }

    char resolved[MAXPATH];
    resolve_path(argv[1], resolved);

    // Build text from remaining arguments
    char text[MAXLINE];
    text[0] = '\0';
    for (int i = 2; i < argc; i++) {
        if (i > 2) strncat(text, " ", MAXLINE - strlen(text) - 1);
        strncat(text, argv[i], MAXLINE - strlen(text) - 1);
    }

    struct inode *ip = namei(resolved);
    if (ip == NULL) {
        // Create the file
        ip = create(resolved, T_FILE, 0, 0);
        if (ip == NULL) {
            printf("append: %s: Cannot create file\n", argv[1]);
            return -1;
        }
    } else {
        begin_op();
        ilock(ip);
    }

    // Append at end of file
    int n = writei(ip, text, ip->size, strlen(text));
    if (n != (int)strlen(text)) {
        printf("append: error writing to file\n");
        iunlock(ip);
        iput(ip);
        end_op();
        return -1;
    }

    iunlock(ip);
    iput(ip);
    end_op();
    return 0;
}

// Command: stat - show file/directory info
static int cmd_stat(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: stat <path>\n");
        return -1;
    }

    char resolved[MAXPATH];
    resolve_path(argv[1], resolved);

    struct inode *ip = namei(resolved);
    if (ip == NULL) {
        printf("stat: %s: No such file or directory\n", argv[1]);
        return -1;
    }

    ilock(ip);
    printf("  File: %s\n", resolved);
    printf("  Type: %s\n", type_to_string(ip->type));
    printf("  Size: %u bytes\n", ip->size);
    printf(" Inode: %u\n", ip->inum);
    printf(" Links: %u\n", ip->nlink);
    iunlock(ip);
    iput(ip);
    return 0;
}

// Command: help - show available commands
static int cmd_help(int argc, char **argv) {
    (void)argc; (void)argv;
    printf("Available commands:\n");
    for (struct command *cmd = commands; cmd->name != NULL; cmd++) {
        printf("  %-25s %s\n", cmd->usage, cmd->description);
    }
    return 0;
}

// Command: exit - exit shell
static int cmd_exit(int argc, char **argv) {
    (void)argc; (void)argv;
    shell.running = 0;
    return 0;
}

// Main function
int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <fs.img>\n", argv[0]);
        return 1;
    }

    // Open the disk
    if (disk_open(argv[1]) < 0) {
        fprintf(stderr, "Failed to open disk: %s\n", argv[1]);
        return 1;
    }

    // Read superblock
    char buf[BSIZE];
    disk_read(1, buf);
    memcpy(&shell.sb, buf, sizeof(shell.sb));

    if (shell.sb.magic != FSMAGIC) {
        fprintf(stderr, "Invalid filesystem magic number\n");
        disk_close();
        return 1;
    }

    // Initialize subsystems
    binit_cache();
    loginit(0, &shell.sb);
    binit(&shell.sb);
    iinit(&shell.sb);

    // Initialize shell state
    strcpy(shell.cwd, "/");
    shell.running = 1;

    printf("Simple File System Shell\n");
    printf("Type 'help' for available commands.\n\n");

    // Main loop
    char line[MAXLINE];
    char *args[MAXARGS];

    while (shell.running) {
        printf("sfs:%s$ ", shell.cwd);
        fflush(stdout);

        if (fgets(line, MAXLINE, stdin) == NULL) {
            printf("\n");
            break;
        }

        // Remove newline
        line[strcspn(line, "\n")] = '\0';

        // Skip empty lines
        if (line[0] == '\0') continue;

        // Parse and execute
        int nargs = parse_line(line, args);
        if (nargs > 0) {
            execute_command(nargs, args);
        }
    }

    // Cleanup
    disk_sync();
    disk_close();

    return 0;
}