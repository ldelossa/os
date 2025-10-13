#ifndef IO_H
#define IO_H

#include <stdint.h>

// Read a byte from the specified port.
uint8_t io_ins8(uint16_t port);

// Read a 16 bit value from the specified port.
uint16_t io_ins16(uint16_t port);

// Write a byte to the specified port.
void io_out8(uint16_t port, uint8_t value);

// Write a 16 bit value to the specified port.
void io_out16(uint16_t port, uint16_t value);

#endif // IO_H
