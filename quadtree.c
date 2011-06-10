

#define _GNU_SOURCE

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef NDEBUG
#include <malloc.h>
#endif


#include "quadtree.h"
#include "quadtree_private.h"














/* Expect this to be faster than a memcpy */
inline void _nullify_quadrants(TransNode **quadrants) {
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
    fprintf(stderr, "malloc: couldn't allocate %ld bytes", size);
    perror("malloc");
  }
  return ptr;
}


















_Bool in_quadrant(Item *i, Quadrant *q) {
  return ((i->coords[X] >= q->sw[X]) && (i->coords[X] <= q->ne[X]) &&
          (i->coords[Y] >= q->sw[Y]) && (i->coords[Y] <= q->ne[Y]));

}





QuadTree *create_quadtree(Quadrant *region, BUCKETSIZE maxfill) {

  QuadTree *qt = (QuadTree *)_malloc(sizeof(QuadTree));

  qt->region = *region;

  qt->size = 0;

  qt->mem.as_void = NULL;
  qt->divider     = NULL;

  qt->maxdepth = 0;
  qt->maxfill  = maxfill;

  qt->ninners  = 1;
  qt->nleafs   = 0;

  _init_root(qt);

  return qt;
}


void _init_root(QuadTree *qt) {
  TransNode *root = _malloc(sizeof(TransNode));

  root->is_inner = 1;

  _nullify_quadrants(root->quadrants);

  qt->root = (Node *)root;

  assert(qt->ninners == 1);
}



void insert(QuadTree *qt, ITEM value, FLOAT coords[2]) {

  if (qt->mem.as_void != NULL) {
    fprintf(stderr, "error: attempt to insert into the quadtree after finalisation\n");
    exit(1);
  }

  qt->size++;


  Item *item = _malloc(sizeof(Item));

  item->value     = value;
  item->coords[0] = coords[0];
  item->coords[1] = coords[1];

  Quadrant quadrant = qt->region;

  _insert(qt, (TransNode *)qt->root, item, &quadrant);

}


/* Note: *quadrant _is_ modified, but after _insert returns, it is no longer needed
 * (i.e., it's safe to use the address of an auto variable */
void _insert(QuadTree *qt, TransNode *node, Item *item, Quadrant *q) {

 RESTART:

  assert(in_quadrant(item, q));

  assert(q->ne[X] > q->sw[X]);
  assert(q->ne[Y] > q->sw[Y]);

  if (node->is_inner) {

    FLOAT div_x = q->sw[X] + (q->ne[X] - q->sw[X]) / 2;
    FLOAT div_y = q->sw[Y] + (q->ne[Y] - q->sw[Y]) / 2;

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
      goto RESTART;

    node->leaf.items[node->leaf.n++] = item;
  }
}




int _itemcmp(Item **aptr, Item **bptr) {

  Item *a = *aptr, *b = *bptr;

  /* The C99 standard defines '||' so that it returns 0 or 1, so we
     can't use the more natural expression
     'return wrtx || wrty'
     Also, don't be introducing any 'if's to this function.
  */


  int wrtx = _FLOATcmp(&a->coords[X], &b->coords[X]);
  int wrty = _FLOATcmp(&a->coords[Y], &b->coords[Y]);

  int v = wrtx + (wrty * !wrtx);

  assert(v == -1 || v == 0 || v == 1);

  assert((wrtx >  0 ) ? (v == 1   ) : 1);
  assert((wrtx == 0 ) ? (v == wrty) : 1);
  assert((wrtx <  0 ) ? (v == -1  ) : 1);

  return v;
}

inline int _FLOATcmp(FLOAT *a, FLOAT *b) {
  return (*a > *b) - (*a < *b);
}


inline int _count_distinct_items(TransNode *node) {
  /* This is pretty inefficient, since we only want to find if
     the number of distinct nodes is >1. However, I'm keeping this
     logic until there's a motivation to simplify it. */


  qsort(node->leaf.items, node->leaf.n, sizeof(Item *), (__compar_fn_t)_itemcmp);

  int r = node->leaf.n > 0;  /* 1 or 0 */

  BUCKETSIZE i;
  for (i=1; i<node->leaf.n; i++) {
    if (_itemcmp(&node->leaf.items[i-1], &node->leaf.items[i]))
      r++;
  }
  return r;
}


inline void _init_leaf_node(QuadTree *qt, TransNode *node) {
  node->is_inner = 0;
  node->leaf.size = qt->maxfill;
  node->leaf.items = _malloc(sizeof(Item *) * node->leaf.size);
  node->leaf.n = 0;
  qt->nleafs++;
}

inline void _ensure_child_quad(QuadTree *qt, TransNode *node, quadindex quad, Item *item) {

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

  if (node->leaf.n+1 >= node->leaf.size) {
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

  assert(!node->is_inner);

  int distinct = _count_distinct_items(node);

  if (distinct == 1) {

    /* Nothing we can do to further split the nodes */
    node->leaf.size *= 2;
    node->leaf.items = realloc(node->leaf.items, node->leaf.size*sizeof(Item *));

  } else {

    TransNode cpy = *node;

    node->is_inner = 1;

    qt->ninners++;
    qt->nleafs--;

    _nullify_quadrants(node->quadrants);

    Quadrant quadrant_;
    int i;
    for (i=0; i<cpy.leaf.n; i++) {
      quadrant_ = *quadrant;

      _insert(qt, node, cpy.leaf.items[i], &quadrant_);
    }

    free(cpy.leaf.items);
  }
}






void finalise(QuadTree *qt) {

  if (qt->mem.as_void != NULL) {
    fprintf(stderr, "error: quadtree already finalised");
    exit(1);
  }

  FinaliseState st;

  u_int64_t bytes = sizeof(Inner)*qt->ninners + sizeof(Leaf)*qt->nleafs + sizeof(Item)*qt->size;

  qt->mem.as_void = _malloc(bytes);

  qt->divider       = qt->mem.as_void + qt->ninners*sizeof(Inner);

  qt->items.as_void = qt->divider + sizeof(Leaf)*qt->nleafs;

  st.quadtree = qt;
  st.ninners  = 0;
  st.nleafs   = 0;
  st.nextleaf.as_void = qt->divider;
  st.cur              = qt->root;

  _finalise(&st);

}

void _finalise_inner(FinaliseState *st) {

  int i;
  TransNode *cur = st->cur;
  Inner *inner = st->quadtree->mem.as_inner + st->ninners++;

  for (i=0; i<4; i++) {
    st->cur = cur->quadrants[i];

    if (st->cur == NULL) {
      inner->quadrants[i] = 0;
    } else {
      inner->quadrants[i] = st->ninners;
      _finalise(st);
    }
  }

  free(cur);

#ifndef NDEBUG
  st->cur = NULL;
#endif
}

void _finalise_leaf(FinaliseState *st) {

  int i;
  TransNode *cur = st->cur;

  Leaf *leaf = st->nextleaf.as_leaf;

  leaf->n = cur->leaf.n;

  for (i=0; i<leaf->n; i++) {
    leaf->items[i] = *cur->leaf.items[i];

    free(cur->leaf.items[i]);
  }

  st->nextleaf.as_void += sizeof(Leaf) + sizeof(Item)*leaf->n;
  st->nleafs++;

  free(cur->leaf.items);
  free(cur);

#ifndef NDEBUG
  st->cur = NULL;
#endif
}




inline void _finalise(FinaliseState *st) {

  assert(st->cur != NULL);

  if (st->cur->is_inner) {
    _finalise_inner(st);
  } else {
    _finalise_leaf(st);
  }
}
