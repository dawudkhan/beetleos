#include "uart.h"
#include <stdint.h>

#define UART0 0x10000000UL

void uart_putc(char c) {
    volatile uint8_t *uart = (volatile uint8_t *)UART0;
    *uart = (uint8_t)c;
}

void uart_puts(const char *s) {
    while (*s) {
        uart_putc(*s);
        s++;
    }
}
