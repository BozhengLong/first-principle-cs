#ifndef LOCK_H
#define LOCK_H

#include <pthread.h>

// Spinlock structure (using pthread_mutex for simplicity)
struct spinlock {
    pthread_mutex_t lock;
    char *name;              // Name of lock (for debugging)
    int locked;              // Is the lock held?
};

// Lock operations
void initlock(struct spinlock *lk, char *name);
void acquire(struct spinlock *lk);
void release(struct spinlock *lk);
int holding(struct spinlock *lk);

#endif // LOCK_H
