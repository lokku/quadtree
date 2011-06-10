#ifndef QUADTREE_h
#define QUADTREE_h


#include <sys/types.h>


typedef struct QuadTree QuadTree;

typedef u_int64_t      ITEM;
typedef double         FLOAT;
typedef unsigned int   BUCKETSIZE;






typedef struct {
  FLOAT ne[2];
  FLOAT sw[2];
} Quadrant;





QuadTree *create_quadtree(Quadrant *region, BUCKETSIZE maxfill);

void insert(QuadTree *qt, ITEM item, FLOAT coords[2]);
void finalise(QuadTree *qt);


typedef enum {
  X, Y
} coords;


typedef enum {
  NW, NE, SW, SE
} quadindex;


#endif
