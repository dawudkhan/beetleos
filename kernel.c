#include "kalloc.h"
#include "panic.h"
#include "proc.h"
#include "trap.h"
#include "uart.h"
#include "vm.h"
#include <stdint.h>

#define UART0 0x10000000UL
#define KERNEL_BASE 0x80000000UL
#define PAGE_SIZE 0x1000UL

extern char kernel_end[];
extern void enter_user_mode(uint64_t satp_val, uintptr_t entry_pc);

// user_code_p1 and user_code_p2 are 2 AI generated test programs to test the syscall and scheduler.

// hand-assembled: la a1, msg; li a0,1; li a2,3; li a7,SYS_WRITE; ecall;
// li a7,SYS_EXIT; li a0,0; ecall; j .; msg: "p1\n"
static const uint32_t user_code_p1[] = {
    0x00000597, // auipc a1, 0
    0x02858593, // addi  a1, a1, 40     -- a1 = &msg
    0x00100513, // li a0, 1             -- fd
    0x00300613, // li a2, 3             -- len
    0x04000893, // li a7, 64            -- SYS_WRITE
    0x00000073, // ecall
    0x05d00893, // li a7, 93            -- SYS_EXIT
    0x00000513, // li a0, 0             -- status
    0x00000073, // ecall
    0x0000006f, // j .
    0x000a3170, // msg: "p1\n\0"
};

static const uint32_t user_code_p2[] = {
    0x00000597, // auipc a1, 0
    0x02858593, // addi  a1, a1, 40     -- a1 = &msg
    0x00100513, // li a0, 1             -- fd
    0x00300613, // li a2, 3             -- len
    0x04000893, // li a7, 64            -- SYS_WRITE
    0x00000073, // ecall
    0x05d00893, // li a7, 93            -- SYS_EXIT
    0x00000513, // li a0, 0             -- status
    0x00000073, // ecall
    0x0000006f, // j .
    0x000a3270, // msg: "p2\n\0"
};

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
    vm_map_kernel(p2->pagetable);

    uint32_t *p1_code = kalloc();
    uint32_t *p2_code = kalloc();
    if (p1_code == 0 || p2_code == 0)
        panic("failed to allocate user code page");

    for (uint64_t i = 0; i < sizeof(user_code_p1) / sizeof(user_code_p1[0]); i++)
        p1_code[i] = user_code_p1[i];

    for (uint64_t i = 0; i < sizeof(user_code_p2) / sizeof(user_code_p2[0]); i++)
        p2_code[i] = user_code_p2[i];

    vm_map(p1->pagetable, (uintptr_t)p1_code, (uintptr_t)p1_code, PTE_R | PTE_X | PTE_U);
    vm_map(p2->pagetable, (uintptr_t)p2_code, (uintptr_t)p2_code, PTE_R | PTE_X | PTE_U);

    p1->pc = (uintptr_t)p1_code;
    p2->pc = (uintptr_t)p2_code;

    struct proc *first = proc_schedule();
    if (first == 0)
        panic("no runnable process");

    first->state = RUNNING;
    current_proc = first;

    uint64_t first_satp = (8UL << 60) | ((uintptr_t)first->pagetable >> 12);

    uart_puts("starting scheduler...\n");
    enter_user_mode(first_satp, first->pc);

    while (1) {
        __asm__ volatile("wfi");
    }
}
