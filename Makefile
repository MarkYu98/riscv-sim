CXX := g++
CXXFlags := -I. -O3
targets := riscv-sim riscv-sim-pipe
srcs := riscv-sim.cpp riscv_proc.cpp
headers := riscv_config.hpp riscv_isa.hpp riscv_proc.hpp

RVCC := riscv64-unknown-elf-gcc
RVISA := -Wa,-march=rv64i
RVLIBCFLAGS := -ffunction-sections -fdata-sections
RVAR := riscv64-unknown-elf-ar vr
RVOBJDUMP := riscv64-unknown-elf-objdump 

SUBDIRS = $(shell find . * -type d | grep -v "\.")

.PHONY:all clean

all: $(targets) tests mytests

tests: 
	$(MAKE) -C test

mytests: librvsys.a
	$(MAKE) -C mytest

riscv-sim: $(srcs) $(headers)
	$(CXX) -o riscv-sim $(srcs) $(CXXFlags)

riscv-sim-pipe: $(srcs) $(headers)
	$(CXX) -o riscv-sim-pipe $(srcs) $(CXXFlags) -DPIPE

librvsys.a: riscv_syscall.o riscv_memlib.o
	$(RVAR) librvsys.a riscv_syscall.o riscv_memlib.o

riscv_memlib.o: riscv_memlib.c riscv_memlib.h
	$(RVCC) $(RVISA) $(RVLIBCFLAGS) -c riscv_memlib.c

riscv_syscall.o: riscv_syscall.c riscv_syscall.h
	$(RVCC) $(RVISA) $(RVLIBCFLAGS) -c riscv_syscall.c

clean:
	rm -f *.o *.a $(targets)
	make -C test clean
	make -C mytest clean