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
	.bss : ALIGN(16) {
		*(.bss);
	}

	/DISCARD/ : {
		/* Keep debug sections, discard everything else */
	}

	.debug_info     0 : { *(.debug_info) }
	.debug_abbrev   0 : { *(.debug_abbrev) }
	.debug_line     0 : { *(.debug_line) }
	.debug_str      0 : { *(.debug_str) }
	.debug_loc      0 : { *(.debug_loc) }
	.debug_ranges   0 : { *(.debug_ranges) }
}
