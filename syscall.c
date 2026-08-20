#include "syscall.h"
#include "proc.h"
#include "uart.h"

// using linux compatible syscall numbers for convenience.
#define SYS_WRITE 64
#define SYS_EXIT 93

#define SSTATUS_SUM (1UL << 18)

static uint64_t sys_write(int fd, const char *buf, uint64_t len) {
    // ignored for now.
    (void)fd;

    // enable sstatus.SUM so s-mode can read PTE_U page (buf is a user pointer)
    // enabled only for the duration of the read.
    __asm__ volatile("csrs sstatus, %0" ::"r"(SSTATUS_SUM));
    for (uint64_t i = 0; i < len; i++)
        uart_putc(buf[i]);
    __asm__ volatile("csrc sstatus, %0" ::"r"(SSTATUS_SUM));

    return len;
}

static uint64_t sys_exit(int status) {
    (void)status;
    current_proc->state = UNUSED;
    return 0;
}

void syscall_dispatch(uint64_t *regs) {
    uint64_t a0 = regs[10];
    uint64_t a1 = regs[11];
    uint64_t a2 = regs[12];

    switch (regs[17]) {
        case SYS_WRITE:
            regs[10] = sys_write((int)a0, (const char *)a1, a2);
            break;
        case SYS_EXIT:
            regs[10] = sys_exit((int)a0);
            break;
        default:
            regs[10] = (uint64_t)-1;
            break;
    }
}
