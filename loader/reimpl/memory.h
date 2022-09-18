#ifndef __MEMORY_H
#define __MEMORY_H

#include <malloc.h>

#define ALIGN(x, a) (((x) + ((a)-1)) & ~((a)-1))

void init_memory();
void term_memory();

void free_hook(void *ptr);
void *memalign_hook(size_t alignment, size_t size);
void *malloc_hook(size_t size);
void *calloc_hook(size_t num, size_t size);
void *realloc_hook(void *ptr, size_t size);

#endif
