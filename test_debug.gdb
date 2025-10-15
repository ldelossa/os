# Test GDB configuration
# Connect first (QEMU reports x86-64 initially)
target remote localhost:1234
# Load symbols and force 32-bit interpretation
symbol-file ./src/kernel/kernel
set architecture i386

# Set breakpoint at heap_malloc
break heap_malloc

# Continue to the breakpoint
continue

# Show info when hit
info registers
info frame
print size
print/x $esp
x/4wx $esp

# Show disassembly
disassemble heap_malloc
