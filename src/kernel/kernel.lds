SECTIONS
{
	. = 1M;
	.text : ALIGN(16) {
		*(.text);
	}
	.rodata : ALIGN(16) {
		*(.rodata);
	}
	.data : ALIGN(16) {
		*(.data);
	}
	bss_start = .;
	.bss : ALIGN(16) {
		*(.bss);
	}
	bss_end = .;
}
