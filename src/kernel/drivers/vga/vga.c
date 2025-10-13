#include "vga.h"

#include <stdint.h>

#define VGA_MEMORY_COLOR 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

// Tracks next cell to write in.
struct {
    uint8_t x, y;
} cursor = {.x = 0, .y = 0};

int vga_clear(const vga_txt_char *const c) {
    volatile vga_txt_char *vga = (vga_txt_char *)VGA_MEMORY_COLOR;
    for (uint16_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga[i] = *c;
    }
    cursor.x = 0;
    cursor.y = 0;
    return 0;
}

int vga_set_cursor(uint8_t x, uint8_t y) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) {
        return -1;
    }
    cursor.x = x;
    cursor.y = y;
    return 0;
}

void vga_new_line() {
    cursor.x = 0;
    cursor.y++;
    if (cursor.y >= VGA_HEIGHT) {
        cursor.y = 0;
    }
}

void vga_increment_cursor(int x) {
    cursor.x += x;
    if (cursor.x >= VGA_WIDTH) {
        cursor.x = 0;
        cursor.y++;
    }

    if (cursor.y >= VGA_HEIGHT) {
        cursor.y = 0;
    }
}

int vga_write_char(const vga_txt_char *const c) {
    volatile vga_txt_char *vga = (vga_txt_char *)VGA_MEMORY_COLOR;

    // We are writing to our screen origin, this maybe a wrap around from a
    // previous write at the last cell, so clear our screen prior to writing.
    if (cursor.x == 0 && cursor.y == 0) {
        vga_clear(&(vga_txt_char){.code = ' ', .bg = VGA_DEFAULT_BG});
    }

    switch (c->code) {
        case '\n':
            vga_new_line();
            return 0;
        case '\t':
            vga_increment_cursor(4);
            return 0;
    }

    uint16_t pos = cursor.y * VGA_WIDTH + cursor.x;
    vga[pos] = *c;

    vga_increment_cursor(1);

    return 0;
}

int vga_write_str(const char *const str, const vga_txt_char *const c) {
    const char *p = str;

    vga_txt_char cc = {
        .bg = c->bg,
        .fg = c->fg,
    };

    while (*p != 0) {
        cc.code = *p;
        vga_write_char(&cc);
        p++;
    }

    return 0;
}
