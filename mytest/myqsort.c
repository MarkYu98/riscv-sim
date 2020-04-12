/*
	Quick sort with input and output
*/

#include <riscv_syscall.h>
#include <riscv_memlib.h>

void quick_sort(int arr[], int l, int r)
{
	int pivot = arr[(l + r) >> 1], i = l, j = r;
	while (i <= j) {
		while (i <= j && arr[i] < pivot) i++;
		while (i <= j && arr[j] > pivot) j--;
		if (i <= j) {
			int t = arr[i];
			arr[i] = arr[j];
			arr[j] = t;
			i++;
			j--;
		}
	}
	if (l < j) 
		quick_sort(arr, l, j);
	if (i < r) 
		quick_sort(arr, i, r);
}

int main()
{
	int n = read_i();
	int *a = (int *)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) 
        a[i] = read_i();
    quick_sort(a, 0, n - 1);
    for (int i = 0; i < n; i++) 
        print_i(a[i]);
	return 0;
}