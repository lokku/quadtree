
#define _GNU_SOURCE

#include <stdlib.h>
#include <sys/types.h>
#include <assert.h>


#include "quadtree.h"
#include "quadtree_private.h"







typedef struct {
  FLOAT ne[2];
  FLOAT sw[2];

  u_int64_t n;
  u_int64_t maxn;

  ITEM *ids;
} region;



inline FLOAT rnd() {
  return ((FLOAT)rand())/((FLOAT)RAND_MAX);
}

_Bool in_region(region *r, FLOAT coords[2]) {
  return ((coords[X] >= r->sw[X]) && (coords[X] <= r->ne[X]) &&
          (coords[Y] >= r->sw[Y]) && (coords[Y] <= r->ne[Y]));

}

void populate(QuadTree *qt, region *regions, short int nregions, ITEM n) {
  ITEM i;
  short int j;
  FLOAT coords[2];
  for (i=0; i<n; i++) {

    coords[0] = rnd();
    coords[1] = rnd();

    insert(qt, i, coords);

    for (j=0; j<nregions; j++) {
      if (in_region(regions+j, coords)) {
        if ((regions[j].ids == NULL) || (regions[j].n+1 > regions[j].maxn)) {
          if (regions[j].maxn == 0)
            regions[j].maxn = 32;
          regions[j].maxn  *= 2;
          regions[j].ids = realloc(regions[j].ids, sizeof(ITEM)*regions[j].maxn);
        }
        regions[j].ids[regions[j].n++] = i;
      }
    }

  }
}

void populate_regions(QuadTree *qt, region *regions, short int nregions) {
  int i,j;
  for (i=0; i<nregions; i++) {

    FLOAT rands[4];
    for (j=0; j<4; j++)
      rands[j] = rnd();

    qsort(rands,   4, sizeof(FLOAT), (__compar_fn_t)_FLOATcmp);
    /*    qsort(rands+2, 2, sizeof(FLOAT), (__compar_fn_t)_FLOATcmp);*/

    regions[i].ne[0] = rands[1];
    regions[i].sw[0] = rands[0];

    regions[i].ne[1] = rands[3];
    regions[i].sw[1] = rands[2];

    regions[i].ids   = NULL;
    regions[i].n     = 0;
    regions[i].maxn  = 0;


    assert(regions[i].ne[0] >= regions[i].sw[0]);
    assert(regions[i].ne[1] >= regions[i].sw[1]);
  }
}



int main(int argc, char **argv) {

  Quadrant quadrant;

  quadrant.ne[X] = 1;
  quadrant.ne[Y] = 1;
  quadrant.sw[X] = 0;
  quadrant.sw[Y] = 0;

  QuadTree *qt = create_quadtree(&quadrant, 10);

  short int nregions = 100;

  region *regions = malloc(sizeof(region) * nregions);

  populate_regions(qt, regions, nregions);

  populate(qt, regions, nregions, 999999);

  finalise(qt);

  return 0;
}
