#ifndef RISCV_SYSCALL_H
#define RISCV_SYSCALL_H

void        print_i(long long x);
void        print_c(char c);
void        print_s(const char* c);
void        sysexit(int status);
long long   read_i();
char        read_c();

#endif // RISCV_SYSCALL_H