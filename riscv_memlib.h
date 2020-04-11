#ifndef RISCV_MEMLIB_H
#define RISCV_MEMLIB_H

#include "riscv_syscall.h"

void *malloc (size_t size);
void free (void *ptr);
void *realloc(void *ptr, size_t size);
void *calloc (size_t nmemb, size_t size);
int mm_init(void);

#endif // RISCV_MEMLIB_H



