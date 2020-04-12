/*
	Matrix multiplication
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
				C[i][j] += A[i][k] * B[k][j];
}

int main()
{
	int n;
	long long seed = 1600012998;
	char ch[15];

	int a[3][3] = { {1, 2, 3}, {4, 5, 6}, {7, 8, 0} };
	int b[3][3] = { {1, 2, 1}, {1, 1, 2}, {2, 1, 1} };
	int c[3][3] = {{0}};
	matrix_mul(3, a, b, c);		// { {9, 7, 28}, {21, 19, 20}, {15, 22, 23} };
	print_s("result: ");

	for (int i = 0; i < 3; i++) {
		char line[15];
		sprintf(line, "%d %d %d", c[i][0], c[i][1], c[i][2]);
		print_s(line);
	}

	n = read_i();
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