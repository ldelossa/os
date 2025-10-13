#include "idt.h"

#include "drivers/pic/pic.h"
#include "drivers/vga/vga.h"
#include "io/io.h"
#include "memory/memory.h"

// interrupt descriptor table
idt_descriptor idt[IDT_MAX_INTERRUPTS] __attribute__((aligned(8)));
idtr_descriptor idtr;

#define idt_handler(name, handler)       \
    __attribute__((naked)) void name() { \
        asm volatile("pusha");           \
        asm volatile("push %esp");       \
        asm volatile("call " #handler);  \
        asm volatile("add $4, %esp");    \
        asm volatile("popa");            \
        asm volatile("iret");            \
    }

void idt_no_interrupt() { io_out8(PIC_MASTER_CMD_PORT, 0x20); };
idt_handler(idt_handler_no_interrupt, idt_no_interrupt);

void halt() {
    vga_write_str("System halted.\n", VGA_DEFAULT_CHAR);
    while (1) {
    };
}

void idt_div_by_zero(uint32_t *stack) {
    vga_write_str("Division by zero exception\n", VGA_DEFAULT_CHAR);
    // Stack pointer points to the following stack layout of the interrupt
    // handler assembly wrapper.
    //
    // EFLAGS (4 bytes) -------------------
    // CS (4 bytes)		| Interrupt stack |
    // EIP (4 bytes)	|_________________|
    // EAX (4 bytes)	-------------------
    // ECX (4 bytes)	|                 |
    // EDX (4 bytes)	|                 |
    // EBX (4 bytes)	|                 |
    // ESP (4 bytes)	|     pusha       |
    // EBP (4 bytes)	|                 |
    // ESI (4 bytes)	|                 |
    // EDI (4 bytes)	|_________________| <-- *stack
    //
    // therefore to get to EIP we must walk up the stack 8 times, scaling by
    // 32bits
    *(stack + 8) = (uint32_t)halt;
}
idt_handler(idt_handler_div_by_zero, idt_div_by_zero);

void idt_int21_keyboard(uint32_t *stack) {
    vga_write_str("Keyboard interrupt received\n", VGA_DEFAULT_CHAR);
    io_out8(PIC_MASTER_CMD_PORT, 0x20);
}
idt_handler(idt_handler_int21_keyboard, idt_int21_keyboard);

// set the interrupt handler address for the given interrupt number.
int idt_set(uint16_t interrupt_num, void *address) {
    if (interrupt_num >= IDT_MAX_INTERRUPTS) {
        return 0;
    }
    idt_descriptor *d = &idt[interrupt_num];
    d->offset_1 = (uint32_t)address & 0xFFFF;
    d->selector = 0x08;
    d->type_attr = 0xEE;
    d->offset_2 = ((uint32_t)address >> 16) & 0xFFFF;
    return 0;
}

void idt_set_idtr() {
    idtr.limit = (sizeof(idt_descriptor) * IDT_MAX_INTERRUPTS) - 1;
    idtr.base = (uint32_t)&idt;
    asm volatile("lidt %0" : : "m"(idtr));
}

int idt_init() {
    memset(idt, 0, sizeof(idt));
    memset(&idtr, 0, sizeof(idtr_descriptor));

    for (uint16_t i = 0; i < IDT_MAX_INTERRUPTS; i++) {
        idt_set(i, idt_handler_no_interrupt);
    }

    idt_set(0, idt_handler_div_by_zero);
    idt_set(0x21, idt_handler_int21_keyboard);

    idt_set_idtr();

    return 1;
}
