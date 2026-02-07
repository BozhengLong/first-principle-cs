#include "types.h"
#include "defs.h"
#include "proc/proc.h"

// CPU state
struct cpu cpus[1];
int ncpu = 1;

// Kernel main function
void main(void) {
    // Initialize console
    printf("mini-os: starting\n");

    // Initialize memory allocator
    printf("mini-os: initializing memory\n");
    kinit();

    // Initialize virtual memory
    printf("mini-os: initializing virtual memory\n");
    kvminit();
    seginit();

    // Initialize interrupt descriptor table
    printf("mini-os: initializing interrupts\n");
    idtinit();

    // Initialize process table
    printf("mini-os: initializing processes\n");
    procinit();

    // Create first user process
    printf("mini-os: creating init process\n");
    userinit();

    // Start scheduler (never returns)
    printf("mini-os: starting scheduler\n");
    scheduler();
}
