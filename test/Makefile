RVCC := riscv64-unknown-elf-gcc
RV64I := -Wa,-march=rv64i
RV64M := -Wa,-march=rv64im
RVAR := riscv64-unknown-elf-ar vr
RVOBJDUMP := riscv64-unknown-elf-objdump -Ss

tests_i := add mul-div n! qsort simple-function
tests_m := mul-div_m n!_m

all: $(tests_i) $(tests_m)
rv64i: $(tests_i)
rv64m: $(tests_m)

# Tests
add: add.c
	$(RVCC) $(RV64I) -o add add.c
	# $(RVOBJDUMP) add > add.obj

mul-div: mul-div.c
	$(RVCC) $(RV64I) -o mul-div mul-div.c
	# $(RVOBJDUMP) mul-div > mul-div.obj

mul-div_m: mul-div.c
	$(RVCC) $(RV64M) -o mul-div_m mul-div.c
	# $(RVOBJDUMP) mul-div_m > mul-div_m.obj

n!: n!.c
	$(RVCC) $(RV64I) -o n! n!.c
	# $(RVOBJDUMP) n! > n!.obj

n!_m: n!.c
	$(RVCC) $(RV64M) -o n!_m n!.c
	# $(RVOBJDUMP) n!_m > n!_m.obj

qsort: qsort.c
	$(RVCC) $(RV64I) -o qsort qsort.c
	# $(RVOBJDUMP) qsort > qsort.obj

simple-function: simple-function.c
	$(RVCC) $(RV64I) -o simple-function simple-function.c
	# $(RVOBJDUMP) simple-function > simple-function.obj

clean: 
	rm -f $(tests_i) $(tests_m) *.obj