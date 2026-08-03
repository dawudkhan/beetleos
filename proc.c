#include "proc.h"
#include "kalloc.h"
#include "panic.h"
#include "vm.h"

#define NPROC 4
#define PAGE_SIZE 0x1000UL

static struct proc proc_table[NPROC];
struct proc *current_proc = 0;

struct proc *proc_schedule(void) {
    static int next = 0;

    for (int i = 0; i < NPROC; i++) {
        int idx = (next + i) % NPROC;
        if (proc_table[idx].state == RUNNABLE) {
            next = (idx + 1) % NPROC;
            return &proc_table[idx];
        }
    }

    return 0;
}

struct proc *proc_alloc(void) {
    for (int i = 0; i < NPROC; i++) {
        if (proc_table[i].state == UNUSED) {
            struct proc *p = &proc_table[i];

            p->pagetable = vm_create();
            if (p->pagetable == 0)
                panic("failed to create process page table");

            // sp must be valid before scheduling; trap.S reuses it as the trap frame base.
            void *stack = kalloc();
            if (stack == 0)
                panic("failed to allocate process stack");
            p->regs[2] = (uint64_t)stack + PAGE_SIZE;

            p->state = RUNNABLE;
            return p;
        }
    }

    return 0;
}
