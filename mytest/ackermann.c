/*
    calculate Ackermann(m, n)
*/

#include <riscv_syscall.h>
#include <stdio.h>

long long ackermann(long long m, long long n)
{
    if (m == 0)
        return n + 1;
    if (n == 0)
        return ackermann(m - 1, 1);
    return ackermann(m - 1, ackermann(m, n - 1));
}

int main()
{
    int m = read_i();
    int n = read_i();
    char c[15];
    sprintf(c, "m=%d n=%d", m, n);
    print_s(c);

    long long result = ackermann(m, n);
    sprintf(c, "result=%lld", result);
    print_s(c);
    sysexit(0);
}