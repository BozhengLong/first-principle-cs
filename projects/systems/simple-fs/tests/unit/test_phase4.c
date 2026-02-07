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

#define TEST_IMG "test_phase4.img"

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

// Test basic transaction
void test_basic_transaction() {
    test_start("basic_transaction");

    // Create a file (create() calls begin_op internally)
    struct inode *ip = create("/logtest1", T_FILE, 0, 0);
    if (ip == NULL) {
        test_fail("create failed");
        return;
    }
    iunlock(ip);
    iput(ip);
    end_op();  // End the transaction started by create()

    // Verify file exists
    ip = namei("/logtest1");
    if (ip == NULL) {
        test_fail("file not found after create");
        return;
    }
    iput(ip);

    test_pass();
}

// Test multi-operation transaction
void test_multi_op_transaction() {
    test_start("multi_op_transaction");

    // Create a file and write data
    struct inode *ip = create("/logtest2", T_FILE, 0, 0);
    if (ip == NULL) {
        test_fail("create failed");
        return;
    }

    // Write some data (within same transaction)
    char wbuf[100];
    memset(wbuf, 'X', sizeof(wbuf));
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
    end_op();

    // Verify data
    ip = namei("/logtest2");
    if (ip == NULL) {
        test_fail("file not found");
        return;
    }

    ilock(ip);
    char rbuf[100];
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

// Test log header is clean after operations
void test_log_header_clean() {
    test_start("log_header_clean");

    // Read log header directly from disk
    char buf[BSIZE];
    disk_read(2, buf);  // logstart = 2
    struct logheader *lh = (struct logheader*)buf;

    // After clean operations, n should be 0
    if (lh->n != 0) {
        test_fail("log header n is not 0");
        return;
    }

    test_pass();
}

// Test unlink with transaction
void test_unlink_transaction() {
    test_start("unlink_transaction");

    // Create a file
    struct inode *ip = create("/deltest", T_FILE, 0, 0);
    if (ip == NULL) {
        test_fail("create failed");
        return;
    }
    iunlock(ip);
    iput(ip);
    end_op();

    // Verify file exists
    ip = namei("/deltest");
    if (ip == NULL) {
        test_fail("file not found after create");
        return;
    }
    iput(ip);

    // Delete the file (unlink calls begin_op/end_op internally)
    if (unlink("/deltest") < 0) {
        test_fail("unlink failed");
        return;
    }

    // Verify file is gone
    ip = namei("/deltest");
    if (ip != NULL) {
        test_fail("file still exists after unlink");
        iput(ip);
        return;
    }

    test_pass();
}

// Test regression: Phase 3 functionality still works
void test_regression_phase3() {
    test_start("regression_phase3");

    // Test buffer cache still works
    struct buf *b = bread(0, 100);
    if (b == NULL) {
        test_fail("bread failed");
        return;
    }

    if (b->blockno != 100) {
        test_fail("blockno mismatch");
        brelse(b);
        return;
    }

    brelse(b);
    test_pass();
}

int main() {
    printf("=== Phase 4 Unit Tests ===\n\n");

    // Create file system using mkfs
    printf("Creating filesystem...\n");
    int ret = system("./mkfs test_phase4.img > /dev/null 2>&1");
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

    // Initialize logging (performs recovery if needed)
    loginit(0, &sb);

    // Initialize block allocator and inode table
    binit(&sb);
    iinit(&sb);

    // Run tests
    test_basic_transaction();
    test_multi_op_transaction();
    test_log_header_clean();
    test_unlink_transaction();
    test_regression_phase3();

    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);

    disk_close();

    return tests_failed > 0 ? 1 : 0;
}
