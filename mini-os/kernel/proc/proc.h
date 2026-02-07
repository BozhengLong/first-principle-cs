#ifndef PROC_H
#define PROC_H

#include "types.h"

// Process states
enum proc_state {
    UNUSED,
    EMBRYO,
    SLEEPING,
    RUNNABLE,
    RUNNING,
    ZOMBIE
};

// Saved registers for context switches
struct context {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebx;
    uint32_t ebp;
    uint32_t eip;
};

// Per-CPU state
struct cpu {
  uint8_t apicid;              // Local APIC ID
  struct context *scheduler;   // swtch() here to enter scheduler
  volatile uint started;       // Has the CPU started?
  int ncli;                    // Depth of pushcli nesting
  int intena;                  // Were interrupts enabled before pushcli?
  struct proc *proc;           // The process running on this cpu or null
};

// Per-process state
struct proc {
    uint sz;                     // Size of process memory (bytes)
    pde_t* pgdir;               // Page table
    char *kstack;               // Bottom of kernel stack for this process
    enum proc_state state;      // Process state
    int pid;                    // Process ID
    struct proc *parent;        // Parent process
    struct trapframe *tf;       // Trap frame for current syscall
    struct context *context;    // swtch() here to run process
    void *chan;                 // If non-zero, sleeping on chan
    int killed;                 // If non-zero, have been killed
    char name[16];              // Process name (debugging)
};

// Process memory is laid out contiguously, low addresses first:
//   text
//   original data and bss
//   fixed-size stack
//   expandable heap

#define NPROC        64  // Maximum number of processes
#define KSTACKSIZE 4096  // Size of per-process kernel stack

// Spinlock for process table
struct spinlock {
    uint locked;       // Is the lock held?
    char *name;        // Name of lock
    int cpu;           // CPU holding lock
};

#endif // PROC_H
