#ifndef __THREADPOOL_H_
#define __THREADPOOL_H_

#include <pthread.h>

typedef struct thread_pool thread_pool;

// Creates a thread pool and returns a pointer to it.
thread_pool *pool_init(int max_threads);

// Return maximum number of threads.
int pool_get_max_threads(thread_pool *);

// Insert task into thread pool. The pool will call work_routine and pass arg
// as an argument to it. This is a similar interface to pthread_create.
void pool_add_task(thread_pool *, void *(*work_routine)(void *), void *arg);

// Blocks until the thread pool is done executing its tasks.
void pool_wait(thread_pool *);

// Cleans up the thread pool, frees memory. Waits until work is done.
void pool_destroy(thread_pool *);

#endif

