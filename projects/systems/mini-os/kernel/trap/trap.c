#include "types.h"
#include "defs.h"
#include "trap/trap.h"
#include "proc/proc.h"
#include "x86.h"

// Interrupt descriptor table (IDT)
struct gatedesc {
    uint off_15_0 : 16;   // low 16 bits of offset in segment
    uint cs : 16;         // code segment selector
    uint args : 5;        // # args, 0 for interrupt/trap gates
    uint rsv1 : 3;        // reserved(should be zero I guess)
    uint type : 4;        // type(STS_{TG,IG32,TG32})
    uint s : 1;           // must be 0 (system)
    uint dpl : 2;         // descriptor(meaning new) privilege level
    uint p : 1;           // Present
    uint off_31_16 : 16;  // high bits of offset in segment
};

struct gatedesc idt[256];

extern uint vectors[];  // in vectors.S: array of 256 entry pointers

#define SETGATE(gate, istrap, sel, off, d)                \
{                                                          \
  (gate).off_15_0 = (uint)(off) & 0xffff;                \
  (gate).cs = (sel);                                      \
  (gate).args = 0;                                        \
  (gate).rsv1 = 0;                                        \
  (gate).type = (istrap) ? 0xF : 0xE;                    \
  (gate).s = 0;                                           \
  (gate).dpl = (d);                                       \
  (gate).p = 1;                                           \
  (gate).off_31_16 = (uint)(off) >> 16;                  \
}

static void
lidt(struct gatedesc *p, int size)
{
  volatile uint16_t pd[3];

  pd[0] = size-1;
  pd[1] = (uint)p;
  pd[2] = (uint)p >> 16;

  asm volatile("lidt (%0)" : : "r" (pd));
}

void idtinit(void) {
    int i;

    for (i = 0; i < 256; i++) {
        SETGATE(idt[i], 0, 0x08, vectors[i], 0);
    }
    // System call gate - allow user mode to call
    SETGATE(idt[T_SYSCALL], 1, 0x08, vectors[T_SYSCALL], DPL_USER);

    // Load IDT
    lidt(idt, sizeof(idt));
}

// Trap handler
void trap(struct trapframe *tf) {
    if (tf->trapno == T_SYSCALL) {
        if (myproc()->killed) {
            exit();
        }
        myproc()->tf = tf;
        syscall();
        if (myproc()->killed) {
            exit();
        }
        return;
    }

    switch (tf->trapno) {
    case T_IRQ0 + IRQ_TIMER:
        // Timer interrupt
        // Increment tick count
        // Call scheduler if time slice expired
        if (myproc() != 0 && myproc()->state == RUNNING) {
            yield();
        }
        break;

    case T_IRQ0 + IRQ_KBD:
        // Keyboard interrupt
        break;

    default:
        if (myproc() == 0 || (tf->cs & 3) == 0) {
            // In kernel, panic
            printf("unexpected trap %d from cpu %d eip %x\n",
                   tf->trapno, 0, tf->eip);
            panic("trap");
        }
        // In user space, kill process
        printf("pid %d: trap %d err %d on cpu %d eip 0x%x\n",
               myproc()->pid, tf->trapno, tf->err, 0, tf->eip);
        myproc()->killed = 1;
    }

    // Force process exit if it has been killed and is in user space
    if (myproc() && myproc()->killed && (tf->cs & 3) == DPL_USER) {
        exit();
    }

    // Force process to give up CPU on clock tick
    if (myproc() && myproc()->state == RUNNING &&
        tf->trapno == T_IRQ0 + IRQ_TIMER) {
        yield();
    }

    // Check if the process has been killed since we yielded
    if (myproc() && myproc()->killed && (tf->cs & 3) == DPL_USER) {
        exit();
    }
}
