# mini-os Design Document

## 1. Introduction

### 1.1 Purpose
mini-os is a minimal operating system kernel designed for learning OS fundamentals. It implements core abstractions: processes, virtual memory, system calls, and interrupt handling.

### 1.2 Goals
- **Educational**: Clear, well-documented code for learning
- **Minimal**: Only essential features, no unnecessary complexity
- **Correct**: Strong invariants and comprehensive testing
- **Verifiable**: Testable components with clear specifications

### 1.3 Non-Goals
- Production use
- Full POSIX compatibility
- Multi-core support (initial version)
- Complete device driver support
- Network stack
- File system (separate project: simple-fs)

## 2. System Architecture

### 2.1 Overview
mini-os follows a monolithic kernel architecture where all kernel services run in kernel space with full hardware access.

```
+------------------+
|  User Programs   |  (Ring 3)
+------------------+
|  System Calls    |  (Interface)
+------------------+
|  Kernel Services |  (Ring 0)
| - Scheduler      |
| - Memory Manager |
| - Interrupt      |
+------------------+
|    Hardware      |
+------------------+
```

### 2.2 Key Components
1. **Bootloader**: Loads kernel into memory
2. **Process Scheduler**: Manages process execution
3. **Memory Manager**: Handles virtual memory and page tables
4. **System Call Handler**: Provides user-kernel interface
5. **Interrupt Handler**: Processes hardware/software interrupts

## 3. Design Principles

### 3.1 Interfaces and Invariants

Each component has clear interfaces and maintains critical invariants:

**Process Scheduler**
- Interface: `schedule()`, `yield()`, `sleep()`, `wakeup()`
- Invariants:
  - All ready processes eventually get scheduled (fairness)
  - No deadlocks in scheduler
  - Context switches are atomic

**Memory Manager**
- Interface: `kalloc()`, `kfree()`, `mappages()`, `walkpgdir()`
- Invariants:
  - Process isolation: processes cannot access each other's memory
  - Memory protection: user mode cannot access kernel memory
  - Page table consistency: virtual addresses correctly map to physical addresses

**System Call Handler**
- Interface: System call numbers and argument conventions
- Invariants:
  - All user arguments are validated
  - System calls are atomic
  - Privilege separation maintained

### 3.2 Failure Model

The system handles failures gracefully:

1. **Process Crashes**: Illegal instructions, invalid memory access
   - Action: Terminate process, clean up resources
   - Invariant: Kernel remains stable

2. **Memory Exhaustion**: Physical memory allocation fails
   - Action: Return error to caller
   - Invariant: No kernel panic

3. **Invalid System Call Arguments**: Bad pointers, invalid file descriptors
   - Action: Return error code (-1)
   - Invariant: Kernel validates all user input

4. **Scheduler Deadlock**: All processes waiting
   - Prevention: Always have idle process
   - Invariant: System never hangs

### 3.3 Verification Strategy

- **Unit Tests**: Test individual functions and components
- **Integration Tests**: Test end-to-end scenarios
- **Fault Injection**: Test error handling paths
- **Benchmarks**: Measure performance characteristics

## 4. Component Design

### 4.1 Process Management

**Data Structures**:
```c
enum proc_state { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

struct context {
    uint32_t edi, esi, ebx, ebp, eip;
};

struct proc {
    int pid;
    enum proc_state state;
    struct context *context;
    void *kstack;
    struct pagetable *pagetable;
    struct proc *parent;
};
```

**Scheduling Algorithm**: Time-slice round-robin
- Each process gets equal time slice (e.g., 10ms)
- Simple, fair, easy to understand
- Trade-off: Not optimal for I/O-bound processes

### 4.2 Memory Management

**Virtual Memory Layout**:
```
0xFFFFFFFF +------------------+
           |  Kernel Space    |
0xC0000000 +------------------+
           |  User Space      |
0x00000000 +------------------+
```

**Page Table Structure**: Two-level page tables (x86 standard)
- Page Directory: 1024 entries
- Page Tables: 1024 entries each
- Page Size: 4KB

**Physical Memory Allocator**: Free list allocator
- Simple linked list of free pages
- O(1) allocation and deallocation
- Trade-off: No coalescing, external fragmentation

### 4.3 System Calls

**Mechanism**: INT 0x80 software interrupt
- User sets system call number in EAX
- Arguments in EBX, ECX, EDX, etc.
- Return value in EAX

**System Call Table**:
```c
#define SYS_fork    1
#define SYS_exit    2
#define SYS_wait    3
#define SYS_exec    4
#define SYS_read    5
#define SYS_write   6
#define SYS_getpid  7
#define SYS_sbrk    8
```

### 4.4 Interrupt Handling

**Interrupt Descriptor Table (IDT)**: 256 entries
- 0-31: CPU exceptions (divide by zero, page fault, etc.)
- 32-47: Hardware interrupts (timer, keyboard, etc.)
- 128 (0x80): System call interrupt

**Timer Interrupt**: Drives scheduling
- Frequency: 100 Hz (10ms intervals)
- Handler: Increments tick counter, calls scheduler

## 5. Implementation Details

### 5.1 Boot Process

1. BIOS loads bootloader from disk
2. Bootloader loads kernel into memory
3. Bootloader switches to protected mode (32-bit)
4. Bootloader jumps to kernel entry point
5. Kernel initializes hardware and data structures
6. Kernel creates first process (init)
7. Kernel starts scheduler

### 5.2 Context Switch

1. Timer interrupt fires
2. Save current process context (registers)
3. Scheduler selects next process
4. Load next process context
5. Return from interrupt to new process

### 5.3 System Call Flow

1. User program executes INT 0x80
2. CPU switches to kernel mode
3. Interrupt handler saves user context
4. System call dispatcher validates arguments
5. System call handler executes
6. Return value placed in EAX
7. Return to user mode

### 5.4 Fork Implementation

1. Allocate new process structure
2. Copy parent's page table (copy-on-write optional)
3. Copy parent's kernel stack
4. Set child's return value to 0
5. Set parent's return value to child PID
6. Add child to scheduler queue

## 6. Testing Strategy

### 6.1 Unit Tests
- Test each component in isolation
- Mock dependencies
- Verify invariants

### 6.2 Integration Tests
- Test end-to-end scenarios
- Multiple processes, system calls
- Verify system behavior

### 6.3 Fault Injection
- Simulate failures (memory exhaustion, invalid arguments)
- Verify error handling
- Ensure kernel stability

### 6.4 Benchmarks
- Measure context switch overhead
- Measure system call latency
- Measure memory allocation performance

## 7. Future Enhancements

- Copy-on-write fork
- Demand paging
- Priority scheduling
- Multi-core support
- More system calls (pipe, dup, etc.)

## 8. References

- xv6: A simple, Unix-like teaching operating system
- MIT 6.828: Operating System Engineering
- "Operating Systems: Three Easy Pieces" by Remzi & Andrea

