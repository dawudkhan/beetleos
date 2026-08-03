#ifndef PROC_H
#define PROC_H

#include "vm.h"
#include <stdint.h>

enum proc_state { UNUSED, RUNNABLE, RUNNING };

struct proc {
    pagetable_t pagetable;
    uint64_t regs[32];
    uint64_t pc;
    enum proc_state state;
};

extern struct proc *current_proc;

struct proc *proc_alloc(void);
// round robin over runnable processes, or 0 if none are runnable.
struct proc *proc_schedule(void);

#endif
