#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../../src/types.h"
#include "../../src/param.h"
#include "../../src/fs.h"
#include "../../src/disk.h"
#include "../../src/block.h"
#include "../../src/inode.h"
#include "../../src/dir.h"
#include "../../src/namei.h"
#include "../../src/file.h"
#include "../../src/buf.h"
#include "../../src/log.h"

#define TEST_IMG "test_phase2.img"

static int tests_passed = 0;
static int tests_failed = 0;

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

// Test inode allocation and persistence
void test_inode_allocation() {
    test_start("inode_allocation");

    // Allocate several inodes
    uint inum1 = ialloc(0, T_FILE);
    uint inum2 = ialloc(0, T_DIR);
    uint inum3 = ialloc(0, T_FILE);

    if (inum1 == 0 || inum2 == 0 || inum3 == 0) {
        test_fail("ialloc returned 0");
        return;
    }

    if (inum1 == inum2 || inum2 == inum3 || inum1 == inum3) {
        test_fail("ialloc returned duplicate inums");
        return;
    }

    // Verify inodes can be retrieved
    struct inode *ip1 = iget(0, inum1);
    ilock(ip1);
    if (ip1->type != T_FILE) {
        test_fail("inode type mismatch");
        iunlock(ip1);
        iput(ip1);
        return;
    }
    iunlock(ip1);
    iput(ip1);

    test_pass();
}

// Test inode cache and reference counting
void test_inode_cache() {
    test_start("inode_cache");

    uint inum = ialloc(0, T_FILE);
    struct inode *ip1 = iget(0, inum);
    struct inode *ip2 = iget(0, inum);

    // Should return same inode
    if (ip1 != ip2) {
        test_fail("iget returned different inodes for same inum");
        iput(ip1);
        iput(ip2);
        return;
    }

    // Reference count should be 2
    if (ip1->ref != 2) {
        test_fail("reference count incorrect");
        iput(ip1);
        iput(ip2);
        return;
    }

    iput(ip1);
    if (ip1->ref != 1) {
        test_fail("reference count not decremented");
        iput(ip2);
        return;
    }

    iput(ip2);
    test_pass();
}

// Test inode I/O operations
void test_inode_io() {
    test_start("inode_io");

    uint inum = ialloc(0, T_FILE);
    struct inode *ip = iget(0, inum);
    ilock(ip);

    // Write some data
    char wbuf[100];
    memset(wbuf, 'A', sizeof(wbuf));
    int n = writei(ip, wbuf, 0, sizeof(wbuf));
    if (n != sizeof(wbuf)) {
        test_fail("writei failed");
        iunlock(ip);
        iput(ip);
        return;
    }

    // Read it back
    char rbuf[100];
    memset(rbuf, 0, sizeof(rbuf));
    n = readi(ip, rbuf, 0, sizeof(rbuf));
    if (n != sizeof(rbuf)) {
        test_fail("readi failed");
        iunlock(ip);
        iput(ip);
        return;
    }

    if (memcmp(wbuf, rbuf, sizeof(wbuf)) != 0) {
        test_fail("data mismatch");
        iunlock(ip);
        iput(ip);
        return;
    }

    iunlock(ip);
    iput(ip);
    test_pass();
}

// Test block mapping
void test_block_mapping() {
    test_start("block_mapping");

    uint inum = ialloc(0, T_FILE);
    struct inode *ip = iget(0, inum);
    ilock(ip);

    // Write data that spans multiple blocks
    char wbuf[BSIZE * 3];
    memset(wbuf, 'B', sizeof(wbuf));
    int n = writei(ip, wbuf, 0, sizeof(wbuf));
    if (n != sizeof(wbuf)) {
        test_fail("writei failed for multi-block");
        iunlock(ip);
        iput(ip);
        return;
    }

    // Verify size
    if (ip->size != sizeof(wbuf)) {
        test_fail("size incorrect");
        iunlock(ip);
        iput(ip);
        return;
    }

    // Read it back
    char rbuf[BSIZE * 3];
    memset(rbuf, 0, sizeof(rbuf));
    n = readi(ip, rbuf, 0, sizeof(rbuf));
    if (n != sizeof(rbuf)) {
        test_fail("readi failed for multi-block");
        iunlock(ip);
        iput(ip);
        return;
    }

    if (memcmp(wbuf, rbuf, sizeof(wbuf)) != 0) {
        test_fail("multi-block data mismatch");
        iunlock(ip);
        iput(ip);
        return;
    }

    iunlock(ip);
    iput(ip);
    test_pass();
}

// Test directory operations
void test_directory_operations() {
    test_start("directory_operations");

    uint inum = ialloc(0, T_DIR);
    struct inode *dp = iget(0, inum);
    ilock(dp);

    // Add some entries
    if (dirlink(dp, "file1", 10) < 0) {
        test_fail("dirlink failed");
        iunlock(dp);
        iput(dp);
        return;
    }

    if (dirlink(dp, "file2", 20) < 0) {
        test_fail("dirlink failed for second entry");
        iunlock(dp);
        iput(dp);
        return;
    }

    // Lookup entries
    struct inode *ip = dirlookup(dp, "file1", 0);
    if (ip == NULL || ip->inum != 10) {
        test_fail("dirlookup failed");
        if (ip) iput(ip);
        iunlock(dp);
        iput(dp);
        return;
    }
    iput(ip);

    // Try to add duplicate
    if (dirlink(dp, "file1", 30) == 0) {
        test_fail("dirlink allowed duplicate");
        iunlock(dp);
        iput(dp);
        return;
    }

    iunlock(dp);
    iput(dp);
    test_pass();
}

// Test path resolution
void test_path_resolution() {
    test_start("path_resolution");

    // Create a directory structure
    struct inode *root = iget(0, ROOTINO);
    ilock(root);

    // Create /testdir
    uint dirinum = ialloc(0, T_DIR);
    struct inode *testdir = iget(0, dirinum);
    ilock(testdir);
    dirlink(testdir, ".", dirinum);
    dirlink(testdir, "..", ROOTINO);
    iunlock(testdir);
    iput(testdir);

    dirlink(root, "testdir", dirinum);
    iunlock(root);
    iput(root);

    // Resolve /testdir
    struct inode *ip = namei("/testdir");
    if (ip == NULL) {
        test_fail("namei failed for /testdir");
        return;
    }
    if (ip->inum != dirinum) {
        test_fail("namei returned wrong inode");
        iput(ip);
        return;
    }
    iput(ip);

    test_pass();
}

// Test file creation
void test_file_creation() {
    test_start("file_creation");

    // Create a file
    struct inode *ip = create("/testfile", T_FILE, 0, 0);
    if (ip == NULL) {
        test_fail("create failed");
        return;
    }

    // Write some data
    char wbuf[50];
    memset(wbuf, 'C', sizeof(wbuf));
    int n = writei(ip, wbuf, 0, sizeof(wbuf));
    if (n != sizeof(wbuf)) {
        test_fail("writei failed");
        iunlock(ip);
        iput(ip);
        end_op();
        return;
    }

    iunlock(ip);
    iput(ip);
    end_op();  // End transaction started by create()

    // Verify we can find it
    ip = namei("/testfile");
    if (ip == NULL) {
        test_fail("namei failed for created file");
        return;
    }

    ilock(ip);
    if (ip->size != sizeof(wbuf)) {
        test_fail("file size incorrect");
        iunlock(ip);
        iput(ip);
        return;
    }

    iunlock(ip);
    iput(ip);
    test_pass();
}

// Test file deletion
void test_file_deletion() {
    test_start("file_deletion");

    // Create a file
    struct inode *ip = create("/delfile", T_FILE, 0, 0);
    if (ip == NULL) {
        test_fail("create failed");
        return;
    }
    iunlock(ip);
    iput(ip);
    end_op();  // End transaction started by create()

    // Delete it
    if (unlink("/delfile") < 0) {
        test_fail("unlink failed");
        return;
    }

    // Verify it's gone
    ip = namei("/delfile");
    if (ip != NULL) {
        test_fail("file still exists after unlink");
        iput(ip);
        return;
    }

    test_pass();
}

// Test integration scenario
void test_integration() {
    test_start("integration");

    // Create directory structure
    struct inode *dir1 = create("/dir1", T_DIR, 0, 0);
    if (dir1 == NULL) {
        test_fail("create dir1 failed");
        return;
    }
    iunlock(dir1);
    iput(dir1);
    end_op();  // End transaction started by create()

    // Create file in directory
    struct inode *file1 = create("/dir1/file1", T_FILE, 0, 0);
    if (file1 == NULL) {
        test_fail("create file1 failed");
        return;
    }

    // Write data
    char wbuf[200];
    memset(wbuf, 'D', sizeof(wbuf));
    int n = writei(file1, wbuf, 0, sizeof(wbuf));
    if (n != sizeof(wbuf)) {
        test_fail("writei failed");
        iunlock(file1);
        iput(file1);
        end_op();
        return;
    }
    iunlock(file1);
    iput(file1);
    end_op();  // End transaction started by create()

    // Read it back via path
    struct inode *file2 = namei("/dir1/file1");
    if (file2 == NULL) {
        test_fail("namei failed");
        return;
    }

    ilock(file2);
    char rbuf[200];
    n = readi(file2, rbuf, 0, sizeof(rbuf));
    if (n != sizeof(rbuf) || memcmp(wbuf, rbuf, sizeof(wbuf)) != 0) {
        test_fail("data mismatch");
        iunlock(file2);
        iput(file2);
        return;
    }
    iunlock(file2);
    iput(file2);

    test_pass();
}

int main() {
    printf("=== Phase 2 Unit Tests ===\n\n");

    // Create file system using mkfs
    printf("Creating filesystem...\n");
    int ret = system("./mkfs test_phase2.img > /dev/null 2>&1");
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

    // Initialize buffer cache FIRST
    binit_cache();

    // Initialize logging
    loginit(0, &sb);

    // Initialize block allocator and inode table
    binit(&sb);
    iinit(&sb);

    // Run tests
    test_inode_allocation();
    test_inode_cache();
    test_inode_io();
    test_block_mapping();
    test_directory_operations();
    test_path_resolution();
    test_file_creation();
    test_file_deletion();
    test_integration();

    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);

    disk_close();

    return tests_failed > 0 ? 1 : 0;
}
