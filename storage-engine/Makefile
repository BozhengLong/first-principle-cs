CC = gcc
CFLAGS = -Wall -Wextra -g -O0 -Isrc
LDFLAGS = -lpthread

# Phase 1 source files
SKIPLIST_SRC = src/skiplist.c
MEMTABLE_SRC = src/memtable.c
STORAGE_SRC = src/storage.c

# Phase 2 source files
WAL_SRC = src/wal.c
CRC32_SRC = src/crc32.c

# Phase 3 source files
SSTABLE_SRC = src/sstable.c
BLOOM_SRC = src/bloom.c

# Phase 4 source files
LEVEL_SRC = src/level.c
COMPACT_SRC = src/compact.c
MANIFEST_SRC = src/manifest.c

# Phase 5 source files
CACHE_SRC = src/cache.c
BENCH_SRC = src/bench.c

# Object files
SKIPLIST_OBJ = $(SKIPLIST_SRC:.c=.o)
MEMTABLE_OBJ = $(MEMTABLE_SRC:.c=.o)
STORAGE_OBJ = $(STORAGE_SRC:.c=.o)

WAL_OBJ = $(WAL_SRC:.c=.o)
CRC32_OBJ = $(CRC32_SRC:.c=.o)

SSTABLE_OBJ = $(SSTABLE_SRC:.c=.o)
BLOOM_OBJ = $(BLOOM_SRC:.c=.o)

LEVEL_OBJ = $(LEVEL_SRC:.c=.o)
COMPACT_OBJ = $(COMPACT_SRC:.c=.o)
MANIFEST_OBJ = $(MANIFEST_SRC:.c=.o)

CACHE_OBJ = $(CACHE_SRC:.c=.o)
BENCH_OBJ = $(BENCH_SRC:.c=.o)

PHASE1_OBJ = $(SKIPLIST_OBJ) $(MEMTABLE_OBJ) $(STORAGE_OBJ)
PHASE2_OBJ = $(WAL_OBJ) $(CRC32_OBJ)
PHASE3_OBJ = $(SSTABLE_OBJ) $(BLOOM_OBJ)
PHASE4_OBJ = $(LEVEL_OBJ) $(COMPACT_OBJ) $(MANIFEST_OBJ)
PHASE5_OBJ = $(CACHE_OBJ)

# Targets
all: storage-bench

storage-bench: $(BENCH_SRC) $(PHASE1_OBJ) $(PHASE2_OBJ) $(PHASE3_OBJ) $(PHASE4_OBJ) $(PHASE5_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Unit tests - Phase 1-3 now need Phase 4 objects due to level manager integration
test_phase1: tests/unit/test_phase1.c $(PHASE1_OBJ) $(PHASE2_OBJ) $(PHASE3_OBJ) $(PHASE4_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_phase2: tests/unit/test_phase2.c $(PHASE1_OBJ) $(PHASE2_OBJ) $(PHASE3_OBJ) $(PHASE4_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_phase3: tests/unit/test_phase3.c $(PHASE1_OBJ) $(PHASE2_OBJ) $(PHASE3_OBJ) $(PHASE4_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_phase4: tests/unit/test_phase4.c $(PHASE1_OBJ) $(PHASE2_OBJ) $(PHASE3_OBJ) $(PHASE4_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_phase5: tests/unit/test_phase5.c $(PHASE1_OBJ) $(PHASE2_OBJ) $(PHASE3_OBJ) $(PHASE4_OBJ) $(PHASE5_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

test: test_phase1 test_phase2 test_phase3 test_phase4 test_phase5
	@echo "Running Phase 1 tests..."
	./test_phase1
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
	rm -f storage-bench test_phase1 test_phase2 test_phase3 test_phase4 test_phase5
	rm -f src/*.o
	rm -rf *.dSYM
	rm -f *.db *.wal *.sst test_*.img
	rm -rf test_phase*_db

.PHONY: all test clean
