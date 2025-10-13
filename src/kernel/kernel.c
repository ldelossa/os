#include "drivers/vga/vga.h"
#include "drivers/pic/pic.h"
#include "idt.h"


void kernel_main() {
    vga_write_str("Initializing kernel...\n", VGA_DEFAULT_CHAR);
    vga_write_str("VGA initialized\n", VGA_DEFAULT_CHAR);

	if (!idt_init()){
		vga_write_str("Failed to initialize IDT\n", VGA_DEFAULT_CHAR);
		while (1);
	}
	vga_write_str("IDT initialized\n", VGA_DEFAULT_CHAR);


	pic_init();
	vga_write_str("PIC initialized\n", VGA_DEFAULT_CHAR);

    while (1) {
        // Spin forever
    }
}
