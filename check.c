/*
 * Copyright (C) 2011-2012 Lokku ltd. and contributors
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


#include "quadtree.h"
#include "quadtree_private.h"







typedef struct {

  Quadrant region;

  u_int64_t n;
  u_int64_t maxn;

  Item *items;
} region;



inline FLOAT rnd() {
  return ((FLOAT)rand())/((FLOAT)RAND_MAX);
}

void populate(UFQuadTree *qt, region *regions, short int nregions, ITEM n) {
  ITEM i;
  short int j;
  for (i=0; i<n; i++) {

    Item item;

    item.value     = i;

    item.coords[0] = rnd();
    item.coords[1] = rnd();

    qt_insert(qt, &item);

    for (j=0; j<nregions; j++) {
      if (in_quadrant(&item, &regions[j].region)) {

        /* Grow/alloc memory if needed */
        if ((regions[j].items == NULL) || (regions[j].n+1 > regions[j].maxn)) {
          if (regions[j].maxn == 0)
            regions[j].maxn = 32;
          regions[j].maxn  *= 2;
          regions[j].items = realloc(regions[j].items, sizeof(Item)*regions[j].maxn);
        }

        /* Add item to region */
        regions[j].items[regions[j].n++] = item;
      }
    }
  }


  /* Sort the regions by their coordinates so as to be able to compare them
   * to the quadtree output when querying later */
  for (j=0; j<nregions; j++) {
    qsort(regions[j].items, regions[j].n, sizeof(Item), (__compar_fn_t)_itemcmp_direct);
  }


}

void free_regions(region *regions, short int nregions) {
  short int i;
  for (i=0; i<nregions; i++)
    free(regions[i].items);
  free(regions);
}

void populate_regions(UFQuadTree *qt, region *regions, short int nregions) {
  int i,j;
  for (i=0; i<nregions; i++) {

    FLOAT rands[4];
    for (j=0; j<4; j++)
      rands[j] = rnd();

    qsort(rands,   2, sizeof(FLOAT), (__compar_fn_t)_FLOATcmp);
    qsort(rands+2, 2, sizeof(FLOAT), (__compar_fn_t)_FLOATcmp);

    regions[i].region.ne[0] = rands[1];
    regions[i].region.sw[0] = rands[0];

    regions[i].region.ne[1] = rands[3];
    regions[i].region.sw[1] = rands[2];

    regions[i].items = NULL;
    regions[i].n     = 0;
    regions[i].maxn  = 0;


    assert(regions[i].region.ne[0] >= regions[i].region.sw[0]);
    assert(regions[i].region.ne[1] >= regions[i].region.sw[1]);
  }
}

u_int64_t check(const QuadTree *qt, const region *regions, short int nregions) {

  u_int64_t errors = 0;

  short int i;
  for (i=0; i<nregions; i++) {

    /*
      printf("region: sw[X], sw[Y], ne[X], ne[Y] = %lf, %lf, %lf, %lf\n\n",
      regions[i].region.sw[X], regions[i].region.sw[Y], regions[i].region.ne[X], regions[i].region.ne[Y]);
    */

    u_int64_t maxn = 0;

    /* Query quadtree */
    Item **items = qt_query_ary_fast(qt, &regions[i].region, &maxn);

    /* Sort results so comparable with those in regions[i] */
    qsort(items, maxn, sizeof(Item *), (__compar_fn_t)_itemcmp);

    /* Check the number of records returned */
    if (maxn != regions[i].n) {
      errors+= regions[i].n;
      printf("error: got %ld records, expected %ld\n", maxn, regions[i].n);

      int j;
      u_int64_t ncorrect=0;
      for (j=0; j<maxn; j++) {
        if (!in_quadrant(items[j], &regions[i].region)) {
          printf("error: { value = %ld, x = %lf, y = %lf }\n",
               items[j]->value, items[j]->coords[0], items[j]->coords[1]);
        } else {
          ncorrect++;
        }
      }

      printf("ncorrect: %ld\n\n\n\n", ncorrect);



      goto CONTINUE;
    }


    /* Compare results with expected results */
    int j;
    for (j=0; j<maxn; j++) {
      if (0 != _itemcmp_direct(items[j], &regions[i].items[j])) {
        errors++;
      } else {
        /*        printf("correct: { value = %ld, x = %lf, y = %lf }\n",
                  items[j]->value, items[j]->coords[0], items[j]->coords[1]); */
      }
    }

  CONTINUE:
    free(items);

  }

  return errors;

}



int main(int argc, char **argv) {

  Quadrant quadrant;
  u_int64_t errors = 0;

  quadrant.ne[X] = 1;
  quadrant.ne[Y] = 1;
  quadrant.sw[X] = 0;
  quadrant.sw[Y] = 0;

  UFQuadTree *ufqt = qt_create_quadtree(&quadrant, 1);

  short int nregions = 100;

  region *regions = malloc(sizeof(region) * nregions);

  populate_regions(ufqt, regions, nregions);

  populate(ufqt, regions, nregions, 99999);

  const QuadTree *qt = qt_finalise(ufqt, "_check.dat");
  qtuf_free(ufqt);

  errors += check(qt, regions, nregions);

  printf("%ld errors\n", errors);
  printf("qt_free()ing and qt_load()ing...\n");

  qt_free((QuadTree *)qt);

  qt  = qt_load("_check.dat");
  errors += check(qt, regions, nregions);


  qt_free((QuadTree *)qt);
  free_regions(regions, nregions);

#ifndef NDEBUG
  printf("withins: %ld, nwithins: %ld\n", withins, nwithins);
#endif

  if (errors) {
    printf("errors: %ld\n", errors);
    exit(!!errors);
  }

  return 0;
}
