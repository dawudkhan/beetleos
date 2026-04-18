#include "vm.h"
#include "kalloc.h"
#include "panic.h"

static uint64_t vpn_index(uintptr_t va, int level) {
    return (va >> (12 + 9 * level)) & 0x1ff;
}

static pagetable_t alloc_pagetable(void) {
    pagetable_t page_table = (pagetable_t)kalloc();

    if (page_table == 0)
        panic("failed to allocate page table");
    
    // zero the newly allocated memory to remove garbage.
    for (int i = 0; i < 512; i++)
        page_table[i] = 0;

    return page_table;
}

pagetable_t vm_create(void) {
    return alloc_pagetable();
}

void vm_map(pagetable_t root, uintptr_t va, uintptr_t pa, uint64_t flags) {
    pagetable_t table = root;

    for (int level = 2; level > 0; level--) {
        uint64_t index = vpn_index(va, level);
        pte_t pte = table[index];

        if ((pte & PTE_V) == 0) {
            pagetable_t next = alloc_pagetable();

            table[index] = (((uintptr_t)next >> 12) << 10) | PTE_V;

            pte = table[index];
        }

        table = (pagetable_t)((pte >> 10) << 12);
    }

    uint64_t index = vpn_index(va, 0);

    table[index] = ((pa >> 12) << 10) | flags | PTE_V;
}

pte_t *vm_walk(pagetable_t root, uintptr_t va) {
    pagetable_t table = root;

    for (int level = 2; level > 0; level--) {
        uint64_t index = vpn_index(va, level);
        pte_t pte = table[index];

        if ((pte & PTE_V) == 0)
            return 0;

        table = (pagetable_t)((pte >> 10) << 12);
    }

    uint64_t index = vpn_index(va, 0);

    return &table[index];
}