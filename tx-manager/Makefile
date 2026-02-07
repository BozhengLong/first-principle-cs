# tx-manager Makefile

CC = gcc
CFLAGS = -Wall -Wextra -g -O0 -Isrc -I../storage-engine/src
LDFLAGS = -lpthread

# Storage engine object files
STORAGE_ENGINE_PATH = ../storage-engine
STORAGE_OBJS = $(STORAGE_ENGINE_PATH)/src/skiplist.o \
               $(STORAGE_ENGINE_PATH)/src/memtable.o \
               $(STORAGE_ENGINE_PATH)/src/storage.o \
               $(STORAGE_ENGINE_PATH)/src/wal.o \
               $(STORAGE_ENGINE_PATH)/src/crc32.o \
               $(STORAGE_ENGINE_PATH)/src/sstable.o \
               $(STORAGE_ENGINE_PATH)/src/bloom.o \
               $(STORAGE_ENGINE_PATH)/src/level.o \
               $(STORAGE_ENGINE_PATH)/src/compact.o \
               $(STORAGE_ENGINE_PATH)/src/manifest.o \
               $(STORAGE_ENGINE_PATH)/src/cache.o

# Phase 1 sources (includes conflict.c and tx_wal.c since tx_manager depends on them)
PHASE1_SRCS = src/version.c src/tx.c src/tx_manager.c src/conflict.c src/tx_wal.c
PHASE1_OBJS = $(PHASE1_SRCS:.c=.o)

# Phase 2 sources (adds visibility)
PHASE2_SRCS = $(PHASE1_SRCS) src/visibility.c
PHASE2_OBJS = $(PHASE2_SRCS:.c=.o)

# Phase 3 sources (adds write_set)
PHASE3_SRCS = $(PHASE2_SRCS) src/write_set.c
PHASE3_OBJS = $(PHASE3_SRCS:.c=.o)

# Phase 4 sources (adds recovery)
PHASE4_SRCS = $(PHASE3_SRCS) src/recovery.c
PHASE4_OBJS = $(PHASE4_SRCS:.c=.o)

# Phase 5 sources (adds GC and iterator)
PHASE5_SRCS = $(PHASE4_SRCS) src/gc.c src/tx_iter.c
PHASE5_OBJS = $(PHASE5_SRCS:.c=.o)

# All sources
ALL_SRCS = $(PHASE5_SRCS)
ALL_OBJS = $(ALL_SRCS:.c=.o)

.PHONY: all clean test storage_objs

all: test_phase1 test_phase2 test_phase3 test_phase4 test_phase5

# Build storage engine objects if needed
storage_objs:
	$(MAKE) -C $(STORAGE_ENGINE_PATH) src/skiplist.o src/memtable.o \
		src/storage.o src/wal.o src/crc32.o src/sstable.o src/bloom.o \
		src/level.o src/compact.o src/manifest.o src/cache.o

# Compile tx-manager objects
src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Phase 1 test
test_phase1: storage_objs $(PHASE1_OBJS) tests/unit/test_phase1.c
	$(CC) $(CFLAGS) -o $@ tests/unit/test_phase1.c $(PHASE1_OBJS) \
		$(STORAGE_OBJS) $(LDFLAGS)

# Phase 2 test
test_phase2: storage_objs $(PHASE2_OBJS) tests/unit/test_phase2.c
	$(CC) $(CFLAGS) -o $@ tests/unit/test_phase2.c $(PHASE2_OBJS) \
		$(STORAGE_OBJS) $(LDFLAGS)

# Phase 3 test
test_phase3: storage_objs $(PHASE3_OBJS) tests/unit/test_phase3.c
	$(CC) $(CFLAGS) -o $@ tests/unit/test_phase3.c $(PHASE3_OBJS) \
		$(STORAGE_OBJS) $(LDFLAGS)

# Phase 4 test
test_phase4: storage_objs $(PHASE4_OBJS) tests/unit/test_phase4.c
	$(CC) $(CFLAGS) -o $@ tests/unit/test_phase4.c $(PHASE4_OBJS) \
		$(STORAGE_OBJS) $(LDFLAGS)

# Phase 5 test
test_phase5: storage_objs $(PHASE5_OBJS) tests/unit/test_phase5.c
	$(CC) $(CFLAGS) -o $@ tests/unit/test_phase5.c $(PHASE5_OBJS) \
		$(STORAGE_OBJS) $(LDFLAGS)

test: test_phase1 test_phase2 test_phase3 test_phase4 test_phase5
	./test_phase1
	./test_phase2
	./test_phase3
	./test_phase4
	./test_phase5

clean:
	rm -f src/*.o test_phase1 test_phase2 test_phase3 test_phase4 test_phase5 tx-bench
	rm -rf /tmp/tx_test_*
