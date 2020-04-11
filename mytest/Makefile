RVCC := riscv64-unknown-elf-gcc
RV64I := -Wa,-march=rv64i
RV64M := -Wa,-march=rv64im
RVLIBLDFLAGS := -Wl,--gc-sections
RVOBJDUMP := riscv64-unknown-elf-objdump -Ss

tests_i = ackermann matmul # myqsort
tests_m = ackermann_m matmul_m # myqsort_m

all: $(tests_i) $(tests_m)
rv64i: $(tests_i)
rv64m: $(tests_m)

ackermann: ackermann.c ../librvsysi.a
	$(RVCC) $(RV64I) $(RVLIBLDFLAGS) -o ackermann ackermann.c -I.. -L.. -lrvsysi
	# $(RVOBJDUMP) ackermann > ackermann.obj

ackermann_m: ackermann.c  ../librvsysm.a
	$(RVCC) $(RV64M) $(RVLIBLDFLAGS) -o ackermann_m ackermann.c -I.. -L.. -lrvsysm
	# $(RVOBJDUMP) ackermann_m > ackermann.obj

matmul: matmul.c ../librvsysi.a
	$(RVCC) $(RV64I) $(RVLIBLDFLAGS) -o matmul matmul.c -I.. -L.. -lrvsysi
	# $(RVOBJDUMP) matmul > matmul.obj

matmul_m: matmul.c ../librvsysm.a
	$(RVCC) $(RV64M) $(RVLIBLDFLAGS) -o matmul_m matmul.c -I.. -L.. -lrvsysm
	# $(RVOBJDUMP) matmul_m > matmul.obj

myqsort: myqsort.c ../librvsysi.a
	$(RVCC) $(RV64I) $(RVLIBLDFLAGS) -o myqsort myqsort.c -I.. -L.. -lrvsysi
	# $(RVOBJDUMP) myqsort > myqsort.obj

myqsort_m: myqsort.c ../librvsysm.a
	$(RVCC) $(RV64M) $(RVLIBLDFLAGS) -o myqsort_m myqsort.c -I.. -L.. -lrvsysm
	# $(RVOBJDUMP) myqsort_m > myqsort.obj

clean: 
	rm -f $(tests_i) $(tests_m) *.obj