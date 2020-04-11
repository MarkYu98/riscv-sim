#ifndef RISCV_SYSCALL_H
#define RISCV_SYSCALL_H

typedef unsigned long size_t;

void        print_i(long x);
void        print_c(char c);
void        print_s(const char* c);
void        sysexit(int status);
void *      mem_sbrk(size_t size);
void *      mem_heap_lo();
void *      mem_heap_hi();
long long   read_i();
char        read_c();

#endif // RISCV_SYSCALL_H