#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <quadtree.h>

/* Stress test, using >4GiB of RAM, inserting a ton of items and querying a ton
 * of times. */

/* 50 million items */
int number_of_items = 50*1000*1000;

/* 50 million queries */
int number_of_queries = 50*1000*1000;

inline FLOAT rnd() {
    return ((FLOAT)rand())/((FLOAT)RAND_MAX);
}

#define K (1024)
#define M (1024*K)
#define G (1024*M)

int main(void) {
    /* Make sure we have enough memory */
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    long total_memory = pages * page_size;

    printf("Total memory: %1.2fGB\n", (double)total_memory/(double)G);

    /* It uses ~4GiB, but we'll skip unless there is 6GiB */
    if ((total_memory/G) < 6) {
        fprintf(stderr, "SKIP: Atleast 6GiB of RAM required for this stress test\n");
        return 0;
    }

    /* Set up quadrant */
    Quadrant quadrant;
    quadrant.ne[X] = 1;
    quadrant.ne[Y] = 1;
    quadrant.sw[X] = 0;
    quadrant.sw[Y] = 0;

    /* Create unfinalised quadtree with bucket size 200 */
    UFQuadTree *ufqt = qt_create_quadtree(&quadrant, 200);

    /* Populate with all items */
    int i;
    for (i=0; i<number_of_items; i++) {
        Item item;
        item.value = i;
        item.coords[0] = rnd();
        item.coords[1] = rnd();
        qt_insert(ufqt, &item);
        if (i%(1000*1000) == 0) {
            printf("Added %d million items.\n", i/(1000*1000) + 1);
        }
    }

    /* Finalise */
    const QuadTree *qt = qt_finalise(ufqt, NULL);

    /* Free the unfinalised tree */
    qtuf_free(ufqt);

    /* Keep a total of queried items */
    u_int64_t total = 0;

    /* Run all queries */
    FLOAT radius = 0.75;
    for (i=0; i<number_of_queries; i++) {
        /* Quadrant to query */
        Quadrant region;
        region.ne[X] = 1/2 + radius;
        region.ne[Y] = 1/2 + radius;
        region.sw[X] = 1/2 - radius;
        region.sw[Y] = 1/2 - radius;

        /* Return an unlimited number of items */
        u_int64_t maxn = 0;

        /* Query quadtree */
        Item **items = qt_query_ary_fast(qt, &region, &maxn);

        /* The number of items returned is put into maxn */
        total += maxn;
        /* Free memory */
        free(items);

        /* Change search radius */
        radius /= 2;

        if (i%(1000*1000) == 0) {
            printf("Queried %d million times\n", i/(1000*1000) + 1);
        }
    }

    printf("Queried a total of %lu items\n", (unsigned  long)total);

    /* Free the tree */
    qt_free((QuadTree *)qt);

    /* Error if less than 1/5 of items are returned */
    if (total < number_of_items*0.20) {
        fprintf(stderr, "Error: Less than 20%% of items returned\n");
        return 1;
    }

    /* Just make sure we don't crash */
    return 0;
}

