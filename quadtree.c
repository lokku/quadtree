

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

inline void *_realloc(void *ptr, size_t size) {
  ptr = realloc(ptr, size);
  if (ptr == NULL) {
    fprintf(stderr, "realloc: couldn't allocate %ld bytes", size);
    perror("realloc");
  }
  return ptr;
}



















_Bool in_quadrant(const Item *i, const Quadrant *q) {
  return ((i->coords[X] >= q->sw[X]) && (i->coords[X] <= q->ne[X]) &&
          (i->coords[Y] >= q->sw[Y]) && (i->coords[Y] <= q->ne[Y]));

}





QuadTree *qt_create_quadtree(Quadrant *region, BUCKETSIZE maxfill) {

  QuadTree *qt = (QuadTree *)_malloc(sizeof(QuadTree));

  qt->region = *region;

  qt->size = 0;

  qt->mem.as_void     = NULL;
  qt->divider.as_void = NULL;

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



void qt_insert(QuadTree *qt, Item item) {

  if (qt->mem.as_void != NULL) {
    fprintf(stderr, "error: attempt to insert into the quadtree after finalisation\n");
    exit(1);
  }

  qt->size++;


  Item *itmcpy = _malloc(sizeof(Item));

  *itmcpy = item;

  Quadrant quadrant = qt->region;

  _qt_insert(qt, (TransNode *)qt->root, itmcpy, &quadrant, 0);

}


/* Note: *quadrant _is_ modified, but after _insert returns, it is no longer needed
 * (i.e., it's safe to use the address of an auto variable */
void _qt_insert(QuadTree *qt, TransNode *node, Item *item, Quadrant *q, unsigned int depth) {

  if (++depth > qt->maxdepth)
    qt->maxdepth = depth;


 RESTART:

  assert(in_quadrant(item, q));

  assert(q->ne[X] > q->sw[X]);
  assert(q->ne[Y] > q->sw[Y]);

  if (node->is_inner) {

    quadindex quad = 0;

    FLOAT div_x, div_y;
    CALCDIVS(div_x, div_y, q);

    if (item->coords[X] >= div_x) {
      EAST(quad);
    } else {
      WEST(quad);
    }

    if (item->coords[Y] >= div_y) {
      NORTH(quad);
    } else {
      SOUTH(quad);
    }

    _target_quadrant(quad, q);
    _ensure_child_quad(qt, node, quad, item);
    _qt_insert(qt, node->quadrants[quad], item, q, depth);

  } else {  /* $node is a leaf */

    _ensure_bucket_size(qt, node, q, depth);

    if (node->is_inner)
      goto RESTART;

    node->leaf.items[node->leaf.n++] = item;
  }
}




int _itemcmp_direct(Item *a, Item *b) {
  return _itemcmp(&a, &b);
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
inline void _ensure_bucket_size(QuadTree *qt, TransNode *node, const Quadrant *quadrant, unsigned int depth) {

  assert(!node->is_inner);

  if (node->leaf.n+1 >= node->leaf.size) {
    _split_node(qt, node, quadrant, depth);
  }

#ifndef NDEBUG
  if (!node->is_inner) {
    assert(node->leaf.items != NULL);
    assert(malloc_usable_size(node->leaf.items) >= sizeof(*node->leaf.items)*node->leaf.n+1);
    assert(malloc_usable_size(node->leaf.items) >= sizeof(*node->leaf.items)*node->leaf.size);
  }
#endif

}


void _split_node(QuadTree *qt, TransNode *node, const Quadrant *quadrant, unsigned int depth) {

  assert(!node->is_inner);

  int distinct = _count_distinct_items(node);

  if (distinct == 1) {

    /* Nothing we can do to further split the nodes */
    node->leaf.size *= 2;
    node->leaf.items = _realloc(node->leaf.items, node->leaf.size*sizeof(Item *));

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

      _qt_insert(qt, node, cpy.leaf.items[i], &quadrant_, depth-1);
    }

    free(cpy.leaf.items);
  }
}






void qt_finalise(QuadTree *qt) {

  if (qt->mem.as_void != NULL) {
    fprintf(stderr, "error: quadtree already qt_finalised");
    exit(1);
  }

  FinaliseState st;

  u_int64_t bytes = sizeof(Inner)*qt->ninners + sizeof(Leaf)*qt->nleafs + sizeof(Item)*qt->size;

  qt->mem.as_void     = _malloc(bytes);

  qt->divider.as_void = (void *)&qt->mem.as_inner[qt->ninners];

  st.quadtree = qt;
  st.ninners  = 0;
  st.cur_trans        = qt->root;
  st.cur_node.as_void = qt->mem.as_void;
  st.next_leaf        = qt->divider.as_void;

  _qt_finalise(&st);

  qt->root = qt->mem.as_void;

}

/*
 *
 * pre-condition:
 *   * st->cur_node points to the memory address for the translated
 *     st->cur_trans node.
 *
 * Side-effects:
 *   * increments st->ninners
 *   * updates st->cur to child node
 *   * updates (st->quadtree->mem.as_inner+X)->quadrants[Y]
 */
void _qt_finalise_inner(FinaliseState *st) {

  int i;
  TransNode *const cur = st->cur_trans;
  Inner *const inner = st->cur_node.as_inner;

  st->ninners++;

  for (i=0; i<4; i++) {
    /* Set st->cur_trans to the child, ready for recursion */
    st->cur_trans = cur->quadrants[i];

    if (st->cur_trans == NULL) {
      inner->quadrants[i] = ROOT;
    } else {
      /* Note: st->ninners is the offset within mem of the _next_
       * node to be finalised (i.e., _not_ this one).
       */
      st->cur_node.as_void = st->cur_trans->is_inner ?
        (void *) &st->quadtree->mem.as_inner[st->ninners] :
        (void *) st->next_leaf;

      inner->quadrants[i] = st->cur_node.as_void - st->quadtree->mem.as_void;

      _qt_finalise(st);
    }
  }

  free(cur);

#ifndef NDEBUG
  st->cur_trans = NULL;
#endif
}

void _qt_finalise_leaf(FinaliseState *st) {

  assert(st->cur_node.as_void >= st->quadtree->divider.as_void);

  st->next_leaf+= sizeof(Leaf) + st->cur_trans->leaf.n*sizeof(Item);

  int i;
  TransNode *cur = st->cur_trans;

  Leaf *leaf = st->cur_node.as_leaf;

  leaf->n = cur->leaf.n;

  for (i=0; i<leaf->n; i++) {
    leaf->items[i] = *cur->leaf.items[i];

    free(cur->leaf.items[i]);
  }

  free(cur->leaf.items);
  free(cur);

#ifndef NDEBUG
  st->cur_trans = NULL;
#endif
}




inline void _qt_finalise(FinaliseState *st) {

  assert(st->cur_trans != NULL);

  if (st->cur_trans->is_inner) {
    _qt_finalise_inner(st);
  } else {
    _qt_finalise_leaf(st);
  }
}


inline Qt_Iterator *qt_query_itr(const QuadTree *qt, const Quadrant *region) {

  Qt_Iterator *itr = (Qt_Iterator *)_malloc(sizeof(Qt_Iterator));

  itr->quadtree = qt;
  itr->region = *region;

  itr->stack = _malloc(sizeof(*itr->stack) * qt->maxdepth);

  itr->so = 0;

  itr->stack[0].node.as_node = qt->root;
  itr->stack[0].region       = qt->region;
  itr->stack[0].quadrant     = 0;

  _itr_next_recursive(itr);

  return itr;
}


inline Item *qt_itr_next(Qt_Iterator *itr) {

 ENTER:

  /* Cunning use of '*' instead of '&&' to avoid a pipeline stall.
   * Note that no shortcutting is done, so the second operand to '*'
   * is always evaluated, regardless of the outcome of the first,
   * but this doesn't matter: we know that itr->sp points to malloc()ed
   * memory, so won't segfault even if the actual data is not really
   * a Leaf.
   */
  if (itr->lp == NULL) {
    return NULL;
  } else {
    while (itr->stack[(itr->so * !! itr->lp)].node.as_leaf->n -1 >= itr->cur_item) {

      Item *itm = &itr->lp->items[itr->cur_item++];
      if (in_quadrant(itm, &itr->region))
        return itm;
    }

    itr->so--;
    itr->stack[itr->so].quadrant++;
    _itr_next_recursive(itr);
    goto ENTER;

  }
}


void _itr_next_recursive(Qt_Iterator *itr) {

  assert(itr->so >= 0);

  if (IS_LEAF(itr->quadtree, itr->stack[itr->so].node.as_node)) {

    itr->cur_item = 0;
    itr->lp = itr->stack[itr->so].node.as_node;
    return;

    /* Done. (Success)
     *
     * post-conditions:
     *   * itr->stack[itr->so].quadrant     has _not_ been traversed yet.
     *   * itr->stack[itr->so -1].quadrant  has _not_ been traversed yet.
     *   * itr->stack[itr->so].curitem      has _not_ been returned yet.
     *   * itr->stack[itr->so   ]           is the currently-visited node
     */


  } else {

    Quadrant rgncpy;

    /*
     * invarient: itr->stack[itr->so].quadrant has _not_ been traversed yet.
     */

    while (itr->so >= 0) {
      assert(IS_INNER(itr->quadtree, itr->stack[itr->so].node.as_node));

      /* Loop through each quadrant */
      while (itr->stack[itr->so].quadrant != QUAD) {

        /* Skip empty/uninitialised quadrants */
        if (itr->stack[itr->so].node.as_inner->quadrants[itr->stack[itr->so].quadrant] == ROOT)
          goto CONTINUE;

        rgncpy = itr->stack[itr->so].region;

        _target_quadrant(itr->stack[itr->so].quadrant, &rgncpy);

        if (OVERLAP(itr->region, rgncpy))
          goto RECURSE;

      CONTINUE:

        /* Skip to the next quadrant */
        itr->stack[itr->so].quadrant++;

      }

      /* No quadrants on this node remaining --- backtrack one node */
      --itr->so;

      if (itr->so >= 0)
        itr->stack[itr->so].quadrant++;


    }

    assert(itr->so == -1);

    itr->lp = NULL;

    /* Failure.
     *
     * post-condition:
     *   itr->lp == NULL, and by virtue of this,
     *   the next call to qt_itr_next() will return NULL.
     */
    return;

  RECURSE:

    assert(itr->so >= 0);
    assert(OVERLAP(itr->stack[itr->so].region, itr->region));

    itr->so++;

    itr->stack[itr->so].quadrant = 0;
    itr->stack[itr->so].region   = rgncpy;

    itr->stack[itr->so].within_parent =
      CONTAINED(rgncpy, itr->region);


    itr->stack[itr->so].node.as_node = (Node *)
      (itr->quadtree->mem.as_void +
       itr->stack[itr->so-1].node.as_inner->quadrants[itr->stack[itr->so-1].quadrant]);


    /* Recurse.
     *
     * post-conditions:
     *   * itr->stack[itr->so].quadrant     has _not_ been traversed yet.
     *   * itr->stack[itr->so -1].quadrant  has _not_ been traversed yet.
     *   * itr->stack[itr->so   ]           is the currently-visited node
     */

    return _itr_next_recursive(itr);


  }
}





inline void _target_quadrant(quadindex q, Quadrant *region) {


  ASSERT_REGION_SANE(region);


  FLOAT div_x, div_y;
  CALCDIVS(div_x, div_y, region);


  if (ISSOUTH(q)) {
    region->ne[1] = div_y;
  } else {
    region->sw[1] = div_y;
  }

  if (ISEAST(q)) {
    region->sw[0] = div_x;
  } else {
    region->ne[0] = div_x;
  }


  ASSERT_REGION_SANE(region);


}

Item **qt_query_ary(const QuadTree *quadtree, const Quadrant *region, u_int64_t *maxn) {

  u_int64_t alloced = 32;
  Item **items = _malloc(sizeof(*items) * alloced);

  Qt_Iterator *itr = qt_query_itr(quadtree, region);

  u_int64_t i;
  for (i=0; (items[i] = qt_itr_next(itr)) != NULL; i++) {
    if (*maxn > 0 && i >= *maxn)
      break;
    if (i+1 >= alloced) {
      alloced*= 2;
      items = _realloc(items, sizeof(*items) * alloced);
    }
    /*    printf("item[%ld] = { value = %ld, x = %lf, y = %lf }\n",
           i, items[i]->value, items[i]->coords[0], items[i]->coords[1]);
           printf("itr->curitem: %ld\n", itr->cur_item);
    int j;
    for (j=0; j<itr->so; j++) printf(" %d", itr->stack[j].quadrant);
    printf("\n");
    */
  }

  /*  printf("\n\n\n-----------------\n\n\n"); */

  *maxn = i;

  free(itr);

  return items;
}



Item **qt_query_ary_fast(const QuadTree *quadtree, const Quadrant *region, u_int64_t *maxn) {

  u_int64_t alloced = 256;
  Item **items = _malloc(sizeof(*items) * alloced);

  Qt_Iterator *itr = qt_query_itr(quadtree, region);

  u_int64_t i=0;

  while (itr->lp != NULL) {

    if ((*maxn != 0) && (i >= *maxn)) break;

    _include_leaf(&items, &i, &alloced, itr->lp, &itr->region, itr->stack[itr->so].within_parent);

    itr->so--;
    itr->stack[itr->so].quadrant++;
    _itr_next_recursive(itr);

  }

  *maxn = i;

  free(itr);

  return items;

}












/*
 * Note: offset is the position at which we can start storing items
 * (i.e., *items+offset must not already contain an item).
 */
inline void _include_leaf(Item ***items, u_int64_t *offset, u_int64_t *size, Leaf *leaf, Quadrant *quadrant, _Bool within) {

  assert(leaf != NULL);

  u_int32_t i;

  /* Do _not_ put this inside the for() loop to try and save memory! */
  u_int64_t required = *offset + leaf->n;  /* _not_ +1 */

  if (required > *size) {
    *size = required * 2;
    *items = (Item **)realloc(*items, sizeof(Item *) * *size);
  }

  if (within) {

    /* It would be so cool to bypass the L2 cache right now, writing direct to memory */
    for (i=0; i<leaf->n; i++)
      (*items)[*offset + i] = &leaf->items[i];

  } else {

    u_int32_t j;
    for (i=0,j=0; j<leaf->n; j++) {
      /* Cleverly avoiding an IF :-) */
      (*items)[*offset + i] = &leaf->items[j];
      i += in_quadrant(&leaf->items[j], quadrant);
    }
  }

  *offset+= i;

}
