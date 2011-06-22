#ifndef QUADTREE_h
#define QUADTREE_h


#include <sys/types.h>


typedef struct QuadTree    QuadTree;
typedef struct Qt_Iterator Qt_Iterator;

typedef u_int64_t      ITEM;
typedef double         FLOAT;
typedef unsigned int   BUCKETSIZE;


#ifndef NDEBUG
unsigned long int withins;
unsigned long int nwithins;
#endif


typedef struct __attribute__ ((__packed__)) {
  FLOAT ne[2];
  FLOAT sw[2];
} Quadrant;



struct __attribute__ ((__packed__)) Item {

  ITEM  value;
  FLOAT coords[2];

};

typedef struct Item Item;





extern QuadTree *qt_create_quadtree(Quadrant *region, BUCKETSIZE maxfill);
extern void qt_free(QuadTree *quadtree);

/* qt_insert: item can be free()d immediately after qt_insert */
extern void qt_insert(QuadTree *quadtree, const Item *item);
extern void qt_finalise(QuadTree *quadtree, const char *file);



extern Item **qt_query_ary(const QuadTree *quadtree, const Quadrant *region, u_int64_t *maxn);
extern Item **qt_query_ary_fast(const QuadTree *quadtree, const Quadrant *region, u_int64_t *maxn);

extern Qt_Iterator *qt_query_itr(const QuadTree *quadtree, const Quadrant *region);
extern Item *qt_itr_next(Qt_Iterator *itr);

extern QuadTree *qt_load(const char *file);


typedef enum {
  X, Y, COORD
} coords;


typedef enum {
  NW, NE, SW, SE, QUAD
} quadindex;


#endif
