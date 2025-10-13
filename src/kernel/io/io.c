#include "io.h"

uint8_t io_ins8(uint16_t port) {
    uint8_t byte;

	asm volatile("inb %1, %0" : "=a"(byte) : "d"(port));

    return byte;
}

uint16_t io_ins16(uint16_t port) {
	uint16_t word;

	asm volatile("inw %1, %0" : "=a"(word) : "d"(port));

	return word;
}

void io_out8(uint16_t port, uint8_t value) {
	asm volatile("outb %1, %0" : : "d"(port), "a"(value));
}

void io_out16(uint16_t port, uint16_t value) {
	asm volatile("outw %1, %0" : : "d"(port), "a"(value));
}
