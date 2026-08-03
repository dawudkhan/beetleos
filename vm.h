#ifndef VM_H
#define VM_H

#include <stdint.h>

typedef uint64_t pte_t;
typedef pte_t *pagetable_t;

#define PTE_V (1UL << 0)
#define PTE_R (1UL << 1)
#define PTE_W (1UL << 2)
#define PTE_X (1UL << 3)
#define PTE_U (1UL << 4)

pagetable_t vm_create(void);

void vm_map(pagetable_t root, uintptr_t va, uintptr_t pa, uint64_t flags);

pte_t *vm_walk(pagetable_t root, uintptr_t va);

// base mappings (kernel image, ram direct map, uart) every page table needs.
void vm_map_kernel(pagetable_t root);

#endif