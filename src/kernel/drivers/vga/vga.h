#ifndef VGA_H
#define VGA_H

#include <stdint.h>

#define VGA_DEFAULT_BG 1

#define VGA_DEFAULT_CHAR &(vga_txt_char){.bg = VGA_DEFAULT_BG, .fg = 0xF}

typedef struct vga_txt_char_16 {
    uint8_t code;
    uint8_t fg : 4;
    uint8_t bg : 3;
    uint8_t blink : 1;
} __attribute__((packed)) vga_txt_char;

// Set the VGA cursor.
// Setting outside the bounds of the configured vga screen will return an error.
//
// Subsequent calls to vga_write_char will write to this position and increment
// the cursor position.
int vga_set_cursor(uint8_t x, uint8_t y);

// Clears the screen and sets the cursor to 0,0.
// The 'bg' field in the `c` parameter will be used to set the background color.
// All other fields are ignored.
int vga_clear(const vga_txt_char *const c);

// Writes a character to the next cursor position.
//
// If this write exceeds the bounds of the configured vga screen, the screen
// will be cleared first and then the character will be written at 0,0.
int vga_write_char(const vga_txt_char *const c);

// Writes a null terminated string to the display.
//
// The 'fg' and 'bg' fields in the `c` parameter will be used to set the
// foreground and background colors for all characters in the string.
// All other fields are ignored.
int vga_write_str(const char *const str, const vga_txt_char *const c);

#endif  // VGA_H
