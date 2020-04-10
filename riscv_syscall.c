#include "riscv_syscall.h"

void print_i(long long x)
{
    asm("li a7, 1");
    asm("ecall");
}

void print_c(char c)
{
    asm("li a7, 2");
    asm("ecall");
}

void print_s(const char* c)
{
    asm("li a7, 3");
    asm("ecall");
}

void sysexit(int status)
{
    asm("li a7, 93");
    asm("ecall");
}

long long read_i()
{
    asm("li a7, 4");
    asm("ecall");
}

char read_c()
{
    asm("li a7, 5");
    asm("ecall");
}