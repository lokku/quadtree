
CC=gcc
CFLAGS=-Wall -g -O0

all: check

prod:
	make CFLAGS=-DNDEBUG

check: check.o quadtree.o


quadtree.o: quadtree.c quadtree.h quadtree_private.h

clean:
	rm *.o check
