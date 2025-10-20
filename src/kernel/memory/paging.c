#include "paging.h"

#include <stdbool.h>

#include "heap.h"

int8_t paging_remap(page_directory_entry *table, uint32_t linear_addr,
                    uint32_t physical_addr, uint32_t size, uint32_t flags,
                    bool invalidate) {
    if (!table || !size || (size & PAGE_ALIGN_MASK) != 0) {
        return 0;
    }

    if ((linear_addr & PAGE_ALIGN_MASK) != 0 ||
        (physical_addr & PAGE_ALIGN_MASK) != 0) {
        return 0;
    }

    uint32_t pages_n = size / PAGE_SIZE;
    uint32_t linear_frame = PAGING_FRAME(linear_addr);
    uint32_t physical_frame = PAGING_FRAME(physical_addr);

    if (linear_frame + pages_n > PAGING_MAX_FRAME ||
        physical_frame + pages_n > PAGING_MAX_FRAME) {
        return 0;
    }

    page_table_entry *page_table = 0;
    for (uint32_t i = 0; i < pages_n; i++) {
        uint32_t lframe = i + linear_frame;
        uint32_t pframe = i + physical_frame;

        if (page_table && PAGING_PT_INDEX(lframe)) goto skip_lookup;

        page_directory_entry *dte = &table[PAGING_DTE_INDEX(lframe)];

        if (!dte->s.present) {
            page_table = heap_zalloc(PAGE_SIZE);
            if (!page_table) {
                return 0;
            }
            dte->s.frame = ((uint32_t)page_table) >> PAGE_SIZE_SHIFT;
            dte->i |= flags;
        } else {
            page_table = (page_table_entry *)(dte->s.frame << PAGE_SIZE_SHIFT);
        }

    skip_lookup:
        page_table[PAGING_PT_INDEX(lframe)].s.frame = pframe;
        page_table[PAGING_PT_INDEX(lframe)].i |= flags;
    }

    if (invalidate) {
        paging_set_directory_table(table);
    }

    return 1;
}

page_directory_entry *paging_identity_map(uint32_t size, uint32_t flags) {
    page_directory_entry *page_directory = heap_zalloc(PAGE_SIZE);
    if (!page_directory) {
        return 0;
    }

    if (!paging_remap(page_directory, 0, 0, size, flags, false)) {
        for (uint32_t i = 0; i < PAGING_PD_SIZE; i++) {
            if (!page_directory[i].s.present) {
                continue;
            }
            heap_free((void *)(page_directory[i].s.frame << PAGE_SIZE_SHIFT));
        }
        heap_free(page_directory);
        return 0;
    }

    return page_directory;
}

int paging_set_directory_table(page_directory_entry *table) {
    asm volatile("mov %0, %%cr3" : : "r"((uint32_t)table) : "memory");
    return 0;
}

int paging_enable() {
    asm volatile(
        "mov %%cr0, %%eax\n\t"
        "or $0x80000000, %%eax\n\t"
        "mov %%eax, %%cr0"
        :
        :
        : "eax", "memory");
    return 0;
}
