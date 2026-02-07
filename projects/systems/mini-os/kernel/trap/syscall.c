#include "types.h"
#include "defs.h"
#include "trap/trap.h"
#include "proc/proc.h"

// System call numbers
#define SYS_fork    1
#define SYS_exit    2
#define SYS_wait    3
#define SYS_pipe    4
#define SYS_read    5
#define SYS_kill    6
#define SYS_exec    7
#define SYS_fstat   8
#define SYS_chdir   9
#define SYS_dup    10
#define SYS_getpid 11
#define SYS_sbrk   12
#define SYS_sleep  13
#define SYS_uptime 14
#define SYS_open   15
#define SYS_write  16
#define SYS_mknod  17
#define SYS_unlink 18
#define SYS_link   19
#define SYS_mkdir  20
#define SYS_close  21

// System call implementations
int sys_fork(void) {
    return fork();
}

int sys_exit(void) {
    exit();
    return 0;  // not reached
}

int sys_wait(void) {
    return wait();
}

int sys_kill(void) {
    int pid;

    if (argint(0, &pid) < 0) {
        return -1;
    }
    return kill(pid);
}

int sys_getpid(void) {
    return myproc()->pid;
}

int sys_sbrk(void) {
    int addr;
    int n;

    if (argint(0, &n) < 0) {
        return -1;
    }
    addr = myproc()->sz;
    if (growproc(n) < 0) {
        return -1;
    }
    return addr;
}

int sys_sleep(void) {
    int n;
    uint ticks0;

    if (argint(0, &n) < 0) {
        return -1;
    }
    // Simplified: real implementation would use timer
    return 0;
}

// Fetch the int at addr from the current process
int fetchint(uint addr, int *ip) {
    struct proc *curproc = myproc();

    if (addr >= curproc->sz || addr + 4 > curproc->sz) {
        return -1;
    }
    *ip = *(int*)(addr);
    return 0;
}

// Fetch the nul-terminated string at addr from the current process
// Returns length of string, not including nul
int fetchstr(uint addr, char **pp) {
    char *s, *ep;
    struct proc *curproc = myproc();

    if (addr >= curproc->sz) {
        return -1;
    }
    *pp = (char*)addr;
    ep = (char*)curproc->sz;
    for (s = *pp; s < ep; s++) {
        if (*s == 0) {
            return s - *pp;
        }
    }
    return -1;
}

// Fetch the nth 32-bit system call argument
int argint(int n, int *ip) {
    return fetchint((myproc()->tf->esp) + 4 + 4*n, ip);
}

// Fetch the nth word-sized system call argument as a pointer
// to a block of memory of size bytes. Check that the pointer
// lies within the process address space
int argptr(int n, char **pp, int size) {
    int i;
    struct proc *curproc = myproc();

    if (argint(n, &i) < 0) {
        return -1;
    }
    if (size < 0 || (uint)i >= curproc->sz || (uint)i+size > curproc->sz) {
        return -1;
    }
    *pp = (char*)i;
    return 0;
}

// Fetch the nth word-sized system call argument as a string pointer
// Check that the pointer is valid and the string is nul-terminated
int argstr(int n, char **pp) {
    int addr;
    if (argint(n, &addr) < 0) {
        return -1;
    }
    return fetchstr(addr, pp);
}

// System call dispatcher
void syscall(void) {
    int num;
    struct proc *curproc = myproc();

    num = curproc->tf->eax;
    if (num > 0 && num < NELEM(syscalls) && syscalls[num]) {
        curproc->tf->eax = syscalls[num]();
    } else {
        printf("%d %s: unknown sys call %d\n",
               curproc->pid, curproc->name, num);
        curproc->tf->eax = -1;
    }
}

// System call table
static int (*syscalls[])(void) = {
[SYS_fork]    sys_fork,
[SYS_exit]    sys_exit,
[SYS_wait]    sys_wait,
[SYS_kill]    sys_kill,
[SYS_getpid]  sys_getpid,
[SYS_sbrk]    sys_sbrk,
[SYS_sleep]   sys_sleep,
};
