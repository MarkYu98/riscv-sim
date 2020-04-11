RVCC := riscv64-unknown-elf-gcc
RVISA := -Wa,-march=rv64i
RVLIBLDFLAGS := -Wl,--gc-sections
RVOBJDUMP := riscv64-unknown-elf-objdump -Ss

tests = ackermann # matmul myqsort
all: $(tests) 

# My tests
ackermann: ackermann.c 
	$(RVCC) $(RVISA) $(RVLIBLDFLAGS) -o ackermann ackermann.c -I.. -L.. -lrvsys
	$(RVOBJDUMP) ackermann > ackermann.obj

matmul: matmul.c
	$(RVCC) $(RVISA) $(RVLIBLDFLAGS) -o matmul matmul.c -I.. -L.. -lrvsys
	$(RVOBJDUMP) matmul > matmul.obj

myqsort: myqsort.c
	$(RVCC) $(RVISA) $(RVLIBLDFLAGS) -o myqsort myqsort.c -I.. -L.. -lrvsys
	$(RVOBJDUMP) myqsort > myqsort.obj

clean: 
	rm -f $(tests) *.obj