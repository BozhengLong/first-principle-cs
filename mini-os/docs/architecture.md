# mini-os Architecture

## 1. System Overview

mini-os is a monolithic kernel with clear component boundaries. This document describes the architecture of each major component and their interactions.

## 2. Component Architecture

### 2.1 Boot Loader (`kernel/boot/`)

**Purpose**: Load kernel into memory and transfer control

**Files**:
- `boot.S`: Assembly boot code

**Flow**:
1. BIOS loads first sector (512 bytes) into memory at 0x7C00
2. Boot code switches CPU to protected mode (32-bit)
3. Boot code loads kernel from disk
4. Boot code jumps to kernel entry point

**Interface**:
- Entry: BIOS calls boot sector at 0x7C00
- Exit: Jumps to kernel `main()` function

### 2.2 Process Management (`kernel/proc/`)

**Purpose**: Manage process lifecycle and scheduling

**Files**:
- `proc.h`: Process structures and constants
- `proc.c`: Process creation, termination, management
- `sched.c`: Scheduler implementation

**Key Data Structures**:
```c
struct proc {
    int pid;                    // Process ID
    enum proc_state state;      // UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE
    struct context *context;    // Saved registers
    void *kstack;              // Kernel stack
    struct pagetable *pagetable; // Page table
    struct proc *parent;        // Parent process
    int killed;                // Killed flag
    void *chan;                // Sleep channel
};

struct context {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebx;
    uint32_t ebp;
    uint32_t eip;
};
```

**Key Functions**:
- `allocproc()`: Allocate new process structure
- `freeproc()`: Free process resources
- `fork()`: Create child process
- `exit()`: Terminate process
- `wait()`: Wait for child process
- `schedule()`: Select next process to run
- `yield()`: Voluntarily give up CPU
- `sleep()`: Sleep on channel
- `wakeup()`: Wake up processes on channel

**Scheduling Algorithm**:
- Round-robin with time slicing
- Each process gets equal time quantum (10ms)
- Simple FIFO ready queue

**Invariants**:
- At most one process in RUNNING state per CPU
- All RUNNABLE processes eventually scheduled
- Parent-child relationships maintained

### 2.3 Memory Management (`kernel/mm/`)

**Purpose**: Manage virtual and physical memory

**Files**:
- `vm.h`: Virtual memory structures and constants
- `vm.c`: Page table management, address translation
- `kalloc.c`: Physical memory allocator

**Key Data Structures**:
```c
// Page directory entry
typedef uint32_t pde_t;

// Page table entry
typedef uint32_t pte_t;

// Page table
struct pagetable {
    pde_t *pgdir;  // Page directory
};
```

**Memory Layout**:
```
Virtual Address Space (per process):
0xFFFFFFFF +------------------+
           |  Kernel Space    | (shared, protected)
0xC0000000 +------------------+
           |  User Stack      |
           |        ↓         |
           |                  |
           |        ↑         |
           |  User Heap       |
           |  User Data       |
           |  User Code       |
0x00000000 +------------------+
```

**Key Functions**:
- `kalloc()`: Allocate physical page
- `kfree()`: Free physical page
- `mappages()`: Map virtual pages to physical pages
- `walkpgdir()`: Walk page table to find PTE
- `copyuvm()`: Copy page table (for fork)
- `freevm()`: Free page table and pages

**Physical Memory Allocator**:
- Free list of 4KB pages
- Simple first-fit allocation
- O(1) allocation and deallocation

**Invariants**:
- User processes cannot access kernel memory
- Processes cannot access each other's memory
- All virtual addresses correctly mapped

### 2.4 Interrupt and Trap Handling (`kernel/trap/`)

**Purpose**: Handle interrupts, exceptions, and system calls

**Files**:
- `trap.h`: Trap numbers and structures
- `trap.c`: Interrupt/exception handlers
- `syscall.c`: System call dispatcher

**Interrupt Descriptor Table (IDT)**:
```
Vector  Type        Handler
------  ----------  -------
0-31    Exceptions  Exception handlers
32      Timer       Timer interrupt handler
128     Syscall     System call handler
```

**Key Data Structures**:
```c
struct trapframe {
    // Registers pushed by hardware
    uint32_t edi, esi, ebp, esp;
    uint32_t ebx, edx, ecx, eax;
    uint16_t gs, padding1;
    uint16_t fs, padding2;
    uint16_t es, padding3;
    uint16_t ds, padding4;
    uint32_t trapno;

    // Pushed by hardware
    uint32_t err;
    uint32_t eip;
    uint16_t cs, padding5;
    uint32_t eflags;

    // Only when crossing rings
    uint32_t esp_user;
    uint16_t ss, padding6;
};
```

**Key Functions**:
- `trap()`: Main trap handler
- `syscall()`: System call dispatcher
- `timer_interrupt()`: Timer interrupt handler

**System Call Mechanism**:
1. User executes INT 0x80
2. Hardware saves state, switches to kernel mode
3. `trap()` called with trapframe
4. `syscall()` dispatches based on EAX
5. System call handler executes
6. Return value in EAX
7. `trap()` returns, hardware restores state

**Invariants**:
- All interrupts properly handled
- User arguments validated before use
- Kernel state consistent after trap

### 2.5 Kernel Library (`kernel/lib/`)

**Purpose**: Provide utility functions for kernel

**Files**:
- `string.c`: String manipulation (memcpy, memset, strlen, etc.)
- `printf.c`: Kernel printf for debugging

**Key Functions**:
- `memcpy()`, `memset()`, `memmove()`
- `strlen()`, `strcmp()`, `strncpy()`
- `printf()`: Formatted output to console

### 2.6 User Library (`user/lib/`)

**Purpose**: Provide C library for user programs

**Files**:
- `ulib.c`: User-space library functions
- `usys.S`: System call wrappers

**Key Functions**:
- System call wrappers: `fork()`, `exit()`, `wait()`, etc.
- Library functions: `strlen()`, `strcmp()`, `printf()`, etc.

**System Call Wrapper Example**:
```asm
.globl fork
fork:
    movl $SYS_fork, %eax
    int $0x80
    ret
```

### 2.7 User Programs (`user/programs/`)

**Purpose**: Example user-space programs

**Files**:
- `init.c`: First user process
- `sh.c`: Simple shell
- `echo.c`: Echo program
- `fork_test.c`: Fork test program

## 3. Execution Flow

### 3.1 Boot Sequence

```
BIOS → Bootloader → Kernel main() → Hardware init →
Process init → Create init process → Start scheduler →
Run init → Init spawns shell → User interaction
```

### 3.2 System Call Flow

```
User program → INT 0x80 → trap() → syscall() →
System call handler → Return value → trap() returns →
User program continues
```

### 3.3 Context Switch Flow

```
Timer interrupt → trap() → yield() → schedule() →
Save current context → Select next process →
Load next context → Return from trap → New process runs
```

### 3.4 Fork Flow

```
User calls fork() → INT 0x80 → sys_fork() →
Allocate child process → Copy page table →
Copy kernel stack → Set return values →
Add to scheduler → Return to user →
Parent gets child PID, child gets 0
```

## 4. Concurrency and Synchronization

### 4.1 Locking Strategy

- **Process table lock**: Protects process table
- **Memory allocator lock**: Protects free list
- **Per-process lock**: Protects process state

### 4.2 Sleep/Wakeup

- Processes sleep on channels (arbitrary addresses)
- Wakeup wakes all processes on channel
- Used for wait(), pipe(), etc.

## 5. Error Handling

### 5.1 Kernel Panics

- Unrecoverable errors cause kernel panic
- Print error message and halt
- Examples: corrupted data structures, impossible states

### 5.2 Process Errors

- Recoverable errors terminate process
- Examples: illegal instruction, invalid memory access
- Kernel remains stable

### 5.3 System Call Errors

- Invalid arguments return -1
- Examples: bad pointer, invalid file descriptor
- Process continues execution

## 6. Performance Considerations

### 6.1 Context Switch Overhead

- Minimize saved state (only callee-saved registers)
- Fast path for common case
- Typical overhead: ~1-2 microseconds

### 6.2 System Call Overhead

- Direct dispatch (no table lookup)
- Minimal argument copying
- Typical overhead: ~0.5-1 microseconds

### 6.3 Memory Allocation

- Simple free list (O(1) allocation)
- No coalescing (trade-off: simplicity vs. fragmentation)
- Typical allocation: ~100 nanoseconds

## 7. Testing Architecture

### 7.1 Unit Tests

- Test individual functions in isolation
- Mock dependencies (e.g., hardware)
- Run in user space when possible

### 7.2 Integration Tests

- Test complete scenarios
- Run in QEMU
- Verify end-to-end behavior

### 7.3 Fault Injection

- Simulate failures (memory exhaustion, invalid input)
- Verify error handling
- Ensure kernel stability

## 8. Debugging

### 8.1 QEMU + GDB

- Run kernel in QEMU with GDB stub
- Set breakpoints, inspect memory
- Step through code

### 8.2 Kernel Printf

- Print debug messages to console
- Useful for tracing execution
- Can be disabled in production

### 8.3 Assertions

- Check invariants at runtime
- Panic if violated
- Helps catch bugs early

## 9. Future Extensions

### 9.1 Copy-on-Write Fork

- Share pages between parent and child
- Copy only when written
- Reduces fork overhead

### 9.2 Demand Paging

- Allocate pages on first access
- Reduces memory usage
- Enables larger address spaces

### 9.3 Multi-Core Support

- Per-CPU scheduler
- Spinlocks for synchronization
- Load balancing

### 9.4 More System Calls

- File operations (open, close, read, write)
- Pipes (pipe, dup)
- Signals (kill, signal)

