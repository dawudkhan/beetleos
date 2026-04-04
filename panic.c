#include "panic.h"

#define UART0 0x10000000UL

static void uart_putc(char c)
{
    volatile unsigned char *uart = (volatile unsigned char *)UART0;
    *uart = (unsigned char)c;
}

static void uart_puts(const char *s)
{
    while (*s) {
        uart_putc(*s);
        s++;
    }
}

void panic(const char *message)
{
    uart_puts("PANIC: ");
    uart_puts(message);
    uart_puts("\n");

    while (1) {
        __asm__ volatile("wfi");
    }
}