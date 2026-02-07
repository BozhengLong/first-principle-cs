#ifndef VM_H
#define VM_H

#include "types.h"

// Page directory and page table constants
#define PGSIZE          4096    // bytes per page
#define PGSHIFT         12      // log2(PGSIZE)

// Page table/directory entry flags
#define PTE_P           0x001   // Present
#define PTE_W           0x002   // Writeable
#define PTE_U           0x004   // User
#define PTE_PS          0x080   // Page Size

// Address in page table or page directory entry
#define PTE_ADDR(pte)   ((uint)(pte) & ~0xFFF)
#define PTE_FLAGS(pte)  ((uint)(pte) &  0xFFF)

// Construct virtual address from indexes and offset
#define PGADDR(d, t, o) ((uint)((d) << 22 | (t) << 12 | (o)))

// Page directory index
#define PDX(va)         (((uint)(va) >> 22) & 0x3FF)

// Page table index
#define PTX(va)         (((uint)(va) >> 12) & 0x3FF)

// Page number
#define PPN(pa)         (((uint)(pa)) >> PGSHIFT)

// Construct physical address from PPN and offset
#define PGADDR_PHYS(ppn, o) (((ppn) << PGSHIFT) | (o))

// Round down to the nearest multiple of PGSIZE
#define PGROUNDDOWN(a) (((a)) & ~(PGSIZE-1))

// Round up to the nearest multiple of PGSIZE
#define PGROUNDUP(a) (((a) + PGSIZE - 1) & ~(PGSIZE-1))

typedef uint pte_t;
typedef uint pde_t;

// Segment Descriptor
struct segdesc {
    uint lim_15_0 : 16;  // Low bits of segment limit
    uint base_15_0 : 16; // Low bits of segment base address
    uint base_23_16 : 8; // Middle bits of segment base address
    uint type : 4;       // Segment type (see STS_ constants)
    uint s : 1;          // 0 = system, 1 = application
    uint dpl : 2;        // Descriptor Privilege Level
    uint p : 1;          // Present
    uint lim_19_16 : 4;  // High bits of segment limit
    uint avl : 1;        // Unused (available for software use)
    uint rsv1 : 1;       // Reserved
    uint db : 1;         // 0 = 16-bit segment, 1 = 32-bit segment
    uint g : 1;          // Granularity: limit scaled by 4K when set
    uint base_31_24 : 8; // High bits of segment base address
};

// Segment selectors
#define SEG_KCODE 1  // kernel code
#define SEG_KDATA 2  // kernel data+stack
#define SEG_UCODE 3  // user code
#define SEG_UDATA 4  // user data+stack

#define DPL_USER    0x3     // User DPL

// Application segment type bits
#define STA_X       0x8     // Executable segment
#define STA_W       0x2     // Writeable (non-executable segments)
#define STA_R       0x2     // Readable (executable segments)

#endif // VM_H
