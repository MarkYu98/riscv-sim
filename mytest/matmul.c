/*
	Usage: matrix_mul n t seed
	- `n` is the size of the matrix
	- `t` is the number of times to run
	- `seed` is the random seed
*/

#include <riscv_syscall.h>
#include <riscv_memlib.h>
#include <stdio.h>

void matrix_mul(int n, int A[][n], int B[][n], int C[][n])
{
	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++)
			C[i][j] = 0;
	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++)
			for (int k = 0; k < n; k++)
				C[i][j] += A[i][j] * B[j][k];
}

int main()
{
	int n = read_i();
	long long seed = read_i();
	char ch[15];
	sprintf(ch, "n=%d seed=%d\n", n, (int)seed);
	print_s(ch);

	int (*A)[n] = (int(*)[n])malloc(n * n * sizeof(int));
	int (*B)[n] = (int(*)[n])malloc(n * n * sizeof(int));
	int (*C)[n] = (int(*)[n])malloc(n * n * sizeof(int));
	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++) {
		seed = seed * 48271 % 2147483647;
		A[i][j] = seed % 10000;
		seed = seed * 48271 % 2147483647;
		B[i][j] = seed % 10000;
	}

	matrix_mul(n, A, B, C);

	return 0;
}