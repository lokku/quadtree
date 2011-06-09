
#include <stdio.h>
#include <stdlib.h>



int main(int argc, char **argv) {

  int size = atoi(argv[1]);

  int *as = (int *)malloc(sizeof(int) * size);
  int *bs = (int *)malloc(sizeof(int) * size);

  if (as == NULL || bs == NULL)
    perror("malloc");

  int i, ix;

  int acc = 0;

  for (i=0; i<size; i++) {
    ix = i * 2;
    acc+= as[i];
    acc+= bs[i];
  }

  printf("acc: %d\n", acc);

  return 0;
}
