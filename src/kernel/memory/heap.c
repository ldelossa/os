#include "heap.h"

#include <stdint.h>

#include "memory.h"

// heap size is 100MB
#define HEAP_SIZE (100 * 1024 * 1024)
// page size is 4KB
#define HEAP_PAGE_SIZE 4096

#if HEAP_SIZE <= HEAP_PAGE_SIZE
#error "HEAP_SIZE must be larger than HEAP_PAGE_SIZE"
#endif
#if (HEAP_SIZE % HEAP_PAGE_SIZE) != 0
#error "HEAP_SIZE must be a multiple of HEAP_PAGE_SIZE"
#endif

// number of pages in heap
#define HEAP_PAGE_COUNT (HEAP_SIZE / HEAP_PAGE_SIZE)
// heap starts at 16MiB boundary
#define HEAP_START 0x1000000

typedef union {
    uint8_t byte;
    struct {
        uint8_t pad : 6;
        uint8_t last : 1;
        uint8_t first : 1;
    } s;
} page_descriptor;

page_descriptor heap[HEAP_PAGE_COUNT] = {0};

void heap_init() { memset(heap, 0, sizeof(heap)); }

void *heap_malloc(uint32_t size) {
    if (!size) {
        return 0;
    }

    uint32_t pages = (size + (HEAP_PAGE_SIZE - 1)) & ~(HEAP_PAGE_SIZE - 1);
    pages = pages / HEAP_PAGE_SIZE;

    for (uint32_t i = 0; i < HEAP_PAGE_COUNT; i++) {
        // Start of a previous allocation, run until we find the end,
        // then evaluate the following page.
        //
        // We are guaranteed to find a subsequent .last page prior to the end
        // of the heap.
        if (heap[i].s.first) {
            for (uint32_t j = i; j < HEAP_PAGE_COUNT; j++) {
                if (heap[j].s.last) {
                    i = j;
                    goto again;
                }
            }
        }

        // Found free starting page.. do we have a contiguous block?
        for (uint32_t j = i; j < HEAP_PAGE_COUNT; j++) {
            if (heap[j].byte != 0) {
                // we found a non-zero page which must be a .first page, start
                // our loop again pointing to this page to find the .last page.
                i = (j - 1);
                goto again;
            }
            // we have the number of pages we require, return the allocation.
            if ((j - i) == pages - 1) {
                heap[i].s.first = 1;
                heap[j].s.last = 1;
                return (void *)(HEAP_START + (i * HEAP_PAGE_SIZE));
            }
        }

    again:
        continue;
    }

    return 0;
};

void *heap_zalloc(uint32_t size) {
    void *p = heap_malloc(size);
    if (!p) {
        return p;
    }
    memset(p, 0, size);
    return p;
}

void heap_free(void *ptr) {
    uint32_t offset = ((uint32_t)ptr - HEAP_START) / HEAP_PAGE_SIZE;

    if (offset >= HEAP_PAGE_COUNT || !heap[offset].s.first) {
        // invalid pointer
        return;
    }

    uint32_t last = offset;
    for (; last < HEAP_PAGE_COUNT; last++) {
        if (heap[last].s.last == 1) {
            break;
        }
    }

    heap[offset].byte = 0;
    heap[last].byte = 0;
}
