RVCC := riscv64-unknown-elf-gcc
RVISA := -Wa,-march=rv64i
RVAR := riscv64-unknown-elf-ar vr
RVOBJDUMP := riscv64-unknown-elf-objdump -Ss

tests = ackermann # matmul myqsort
all: $(tests)

# My tests
ackermann: ackermann.c
	$(RVCC) $(RVISA) -o ackermann ackermann.c -I.. -L.. -lrvecall
	$(RVOBJDUMP) ackermann > ackermann.obj

matmul: matmul.c
	$(RVCC) $(RVISA) -o matmul matmul.c -I.. -L.. -lrvecall
	$(RVOBJDUMP) matmul > matmul.obj

myqsort: myqsort.c
	$(RVCC) $(RVISA) -o myqsort myqsort.c -I.. -L.. -lrvecall
	$(RVOBJDUMP) myqsort > myqsort.obj

clean: 
	rm -f $(tests) *.obj