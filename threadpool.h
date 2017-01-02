#ifndef __THREADPOOL_H_
#define __THREADPOOL_H_

#include <pthread.h>

typedef struct thread_pool thread_pool;

int pool_get_max_threads();
thread_pool *init_thread_pool();
void destroy_thread_pool(thread_pool *);
void queue_work_item(thread_pool *, void *(*work_routine)(void *), void *arg);

void debug(thread_pool *);

#endif

