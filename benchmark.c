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

#define _GNU_SOURCE

#include <stdlib.h>
#include <sys/types.h>
#include <assert.h>
#include <time.h>


#include "quadtree.h"
#include "quadtree_private.h"


#define M (K*K)
#define K 1000


inline FLOAT rnd() {
  return ((FLOAT)rand())/((FLOAT)RAND_MAX);
}

void populate(UFQuadTree *qt, u_int64_t n) {
  ITEM i;
  for (i=0; i<n; i++) {

    Item item;

    item.value     = i;

    item.coords[0] = rnd();
    item.coords[1] = rnd();

    qt_insert(qt, &item);
  }
}


void benchmark(const QuadTree *qt, int n, FLOAT radius, u_int64_t *total) {

  Quadrant region;

  int i;
  for (i=0; i<=n; i++) {


    u_int64_t maxn = 0;


    region.ne[X] = 1/2 + radius;
    region.ne[Y] = 1/2 + radius;
    
    region.sw[X] = 1/2 - radius;
    region.sw[Y] = 1/2 - radius;

    /* Query quadtree */
    Item **items = qt_query_ary_fast(qt, &region, &maxn);

    *total+= maxn;

    free(items);

    radius /= 2;

  }
}



int main(int argc, char **argv) {


  int n_points  = M/2;
  int n_splits  = 2;
  int n_tests   = M;
  int n_buckets = 200;

  FLOAT init_radius = 1.0/64.0;



  Quadrant quadrant;

  quadrant.ne[X] = 1;
  quadrant.ne[Y] = 1;
  quadrant.sw[X] = 0;
  quadrant.sw[Y] = 0;

  UFQuadTree *ufqt = qt_create_quadtree(&quadrant, n_buckets);

  populate(ufqt, n_points);

  const QuadTree *qt = qt_finalise(ufqt, "_benchmark.dat");
  qtuf_free(ufqt);
  qt_free((QuadTree *)qt);
  qt = qt_load("_benchmark.dat");


  clock_t start, end;
  double elapsed;

  u_int64_t total = 0;

  start = clock();
  int i;
  for (i=0; i<n_tests; i++) {
    benchmark(qt, n_splits, init_radius, &total);
  }
  end = clock();
  elapsed = ((double) (end - start)) / CLOCKS_PER_SEC;

  printf("elapsed: %lf; total: %ld\n", elapsed, total);

#ifndef NDEBUG
  printf("withins: %ld, nwithins: %ld\n", withins, nwithins);
#endif

  qt_free((QuadTree *)qt);

  return 0;
}
