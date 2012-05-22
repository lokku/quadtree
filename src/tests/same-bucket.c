#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <quadtree.h>

/* Test 1,000,000 items in the same bucket */
int number_of_items=1*1000*1000;

int main(void) {
    /* Set up quadrant */
    Quadrant quadrant;
    quadrant.ne[X] = 1;
    quadrant.ne[Y] = 1;
    quadrant.sw[X] = 0;
    quadrant.sw[Y] = 0;

    /* Create unfinalised quadtree with bucket size 200 */
    UFQuadTree *ufqt = qt_create_quadtree(&quadrant, 200);

    /* Populate with all items in the same location */
    int i;
    for (i=0; i<number_of_items; i++) {
        Item item;
        item.value = i;
        item.coords[0] = 0.5;
        item.coords[1] = 0.5;
        qt_insert(ufqt, &item);
    }

    /* Finalise */
    const QuadTree *qt = qt_finalise(ufqt, NULL);

    /* Free the unfinalised tree */
    qtuf_free(ufqt);

    /* Keep a total of queried items */
    u_int64_t total = 0;

    /* Quadrant to query */
    Quadrant region;
    region.ne[X] = 0.6;
    region.ne[Y] = 0.6;
    region.sw[X] = 0;
    region.sw[Y] = 0;

    /* Return an unlimited number of items */
    u_int64_t maxn = 0;

    /* Query quadtree */
    Item **items = qt_query_ary_fast(qt, &region, &maxn);

    /* The number of items returned is put into maxn */
    total += maxn;

    /* Free memory */
    free(items);

    /* Free the tree */
    qt_free((QuadTree *)qt);

    /* We should get every single item back */
    if (total != (u_int64_t)number_of_items) {
        return 1;
    }

    return 0;
}

