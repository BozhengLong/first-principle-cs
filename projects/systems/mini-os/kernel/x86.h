#ifndef X86_H
#define X86_H

#include "types.h"

// Inline assembly functions for x86 operations

static inline uint8_t
inb(uint16_t port)
{
  uint8_t data;
  __asm__ __volatile__("inb %w1, %b0" : "=a" (data) : "Nd" (port) : "memory");
  return data;
}

static inline void
outb(uint16_t port, uint8_t data)
{
  __asm__ __volatile__("outb %b0, %w1" : : "a" (data), "Nd" (port) : "memory");
}

static inline void
cli(void)
{
  asm volatile("cli");
}

static inline void
sti(void)
{
  asm volatile("sti");
}

static inline uint
xchg(volatile uint *addr, uint newval)
{
  uint result;
  asm volatile("lock; xchgl %0, %1" :
               "+m" (*addr), "=a" (result) :
               "1" (newval) :
               "cc");
  return result;
}

static inline uint
readeflags(void)
{
  uint eflags;
  asm volatile("pushfl; popl %0" : "=r" (eflags));
  return eflags;
}

#define FL_IF 0x00000200  // Interrupt Enable

#endif // X86_H
