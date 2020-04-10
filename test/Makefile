RVCC := riscv64-unknown-elf-gcc
RVISA := -Wa,-march=rv64i
RVAR := riscv64-unknown-elf-ar vr
RVOBJDUMP := riscv64-unknown-elf-objdump -s

tests := add mul-div n! qsort simple-function

all: $(tests)

# Tests
add: add.c
	$(RVCC) $(RVISA) -o add add.c
	$(RVOBJDUMP) add > add.obj

mul-div: mul-div.c
	$(RVCC) $(RVISA) -o mul-div mul-div.c
	$(RVOBJDUMP) mul-div > mul-div.obj

n!: n!.c
	$(RVCC) $(RVISA) -o n! n!.c
	$(RVOBJDUMP) n! > n!.obj

qsort: qsort.c
	$(RVCC) $(RVISA) -o qsort qsort.c
	$(RVOBJDUMP) qsort > qsort.obj

simple-function: simple-function.c
	$(RVCC) $(RVISA) -o simple-function simple-function.c
	$(RVOBJDUMP) simple-function > simple-function.obj

clean: 
	rm -f $(tests) *.obj