#include "kalloc.h"
#include <stdint.h>

#define PAGE_SIZE  0x1000UL
#define RAM_END    0x88000000UL

extern char kernel_end[];

static struct page *free_list;

static uintptr_t align_up(uintptr_t address, uintptr_t alignment) {
    return (address + alignment - 1) & ~(alignment - 1);
}

void kalloc_init(void) {
    uintptr_t start = align_up((uintptr_t)kernel_end, PAGE_SIZE);

    for (uintptr_t addr = start;
         addr < RAM_END;
         addr += PAGE_SIZE) {

        struct page *page = (struct page *)addr;
        page->next = free_list;
        free_list = page;
    }
}

void *kalloc(void) {
    // end of list.
    if (free_list == 0)
        return 0;

    struct page *page = free_list;
    free_list = page->next;

    return page;
}

void kfree(void *page) {
    // no op.
    if (page == 0)
        return;

    struct page *p = (struct page *)page;
    p->next = free_list;
    free_list = p;
}