# System Call Specification

## 1. Overview

This document specifies the system call interface for mini-os. System calls provide the interface between user programs and the kernel.

## 2. System Call Mechanism

### 2.1 Invocation

System calls are invoked using the INT 0x80 software interrupt:

```c
// User space
int result = fork();

// Expands to (in usys.S):
movl $SYS_fork, %eax    // System call number in EAX
int $0x80                // Trigger interrupt
// Return value in EAX
```

### 2.2 Calling Convention

- **System call number**: EAX register
- **Arguments**: EBX, ECX, EDX, ESI, EDI (up to 5 arguments)
- **Return value**: EAX register
  - Success: Non-negative value
  - Error: -1

### 2.3 System Call Numbers

```c
#define SYS_fork    1
#define SYS_exit    2
#define SYS_wait    3
#define SYS_exec    4
#define SYS_read    5
#define SYS_write   6
#define SYS_getpid  7
#define SYS_sbrk    8
#define SYS_sleep   9
#define SYS_kill    10
```

## 3. Process Management System Calls

### 3.1 fork

**Description**: Create a child process

**Prototype**:
```c
int fork(void);
```

**Arguments**: None

**Return Value**:
- Parent: Child's PID (positive integer)
- Child: 0
- Error: -1

**Behavior**:
1. Allocate new process structure
2. Copy parent's address space
3. Copy parent's file descriptors
4. Set child's parent pointer
5. Add child to scheduler queue

**Errors**:
- Out of memory
- Process table full

**Example**:
```c
int pid = fork();
if (pid == 0) {
    // Child process
    printf("I am the child\n");
} else if (pid > 0) {
    // Parent process
    printf("Child PID: %d\n", pid);
} else {
    // Error
    printf("Fork failed\n");
}
```

### 3.2 exit

**Description**: Terminate the calling process

**Prototype**:
```c
void exit(int status);
```

**Arguments**:
- `status`: Exit status (0 = success, non-zero = error)

**Return Value**: Does not return

**Behavior**:
1. Close all file descriptors
2. Free process memory
3. Set state to ZOMBIE
4. Wake up parent (if waiting)
5. Reparent children to init
6. Yield CPU

**Example**:
```c
if (error) {
    exit(1);
}
exit(0);
```

### 3.3 wait

**Description**: Wait for a child process to exit

**Prototype**:
```c
int wait(int *status);
```

**Arguments**:
- `status`: Pointer to store child's exit status (can be NULL)

**Return Value**:
- Success: PID of exited child
- Error: -1 (no children)

**Behavior**:
1. Check if any child is ZOMBIE
2. If yes, clean up child and return its PID
3. If no, sleep until a child exits
4. If no children, return -1

**Errors**:
- No children
- Invalid status pointer

**Example**:
```c
int status;
int pid = wait(&status);
if (pid > 0) {
    printf("Child %d exited with status %d\n", pid, status);
}
```

### 3.4 exec

**Description**: Execute a program

**Prototype**:
```c
int exec(char *path, char **argv);
```

**Arguments**:
- `path`: Path to executable
- `argv`: Argument array (NULL-terminated)

**Return Value**:
- Success: Does not return
- Error: -1

**Behavior**:
1. Load executable from path
2. Allocate new address space
3. Copy arguments to new stack
4. Free old address space
5. Set up new page table
6. Jump to entry point

**Errors**:
- File not found
- Invalid executable format
- Out of memory

**Example**:
```c
char *argv[] = { "echo", "hello", NULL };
if (exec("/echo", argv) < 0) {
    printf("Exec failed\n");
}
```

### 3.5 getpid

**Description**: Get process ID

**Prototype**:
```c
int getpid(void);
```

**Arguments**: None

**Return Value**: Current process ID

**Example**:
```c
int pid = getpid();
printf("My PID: %d\n", pid);
```

### 3.6 sleep

**Description**: Sleep for specified ticks

**Prototype**:
```c
int sleep(int ticks);
```

**Arguments**:
- `ticks`: Number of timer ticks to sleep

**Return Value**:
- Success: 0
- Error: -1

**Behavior**:
1. Calculate wakeup time
2. Sleep on timer channel
3. Wake up when time elapsed

**Example**:
```c
sleep(100);  // Sleep for 100 ticks (~1 second at 100 Hz)
```

### 3.7 kill

**Description**: Send signal to process

**Prototype**:
```c
int kill(int pid);
```

**Arguments**:
- `pid`: Process ID to kill

**Return Value**:
- Success: 0
- Error: -1

**Behavior**:
1. Find process by PID
2. Set killed flag
3. Wake up if sleeping
4. Process will exit at next opportunity

**Errors**:
- Invalid PID
- Permission denied

**Example**:
```c
if (kill(child_pid) < 0) {
    printf("Kill failed\n");
}
```

## 4. Memory Management System Calls

### 4.1 sbrk

**Description**: Grow process heap

**Prototype**:
```c
void* sbrk(int n);
```

**Arguments**:
- `n`: Number of bytes to grow (can be negative to shrink)

**Return Value**:
- Success: Previous heap end address
- Error: (void*)-1

**Behavior**:
1. Calculate new heap size
2. Allocate/free pages as needed
3. Update page table
4. Return old heap end

**Errors**:
- Out of memory
- Invalid size (too large or negative overflow)

**Example**:
```c
void *p = sbrk(4096);  // Allocate 4KB
if (p == (void*)-1) {
    printf("Out of memory\n");
}
```

## 5. I/O System Calls

### 5.1 read

**Description**: Read from file descriptor

**Prototype**:
```c
int read(int fd, void *buf, int n);
```

**Arguments**:
- `fd`: File descriptor
- `buf`: Buffer to read into
- `n`: Number of bytes to read

**Return Value**:
- Success: Number of bytes read
- EOF: 0
- Error: -1

**Behavior**:
1. Validate file descriptor
2. Validate buffer pointer
3. Read from file/device
4. Copy to user buffer
5. Return bytes read

**Errors**:
- Invalid file descriptor
- Invalid buffer pointer
- I/O error

**Example**:
```c
char buf[100];
int n = read(0, buf, sizeof(buf));  // Read from stdin
if (n > 0) {
    buf[n] = '\0';
    printf("Read: %s\n", buf);
}
```

### 5.2 write

**Description**: Write to file descriptor

**Prototype**:
```c
int write(int fd, void *buf, int n);
```

**Arguments**:
- `fd`: File descriptor
- `buf`: Buffer to write from
- `n`: Number of bytes to write

**Return Value**:
- Success: Number of bytes written
- Error: -1

**Behavior**:
1. Validate file descriptor
2. Validate buffer pointer
3. Copy from user buffer
4. Write to file/device
5. Return bytes written

**Errors**:
- Invalid file descriptor
- Invalid buffer pointer
- I/O error

**Example**:
```c
char *msg = "Hello, world!\n";
int n = write(1, msg, strlen(msg));  // Write to stdout
if (n < 0) {
    printf("Write failed\n");
}
```

## 6. Error Handling

### 6.1 Error Codes

All system calls return -1 on error. Specific error information is not provided in this minimal implementation.

### 6.2 Argument Validation

The kernel validates all user-provided arguments:
- Pointers must be in user address space
- File descriptors must be valid
- Sizes must be reasonable

Invalid arguments result in -1 return value.

### 6.3 Kernel Stability

System call errors never crash the kernel. The kernel always:
1. Validates arguments
2. Handles errors gracefully
3. Returns error code to user
4. Maintains consistent state

## 7. System Call Implementation

### 7.1 Kernel Side

```c
// System call handler
void syscall(void) {
    struct proc *p = myproc();
    int num = p->trapframe->eax;

    switch (num) {
    case SYS_fork:
        p->trapframe->eax = sys_fork();
        break;
    case SYS_exit:
        sys_exit();
        break;
    // ... other system calls
    default:
        p->trapframe->eax = -1;
        break;
    }
}
```

### 7.2 User Side

```asm
# System call wrapper (usys.S)
.globl fork
fork:
    movl $SYS_fork, %eax
    int $0x80
    ret
```

## 8. Performance

### 8.1 System Call Overhead

- Context switch: ~1-2 microseconds
- Argument validation: ~100 nanoseconds
- Total overhead: ~1-3 microseconds

### 8.2 Optimization

- Direct dispatch (no table lookup)
- Minimal argument copying
- Fast path for common cases

## 9. Future Extensions

### 9.1 Additional System Calls

- File operations: open, close, dup, pipe
- Directory operations: chdir, mkdir
- Process control: signal, alarm

### 9.2 Error Codes

- Detailed error codes (EINVAL, ENOMEM, etc.)
- errno variable

### 9.3 System Call Tracing

- strace-like functionality
- Performance profiling
