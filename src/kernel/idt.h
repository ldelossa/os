#ifndef IDT_H
#define IDT_H

#include <stdint.h>

#define IDT_MAX_INTERRUPTS 256

// The structures which comprise the interrupt descriptor table.
// Each `idt_descriptor`'s location within the table reflects the interrupt
// number it handles.
typedef struct idt_descriptor {
    uint16_t offset_1;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attr;
    uint16_t offset_2;
} __attribute__((packed)) idt_descriptor;

// Descriptor which is written to the idtr register.
// Points to the base of the interrupt descriptor table.
typedef struct idtr_descriptor {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idtr_descriptor;


int idt_init();

#endif  // IDT_H
