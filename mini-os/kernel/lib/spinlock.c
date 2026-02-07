#include "types.h"
#include "defs.h"
#include "proc/proc.h"
#include "x86.h"

// Initialize a spinlock
void
initlock(struct spinlock *lk, char *name)
{
  lk->name = name;
  lk->locked = 0;
  lk->cpu = -1;
}

// Acquire the lock
// Loops (spins) until the lock is acquired
void
acquire(struct spinlock *lk)
{
  pushcli(); // Disable interrupts to avoid deadlock

  // The xchg is atomic
  while(xchg(&lk->locked, 1) != 0)
    ;

  // Record info about lock acquisition for debugging
  lk->cpu = mycpu() - cpus;
}

// Release the lock
void
release(struct spinlock *lk)
{
  lk->cpu = -1;

  // The xchg serializes, so that reads before release are
  // not reordered after it.  The 1996 PentiumPro manual (Volume 3,
  // 7.2) says reads can be carried out speculatively and in
  // any order, which implies we need to serialize here.
  // But the 2007 Intel 64 Architecture Memory Ordering White
  // Paper says that Intel 64 and IA-32 will not move a load
  // after a store. So lock->locked = 0 would work here.
  // The xchg being asm volatile ensures gcc emits it after
  // the above assignments (and after the critical section).
  xchg(&lk->locked, 0);

  popcli();
}

// Check whether this cpu is holding the lock
int
holding(struct spinlock *lock)
{
  return lock->locked && lock->cpu == (mycpu() - cpus);
}
