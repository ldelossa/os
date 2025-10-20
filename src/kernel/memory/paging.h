#include <stdint.h>
#include <stdbool.h>

#define PAGE_SIZE_SHIFT 12
#define PAGE_SIZE (1 << PAGE_SIZE_SHIFT)  // 4096
#define PAGE_ALIGN_MASK (PAGE_SIZE - 1)   // 0xFFF
#define PAGING_PD_SIZE_SHIFT 10
#define PAGING_PD_SIZE (1 << PAGING_PD_SIZE_SHIFT)  // 1024
#define PAGING_PT_SIZE (1 << PAGING_PD_SIZE_SHIFT)  // 1024
#define PAGING_MAX_FRAME 0x100000                   // 1048576

#define PAGING_DTE_INDEX(frame) (frame >> PAGING_PD_SIZE_SHIFT)
#define PAGING_PT_INDEX(frame) (frame & (PAGING_PT_SIZE - 1))
#define PAGING_FRAME(addr) ((addr) >> PAGE_SIZE_SHIFT)

#define PAGING_PRESENT_F (1 << 0)
#define PAGING_RW_F (1 << 1)
#define PAGING_USER_F (1 << 2)
#define PAGING_WRITE_THROUGH_F (1 << 3)
#define PAGING_CACHE_DISABLE_F (1 << 4)
#define PAGING_ACCESSED_F (1 << 5)
#define PAGING_DIRTY_F (1 << 6)
#define PAGING_PAGE_SIZE_F (1 << 7)
#define PAGING_GLOBAL_F (1 << 8)

typedef union {
    uint32_t i;
    struct {
        uint32_t present : 1;
        uint32_t rw : 1;
        uint32_t user : 1;
        uint32_t write_through : 1;
        uint32_t cache_disable : 1;
        uint32_t accessed : 1;
        uint32_t reserved : 1;
        uint32_t page_size : 1;
        uint32_t available : 4;
        uint32_t frame : 20;
    } s;
} page_directory_entry;

typedef union {
    uint32_t i;
    struct {
        uint32_t present : 1;
        uint32_t rw : 1;
        uint32_t user : 1;
        uint32_t write_through : 1;
        uint32_t cache_disable : 1;
        uint32_t accessed : 1;
        uint32_t dirty : 1;
        uint32_t attribute_index : 1;
        uint32_t global : 1;
        uint32_t available : 3;
        uint32_t frame : 20;
    } s;
} page_table_entry;

// create a 1:1 identity mapping and returns a pointer to the start of a
// page directory.
//
// the identity mapping is created from base address 0x0 to size, where size
// must be a multiple of PAGE_SIZE (4096 by default).
page_directory_entry *paging_identity_map(uint32_t size, uint32_t flags);

// (re)map the linear address space of `size` to the physical address space.
//
// both linear_addr and physical_addr must be aligned to PAGE_SIZE (4096 by
// default), and size must be a multiple of PAGE_SIZE.
int8_t paging_remap(page_directory_entry *table, uint32_t linear_addr,
                    uint32_t physical_addr, uint32_t size, uint32_t flags,
                    bool invalidate);

// set the cr3 register to point to the given page directory table.
int paging_set_directory_table(page_directory_entry *table);

// enable paging by setting the relevant bit in cr0 register.
int paging_enable();
