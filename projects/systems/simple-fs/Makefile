CC = gcc
CFLAGS = -Wall -Wextra -g -O0 -Isrc
LDFLAGS = -lpthread

# Source files
DISK_SRC = src/disk.c
BLOCK_SRC = src/block.c
MKFS_SRC = src/mkfs.c

# Phase 2 source files
LOCK_SRC = src/lock.c
INODE_SRC = src/inode.c
DIR_SRC = src/dir.c
NAMEI_SRC = src/namei.c
FILE_SRC = src/file.c

# Phase 3 source files
BUF_SRC = src/buf.c

# Phase 4 source files
LOG_SRC = src/log.c

# Phase 5 source files
SHELL_SRC = src/shell.c

# Object files
DISK_OBJ = $(DISK_SRC:.c=.o)
BLOCK_OBJ = $(BLOCK_SRC:.c=.o)

# Phase 2 object files
LOCK_OBJ = $(LOCK_SRC:.c=.o)
INODE_OBJ = $(INODE_SRC:.c=.o)
DIR_OBJ = $(DIR_SRC:.c=.o)
NAMEI_OBJ = $(NAMEI_SRC:.c=.o)
FILE_OBJ = $(FILE_SRC:.c=.o)

# Phase 3 object files
BUF_OBJ = $(BUF_SRC:.c=.o)

# Phase 4 object files
LOG_OBJ = $(LOG_SRC:.c=.o)

# Phase 5 object files
SHELL_OBJ = $(SHELL_SRC:.c=.o)

PHASE2_OBJ = $(LOCK_OBJ) $(INODE_OBJ) $(DIR_OBJ) $(NAMEI_OBJ) $(FILE_OBJ)
PHASE3_OBJ = $(BUF_OBJ)
PHASE4_OBJ = $(LOG_OBJ)
PHASE5_OBJ = $(SHELL_OBJ)

# Targets
all: mkfs shell

mkfs: $(MKFS_SRC) $(DISK_OBJ) $(BLOCK_OBJ) $(PHASE3_OBJ) $(PHASE4_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

shell: $(SHELL_SRC) $(DISK_OBJ) $(BLOCK_OBJ) $(PHASE2_OBJ) $(PHASE3_OBJ) $(PHASE4_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Unit tests
test_phase1: tests/unit/test_phase1.c $(DISK_OBJ) $(BLOCK_OBJ) $(PHASE3_OBJ) $(PHASE4_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_phase2: tests/unit/test_phase2.c $(DISK_OBJ) $(BLOCK_OBJ) $(PHASE2_OBJ) $(PHASE3_OBJ) $(PHASE4_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_phase3: tests/unit/test_phase3.c $(DISK_OBJ) $(BLOCK_OBJ) $(PHASE2_OBJ) $(PHASE3_OBJ) $(PHASE4_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_phase4: tests/unit/test_phase4.c $(DISK_OBJ) $(BLOCK_OBJ) $(PHASE2_OBJ) $(PHASE3_OBJ) $(PHASE4_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_phase5: tests/unit/test_phase5.c $(DISK_OBJ) $(BLOCK_OBJ) $(PHASE2_OBJ) $(PHASE3_OBJ) $(PHASE4_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Create a test file system
fs.img: mkfs
	./mkfs fs.img

test: test_phase1 test_phase2 test_phase3 test_phase4 test_phase5 mkfs
	@echo "Running Phase 1 tests..."
	./test_phase1
	@echo ""
	@echo "Creating file system..."
	./mkfs test_phase2.img
	@echo ""
	@echo "Running Phase 2 tests..."
	./test_phase2
	@echo ""
	@echo "Running Phase 3 tests..."
	./test_phase3
	@echo ""
	@echo "Running Phase 4 tests..."
	./test_phase4
	@echo ""
	@echo "Running Phase 5 tests..."
	./test_phase5
	@echo ""
	@echo "All tests completed"

clean:
	rm -f mkfs shell test_phase1 test_phase2 test_phase3 test_phase4 test_phase5 $(DISK_OBJ) $(BLOCK_OBJ) $(PHASE2_OBJ) $(PHASE3_OBJ) $(PHASE4_OBJ) $(PHASE5_OBJ)
	rm -f fs.img test_fs.img test_phase2.img test_phase3.img test_phase4.img test_phase5.img
	rm -f src/*.o

.PHONY: all test clean
