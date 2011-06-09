

#define _GNU_SOURCE

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef NDEBUG
#include <malloc.h>
#endif


#include "quadtree.h"




typedef void node;


struct _item;
struct _inner;
struct _leaf;
struct _transnode;

typedef struct _item      item;
typedef struct _transnode transnode;
typedef struct _inner     inner;
typedef struct _leaf      leaf;





struct _item {

  void *value;
  FLOAT coords[2];

} __packed__;




struct _inner {
  node *quadrants[4];
};

struct _leaf {
  void **items;
  unsigned long int n;
};

/* A transient node: soon it will be either an inner or leaf node */
struct _transnode {
  _Bool is_inner;
  union {
    transnode *quadrants[4];
    struct {
      item **items;
      unsigned int n;
      unsigned int size;

      /* Notes:
       * size:
           if non-0, indicates that this bucket is too big, and that
           the reason for this is that it contains identically-coordinated
           nodes that cannot be split into other buckets. The actual
           value indicates the size (in number of items, not bytes) of
           the memory malloc()ed for the bucket.
      */
    } leaf;
  };  /* "unnamed struct/union": http://gcc.gnu.org/onlinedocs/gcc/Unnamed-Fields.html */
};



struct quadtree {

  node *root;

  FLOAT ne[2];
  FLOAT sw[2];

  unsigned long int size;

  void *mem;

  /* divider: memory addresses to within *mem that are less than
              $divider are considered inner nodes, otherwise they're
              considered leaf nodes. */
  void *divider;

  int maxdepth;
  BUCKETSIZE maxfill;

  void *stack;

};








inline void _ensure_child_quad(quadtree *qt, transnode *node, quadrant quad, item *item, FLOAT *ne, FLOAT *sw);
inline void _ensure_bucket_size(quadtree *qt, transnode *node, const FLOAT *ne, const FLOAT *sw);
inline int _FLOATcmp(FLOAT a, FLOAT b);
inline int _count_distinct_nodes(transnode *node);

void _insert(quadtree *qt, transnode *node, item *item, FLOAT *ne, FLOAT *sw);
int  _itemcmp(item *a, item *b);
void _split_node(quadtree *qt, transnode *node, const FLOAT *ne, const FLOAT *sw);
void _init_root(quadtree *qt);
void _finalise(quadtree *qt);






/*
 * Coordinates:
 *
 *
 * N: 0X
 * S: 1X
 * E: X1
 * W: X0
 *
 * +---------+
 * | 00 | 01 |
 * +----+----+
 * | 10 | 11 |
 * +----+----+
 *
 *
 ***************/




#define NORTH(x) ((x) = (x) & NE)
#define SOUTH(x) ((x) = (x) | SW)
#define  EAST(x) ((x) = (x) | NE)
#define  WEST(x) ((x) = (x) & SW)


/* Expect this to be faster than a memcpy */
inline void _nullify_quadrants(transnode *quadrants[4]) {
  quadrants[NW] = (transnode *)NULL;
  quadrants[NE] = (transnode *)NULL;
  quadrants[SW] = (transnode *)NULL;
  quadrants[SE] = (transnode *)NULL;
}







/* AMD phenom has 64-byte cache line width.
 * sizeof(inner) = sizeof(void *)*4 = 32 bytes
 * We want to make sure that the inner structs are 64-byte
 * aligned.
 * By storing inner in depth-first traversal order, we can
 * expect cache-hits 1/4 of the time.
 */




inline void *_malloc(size_t size) {
  void *ptr = malloc(size);
  if (ptr == NULL) {
    char *str;
    asprintf(&str, "malloc: couldn't allocate %ld bytes", size);
    perror(str);
  }
  return ptr;
}



quadtree *create_quadtree(FLOAT ne[2], FLOAT sw[2], BUCKETSIZE maxfill) {

  quadtree *qt = (quadtree *)_malloc(sizeof(quadtree));

  qt->ne[X] = ne[X];
  qt->ne[Y] = ne[Y];
  qt->sw[X] = sw[X];
  qt->sw[Y] = sw[Y];

  qt->size = 0;

  qt->mem     = NULL;
  qt->divider = NULL;

  qt->maxdepth = 0;
  qt->maxfill  = maxfill;

  _init_root(qt);

  return qt;
}


void _init_root(quadtree *qt) {
  transnode *root = _malloc(sizeof(transnode));
  root->is_inner = 1;

  qt->root = (node *)root;
}



void insert(quadtree *qt, ITEM *value, FLOAT coords[2]) {

  if (qt->mem != NULL) {
    printf("error: attempt to insert into the quadtree after finalisation\n");
    exit(1);
  }




  item *item = _malloc(sizeof(item));

  item->value  = value;
  item->coords[0] = coords[0];
  item->coords[1] = coords[1];

  FLOAT ne[2] = { qt->ne[0], qt->ne[1] };
  FLOAT sw[2] = { qt->sw[0], qt->sw[1] };

  _insert(qt, (transnode *)qt->root, item, ne, sw);

}


void _insert(quadtree *qt, transnode *node, item *item, FLOAT *ne, FLOAT *sw) {

 INSERT:

  if (node->is_inner) {

    FLOAT div_x = (ne[X] - sw[X]) / 2;
    FLOAT div_y = (ne[Y] - sw[Y]) / 2;

    quadrant quad = 0;

    if (item->coords[X] >= div_x) {
      EAST(quad);
      sw[0] = div_x;
    } else {
      WEST(quad);
      ne[0] = div_x;
    }

    if (item->coords[Y] >= div_y) {
      NORTH(quad);
      sw[1] = div_y;
    } else {
      SOUTH(quad);
      ne[1] = div_y;
    }

    _ensure_child_quad(qt, node, quad, item, ne, sw);
    _insert(qt, node->quadrants[quad], item, ne, sw);

  } else {  /* $node is a leaf */

    _ensure_bucket_size(qt, node, ne, sw);

    if (node->is_inner)
      goto INSERT;

    node->leaf.items[node->leaf.n++] = item;
  }
}




int _itemcmp(item *a, item *b) {

  /* The C99 standard defines '||' so that it returns 0 or 1, so we
     can't use the more natural expression
     'return wrtx || wrty'
     Also, don't be introducing any 'if's to this function.
  */


  _Bool wrtx = _FLOATcmp(a->coords[X], b->coords[X]);
  _Bool wrty = _FLOATcmp(a->coords[Y], b->coords[Y]);

  _Bool v = wrtx + (wrty * !wrtx);

  assert(v == -1 || v == 0 || v == 1);

  assert((wrtx >  0 ) ? (v == 1   ) : 1);
  assert((wrtx == 0 ) ? (v == wrty) : 1);
  assert((wrtx <  0 ) ? (v == -1  ) : 1);

  return v;
}

inline int _FLOATcmp(FLOAT a, FLOAT b) {
  return (a > b) - (a < b);
}


inline int _count_distinct_nodes(transnode *node) {
  /* This is pretty inefficient, since we only want to find if
     the number of distinct nodes is >1. However, I'm keeping this
     logic until there's a motivation to simplify it. */


  qsort(*node->leaf.items, node->leaf.n, sizeof(item), (__compar_fn_t)_itemcmp);

  int r = 0;

  BUCKETSIZE i;
  for (i=1; i<node->leaf.n; i++) {
    if (_itemcmp(*node->leaf.items+i-1, *node->leaf.items+i))
      r++;
  }
  return r;
}


inline void _init_leaf_node(quadtree *qt, transnode *node) {
  node->is_inner = 0;
  node->leaf.items = _malloc(sizeof(item) * qt->maxfill);
  node->leaf.n = 0;
  node->leaf.size = 0;
}

inline void _ensure_child_quad(quadtree *qt, transnode *node, quadrant quad, item *item, FLOAT *ne, FLOAT *sw) {

  assert(node->is_inner);

  if (node->quadrants[quad] == NULL) {
    node->quadrants[quad] = (transnode *)_malloc(sizeof(transnode));
    _init_leaf_node(qt, node->quadrants[quad]);
  }
}

/* Ensures the node is suitably split so that it can accept another item */
/* Note that if the node is an inner node when passed into the funciton,
 * it may be a leaf node when the function terminates (i.e., the node's
 * type may change as a side effect of this function).
 */
inline void _ensure_bucket_size(quadtree *qt, transnode *node, const FLOAT *ne, const FLOAT *sw) {

  assert(!node->is_inner);

  if ( (node->leaf.n+1 >= qt->maxfill) && (node->leaf.n+1 >= node->leaf.size)) {
    _split_node(qt, node, ne, sw);
  }

#ifndef NDEBUG
  if (!node->is_inner) {
    assert(node->leaf.items != NULL);
    assert(malloc_usable_size(node->leaf.items) >= sizeof(*node->leaf.items)*node->leaf.n+1);
    assert(malloc_usable_size(node->leaf.items) >= sizeof(*node->leaf.items)*node->leaf.size);
  }
#endif

}


/* Maximum recursion depth: 64 assuming double-type coordinates */
void _split_node(quadtree *qt, transnode *node, const FLOAT *ne, const FLOAT *sw) {

  int distinct = _count_distinct_nodes(node);

  if (distinct == 1) {

    /* Nothing we can do to further split the nodes */
    node->leaf.size *= 2;
    node->leaf.items = realloc(node->leaf.items, node->leaf.size*sizeof(item));

  } else {

    transnode cpy = *node;

    node->is_inner = 1;
    _nullify_quadrants(node->quadrants);

    int i;

    FLOAT ne_[2] = { ne[0], ne[1] };
    FLOAT sw_[2] = { sw[0], sw[1] };

    for (i=0; i<cpy.leaf.n; i++) {
      _insert(qt, node, cpy.leaf.items[i], ne_, sw_);
    }
  }
}








void finalise(quadtree *qt) {



}
