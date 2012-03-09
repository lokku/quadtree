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

#ifndef QUADTREE_h
#define QUADTREE_h


#include <sys/types.h>


typedef u_int64_t      ITEM;
typedef double         FLOAT;
typedef unsigned int   BUCKETSIZE;



/* UnFinalisedQuadTree */
typedef struct UFQuadTree UFQuadTree;


struct Quadrant {
  FLOAT ne[2];
  FLOAT sw[2];
} __attribute__ ((__packed__));

typedef struct Quadrant Quadrant;


struct QuadTree {

  const Quadrant region;

  const u_int64_t size;

  const u_int32_t maxdepth;
  const u_int32_t padding;

  const u_int64_t ninners;
  const u_int64_t nleafs;

} __attribute__ ((__packed__));

typedef struct QuadTree   QuadTree;

typedef struct Qt_Iterator Qt_Iterator;



#ifndef NDEBUG
unsigned long int withins;
unsigned long int nwithins;
#endif


struct Item {

  ITEM  value;
  FLOAT coords[2];

} __attribute__ ((__packed__));

typedef struct Item Item;





extern UFQuadTree *qt_create_quadtree(Quadrant *region, BUCKETSIZE maxfill);
extern void qt_free(QuadTree *quadtree);
extern void qtuf_free(UFQuadTree *quadtree);

/* qt_insert: item can be free()d immediately after qt_insert */
extern void qt_insert(UFQuadTree *quadtree, const Item *item);
extern const QuadTree *qt_finalise(const UFQuadTree *quadtree, const char *file);



extern Item **qt_query_ary(const QuadTree *quadtree, const Quadrant *region, u_int64_t *maxn);
extern Item **qt_query_ary_fast(const QuadTree *quadtree, const Quadrant *region, u_int64_t *maxn);

extern Qt_Iterator *qt_query_itr(const QuadTree *quadtree, const Quadrant *region);
extern Item *qt_itr_next(Qt_Iterator *itr);

extern const QuadTree *qt_load(const char *file);


typedef enum {
  X, Y, COORD
} coords;


typedef enum {
  NW, NE, SW, SE, QUAD
} quadindex;


#endif
