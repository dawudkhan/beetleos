#include "kalloc.h"
#include "panic.h"
#include "vm.h"
#include <stdint.h>

#define UART0 0x10000000UL

static void uart_putc(char c) {
    volatile uint8_t *uart = (volatile uint8_t *)UART0;
    *uart = (uint8_t)c;
}

static void uart_puts(const char *s) {
    while (*s) {
        uart_putc(*s);
        s++;
    }
}

static void test_allocator(void) {
    void *a = kalloc();
    void *b = kalloc();

    if (a == 0)
        panic("first allocation failed");

    if (b == 0)
        panic("second allocation failed");

    if (a == b)
        panic("allocator returned same page twice");

    kfree(a);

    void *c = kalloc();

    if (c != a)
        panic("freed page was not reused");

    uart_puts("allocator test passed!\n");
}

void kernel_main(void) {
    uart_puts("Hello from BeetleOS!\n");

    kalloc_init();
    test_allocator();

    pagetable_t root = vm_create();
    if (root == 0)
        panic("failed to create page table");

    vm_map(root, 0x80000000UL, 0x80000000UL, PTE_R | PTE_W | PTE_X);

    pte_t *pte = vm_walk(root, 0x80000000UL);
    if (pte == 0)
        panic("mapping missing after vm_map");

    uintptr_t mapped_pa = (*pte >> 10) << 12;
    if (mapped_pa != 0x80000000UL)
        panic("mapping points at wrong physical address");

    if ((*pte & (PTE_R | PTE_W | PTE_X)) != (PTE_R | PTE_W | PTE_X))
        panic("mapping missing expected permission bits");

    uart_puts("mapping verified!\n");

    while (1) {
        __asm__ volatile("wfi");
    }
}
