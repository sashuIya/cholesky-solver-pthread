#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "synchronize.h"

void synchronize(int total_threads)
{
  static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  static pthread_cond_t cond_in = PTHREAD_COND_INITIALIZER;
  static pthread_cond_t cond_out = PTHREAD_COND_INITIALIZER;

  static int threads_in = 0;
  static int threads_out = 0;

  if (total_threads <= 1)
      return;

  pthread_mutex_lock(&mutex);

  ++threads_in;

  if (threads_in >= total_threads)
  {
    // last entered
    threads_out = 0;
    pthread_cond_broadcast(&cond_in);
  }
  else
  {
    while (threads_in < total_threads)
    {
        pthread_cond_wait(&cond_in, &mutex);
    }
  }

  threads_out++;

  if (threads_out >= total_threads)
  {
    // last leaving
    threads_in = 0;
    pthread_cond_broadcast(&cond_out);
  }
  else
  {
    while (threads_out < total_threads)
    {
        pthread_cond_wait(&cond_out, &mutex);
    }
  }

  pthread_mutex_unlock(&mutex);
}

