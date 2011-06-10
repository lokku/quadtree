

#define _GNU_SOURCE

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef NDEBUG
#include <malloc.h>
#endif


#include "quadtree.h"




typedef void Node;


struct _item;
struct _inner;
struct _leaf;
struct _TransNode;

typedef struct _item      item;
typedef struct _TransNode TransNode;
typedef struct _inner     inner;
typedef struct _leaf      leaf;





struct _item {

  void *value;
  FLOAT coords[2];

} __packed__;




struct _inner {
  Node *quadrants[4];
};

struct _leaf {
  void **items;
  unsigned long int n;
};

/* A transient node: soon it will be either an inner or leaf node */
struct _TransNode {
  _Bool is_inner;
  union {
    TransNode *quadrants[4];
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




struct QuadTree {

  Node *root;

  Quadrant region;

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





inline void _ensure_child_quad(QuadTree *qt, TransNode *node, quadindex quad, item *item);
inline void _ensure_bucket_size(QuadTree *qt, TransNode *node, const Quadrant *quadrant);
inline int _FLOATcmp(FLOAT a, FLOAT b);
inline int _count_distinct_nodes(TransNode *node);

void _insert(QuadTree *qt, TransNode *node, item *item, Quadrant *quadrant);
int  _itemcmp(item *a, item *b);
void _split_node(QuadTree *qt, TransNode *node, const Quadrant *quadrant);
void _init_root(QuadTree *qt);
void _finalise(QuadTree *qt);






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




/* I have a phobia of floating point arithmatic when conversion to
 * exact integers is required. Could use stdlib's pow() function,
 * but can we be bothered to _prove_ there will never be errors due
 * to conversion from integers to floats, and then back again?
 * For the same reason, I'm using divll instead of '/'.
 *
 * pow4(x): compute 4^x
 */

inline u_64int_t pow4(u_64int_t x) {
  return (1<<x) * (1<<x);
}

inline u_64int_t child(int level, int offset) {
  u_64int_t nodes_above, nodes_left;

  nodes_above = divll(pow4(level+1)-1, 3).quot;
  nodes_left  = 4*offset;

  return nodes_above + nodes_left - 1;
}






/* Expect this to be faster than a memcpy */
inline void _nullify_quadrants(TransNode *quadrants[4]) {
  quadrants[NW] = (TransNode *)NULL;
  quadrants[NE] = (TransNode *)NULL;
  quadrants[SW] = (TransNode *)NULL;
  quadrants[SE] = (TransNode *)NULL;
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



QuadTree *create_quadtree(Quadrant *region, BUCKETSIZE maxfill) {

  QuadTree *qt = (QuadTree *)_malloc(sizeof(QuadTree));

  qt->region = *region;

  qt->size = 0;

  qt->mem     = NULL;
  qt->divider = NULL;

  qt->maxdepth = 0;
  qt->maxfill  = maxfill;

  _init_root(qt);

  return qt;
}


void _init_root(QuadTree *qt) {
  TransNode *root = _malloc(sizeof(TransNode));
  root->is_inner = 1;

  qt->root = (Node *)root;
}



void insert(QuadTree *qt, ITEM *value, FLOAT coords[2]) {

  if (qt->mem != NULL) {
    printf("error: attempt to insert into the quadtree after finalisation\n");
    exit(1);
  }




  item *item = _malloc(sizeof(item));

  item->value  = value;
  item->coords[0] = coords[0];
  item->coords[1] = coords[1];

  Quadrant quadrant = qt->region;

  _insert(qt, (TransNode *)qt->root, item, &quadrant);

}


/* Note: *quadrant _is_ modified, but after _insert returns, it is no longer needed
 * (i.e., it's safe to use an auto variable */
void _insert(QuadTree *qt, TransNode *node, item *item, Quadrant *q) {

 INSERT:

  if (node->is_inner) {

    FLOAT div_x = (q->ne[X] - q->sw[X]) / 2;
    FLOAT div_y = (q->ne[Y] - q->sw[Y]) / 2;

    quadindex quad = 0;

    if (item->coords[X] >= div_x) {
      EAST(quad);
      q->sw[0] = div_x;
    } else {
      WEST(quad);
      q->ne[0] = div_x;
    }

    if (item->coords[Y] >= div_y) {
      NORTH(quad);
      q->sw[1] = div_y;
    } else {
      SOUTH(quad);
      q->ne[1] = div_y;
    }

    _ensure_child_quad(qt, node, quad, item);
    _insert(qt, node->quadrants[quad], item, q);

  } else {  /* $node is a leaf */

    _ensure_bucket_size(qt, node, q);

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


inline int _count_distinct_nodes(TransNode *node) {
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


inline void _init_leaf_node(QuadTree *qt, TransNode *node) {
  node->is_inner = 0;
  node->leaf.items = _malloc(sizeof(item) * qt->maxfill);
  node->leaf.n = 0;
  node->leaf.size = 0;
}

inline void _ensure_child_quad(QuadTree *qt, TransNode *node, quadindex quad, item *item) {

  assert(node->is_inner);

  if (node->quadrants[quad] == NULL) {
    node->quadrants[quad] = (TransNode *)_malloc(sizeof(TransNode));
    _init_leaf_node(qt, node->quadrants[quad]);
  }
}

/* Ensures the node is suitably split so that it can accept another item */
/* Note that if the node is an inner node when passed into the funciton,
 * it may be a leaf node when the function terminates (i.e., the node's
 * type may change as a side effect of this function).
 */
inline void _ensure_bucket_size(QuadTree *qt, TransNode *node, const Quadrant *quadrant) {

  assert(!node->is_inner);

  if ( (node->leaf.n+1 >= qt->maxfill) && (node->leaf.n+1 >= node->leaf.size)) {
    _split_node(qt, node, quadrant);
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
void _split_node(QuadTree *qt, TransNode *node, const Quadrant *quadrant) {

  int distinct = _count_distinct_nodes(node);

  if (distinct == 1) {

    /* Nothing we can do to further split the nodes */
    node->leaf.size *= 2;
    node->leaf.items = realloc(node->leaf.items, node->leaf.size*sizeof(item));

  } else {

    TransNode cpy = *node;

    node->is_inner = 1;
    _nullify_quadrants(node->quadrants);

    int i;

    Quadrant quadrant_ = *quadrant;

    for (i=0; i<cpy.leaf.n; i++) {
      _insert(qt, node, cpy.leaf.items[i], &quadrant_);
    }
  }
}








void finalise(QuadTree *qt) {



}
