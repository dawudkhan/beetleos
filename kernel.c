#include <stdint.h>

#define UART0 0x10000000UL

static void uart_putc(char c)
{
    volatile uint8_t *uart = (volatile uint8_t *)UART0;
    *uart = (uint8_t)c;
}

static void uart_puts(const char *s)
{
    while (*s) {
        uart_putc(*s);
        s++;
    }
}

void kernel_main(void)
{
    uart_puts("Hello from BeetleOS!\n");

    while (1) {
        __asm__ volatile("wfi");
    }
}
