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
; Loads our kernel using ATA READ SECTOR EXT command.
; EXT reads allow for 16 bit sector counts and 48 bit LBA addresses.
;
; persisent registers
; %EBP - 32-bit total sectors to load
; %ESI - LBA48 address to start read from (limited to 32 bits).
; %EDI - 32-bit memory address to load kernel too, incremented by rep insw
;
; temporary registers
; %EDX/DX - used primarily for in/out ports	 / scratch register
; %EAX/AL - used primarily for in/out values / scratch register
; %EBX/BL - used for delay loop counter / per-read-command sector count
ata_load_kernel:
	mov ebp, [kernel_sectors]
	mov esi, KERNEL_LBA
	mov edi, KERNEL_LOAD_ADDR

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

.read_loop_full:
	; READ SECTORS EXT writes 48-bit LBA most significant 3 bytes first, into
	; LBA registers, followed by least significant 3 bytes.
	;
	; We only support 32-bit LBA (2 TiB) so the 2 most significant bytes will
	; be 0.
	mov eax, esi
	shr eax, 24

	; 24:31
	mov dx, B1_LBA_LOW
	out dx, al

	; 32:39
	xor eax, eax
	mov dx, B1_LBA_MID
	out dx, al

	; 40:47
	mov dx, B1_LBA_HI
	out dx, al

	; now least significant 3 bytes.
	mov eax, esi

	; 0:7
	mov dx, B1_LBA_LOW
	out dx, al

	; 8:15
	mov dx, B1_LBA_MID
	shr eax, 8
	out dx, al

	; 16:23
	mov dx, B1_LBA_HI
	shr eax, 8
	out dx, al

	; READ SECTOR EXT can handle a 16-bit sector count (65536 full sectors, where
	; 0 is provided to read 65536 sectors)
	;
	; If provided kernel sectors < 65536, we write the kernel sectors directly
	; to the sector count register.
	;
	; If sectors >= 65536, we write 0 to the sector count register to read
	; the maximum 65536 sectors, performing multiple read commands as needed.
	mov eax, ebp
	cmp eax, 65536
	jge .prepare_sectors_max

	mov dx, B1_SEC_COUNT
	; high byte
	shr eax, 8
	out dx, al
	; low byte
	mov eax, ebp
	out dx, al

	; ebx will be decremented per read loop
	mov ebx, ebp;

	jmp .read_op

; sector count is >= 65536, we can specify zeros to read the maximum segment
; count.
.prepare_sectors_max:
	; specify 0 to both low and high sector count registers to read
	; the maximum 65536 sectors
	mov dx, B1_SEC_COUNT
	xor al, al
	; high byte
	out dx, al
	; low byte
	out dx, al

	; ebx will be decremented per read loop
	mov ebx, 65536

.read_op:
	; send read command
	mov dx, B1_COMMAND
	mov al, 0x24
	out dx, al

.read_loop:
	; confirm status is OK.
	mov dx, B1_ALT_STATUS
	in al, dx
	; check error bit
	test al, 0x1
	jnz abort
	; wait for BSY=0 and DRQ=1 (ready state)
	and al, 0x88
	cmp al, 0x08
	jne .read_loop

	; start the copy
	mov dx, B1_DATA	; insw port
	mov cx, 256		; rep counter, (512 bytes / 2 byte words)
	rep insw		; start copy, will repeat until cx is 0, increments edi
					; (kernel load address) each time.

	inc esi			; Increment LBA
	dec ebp			; Decrement total sector count
	dec ebx			; Decrement per-read sector count
	jnz .read_loop	; Loop if more sectors to read in this command

	cmp ebp, 0
	; full loop, we need to issue a new command for more sectors.
	jnz .read_loop_full

	ret
global abort
abort:
	jmp $
