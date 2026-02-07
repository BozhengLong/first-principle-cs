#ifndef DEFS_H
#define DEFS_H

#include "types.h"

// Forward declarations
struct proc;
struct cpu;
struct spinlock;
struct context;
struct trapframe;
struct inode;
typedef uint pte_t;
typedef uint pde_t;

// string.c
void*           memset(void*, int, uint);
void*           memcpy(void*, const void*, uint);
void*           memmove(void*, const void*, uint);
int             memcmp(const void*, const void*, uint);
int             strlen(const char*);
int             strcmp(const char*, const char*);
int             strncmp(const char*, const char*, uint);
char*           strncpy(char*, const char*, int);

// printf.c
void            printf(const char*, ...);
void            panic(const char*) __attribute__((noreturn));

// proc.c
struct proc*    myproc(void);
void            procinit(void);
void            scheduler(void) __attribute__((noreturn));
void            yield(void);
void            sleep(void*, struct spinlock*);
void            wakeup(void*);
int             fork(void);
void            exit(void);
int             wait(void);
int             kill(int);
void            sched(void);
void            swtch(struct context**, struct context*);
void            userinit(void);

// cpu.c
struct cpu*     mycpu(void);
void            pushcli(void);
void            popcli(void);

// spinlock.c
void            initlock(struct spinlock*, char*);
void            acquire(struct spinlock*);
void            release(struct spinlock*);
int             holding(struct spinlock*);

// vm.c
void            kvminit(void);
void            seginit(void);
pde_t*          setupkvm(void);
void            inituvm(pde_t*, char*, uint);
int             loaduvm(pde_t*, char*, struct inode*, uint, uint);
int             allocuvm(pde_t*, uint, uint);
int             deallocuvm(pde_t*, uint, uint);
void            freevm(pde_t*);
pde_t*          copyuvm(pde_t*, uint);
void            switchuvm(struct proc*);
void            switchkvm(void);
int             copyout(pde_t*, uint, void*, uint);
pte_t*          walkpgdir(pde_t*, const void*, int);
int             mappages(pde_t*, void*, uint, uint, int);

// kalloc.c
void            kinit(void);
char*           kalloc(void);
void            kfree(char*);

// trap.c
void            idtinit(void);
void            trap(struct trapframe*);
void            trapret(void);
void            forkret(void);

// syscall.c
void            syscall(void);

#endif // DEFS_H
