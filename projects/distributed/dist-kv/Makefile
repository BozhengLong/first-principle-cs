# Makefile for dist-kv (Distributed Key-Value Store)

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g -O2
LDFLAGS = -lm

SRC_DIR = src
TEST_DIR = tests
BUILD_DIR = build

# Source files
SRCS = $(SRC_DIR)/hash_ring.c \
       $(SRC_DIR)/node.c \
       $(SRC_DIR)/partition.c \
       $(SRC_DIR)/coordinator.c \
       $(SRC_DIR)/raft_group.c \
       $(SRC_DIR)/storage_adapter.c \
       $(SRC_DIR)/replication.c \
       $(SRC_DIR)/gossip.c \
       $(SRC_DIR)/failover.c \
       $(SRC_DIR)/client.c \
       $(SRC_DIR)/admin.c \
       $(SRC_DIR)/iterator.c
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Integration test sources
NETWORK_SIM = $(TEST_DIR)/integration/network_sim.c
TEST_INTEGRATION = $(TEST_DIR)/integration/test_integration.c

# Test files
TEST_PHASE1 = $(TEST_DIR)/unit/test_phase1.c
TEST_PHASE2 = $(TEST_DIR)/unit/test_phase2.c
TEST_PHASE3 = $(TEST_DIR)/unit/test_phase3.c
TEST_PHASE4 = $(TEST_DIR)/unit/test_phase4.c
TEST_PHASE5 = $(TEST_DIR)/unit/test_phase5.c

.PHONY: all clean test test_phase1 test_phase2 test_phase3 test_phase4 test_phase5 test_phase6

all: $(BUILD_DIR) $(OBJS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Phase 1 tests
test_phase1: $(BUILD_DIR) $(OBJS)
	$(CC) $(CFLAGS) $(TEST_PHASE1) $(OBJS) -o $(BUILD_DIR)/test_phase1 $(LDFLAGS)
	./$(BUILD_DIR)/test_phase1

# Phase 2 tests
test_phase2: $(BUILD_DIR) $(OBJS)
	$(CC) $(CFLAGS) $(TEST_PHASE2) $(OBJS) -o $(BUILD_DIR)/test_phase2 $(LDFLAGS)
	./$(BUILD_DIR)/test_phase2

# Phase 3 tests
test_phase3: $(BUILD_DIR) $(OBJS)
	$(CC) $(CFLAGS) $(TEST_PHASE3) $(OBJS) -o $(BUILD_DIR)/test_phase3 $(LDFLAGS)
	./$(BUILD_DIR)/test_phase3

# Phase 4 tests
test_phase4: $(BUILD_DIR) $(OBJS)
	$(CC) $(CFLAGS) $(TEST_PHASE4) $(NETWORK_SIM) $(OBJS) -o $(BUILD_DIR)/test_phase4 $(LDFLAGS)
	./$(BUILD_DIR)/test_phase4

# Phase 5 tests
test_phase5: $(BUILD_DIR) $(OBJS)
	$(CC) $(CFLAGS) $(TEST_PHASE5) $(OBJS) -o $(BUILD_DIR)/test_phase5 $(LDFLAGS)
	./$(BUILD_DIR)/test_phase5

# Phase 6 tests (integration)
test_phase6: $(BUILD_DIR) $(OBJS)
	$(CC) $(CFLAGS) $(TEST_INTEGRATION) $(NETWORK_SIM) $(OBJS) -o $(BUILD_DIR)/test_phase6 $(LDFLAGS)
	./$(BUILD_DIR)/test_phase6

# Run all tests
test: test_phase1 test_phase2 test_phase3 test_phase4 test_phase5 test_phase6

clean:
	rm -rf $(BUILD_DIR)
