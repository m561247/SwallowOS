
#include <stddef.h>
#include "../include/constant.h"
#include "ram.h"
#include "pagemanager.h"
#include "pgtable.h"
#include <kernel/page.h>
#include <kernel/printk.h>


/* 页分配有很多方法，如bitmap、stack/list、buddy alocations等，这里用最简单的bitmap */
#define MEM_END       (0xffffffff80000000 + 512 * 4096)               /* 暂时只讨论当前映射的一个页目录项 */
#define UINT64_BITS                           64
#define PRE_ALLOCATING_NUM                    20

/* 用来计算内核代码之后的 address of first page frame */
extern uint64_t _kernel_end;



uint64_t npages = 0;                        /* npages表示可分配的页个数，因为链接后才能得到 _kernel_end 的值，所以无法在编译期间计算，要运行之后计算 */
uint64_t *frame_map = NULL;                 /* frame_map标记某个页是否被使用，要放置在_kernel_end，同样也要运行之后计算 */
uint64_t *startframe = NULL;                /* 页帧起点，运行之后确定值 */

/* frame map operation */
static uint64_t get_frame_map(unsigned int index) {
    return (frame_map[index / UINT64_BITS] & (1 << (index % UINT64_BITS)))== 0 ? 0 : 1;
}
static void set_frame_map(unsigned int index, uint64_t val) {
    if (val > 0)
        frame_map[index/UINT64_BITS] |= 1 << (index % UINT64_BITS);
    else
        frame_map[index/UINT64_BITS] &= ~(1 << (index % UINT64_BITS));
}


// 函数 kalloc_frame_init 用于分配并初始化一个页面帧
void kalloc_frame_init() {
    /* init ram */
    init_ram();

    /* frame_map要sizeof(page_status)对齐 */
    if (!frame_map) {
        frame_map = (uint64_t *)(&_kernel_end + 1);
        /* 在frame_map后面填充其数据以及确定startframe */
        /* 首先要选一个合适的npages，设置为一个接近值x，则(x*4096 + x / 64) < (ram_end -  frame_map)， 解出 x = (ram_end - frame_map) / (4096 + 1/64), 实际上除数可以算成4097来估算 */
        npages = (ram_end + HIGHER_HALF_OFFSET - (uint64_t)frame_map) / (PAGE_SIZE + 1);
        /* 得到npages，即可容易算出startframe，注意4k对齐 */
        startframe = frame_map + (npages / UINT64_BITS);
        if ((uint64_t)startframe % PAGE_SIZE != 0)
            startframe = (uint64_t*)(((uint64_t)startframe / PAGE_SIZE + 1) * PAGE_SIZE);
        /* 检查页面是否超出内存，注意这里要考虑到VGA video占用的页，所以在比较的时候要减去4096 */
        while (startframe + PAGE_SIZE * npages > ram_end + HIGHER_HALF_OFFSET - PAGE_SIZE)
            npages--;
        for (uint64_t *p = frame_map; p < startframe; ++p)
            *p = 0;
    }
}

static int check_continuous_free_page(uint64_t start, size_t count) {
    if (start + count >= npages) return -1;     /* out of page frame range */

    for (unsigned int i=0; i<count; ++i) {
        if (get_frame_map(start + i))
            return -1;
    }
    return 0;
}

struct page_alloc alloc_pages(size_t count) {
    struct page_alloc pa = {0, 0};
    if (count >= npages || count == 0)
        return pa;
    for (unsigned int i=0; i <= npages - count; ++i) {
        if (check_continuous_free_page(i, count) == 0) {
            for (unsigned int j=0; j<count; ++j)
                set_frame_map(i + j, 1);
            pa.page = (char*)startframe + (i * PAGE_SIZE);
            pa.npages = count;
            return pa;
        }
    }
    return pa;
}

void free_pages(struct page_alloc *pa) {
    if (pa->npages != 0) {
        unsigned int start = (pa->page - startframe) / PAGE_SIZE;
        for (unsigned int i=start; i<start + pa->npages; ++i)
            set_frame_map(i, 0);
    }
}

void page_fault_handler(struct pt_regs *regs) {
    printk("\nIn exception 14\n Address: %u\n", getcr2());
    __asm__ volatile ("hlt");
}
