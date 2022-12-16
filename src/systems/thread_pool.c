#include "systems/thread_pool.h"

#include <stdio.h>
#include <stdlib.h>

/* PRIVATE MEMBER PROTOTYES */
void *generic_thread_function(void *arg);
void add_work(struct ThreadPool *thread_pool, struct ThreadJob job);

/* Construstors */
/**
 * It creates a thread pool with the specified number of threads, and returns a struct ThreadPool
 *
 * @param num_threads The number of threads to create in the thread pool.
 *
 * @return A struct ThreadPool
 */
struct ThreadPool *thread_pool_constructor(int num_threads)
{
  struct ThreadPool *thread_pool = malloc(sizeof(struct ThreadPool));
  thread_pool->num_threads = num_threads;
  thread_pool->active = 1;
  thread_pool->work = queue_constructor();

  // Initialize the pthread muteces
  thread_pool->lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
  thread_pool->signal = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
  pthread_mutex_lock(&thread_pool->lock);
  thread_pool->pool = malloc(sizeof(pthread_t[num_threads]));

  for (int i = 0; i < num_threads; i++)
  {
    pthread_create(&thread_pool->pool[i], NULL, generic_thread_function, thread_pool);
  }
  pthread_mutex_unlock(&thread_pool->lock);
  thread_pool->add_work = add_work;
  return thread_pool;
}

/**
 * It takes a function pointer and an argument, and returns a struct containing those two things
 *
 * @param job The function that will be executed by the thread.
 * @param arg The argument to pass to the job function.
 *
 * @return A struct ThreadJob.
 */
struct ThreadJob thread_job_constructor(void *(*job)(void *arg), void *arg)
{
  struct ThreadJob thread_job;
  thread_job.job = job;
  thread_job.arg = arg;
  return thread_job;
}

/* destructor */
/**
 * It sets the active flag to 0, signals all the threads to wake up, waits for them to finish, and then frees the thread
 * pool
 *
 * @param thread_pool The thread pool to be destroyed.
 */
void thread_pool_destructor(struct ThreadPool *thread_pool)
{
  thread_pool->active = 0;
  for (int i = 0; i < thread_pool->num_threads; i++)
  {
    pthread_cond_signal(&thread_pool->signal);
  }
  for (int i = 0; i < thread_pool->num_threads; i++)
  {
    pthread_join(thread_pool->pool[i], NULL);
  }
  free(thread_pool->pool);
  queue_destructor(&thread_pool->work);
  free(thread_pool);
}

/**
 * The generic_thread_function is required as the argument
 * for creating each thread in the pool.
 * It allows each thread to await a ThreadJob object and
 * execute its contents.
 * This make the pool dynamic - any function can be passed as a job.
 *
 * @param arg The argument passed to the thread function.
 *
 * @return A pointer void.
 */
void *generic_thread_function(void *arg)
{
  struct ThreadPool *thread_pool = (struct ThreadPool *)arg;
  struct ThreadJob job;
  while (thread_pool->active == 1)
  {
    // Lock the work queue.
    pthread_mutex_lock(&thread_pool->lock);
    pthread_cond_wait(&thread_pool->signal, &thread_pool->lock);
    // Get the job from the queue.
    struct ThreadJob *thread_job = (struct ThreadJob *)thread_pool->work.peek(&thread_pool->work);
    // Unlock the work queue.
    if (thread_job)
    {
      job = *thread_job;
      thread_pool->work.pop(&thread_pool->work);
    }
    pthread_mutex_unlock(&thread_pool->lock);
    // Execute the job.
    if (thread_job != NULL)
    {
      job.job(job.arg);
    }
  }
  return NULL;
}

/**
 * Adds work to the queue in a thread safe way.
 *
 * @param thread_pool The thread pool to add the work to.
 * @param thread_job The job to be added to the queue.
 */
void add_work(struct ThreadPool *thread_pool, struct ThreadJob thread_job)
{
  pthread_mutex_lock(&thread_pool->lock);
  thread_pool->work.push(&thread_pool->work, &thread_job, sizeof(thread_job));
  pthread_cond_signal(&thread_pool->signal);
  pthread_mutex_unlock(&thread_pool->lock);
}
