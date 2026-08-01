#ifndef PROC_H
#define PROC_H

#include "vm.h"
#include <stdint.h>

enum proc_state { UNUSED, RUNNABLE, RUNNING };

struct proc {
    pagetable_t pagetable;
    uint64_t regs[32];
    enum proc_state state;
};

struct proc *proc_alloc(void);

#endif
