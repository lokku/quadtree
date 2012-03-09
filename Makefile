# Copyright (C) 2011-2012 Lokku ltd. and contributors
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

CC=gcc
CFLAGS=-Wall -g -O0

all: check quadtree.o benchmark

prod:
	make clean
	make CFLAGS="-DNDEBUG -O3 -fPIC"
	gcc -shared -Wl,-soname,libquadtree.so.1 -o libquadtree.so.1.0.1 quadtree.o

prof:
	make clean
	make CFLAGS="-DNDEBUG -pg" LDFLAGS="-pg"

check: check.o quadtree.o

benchmark: benchmark.o quadtree.o

quadtree.o: quadtree.c quadtree.h quadtree_private.h

clean:
	rm *.o libquadtree.* check benchmark &>/dev/null || :

install: prod
	./install.sh
