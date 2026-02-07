#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../../src/types.h"
#include "../../src/param.h"
#include "../../src/fs.h"
#include "../../src/disk.h"
#include "../../src/block.h"
#include "../../src/buf.h"
#include "../../src/inode.h"
#include "../../src/dir.h"
#include "../../src/namei.h"
#include "../../src/file.h"
#include "../../src/log.h"
#include "../../src/shell.h"

#define TEST_IMG "test_phase5.img"

static int tests_passed = 0;
static int tests_failed = 0;

// Local implementations of shell helper functions for testing
// (shell.c has main() so we can't link it directly)
static int test_normalize_path(char *path) {
    char *parts[MAXPATH / 2];
    int nparts = 0;
    char temp[MAXPATH];
    strncpy(temp, path, MAXPATH - 1);
    temp[MAXPATH - 1] = '\0';

    char *token = strtok(temp, "/");
    while (token != NULL) {
        if (strcmp(token, ".") == 0) {
            // Skip
        } else if (strcmp(token, "..") == 0) {
            if (nparts > 0) nparts--;
        } else if (strlen(token) > 0) {
            parts[nparts++] = token;
        }
        token = strtok(NULL, "/");
    }

    path[0] = '/';
    path[1] = '\0';
    for (int i = 0; i < nparts; i++) {
        if (i > 0) strncat(path, "/", MAXPATH - strlen(path) - 1);
        strncat(path, parts[i], MAXPATH - strlen(path) - 1);
    }
    return 0;
}

static const char* test_type_to_string(uint16 type) {
    switch (type) {
        case T_DIR:  return "DIR";
        case T_FILE: return "FILE";
        case T_DEV:  return "DEV";
        default:     return "???";
    }
}

void test_start(const char *name) {
    printf("Test: %s... ", name);
    fflush(stdout);
}

void test_pass() {
    printf("PASSED\n");
    tests_passed++;
}

void test_fail(const char *msg) {
    printf("FAILED: %s\n", msg);
    tests_failed++;
}

// Test path normalization
void test_path_normalization() {
    test_start("path_normalization");

    char path[MAXPATH];

    // Test simple path
    strcpy(path, "/foo/bar");
    test_normalize_path(path);
    if (strcmp(path, "/foo/bar") != 0) {
        test_fail("simple path failed");
        return;
    }

    // Test path with .
    strcpy(path, "/foo/./bar");
    test_normalize_path(path);
    if (strcmp(path, "/foo/bar") != 0) {
        test_fail("path with . failed");
        return;
    }

    // Test path with ..
    strcpy(path, "/foo/bar/../baz");
    test_normalize_path(path);
    if (strcmp(path, "/foo/baz") != 0) {
        test_fail("path with .. failed");
        return;
    }

    // Test path with multiple ..
    strcpy(path, "/foo/bar/baz/../../qux");
    test_normalize_path(path);
    if (strcmp(path, "/foo/qux") != 0) {
        test_fail("path with multiple .. failed");
        return;
    }

    // Test .. at root
    strcpy(path, "/../foo");
    test_normalize_path(path);
    if (strcmp(path, "/foo") != 0) {
        test_fail(".. at root failed");
        return;
    }

    test_pass();
}

// Test mkdir basic
void test_mkdir_basic() {
    test_start("mkdir_basic");

    struct inode *ip = create("/testdir", T_DIR, 0, 0);
    if (ip == NULL) {
        test_fail("create directory failed");
        return;
    }
    iunlock(ip);
    iput(ip);
    end_op();

    // Verify directory exists
    ip = namei("/testdir");
    if (ip == NULL) {
        test_fail("directory not found");
        return;
    }

    ilock(ip);
    if (ip->type != T_DIR) {
        test_fail("not a directory");
        iunlock(ip);
        iput(ip);
        return;
    }
    iunlock(ip);
    iput(ip);

    test_pass();
}

// Test touch basic
void test_touch_basic() {
    test_start("touch_basic");

    struct inode *ip = create("/testfile.txt", T_FILE, 0, 0);
    if (ip == NULL) {
        test_fail("create file failed");
        return;
    }
    iunlock(ip);
    iput(ip);
    end_op();

    // Verify file exists
    ip = namei("/testfile.txt");
    if (ip == NULL) {
        test_fail("file not found");
        return;
    }

    ilock(ip);
    if (ip->type != T_FILE) {
        test_fail("not a file");
        iunlock(ip);
        iput(ip);
        return;
    }
    if (ip->size != 0) {
        test_fail("file not empty");
        iunlock(ip);
        iput(ip);
        return;
    }
    iunlock(ip);
    iput(ip);

    test_pass();
}

// Test write and cat
void test_write_and_cat() {
    test_start("write_and_cat");

    // Create and write to file
    struct inode *ip = create("/hello.txt", T_FILE, 0, 0);
    if (ip == NULL) {
        test_fail("create file failed");
        return;
    }

    char *text = "Hello, World!";
    int n = writei(ip, text, 0, strlen(text));
    if (n != (int)strlen(text)) {
        test_fail("write failed");
        iunlock(ip);
        iput(ip);
        end_op();
        return;
    }
    iunlock(ip);
    iput(ip);
    end_op();

    // Read back
    ip = namei("/hello.txt");
    if (ip == NULL) {
        test_fail("file not found");
        return;
    }

    ilock(ip);
    char buf[64];
    n = readi(ip, buf, 0, sizeof(buf) - 1);
    buf[n] = '\0';

    if (strcmp(buf, text) != 0) {
        test_fail("content mismatch");
        iunlock(ip);
        iput(ip);
        return;
    }
    iunlock(ip);
    iput(ip);

    test_pass();
}

// Test ls directory
void test_ls_directory() {
    test_start("ls_directory");

    // Create a directory with some files
    struct inode *dp = create("/lsdir", T_DIR, 0, 0);
    if (dp == NULL) {
        test_fail("create directory failed");
        return;
    }
    iunlock(dp);
    iput(dp);
    end_op();

    // Create files in directory
    struct inode *ip = create("/lsdir/file1", T_FILE, 0, 0);
    if (ip == NULL) {
        test_fail("create file1 failed");
        return;
    }
    iunlock(ip);
    iput(ip);
    end_op();

    ip = create("/lsdir/file2", T_FILE, 0, 0);
    if (ip == NULL) {
        test_fail("create file2 failed");
        return;
    }
    iunlock(ip);
    iput(ip);
    end_op();

    // Verify directory has entries
    dp = namei("/lsdir");
    if (dp == NULL) {
        test_fail("directory not found");
        return;
    }

    ilock(dp);
    int count = 0;
    struct dirent de;
    for (uint off = 0; off < dp->size; off += sizeof(de)) {
        if (readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de)) break;
        if (de.inum != 0) count++;
    }
    iunlock(dp);
    iput(dp);

    // Should have at least . .. file1 file2
    if (count < 4) {
        test_fail("not enough directory entries");
        return;
    }

    test_pass();
}

// Test rm file
void test_rm_file() {
    test_start("rm_file");

    // Create a file
    struct inode *ip = create("/rmtest.txt", T_FILE, 0, 0);
    if (ip == NULL) {
        test_fail("create file failed");
        return;
    }
    iunlock(ip);
    iput(ip);
    end_op();

    // Verify it exists
    ip = namei("/rmtest.txt");
    if (ip == NULL) {
        test_fail("file not found after create");
        return;
    }
    iput(ip);

    // Delete it
    if (unlink("/rmtest.txt") < 0) {
        test_fail("unlink failed");
        return;
    }

    // Verify it's gone
    ip = namei("/rmtest.txt");
    if (ip != NULL) {
        test_fail("file still exists after unlink");
        iput(ip);
        return;
    }

    test_pass();
}

// Test type_to_string
void test_type_to_string_func() {
    test_start("type_to_string");

    if (strcmp(test_type_to_string(T_DIR), "DIR") != 0) {
        test_fail("T_DIR failed");
        return;
    }
    if (strcmp(test_type_to_string(T_FILE), "FILE") != 0) {
        test_fail("T_FILE failed");
        return;
    }
    if (strcmp(test_type_to_string(T_DEV), "DEV") != 0) {
        test_fail("T_DEV failed");
        return;
    }

    test_pass();
}

// Test regression: Phase 4 functionality still works
void test_regression_phase4() {
    test_start("regression_phase4");

    // Test that transactions still work
    struct inode *ip = create("/regtest", T_FILE, 0, 0);
    if (ip == NULL) {
        test_fail("create failed");
        return;
    }

    char *data = "Regression test data";
    int n = writei(ip, data, 0, strlen(data));
    if (n != (int)strlen(data)) {
        test_fail("write failed");
        iunlock(ip);
        iput(ip);
        end_op();
        return;
    }
    iunlock(ip);
    iput(ip);
    end_op();

    // Read back
    ip = namei("/regtest");
    if (ip == NULL) {
        test_fail("file not found");
        return;
    }

    ilock(ip);
    char buf[64];
    n = readi(ip, buf, 0, sizeof(buf) - 1);
    buf[n] = '\0';

    if (strcmp(buf, data) != 0) {
        test_fail("data mismatch");
        iunlock(ip);
        iput(ip);
        return;
    }
    iunlock(ip);
    iput(ip);

    test_pass();
}

int main() {
    printf("=== Phase 5 Unit Tests ===\n\n");

    // Create file system using mkfs
    printf("Creating filesystem...\n");
    int ret = system("./mkfs test_phase5.img > /dev/null 2>&1");
    if (ret != 0) {
        fprintf(stderr, "Failed to create filesystem\n");
        return 1;
    }

    // Open the disk
    if (disk_open(TEST_IMG) < 0) {
        fprintf(stderr, "Failed to open disk\n");
        return 1;
    }

    // Read superblock
    struct superblock sb;
    char buf[BSIZE];
    disk_read(1, buf);
    memcpy(&sb, buf, sizeof(sb));

    // Initialize subsystems
    binit_cache();
    loginit(0, &sb);
    binit(&sb);
    iinit(&sb);

    // Run tests
    test_path_normalization();
    test_mkdir_basic();
    test_touch_basic();
    test_write_and_cat();
    test_ls_directory();
    test_rm_file();
    test_type_to_string_func();
    test_regression_phase4();

    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);

    disk_close();

    return tests_failed > 0 ? 1 : 0;
}