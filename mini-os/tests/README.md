# Test Suite for mini-os

This directory contains the test suite for mini-os.

## Test Categories

### Unit Tests (`unit/`)
Test individual components in isolation:
- Scheduler tests
- Memory manager tests
- System call handler tests
- Context switcher tests
- Interrupt handler tests

### Integration Tests (`integration/`)
Test end-to-end functionality:
- Multi-process execution
- Process isolation
- System call flow
- Fork and exec
- Wait and exit

### Fault Injection Tests (`fault_injection/`)
Test error handling:
- Process crashes (illegal instruction, invalid memory access)
- Memory exhaustion
- Invalid system call arguments
- Scheduler deadlock scenarios

### Benchmark Tests (`benchmark/`)
Measure performance:
- Context switch overhead
- System call latency
- Memory allocation performance
- Process creation overhead

## Running Tests

```bash
# Run all tests
make test

# Run specific test category
make test-unit
make test-integration
make test-fault
make test-benchmark
```

## Test Framework

Tests are written in C and run in QEMU. Each test:
1. Sets up test environment
2. Executes test case
3. Verifies expected behavior
4. Reports pass/fail

## Adding New Tests

1. Create test file in appropriate directory
2. Implement test function
3. Add to test runner
4. Update Makefile

## Test Coverage

Target: 80%+ code coverage for critical components
- Process management: 90%+
- Memory management: 85%+
- System calls: 95%+
- Interrupt handling: 80%+
