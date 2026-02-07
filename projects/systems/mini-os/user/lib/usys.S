# System call stubs for user programs

#include "syscall.h"

# System call stub macro
.macro SYSCALL name, number
.globl \name
\name:
    movl $\number, %eax
    int $0x40
    ret
.endm

# Define all system calls
SYSCALL fork, SYS_fork
SYSCALL exit, SYS_exit
SYSCALL wait, SYS_wait
SYSCALL pipe, SYS_pipe
SYSCALL read, SYS_read
SYSCALL write, SYS_write
SYSCALL close, SYS_close
SYSCALL kill, SYS_kill
SYSCALL exec, SYS_exec
SYSCALL open, SYS_open
SYSCALL mknod, SYS_mknod
SYSCALL unlink, SYS_unlink
SYSCALL fstat, SYS_fstat
SYSCALL link, SYS_link
SYSCALL mkdir, SYS_mkdir
SYSCALL chdir, SYS_chdir
SYSCALL dup, SYS_dup
SYSCALL getpid, SYS_getpid
SYSCALL sbrk, SYS_sbrk
SYSCALL sleep, SYS_sleep
SYSCALL uptime, SYS_uptime
