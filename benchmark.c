
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


void benchmark(QuadTree *qt, int n, FLOAT radius, u_int64_t *total) {

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

  QuadTree *qt = qt_finalise(ufqt, "_benchmark.dat");
  qtuf_free(ufqt);
  qt_free(qt);
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

  return 0;
}
