#include "kalloc.h"
#include "panic.h"
#include "proc.h"
#include "trap.h"
#include "vm.h"
#include <stdint.h>

#define UART0 0x10000000UL
#define KERNEL_BASE 0x80000000UL
#define PAGE_SIZE 0x1000UL

extern char kernel_end[];
extern void enter_user_mode(uint64_t satp_val, uintptr_t entry_pc);

// hand-encoded so it can be copied onto its own kalloc'd page: li a7,42; ecall; j .
static const uint32_t user_code[] = {
    0x02a00893,
    0x00000073,
    0x0000006f,
};

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

    vm_map_kernel(root);

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

    timer_init();
    uart_puts("timer enabled!\n");

    struct proc *p1 = proc_alloc();
    struct proc *p2 = proc_alloc();

    if (p1 == 0 || p2 == 0)
        panic("failed to allocate process");

    void *page1 = kalloc();
    void *page2 = kalloc();

    vm_map(p1->pagetable, 0x1000, (uintptr_t)page1, PTE_R | PTE_W);
    vm_map(p2->pagetable, 0x1000, (uintptr_t)page2, PTE_R | PTE_W);

    pte_t *pte1 = vm_walk(p1->pagetable, 0x1000);
    pte_t *pte2 = vm_walk(p2->pagetable, 0x1000);

    if (pte1 == 0 || pte2 == 0)
        panic("process mapping missing");

    if (((*pte1 >> 10) << 12) == ((*pte2 >> 10) << 12))
        panic("processes share a physical page unexpectedly");

    uart_puts("process isolation verified!\n");

    vm_map_kernel(p1->pagetable);

    uint32_t *user_page = kalloc();
    if (user_page == 0)
        panic("failed to allocate user code page");

    for (uint64_t i = 0; i < sizeof(user_code) / sizeof(user_code[0]); i++)
        user_page[i] = user_code[i];

    uintptr_t user_pc = (uintptr_t)user_page;
    vm_map(p1->pagetable, user_pc, user_pc, PTE_R | PTE_X | PTE_U);

    uint64_t user_satp = (8UL << 60) | ((uintptr_t)p1->pagetable >> 12);

    uart_puts("entering u-mode...\n");
    enter_user_mode(user_satp, user_pc);

    while (1) {
        __asm__ volatile("wfi");
    }
}
