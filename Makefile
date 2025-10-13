MAKEFLAGS += --no-builtin-rules
CFLAGS=-ffreestanding -O0 -g -m32
LD_FLAGS=-m elf_i386 -g
NASM_FLAGS=-f elf32 -F dwarf -g
SRC_DIR=./src
BOOT_DIR=$(SRC_DIR)/boot
KERNEL_DIR=$(SRC_DIR)/kernel
KERNEL_SOURCES=$(shell find $(KERNEL_DIR) -type f -name '*.c')
KERNEL_OBJECTS=$(patsubst %.c,%.o,$(KERNEL_SOURCES))
BIN_DIR=./bin

$(BIN_DIR)/os-image.bin: $(BIN_DIR)/boot.bin $(BIN_DIR)/kernel.bin
	./scripts/build_image.sh

### boot loader ###
$(BIN_DIR)/boot.bin: $(BOOT_DIR)/boot
	x86_64-linux-gnu-objcopy -O binary $< $@

# link boot loader against lds file to set origin to 0x7c00
$(BOOT_DIR)/boot: $(BOOT_DIR)/boot.o
	x86_64-linux-gnu-ld $(LD_FLAGS) -T $(BOOT_DIR)/boot.lds -o $@ $<

# compile ELF from boot loader assembly
$(BOOT_DIR)/boot.o: $(BOOT_DIR)/boot.asm \
					$(BOOT_DIR)/boot.lds \
					$(BOOT_DIR)/ata_32.asm
	nasm $(NASM_FLAGS) -o $@ $<

### kernel ###
$(BIN_DIR)/kernel.bin: $(KERNEL_DIR)/kernel
	x86_64-linux-gnu-objcopy -O binary $< $@

# NOTE: kernel.asm.o must always be linked first as its our kernel bootstrap
# code.
$(KERNEL_DIR)/kernel: $(KERNEL_DIR)/kernel.asm.o $(KERNEL_OBJECTS)
	x86_64-linux-gnu-ld -m elf_i386 -g -T $(KERNEL_DIR)/kernel.lds -o $@ $^

$(KERNEL_DIR)/kernel.asm.o: $(KERNEL_DIR)/kernel.asm
	nasm $(NASM_FLAGS) -o $@ $<

# Recipe to build our C kernel files.
%.o: %.c
	x86_64-linux-gnu-gcc $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $$(find . -type f -name '*.o')
	rm -rf $(BOOT_DIR)/boot
	rm -rf $(KERNEL_DIR)/kernel
	rm -rf ./bin/*

.PHONY:
run:
	qemu-system-x86_64 -d cpu_reset,int -D qemu.log -nographic ./bin/os-image.bin

run-vga:
	qemu-system-x86_64 -d cpu_reset,int -D qemu.log -vnc :1 ./bin/os-image.bin

.PHONY:
run-debug:
	qemu-system-x86_64 -d cpu_reset,int -D qemu.log -nographic -s -S ./bin/os-image.bin

.PHONY:
run-debug-vga:
	qemu-system-x86_64 -d cpu_reset,int -D qemu.log -vnc :1 -s -S ./bin/os-image.bin

.PHONY:
debug:
	gdb -ex "target remote localhost:1234"

.PHONY:
# connect first, then add the symbol file, this avoids a bug where QEMU's gdb
# stub reports the machine as x86_64 and our local gdb is debugging a i386 file.
debug-boot:
	gdb -ex "target remote localhost:1234" -ex "add-symbol-file ./src/boot/boot" -ex "layout asm"

.PHONY:
debug-kernel:
	gdb -ex "target remote localhost:1234" -ex "add-symbol-file ./src/kernel/kernel"
