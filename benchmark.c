
#define _GNU_SOURCE

#include <stdlib.h>
#include <sys/types.h>
#include <assert.h>
#include <time.h>


#include "quadtree.h"
#include "quadtree_private.h"


#define M 1000000
#define K 1000


inline FLOAT rnd() {
  return ((FLOAT)rand())/((FLOAT)RAND_MAX);
}

void populate(QuadTree *qt, u_int64_t n) {
  ITEM i;
  for (i=0; i<n; i++) {

    Item item;

    item.value     = i;

    item.coords[0] = rnd();
    item.coords[1] = rnd();

    qt_insert(qt, item);
  }
}


void benchmark(QuadTree *qt, int n, FLOAT radius) {

  Quadrant region;

  int i;
  for (i=0; i<n; i++) {


    u_int64_t maxn = 0;


    region.ne[X] = 1/2 + radius;
    region.ne[Y] = 1/2 + radius;
    
    region.sw[X] = 1/2 - radius;
    region.sw[Y] = 1/2 - radius;

    /* Query quadtree */
    Item **items = qt_query_ary(qt, &region, &maxn);

    free(items);

    radius /= 2;

  }
}



int main(int argc, char **argv) {


  int n_points = 4*M;
  int n_splits = 5;
  int n_tests  = 1*K;

  FLOAT init_radius = 1/32;



  Quadrant quadrant;

  quadrant.ne[X] = 1;
  quadrant.ne[Y] = 1;
  quadrant.sw[X] = 0;
  quadrant.sw[Y] = 0;

  QuadTree *qt = qt_create_quadtree(&quadrant, 10);

  populate(qt, n_points);

  qt_finalise(qt);


  clock_t start, end;
  double elapsed;

  start = clock();
  int i;
  for (i=0; i<n_tests; i++) {
    benchmark(qt, n_splits, init_radius);
  }
  end = clock();
  elapsed = ((double) (end - start)) / CLOCKS_PER_SEC;

  printf("elapsed: %lf\n", elapsed);

  return 0;
}