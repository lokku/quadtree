
#include <stdio.h>
#include <stdlib.h>



int main(int argc, char **argv) {

  int size = atoi(argv[1]);

  int *arr = (int *)malloc(sizeof(int) * size * 2);

  if (arr == NULL)
    perror("malloc");

  int i,ix;

  int acc = 0;

  for (i=0; i<size; i++) {
    ix = i * 2;
    acc+= arr[ix+0];
    acc+= arr[ix+1];
  }

  printf("acc: %d\n", acc);

  return 0;
}
