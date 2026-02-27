#ifndef CHOLESKY_THREADED 
#define CHOLESKY_THREADED

#include <pthread.h>

typedef struct _CholeskyArgs
{
  int matrix_size; 
  double *matrix; 
  double *diagonal;
  double *workspace;
  int block_size;
  int thread_id;
  int total_threads;
  pthread_barrier_t *barrier;
  int *error;
} CholeskyArgs;

void *cholesky_threaded(void *ptr);

int cholesky(
    int matrix_size, 
    double *matrix, 
    double *diagonal,
    double *workspace,
    int block_size,
    int thread_id,
    int total_threads,
    pthread_barrier_t *barrier,
    int *error
);

#endif

