
#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "data_structures/queue.h"
#include <pthread.h>

struct ThreadJob
{
  void *(*job)(void *arg); // function to be executed.
  void *arg;               // argument to be passed to the function.
};

struct ThreadPool
{
  int num_threads;       // number of threads in the pool.
  int active;            // a control switch for the thread pool.
  struct Queue work;     // a queue to store works.
  pthread_t *pool;       // Mutices for making the pool thread-safe.
  pthread_mutex_t lock;  // Mutices for making the pool thread-safe.
  pthread_cond_t signal; // Condition variable for making the pool thread-safe.

  // A function for safely adding work to the queue.
  void (*add_work)(struct ThreadPool *thread_pool, struct ThreadJob thread_job);
};

// A function for creating a thread pool.
struct ThreadPool *thread_pool_constructor(int num_threads);
// A function for creating a thread job.
struct ThreadJob thread_job_constructor(void *(*job)(void *arg), void *arg);

void thread_pool_destructor(struct ThreadPool *thread_pool);

#endif // THREAD_POOL_H
