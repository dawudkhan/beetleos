#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

// dispatches using regs[17] (a7), then writes the return value into regs[10] (a0).
void syscall_dispatch(uint64_t *regs);

#endif
