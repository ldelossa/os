#include <stdint.h>

void heap_init();

void *heap_malloc(uint32_t size);

void heap_free(void *ptr);
