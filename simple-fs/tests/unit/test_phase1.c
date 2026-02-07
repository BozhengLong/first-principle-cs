#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../src/types.h"
#include "../src/param.h"
#include "../src/fs.h"
#include "../src/disk.h"
#include "../src/block.h"
#include "../src/buf.h"
#include "../src/log.h"

#define TEST_IMG "test_fs.img"
#define TEST_BLOCKS 100

int tests_passed = 0;
int tests_failed = 0;

void test_disk_init() {
    printf("Test: disk_init... ");

    if (disk_init(TEST_IMG, TEST_BLOCKS) < 0) {
        printf("FAILED\n");
        tests_failed++;
        return;
    }

    if (disk_open(TEST_IMG) < 0) {
        printf("FAILED\n");
        tests_failed++;
        return;
    }

    if (disk_size() != TEST_BLOCKS) {
        printf("FAILED (size mismatch)\n");
        tests_failed++;
        disk_close();
        return;
    }

    disk_close();
    printf("PASSED\n");
    tests_passed++;
}

void test_disk_read_write() {
    printf("Test: disk_read_write... ");

    if (disk_open(TEST_IMG) < 0) {
        printf("FAILED (open)\n");
        tests_failed++;
        return;
    }

    // Write test data
    uchar write_buf[BSIZE];
    for (int i = 0; i < BSIZE; i++) {
        write_buf[i] = i % 256;
    }

    if (disk_write(5, write_buf) < 0) {
        printf("FAILED (write)\n");
        tests_failed++;
        disk_close();
        return;
    }

    // Read back
    uchar read_buf[BSIZE];
    if (disk_read(5, read_buf) < 0) {
        printf("FAILED (read)\n");
        tests_failed++;
        disk_close();
        return;
    }

    // Verify
    if (memcmp(write_buf, read_buf, BSIZE) != 0) {
        printf("FAILED (data mismatch)\n");
        tests_failed++;
        disk_close();
        return;
    }

    disk_close();
    printf("PASSED\n");
    tests_passed++;
}

void test_block_allocator() {
    printf("Test: block_allocator... ");

    if (disk_open(TEST_IMG) < 0) {
        printf("FAILED (open)\n");
        tests_failed++;
        return;
    }

    // Initialize superblock
    struct superblock sb;
    sb.magic = FSMAGIC;
    sb.size = TEST_BLOCKS;
    sb.nblocks = TEST_BLOCKS - 10;
    sb.ninodes = 100;
    sb.nlog = 5;
    sb.logstart = 2;
    sb.inodestart = 7;
    sb.bmapstart = 10;

    // Write superblock
    if (disk_write(1, &sb) < 0) {
        printf("FAILED (write sb)\n");
        tests_failed++;
        disk_close();
        return;
    }

    // Initialize bitmap block - mark metadata blocks as allocated
    uchar bitmap[BSIZE];
    memset(bitmap, 0, BSIZE);
    // Mark blocks 0-10 as allocated (boot, sb, log, inodes, bitmap)
    for (int i = 0; i <= 10; i++) {
        bitmap[i/8] |= (1 << (i % 8));
    }
    if (disk_write(sb.bmapstart, bitmap) < 0) {
        printf("FAILED (write bitmap)\n");
        tests_failed++;
        disk_close();
        return;
    }

    // Initialize buffer cache, logging, and block allocator
    binit_cache();
    loginit(0, &sb);
    binit(&sb);

    // Allocate a block
    uint b1 = balloc();
    if (b1 == 0) {
        printf("FAILED (balloc)\n");
        tests_failed++;
        disk_close();
        return;
    }

    // Allocate another block
    uint b2 = balloc();
    if (b2 == 0 || b2 == b1) {
        printf("FAILED (balloc duplicate)\n");
        tests_failed++;
        disk_close();
        return;
    }

    // Free first block
    bfree(b1);

    // Allocate again - should get b1 back
    uint b3 = balloc();
    if (b3 != b1) {
        printf("FAILED (balloc after free)\n");
        tests_failed++;
        disk_close();
        return;
    }

    disk_close();
    printf("PASSED\n");
    tests_passed++;
}

int main() {
    printf("=== Phase 1 Unit Tests ===\n\n");

    test_disk_init();
    test_disk_read_write();
    test_block_allocator();

    printf("\n=== Test Summary ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);

    // Cleanup
    remove(TEST_IMG);

    return tests_failed > 0 ? 1 : 0;
}
