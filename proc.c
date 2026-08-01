#include "proc.h"
#include "panic.h"
#include "vm.h"

#define NPROC 4

static struct proc proc_table[NPROC];

struct proc *proc_alloc(void) {
    for (int i = 0; i < NPROC; i++) {
        if (proc_table[i].state == UNUSED) {
            struct proc *p = &proc_table[i];

            p->pagetable = vm_create();
            if (p->pagetable == 0)
                panic("failed to create process page table");

            p->state = RUNNABLE;
            return p;
        }
    }

    return 0;
}
