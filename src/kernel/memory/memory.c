#include <stdint.h>

void *memset(void *ptr, uint8_t c, uint32_t size) {
    uint8_t *p = (uint8_t *)ptr;
    for (; p < ((uint8_t *)ptr + size); p++) {
        *p = c;
    }
    return ptr;
}
