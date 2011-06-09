
#define _GNU_SOURCE

#include <stdlib.h>
#include <sys/types.h>


#include "quadtree.h"




typedef u_int64_t idt;





typedef struct {
  FLOAT ne[2];
  FLOAT sw[2];

  u_int64_t n;
  u_int64_t maxn;

  idt *ids;
} region;



_Bool in_region(region *r, FLOAT coords[2]) {
  return ((coords[X] >= r->sw[X]) && (coords[X] <= r->ne[X]) &&
          (coords[Y] >= r->sw[Y]) && (coords[Y] <= r->sw[Y]));

}

void populate(quadtree *qt, region *regions, short int nregions, idt n) {
  idt i;
  short int j;
  FLOAT coords[2];
  for (i=0; i<n; i++) {

    coords[0] = ((FLOAT)rand())/RAND_MAX;
    coords[1] = ((FLOAT)rand())/RAND_MAX;

    insert(qt, (ITEM *)i, coords);

    for (j=0; j<nregions; j++) {
      if (in_region(regions+j, coords)) {
        if ((regions[j].ids == NULL) || (regions[j].n+1 > regions[j].maxn)) {
          if (regions[j].maxn == 0)
            regions[j].maxn = 32;
          regions[j].maxn  *= 2;
          regions[j].ids = realloc(regions[j].ids, sizeof(idt)*regions[j].maxn);
        }
        regions[j].ids[regions[j].n++] = i;
      }
    }

  }
}



int main(int argc, char **argv) {

  FLOAT ne[2] = { 1, 1 };
  FLOAT sw[2] = { 0, 0 };

  quadtree *qt = create_quadtree(ne, sw, 10);

  short int nregions = 100;

  region *regions = malloc(sizeof(region) * nregions);

  populate(qt, regions, nregions, 99999);

  return 0;
}
