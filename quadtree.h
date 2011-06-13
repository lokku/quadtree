#ifndef QUADTREE_h
#define QUADTREE_h


#include <sys/types.h>


typedef struct QuadTree    QuadTree;
typedef struct QT_Iterator QT_Iterator;

typedef u_int64_t      ITEM;
typedef double         FLOAT;
typedef unsigned int   BUCKETSIZE;






typedef struct {
  FLOAT ne[2];
  FLOAT sw[2];
} Quadrant;





extern QuadTree *qt_create_quadtree(Quadrant *region, BUCKETSIZE maxfill);

extern void qt_insert(QuadTree *quadtree, ITEM item, FLOAT coords[2]);
extern void qt_finalise(QuadTree *qt);



extern ITEM *qt_query_ary(QuadTree *quadtree, Quadrant *region, u_int64_t maxn);

extern QT_Iterator *qt_query_itr(QuadTree *quadtree, Quadrant *region);

extern inline ITEM *qt_itr_next(QT_Iterator *itr);


typedef enum {
  X, Y, COORD
} coords;


typedef enum {
  NW, NE, SW, SE, QUAD
} quadindex;


#endif
