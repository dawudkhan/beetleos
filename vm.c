#include "vm.h"
#include "kalloc.h"
#include "panic.h"

#define KERNEL_BASE 0x80000000UL
#define RAM_END 0x88000000UL
#define UART0 0x10000000UL
#define PAGE_SIZE 0x1000UL

extern char kernel_end[];

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

void vm_map_kernel(pagetable_t root) {
    uintptr_t kernel_top = ((uintptr_t)kernel_end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    for (uintptr_t addr = KERNEL_BASE; addr < kernel_top; addr += PAGE_SIZE)
        vm_map(root, addr, addr, PTE_R | PTE_W | PTE_X);

    for (uintptr_t addr = kernel_top; addr < RAM_END; addr += PAGE_SIZE)
        vm_map(root, addr, addr, PTE_R | PTE_W);

    vm_map(root, UART0, UART0, PTE_R | PTE_W);
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