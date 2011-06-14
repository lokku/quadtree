#ifndef QUADTREE_h
#define QUADTREE_h


#include <sys/types.h>


typedef struct QuadTree    QuadTree;
typedef struct Qt_Iterator Qt_Iterator;

typedef u_int64_t      ITEM;
typedef double         FLOAT;
typedef unsigned int   BUCKETSIZE;






typedef struct {
  FLOAT ne[2];
  FLOAT sw[2];
} Quadrant;



struct Item {

  ITEM  value;
  FLOAT coords[2];

} __packed__;

typedef struct Item Item;





extern QuadTree *qt_create_quadtree(Quadrant *region, BUCKETSIZE maxfill);

extern void qt_insert(QuadTree *quadtree, Item item);
extern void qt_finalise(QuadTree *quadtree);



extern Item **qt_query_ary(const QuadTree *quadtree, const Quadrant *region, u_int64_t *maxn);

extern Qt_Iterator *qt_query_itr(const QuadTree *quadtree, const Quadrant *region);

extern inline Item *qt_itr_next(Qt_Iterator *itr);


typedef enum {
  X, Y, COORD
} coords;


typedef enum {
  NW, NE, SW, SE, QUAD
} quadindex;


#endif
