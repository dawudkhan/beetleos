#include <stdint.h>
#include "kalloc.h"
#include "panic.h"

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

static void test_allocator(void)
{
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

void kernel_main(void)
{
    uart_puts("Hello from BeetleOS!\n");

    kalloc_init();
    test_allocator();

    while (1) {
        __asm__ volatile("wfi");
    }
}
