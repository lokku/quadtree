


typedef struct quadtree quadtree;

typedef void          *ITEM;
typedef double         FLOAT;
typedef unsigned int   BUCKETSIZE;






quadtree *create_quadtree(FLOAT ne[2], FLOAT sw[2], BUCKETSIZE maxfill);

void insert(quadtree *qt, ITEM *item, FLOAT coords[2]);





typedef enum {
  X, Y
} coords;


typedef enum {
  NW, NE, SW, SE
} quadrant;

