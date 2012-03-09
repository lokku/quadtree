/*
 * Copyright (C) 2011-2012 Lokku ltd. and contributers
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdio.h>
#include <stdlib.h>



int main(int argc, char **argv) {

  int size = atoi(argv[1]);

  int *arr = (int *)malloc(sizeof(int) * size * 2);

  if (arr == NULL)
    perror("malloc");

  int i,ix;

  int acc = 0;

  for (i=0; i<size; i++) {
    ix = i * 2;
    acc+= arr[ix+0];
    acc+= arr[ix+1];
  }

  printf("acc: %d\n", acc);

  return 0;
}
