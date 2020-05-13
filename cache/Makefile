CC=g++

srcs := cache.cc memory.cc
objs := cache.o memory.o
hdrs := cache.h memory.h storage.h
CCFlags := -I. -I../include -O3 -std=c++11 -lstdc++fs

.PHONY: all clean

all: cache-sim
obj: $(objs)

cache-sim: main.cc $(objs) 
	$(CC) -o $@ main.cc $(objs) $(CCFlags)

cache.o: cache.cc cache.h storage.h
	$(CC) -c cache.cc

memory.o: memory.cc memory.h storage.h
	$(CC) -c memory.cc

clean:
	rm -rf cache-sim $(objs)
