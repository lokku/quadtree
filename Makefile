
CC=gcc
CFLAGS=-Wall -g -O0

all: check quadtree.o benchmark

prod:
	make clean
	make CFLAGS="-DNDEBUG -O3"

prof:
	make clean
	make CFLAGS="-DNDEBUG -pg" LDFLAGS="-pg"

check: check.o quadtree.o

benchmark: benchmark.o quadtree.o

quadtree.o: quadtree.c quadtree.h quadtree_private.h

clean:
	rm *.o check
