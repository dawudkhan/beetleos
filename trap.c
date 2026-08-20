#include "trap.h"
#include "panic.h"
#include "proc.h"
#include "syscall.h"
#include "uart.h"
#include <stdint.h>

extern char trap_vector[];

// print a 64-bit value as 0x-prefixed hex, one nibble at a time.
static void uart_put_hex(uint64_t value) {
    uart_puts("0x");
    for (int shift = 60; shift >= 0; shift -= 4) {
        uint8_t nibble = (value >> shift) & 0xf;
        uart_putc(nibble < 10 ? '0' + nibble : 'a' + (nibble - 10));
    }
}

// bit 63 of scause is set for interrupts, the rest is the cause code.
static const char *cause_name(uint64_t scause) {
    uint64_t code = scause & 0x7fffffffffffffffULL;

    if (scause >> 63) {
        switch (code) {
            case 1:
                return "supervisor software interrupt";
            case 5:
                return "supervisor timer interrupt";
            case 9:
                return "supervisor external interrupt";
            default:
                return "unknown interrupt";
        }
    }

    switch (code) {
        case 0:
            return "instruction address misaligned";
        case 1:
            return "instruction access fault";
        case 2:
            return "illegal instruction";
        case 3:
            return "breakpoint";
        case 4:
            return "load address misaligned";
        case 5:
            return "load access fault";
        case 6:
            return "store address misaligned";
        case 7:
            return "store access fault";
        case 8:
            return "ecall from u-mode";
        case 9:
            return "ecall from s-mode";
        case 12:
            return "instruction page fault";
        case 13:
            return "load page fault";
        case 15:
            return "store page fault";
        default:
            return "unknown exception";
    }
}

// save the outgoing process, pick the next runnable one, load it into
// the trap frame so the sret this trap ends with resumes it instead.
static void schedule(uint64_t *regs, uint64_t sepc) {
    struct proc *next = proc_schedule();
    if (next == 0)
        return;

    struct proc *prev = current_proc;

    if (prev != 0) {
        for (int i = 0; i < 32; i++)
            prev->regs[i] = regs[i];
        prev->pc = sepc;
        if (prev->state == RUNNING)
            prev->state = RUNNABLE;
    }

    for (int i = 0; i < 32; i++)
        regs[i] = next->regs[i];

    __asm__ volatile("csrw sepc, %0" ::"r"(next->pc));

    if (prev == 0 || next->pagetable != prev->pagetable) {
        uint64_t satp_value = (8UL << 60) | ((uintptr_t)next->pagetable >> 12);
        __asm__ volatile("csrw satp, %0" ::"r"(satp_value));
        __asm__ volatile("sfence.vma");
    }

    next->state = RUNNING;
    current_proc = next;
}

void trap_handler(uint64_t *regs) {
    uint64_t scause, sepc, stval;
    __asm__ volatile("csrr %0, scause" : "=r"(scause));
    __asm__ volatile("csrr %0, sepc" : "=r"(sepc));
    __asm__ volatile("csrr %0, stval" : "=r"(stval));

    uart_puts("trap: ");
    uart_puts(cause_name(scause));
    uart_puts(" scause=");
    uart_put_hex(scause);
    uart_puts(" sepc=");
    uart_put_hex(sepc);
    uart_puts(" stval=");
    uart_put_hex(stval);
    uart_puts("\n");

    // ecall from s-mode (boot-time trap test)
    if (scause == 9) {
        uart_puts("  a7=");
        uart_put_hex(regs[17]);
        uart_puts("\n");

        sepc += 4;
        __asm__ volatile("csrw sepc, %0" ::"r"(sepc));
        return;
    }

    // ecall from u-mode.
    if (scause == 8) {
        sepc += 4;
        __asm__ volatile("csrw sepc, %0" ::"r"(sepc));

        syscall_dispatch(regs);

        // sys_exit left current_proc unused, do not resume.
        if (current_proc->state != RUNNING)
            schedule(regs, sepc);

        return;
    }

    // supervisor software interrupt, used here as the timer tick.
    if (scause == (1ULL << 63 | 1)) {
        __asm__ volatile("csrc sip, %0" ::"r"(2));
        schedule(regs, sepc);
        return;
    }

    panic("unhandled trap");
}

void trap_init(void) {
    __asm__ volatile("csrw stvec, %0" ::"r"(trap_vector));
}

void timer_init(void) {
    __asm__ volatile("csrs sie, %0" ::"r"(2));
    __asm__ volatile("csrs sstatus, %0" ::"r"(2));
}
