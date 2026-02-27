#ifndef CHOLESKY_THREADED
#define CHOLESKY_THREADED

#include <pthread.h>

// Arguments passed to each worker thread.
typedef struct _CholeskyArgs {
  int matrix_size;             // Total size of the matrix (N x N).
  double* matrix;              // Pointer to the packed matrix data.
  double* diagonal;            // Pointer to the diagonal scaling elements.
  double* workspace;           // Thread-local or shared workspace buffers.
  int block_size;              // Size of the computation blocks (M x M).
  int thread_id;               // Unique ID for the current thread.
  int total_threads;           // Total number of active threads.
  pthread_barrier_t* barrier;  // Synchronization barrier.
  int* error;                  // Shared error flag for re-entrant reporting.
} CholeskyArgs;

// Entry point for pthread_create.
void* cholesky_threaded(void* ptr);

// Core multi-threaded block Cholesky decomposition implementation.
//
// Performs the decomposition in parallel by distributing block updates
// across threads and synchronizing at critical stages.
int cholesky(int matrix_size, double* matrix, double* diagonal, double* workspace, int block_size,
             int thread_id, int total_threads, pthread_barrier_t* barrier, int* error);

#endif  // CHOLESKY_THREADED
