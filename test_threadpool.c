#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "threadpool.h"

void *work_routine(void *arg) {
  int arg_v = *(int *) arg;
  printf("Working on %d\n", arg_v);
  sleep(2);
  printf("Work done (%d).\n", arg_v);
  return NULL;
}

int main() {
  thread_pool *pool = pool_init(4);
  printf("Testing threadpool of %d threads.\n", pool_get_max_threads(pool));

  for (int i = 1; i <= 8; i++) {
    int *arg = malloc(sizeof(int));
    *arg = i;
    pool_add_task(pool, work_routine, (void *)arg);
  }

  srand(time(NULL));
  for (int i = 1; i <= 10; i++) {
    sleep(1);
    if (rand() % 2 == 0) {
      int *arg = malloc(sizeof(int));
      *arg = 1000 + i;
      pool_add_task(pool, work_routine, (void *)arg);
    }
  }

  pool_wait(pool);

  pool_destroy(pool);

  printf("Done.");
}
