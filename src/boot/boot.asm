; We start our x86 machines in 16 bit real mode
BITS 16

section .text.bootloader

loader_start:
jmp start;

; patched during compilation process, once we know the number of sectors our
; kernel image takes up, used in ata_32.asm.
kernel_sectors:
db 0x00

; setup global descriptor table, flat memory model.
gdt_start:
	dq 0x0		; mandatory null segment descriptor
gdt_code:
CODE_SEGMENT equ (gdt_code - gdt_start);
	dw 0xFFFF	; limit set to 4Gb
	dw 0x0		; base[0]
	db 0x0		; base[1]
	db 10011010b	; access byte: ring 0 access, exec bit
	db 11001111b	; flags|limit: flags(page granularity, 32bit segment)
	db 0x0		; base[2]
	; Data segment descriptor
gdt_data:
DATA_SEGMENT equ (gdt_data - gdt_start)
	dw 0xFFFF	; limit set to 4Gb
	dw 0x0		; base[0]
	db 0x0		; base[1]
	db 10010010b	; access byte: ring 0 access, data bit
	db 11001111b	; flags|limit: flags(page granularity, 32bit segment)
	db 0x0		; base[2]
gdt_end:

gdt_descriptor:
	dw (gdt_end - gdt_start) - 1
	dd gdt_start

global start ; using global here makes it available to debugger if an elf file
			 ; is created

start:
	cli		 ; clear interrupts
	; cs is set to 0x0 by bios.
	mov ax, 0x0
	mov cs, ax
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, 0x7C00

	sti		 ; enables interrupt

global protected_mode_switch
protected_mode_switch:
	cli					  ; we will let our kernel enable interrupts
	lgdt [gdt_descriptor] ; load the gdt
	mov  eax, cr0
	or   eax, 1	; set the lowest bit of cr0 to enable protected mode
	mov  cr0, eax

	; will set CS register for us
	jmp CODE_SEGMENT:load_kernel_32

; 32 bit mode from here on out...
[BITS 32]
load_kernel_32:
	; setup our registers and stack
	mov ax, DATA_SEGMENT
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	mov ebp, 0x00200000
	mov esp, ebp

	; set fast a20 gate
	in al, 0x92
	or al, 2
	out 0x92, al

	; if kernel_sectors is zero, just halt the system, our bootloader was never
	; patched with the number of kernel sectors to load.
	cmp byte [kernel_sectors], 0
	je $

	call ata_load_kernel

global jump_kernel
jump_kernel:
	; must far-jump (segment:address) for assembler to produce a jmp to an
	; absolute address.
	jmp CODE_SEGMENT:KERNEL_LOAD_ADDR

	jmp $

; include ATA routine for reading kernel sectors and loading to memory.
%include "src/boot/ata_32.asm"

; ensure our bootloader does not exceed 512 bytes
%define LOADER_SIZE ($ - loader_start)
%if LOADER_SIZE > 512
  %error "Boot loader exceeds 512 bytes"
%endif

; boot signature must be placed at offset 510 (the 511 byte of the sector).
; calculate size of current section ($ - $$) and subtract this from the total
; zero padding necessary to get to 511 byte.
times 510 - ($ - $$) db 0
; write the boot signature word
dw 0xAA55	; NASM will interpret this as little endian, so flip it, so when it
			; writes it to our image its in the correct order.
