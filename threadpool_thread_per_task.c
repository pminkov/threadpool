/*
This is a version which creates a new thread for each task that needs to be executed.
There are at most N threads running at the same time.

This is supposedly slow, because for small tasks we'll have the overhead of creating
one thread per task, but it won't hurt to verify this  claim.
*/

#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "threadpool.h"
#include "threads.h"

//#define DEBUG

#ifdef DEBUG
  #define DEB printf
#else
  #define DEB(...)
#endif

struct task_data {
  void *(*work_routine)(void *);
  void *arg;
};

struct thread_pool {
  // An array that holds the tasks.
  struct task_data* task_buffer;

  // How many tasks do we have that are still not executing.
  int queued_count;

  // How many tasks do we have that are executing. This doesn't become
  // bigger than max threads.
  int working_on;

  // How many worker threads can we have.
  int max_threads;
  
  // The thread that looks for work and starts tasks.
  pthread_t dispatcher_thread;
  pthread_mutex_t mutex;

  // A condition that's signaled on when we go from a state of no work
  // to a state of work available.
  pthread_cond_t work_available;

  // A condition that's signaled on when we got from a state of a full
  // work queue to having one slot available.
  pthread_cond_t work_capacity_available;
};


const int TASK_BUFFER_MAX = 1000;

int pool_get_max_threads(struct thread_pool *pool) {
  return pool->max_threads;
}


struct task_thread_args {
  struct thread_pool* pool;
  struct task_data td;
};


// This is the function that each worker thread executes.
void* task_worker_thread(void *args_param) {
  struct task_thread_args *args = (struct task_thread_args *) args_param;

  args->td.work_routine(args->td.arg);

  Pthread_mutex_lock(&args->pool->mutex);
  args->pool->working_on--;
  Pthread_cond_signal(&args->pool->work_capacity_available);
  Pthread_mutex_unlock(&args->pool->mutex);

  free(args_param);

  return NULL;
}

void launch_work_task(struct thread_pool *pool, struct task_data td) {
  pool->working_on++;

  pthread_t thread;

  struct task_thread_args *args = malloc(sizeof(struct task_thread_args));
  args->pool = pool;
  args->td = td;

  Pthread_create(&thread, NULL, task_worker_thread, (void *)args);
}

void *dispatcher_thread_func(void *pool_arg) {
  struct thread_pool *pool = (struct thread_pool *)pool_arg;

  while (1) {
    Pthread_mutex_lock(&pool->mutex);

    // Wait for the right conditions that allow scheduling a new task.
    while (pool->queued_count == 0 || pool->working_on == pool->max_threads) {
      if (pool->queued_count == 0) {
        DEB("[W] Empty queue. Waiting...\n");
        Pthread_cond_wait(&pool->work_available, &pool->mutex);
      }

      if (pool->working_on == pool->max_threads) {
        DEB("[W] Full thread capacity. Waiting ...\n");
        Pthread_cond_wait(&pool->work_capacity_available, &pool->mutex);
      }
    }

    DEB("[W] Picking item from queue:\n");

    assert(pool->queued_count > 0);
    assert(pool->working_on < pool->max_threads);
    
    pool->queued_count--;
    launch_work_task(pool, pool->task_buffer[pool->queued_count]);

    Pthread_mutex_unlock(&pool->mutex);
  }
  return NULL;
}

void pool_add_task(struct thread_pool *pool, void *(*work_routine)(void*), void *arg) {
  Pthread_mutex_lock(&pool->mutex);
  DEB("[Q] Queueing one item.\n");
  if (pool->queued_count == 0) {
    Pthread_cond_signal(&pool->work_available);
  }

  struct task_data task;
  task.work_routine = work_routine;
  task.arg = arg;
  assert(pool->queued_count < TASK_BUFFER_MAX);
  pool->task_buffer[pool->queued_count++] = task;

  Pthread_mutex_unlock(&pool->mutex);
}

void pool_wait(struct thread_pool *pool) {
  Pthread_mutex_lock(&pool->mutex);

  while (pool->queued_count > 0 || pool->working_on > 0) {
    Pthread_cond_wait(&pool->work_capacity_available, &pool->mutex);
  }

  Pthread_mutex_unlock(&pool->mutex);
}

struct thread_pool* pool_init(int max_threads) {
  struct thread_pool* pool = malloc(sizeof(struct thread_pool));

  pool->queued_count = 0;
  pool->working_on = 0;
  pool->task_buffer = malloc(sizeof(struct task_data) * TASK_BUFFER_MAX);
  pool->max_threads = max_threads;

  Pthread_mutex_init(&pool->mutex);
  Pthread_cond_init(&pool->work_available);
  Pthread_cond_init(&pool->work_capacity_available);

  Pthread_create(&pool->dispatcher_thread, NULL, dispatcher_thread_func, pool);

  return pool;
}

void pool_destroy(struct thread_pool *pool) {
  pool_wait(pool);
  Pthread_detach(pool->dispatcher_thread);
  free(pool->task_buffer);
  free(pool);
}
