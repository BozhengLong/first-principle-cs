#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "lock.h"

// Initialize a lock
void initlock(struct spinlock *lk, char *name) {
    lk->name = name;
    lk->locked = 0;
    pthread_mutex_init(&lk->lock, NULL);
}

// Acquire the lock
void acquire(struct spinlock *lk) {
    pthread_mutex_lock(&lk->lock);
    lk->locked = 1;
}

// Release the lock
void release(struct spinlock *lk) {
    lk->locked = 0;
    pthread_mutex_unlock(&lk->lock);
}

// Check whether this thread is holding the lock
int holding(struct spinlock *lk) {
    return lk->locked;
}
