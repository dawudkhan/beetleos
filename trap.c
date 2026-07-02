#include "trap.h"
#include "panic.h"
#include <stdint.h>

#define UART0 0x10000000UL

extern char trap_vector[];

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

// print a 64-bit value as 0x-prefixed hex, one nibble at a time.
static void uart_put_hex(uint64_t value) {
    uart_puts("0x");
    for (int shift = 60; shift >= 0; shift -= 4) {
        uint8_t nibble = (value >> shift) & 0xf;
        uart_putc(nibble < 10 ? '0' + nibble : 'a' + (nibble - 10));
    }
}

void trap_handler(void) {
    uint64_t scause, sepc, stval;
    __asm__ volatile("csrr %0, scause" : "=r"(scause));
    __asm__ volatile("csrr %0, sepc" : "=r"(sepc));
    __asm__ volatile("csrr %0, stval" : "=r"(stval));

    uart_puts("trap: scause=");
    uart_put_hex(scause);
    uart_puts(" sepc=");
    uart_put_hex(sepc);
    uart_puts(" stval=");
    uart_put_hex(stval);
    uart_puts("\n");

    panic("unhandled trap");
}

void trap_init(void) {
    __asm__ volatile("csrw stvec, %0" ::"r"(trap_vector));
}
