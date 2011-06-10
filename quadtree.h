


typedef struct QuadTree QuadTree;

typedef void          *ITEM;
typedef double         FLOAT;
typedef unsigned int   BUCKETSIZE;






typedef struct {
  FLOAT ne[2];
  FLOAT sw[2];
} Quadrant;





QuadTree *create_QuadTree(Quadrant *region, BUCKETSIZE maxfill);

void insert(QuadTree *qt, ITEM *item, FLOAT coords[2]);



typedef enum {
  X, Y
} coords;


typedef enum {
  NW, NE, SW, SE
} quadindex;

