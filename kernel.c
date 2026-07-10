#include "kalloc.h"
#include "panic.h"
#include "trap.h"
#include "vm.h"
#include <stdint.h>

#define UART0 0x10000000UL
#define KERNEL_BASE 0x80000000UL
#define PAGE_SIZE 0x1000UL

extern char kernel_end[];

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

    uintptr_t kernel_top = ((uintptr_t)kernel_end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    for (uintptr_t addr = KERNEL_BASE; addr < kernel_top; addr += PAGE_SIZE)
        vm_map(root, addr, addr, PTE_R | PTE_W | PTE_X);

    vm_map(root, UART0, UART0, PTE_R | PTE_W);

    if (vm_walk(root, KERNEL_BASE) == 0)
        panic("kernel base not mapped");

    if (vm_walk(root, kernel_top - PAGE_SIZE) == 0)
        panic("last kernel page not mapped");

    if (vm_walk(root, UART0) == 0)
        panic("uart not mapped");

    uart_puts("mapping verified!\n");

    uint64_t satp_value = (8UL << 60) | ((uintptr_t)root >> 12);
    __asm__ volatile("csrw satp, %0" ::"r"(satp_value));
    __asm__ volatile("sfence.vma");

    uart_puts("paging enabled!\n");

    trap_init();
    __asm__ volatile("ecall");

    uart_puts("resumed after trap!\n");

    while (1) {
        __asm__ volatile("wfi");
    }
}
