; kernel runs in 32 bit mode
[BITS 32]

CODE_SEGMENT equ 0x08
DATA_SEGMENT equ 0x10

extern kernel_main
global _start
_start:
	; configure environment for C
	mov ax, DATA_SEGMENT
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	mov ebp, 0x200000
	mov esp, ebp
	call kernel_main         ; call the C main function

	jmp $
