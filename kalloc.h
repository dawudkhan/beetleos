#ifndef KALLOC_H
#define KALLOC_H

struct page {
    struct page *next;
};

void kalloc_init(void);
void *kalloc(void);
void kfree(void *page);

#endif