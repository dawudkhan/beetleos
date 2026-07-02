#include "trap.h"
#include "panic.h"

extern char trap_vector[];

void trap_handler(void) {
    panic("trap taken");
}

void trap_init(void) {
    __asm__ volatile("csrw stvec, %0" ::"r"(trap_vector));
}
