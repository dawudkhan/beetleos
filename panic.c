#include "panic.h"
#include "uart.h"

void panic(const char *message) {
    uart_puts("PANIC: ");
    uart_puts(message);
    uart_puts("\n");

    while (1) {
        __asm__ volatile("wfi");
    }
}