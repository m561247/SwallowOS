#ifndef _PAGEMANAGER_H
#define _PAGEMANAGER_H

#include <stdint.h>
#include <stddef.h>
#include "../cpu/cpu.h"

/* 页帧格式 */
typedef uint64_t *pageframe_t;

struct page_alloc {
    pageframe_t page;
    uint64_t npages;
};

void kalloc_frame_init();
struct page_alloc alloc_pages(size_t count);
void free_pages(struct page_alloc *pa);
void page_fault_handler(struct pt_regs *regs);

#endif