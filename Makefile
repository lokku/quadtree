
CC=gcc
CFLAGS=-Wall -g

all: check

prod:
	make CFLAGS=-DNDEBUG

check: check.o quadtree.o


clean:
	rm *.o check
