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

#define TEST_IMG "test_phase3.img"

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

// Test basic bread/brelse
void test_bread_brelse() {
    test_start("bread_brelse");

    struct buf *b = bread(0, 10);
    if (b == NULL) {
        test_fail("bread returned NULL");
        return;
    }

    if (b->blockno != 10) {
        test_fail("blockno mismatch");
        brelse(b);
        return;
    }

    if (b->refcnt != 1) {
        test_fail("refcnt should be 1");
        brelse(b);
        return;
    }

    brelse(b);
    test_pass();
}

// Test cache hit (same buffer returned)
void test_cache_hit() {
    test_start("cache_hit");

    struct buf *b1 = bread(0, 20);
    if (b1 == NULL) {
        test_fail("first bread returned NULL");
        return;
    }

    // Write some data
    memset(b1->data, 'A', 100);
    brelse(b1);

    // Read same block again
    struct buf *b2 = bread(0, 20);
    if (b2 == NULL) {
        test_fail("second bread returned NULL");
        return;
    }

    // Should be same buffer (cache hit)
    if (b1 != b2) {
        test_fail("different buffer returned (cache miss)");
        brelse(b2);
        return;
    }

    // Data should still be there
    if (b2->data[0] != 'A') {
        test_fail("data not preserved in cache");
        brelse(b2);
        return;
    }

    brelse(b2);
    test_pass();
}

// Test write-through
void test_write_through() {
    test_start("write_through");

    struct buf *b = bread(0, 30);
    if (b == NULL) {
        test_fail("bread returned NULL");
        return;
    }

    // Write data
    memset(b->data, 'X', BSIZE);
    bwrite(b);
    brelse(b);

    // Read directly from disk to verify
    char buf[BSIZE];
    disk_read(30, buf);
    if (buf[0] != 'X' || buf[BSIZE-1] != 'X') {
        test_fail("data not written to disk");
        return;
    }

    test_pass();
}

// Test LRU eviction
void test_lru_eviction() {
    test_start("lru_eviction");

    // Fill cache with NBUF different blocks
    for (int i = 0; i < NBUF; i++) {
        struct buf *b = bread(0, 100 + i);
        if (b == NULL) {
            test_fail("bread failed during fill");
            return;
        }
        // Write unique data
        b->data[0] = (uchar)i;
        bwrite(b);
        brelse(b);
    }

    // Access one more block - should evict LRU
    struct buf *b = bread(0, 200);
    if (b == NULL) {
        test_fail("bread failed for new block");
        return;
    }
    brelse(b);

    // First block (100) should have been evicted
    // Read it again - should come from disk
    b = bread(0, 100);
    if (b == NULL) {
        test_fail("bread failed for evicted block");
        return;
    }

    // Data should still be correct (was written to disk)
    if (b->data[0] != 0) {
        test_fail("evicted block data incorrect");
        brelse(b);
        return;
    }

    brelse(b);
    test_pass();
}

// Test reference counting prevents eviction
void test_refcnt_prevents_eviction() {
    test_start("refcnt_prevents_eviction");

    // Hold a reference to block 50
    struct buf *held = bread(0, 50);
    if (held == NULL) {
        test_fail("bread failed for held block");
        return;
    }
    held->data[0] = 'H';
    // Don't release - keep reference

    // Fill rest of cache
    for (int i = 0; i < NBUF - 1; i++) {
        struct buf *b = bread(0, 300 + i);
        if (b == NULL) {
            test_fail("bread failed during fill");
            brelse(held);
            return;
        }
        brelse(b);
    }

    // held buffer should still be valid
    if (held->blockno != 50) {
        test_fail("held buffer was evicted");
        brelse(held);
        return;
    }

    if (held->data[0] != 'H') {
        test_fail("held buffer data corrupted");
        brelse(held);
        return;
    }

    brelse(held);
    test_pass();
}

// Test regression: Phase 2 functionality still works
void test_regression_phase2() {
    test_start("regression_phase2");

    // Create a file
    struct inode *ip = create("/phase3test", T_FILE, 0, 0);
    if (ip == NULL) {
        test_fail("create failed");
        return;
    }

    // Write some data
    char wbuf[100];
    memset(wbuf, 'R', sizeof(wbuf));
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

    // Read it back via path
    ip = namei("/phase3test");
    if (ip == NULL) {
        test_fail("namei failed");
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

    // Delete the file
    if (unlink("/phase3test") < 0) {
        test_fail("unlink failed");
        return;
    }

    test_pass();
}

int main() {
    printf("=== Phase 3 Unit Tests ===\n\n");

    // Create file system using mkfs
    printf("Creating filesystem...\n");
    int ret = system("./mkfs test_phase3.img > /dev/null 2>&1");
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
    test_bread_brelse();
    test_cache_hit();
    test_write_through();
    test_lru_eviction();
    test_refcnt_prevents_eviction();
    test_regression_phase2();

    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);

    disk_close();

    return tests_failed > 0 ? 1 : 0;
}
