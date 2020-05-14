CXX := g++
CXXFlags := -I. -Iinclude -O3 -std=c++11 -lstdc++fs
targets := riscv-sim riscv-sim-pipe
srcs := riscv-sim.cpp riscv_proc.cpp
hdrs := riscv_config.hpp riscv_isa.hpp riscv_proc.hpp
cache_objs := cache/cache.o cache/memory.o

libsrcs := riscv_memlib.c riscv_syscall.c 
libhdrs := riscv_memlib.h riscv_syscall.h
libobjs := riscv_memlib.o riscv_syscall.o 

RVCC := riscv64-unknown-elf-gcc
RV64I := -Wa,-march=rv64i
RV64M := -Wa,-march=rv64im
RVLIBCFLAGS := -ffunction-sections -fdata-sections
RVAR := riscv64-unknown-elf-ar vr
RVOBJDUMP := riscv64-unknown-elf-objdump 

SUBDIRS = $(shell find . * -type d | grep -v "\.")

.PHONY: all clean rv64i rv64m

all: $(targets) rv64i rv64m _cache
rv64i: $(targets) tests_i mytests_i
rv64m: $(targets) tests_m mytests_m

_cache: 
	$(MAKE) -C cache cache-sim
cache/cache.o: cache/cache.cc cache/cache.h cache/storage.h
	$(MAKE) -C cache cache.o
cache/memory.o: cache/memory.cc cache/memory.h cache/storage.h
	$(MAKE) -C cache memory.o

tests: 
	$(MAKE) -C test
tests_i:
	$(MAKE) -C test rv64i
tests_m:
	$(MAKE) -C test rv64m

mytests: librvsysi.a librvsysm.a
	$(MAKE) -C mytest
mytests_i: librvsysi.a
	$(MAKE) -C mytest rv64i
mytests_m: librvsysm.a
	$(MAKE) -C mytest rv64m

riscv-sim: $(srcs) $(hdrs) $(cache_objs)
	$(CXX) -o riscv-sim $(srcs) $(cache_objs) $(CXXFlags)

riscv-sim-pipe: $(srcs) $(hdrs) $(cache_objs)
	$(CXX) -o riscv-sim-pipe $(srcs) $(cache_objs) $(CXXFlags) -DPIPE

librvsysi.a: $(libsrcs) $(libhdrs)
	$(RVCC) $(RV64I) $(RVLIBCFLAGS) -c $(libsrcs) 
	$(RVAR) librvsysi.a $(libobjs)

librvsysm.a: $(libsrcs) $(libhdrs)
	$(RVCC) $(RV64M) $(RVLIBCFLAGS) -c $(libsrcs) 
	$(RVAR) librvsysm.a $(libobjs)

clean:
	rm -f *.o *.a *.gch $(targets)
	make -C test clean
	make -C mytest clean
	make -C cache clean