CXX := g++
CXXFlags := -I. -O3
targets := riscv-sim riscv-sim-pipe
srcs := riscv-sim.cpp riscv_proc.cpp
headers := riscv_config.hpp riscv_isa.hpp riscv_proc.hpp

RVCC := riscv64-unknown-elf-gcc
RVISA := -Wa,-march=rv64i
RVAR := riscv64-unknown-elf-ar vr
RVOBJDUMP := riscv64-unknown-elf-objdump 

SUBDIRS = $(shell find . * -type d | grep -v "\.")

.PHONY:all clean

all: $(targets) tests mytests

tests: 
	$(MAKE) -C test

mytests: librvecall.a
	$(MAKE) -C mytest

riscv-sim: $(srcs) $(headers)
	$(CXX) -o riscv-sim $(srcs) $(CXXFlags)

riscv-sim-pipe: $(srcs) $(headers)
	$(CXX) -o riscv-sim-pipe $(srcs) $(CXXFlags) -DPIPE

librvecall.a: riscv_syscall.o
	$(RVAR) librvecall.a riscv_syscall.o

riscv_syscall.o: riscv_syscall.c riscv_syscall.h
	$(RVCC) $(RVISA) -c riscv_syscall.c

clean:
	rm -f *.o *.a $(targets)
	make -C test clean
	make -C mytest clean