; BUS 1 ports

; PIO read/write data register
B1_DATA equ 0x1F0
; R: error buffer, W: feature buffer
B1_ERR_FEAT equ 0x1F1
; configure sector count for r/w
B1_SEC_COUNT equ 0x1F2
; LBA low bits
B1_LBA_LOW equ 0x1F3
; LBA middle bits
B1_LBA_MID equ 0x1F4
; LBA high bits
B1_LBA_HI equ 0x1F5
; drive select/addressing/LBA extended bits register.
B1_DRIVE_SELECT equ 0x1F6
; alternative status port
B1_ALT_STATUS equ 0x3F6
; command port
B1_COMMAND equ 0x1F7

; Master drive with LBA mode selected
MASTER_DRIVE equ 0x40

; LBA address kernel begins at (second sector)
KERNEL_LBA equ 0x1
; Load address of kernel
KERNEL_LOAD_ADDR equ 0x100000

global ata_load_kernel
ata_load_kernel:
	; configure drive and LBA addressing mode
	mov dx, B1_DRIVE_SELECT
	mov al, MASTER_DRIVE
	out dx, al

	; delay ~400ns for drive select
	mov dx, B1_ALT_STATUS
	mov bl, 0x4
.delay:
	in al, dx
	dec bl
	jnz .delay

	; configure LBA of 0x1
	mov dx, B1_LBA_LOW
	mov al, KERNEL_LBA
	out dx, al

	mov dx, B1_LBA_MID
	mov al, 0x0
	out dx, al

	mov dx, B1_LBA_HI
	out dx, al

	; configure number of sectors to read
	mov dx, B1_SEC_COUNT
	mov al, [kernel_sectors]
	out dx, al

	; send read command
	mov dx, B1_COMMAND
	mov al, 0x20
	out dx, al

	mov bl, [kernel_sectors]
	mov edi, KERNEL_LOAD_ADDR	; insw load address
.read_loop
	mov dx, B1_ALT_STATUS
	in al, dx
	; check error bit
	test al, 0x1
	jnz abort
	; wait for BSY=0 and DRQ=1 (ready state)
	and al, 0x88
	cmp al, 0x08
	jne .read_loop

	mov dx, B1_DATA	; insw port
	mov cx, 256		; rep counter, (512 bytes / 2 byte words)
	rep insw		; start copy, will repeat until cx is 0

	dec bl			; Decrement sector count
	jnz .read_loop	; Loop if more sectors to read

	ret
global abort
abort:
	jmp $
