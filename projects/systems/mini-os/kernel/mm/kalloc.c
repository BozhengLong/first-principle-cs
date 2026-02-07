#include "types.h"
#include "defs.h"
#include "mm/vm.h"

// Physical memory allocator
// Allocates 4096-byte pages

struct run {
    struct run *next;
};

struct {
    struct spinlock lock;
    struct run *freelist;
} kmem;

extern char end[]; // first address after kernel loaded from ELF file

// Initialize free list of physical pages
void kinit(void) {
    initlock(&kmem.lock, "kmem");
    freerange(end, (void*)0x1000000); // Free memory from end of kernel to 16MB
}

void freerange(void *vstart, void *vend) {
    char *p;
    p = (char*)PGROUNDUP((uint)vstart);
    for (; p + PGSIZE <= (char*)vend; p += PGSIZE) {
        kfree(p);
    }
}

// Free the page of physical memory pointed at by v
void kfree(char *v) {
    struct run *r;

    if ((uint)v % PGSIZE || v < end || (uint)v >= 0x1000000) {
        panic("kfree");
    }

    // Fill with junk to catch dangling refs
    memset(v, 1, PGSIZE);

    acquire(&kmem.lock);
    r = (struct run*)v;
    r->next = kmem.freelist;
    kmem.freelist = r;
    release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory
// Returns a pointer that the kernel can use
// Returns 0 if the memory cannot be allocated
char* kalloc(void) {
    struct run *r;

    acquire(&kmem.lock);
    r = kmem.freelist;
    if (r) {
        kmem.freelist = r->next;
    }
    release(&kmem.lock);

    return (char*)r;
}
