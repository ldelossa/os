#include "drivers/pic/pic.h"
#include "drivers/vga/vga.h"
#include "idt.h"
#include "memory/heap.h"
#include "memory/paging.h"

// bss_start points to the first byte of the bss section.
extern uint8_t bss_start;
// bss_end points to the byte one past the end of the bss section.
extern uint8_t bss_end;

void zero_bss() {
    uint8_t *start = &bss_start;
    while (start < &bss_end) {
        *start = 0;
        start++;
    }
}

void kernel_main() {
    vga_write_str("Initializing kernel...\n", VGA_DEFAULT_CHAR);

    zero_bss();
    vga_write_str("BSS section zeroed\n", VGA_DEFAULT_CHAR);

    if (!idt_init()) {
        vga_write_str("Failed to initialize IDT\n", VGA_DEFAULT_CHAR);
        while (1);
    }
    vga_write_str("IDT initialized\n", VGA_DEFAULT_CHAR);

    pic_init();
    vga_write_str("PIC initialized\n", VGA_DEFAULT_CHAR);

    heap_init();
    vga_write_str("Heap initialized\n", VGA_DEFAULT_CHAR);

    void *table =
        paging_identity_map(0x08000000, PAGING_PRESENT_F | PAGING_RW_F);
    paging_set_directory_table(table);
    paging_enable();
    vga_write_str("Paging enabled\n", VGA_DEFAULT_CHAR);

    while (1) {
        // Spin forever
    }
}
