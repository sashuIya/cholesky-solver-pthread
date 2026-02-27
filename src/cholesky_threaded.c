#include "cholesky_threaded.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array_op.h"
#include "timer.h"

// Entry point for each worker thread.
void* cholesky_threaded(void* ptr) {
  double timer = get_time_pthread();
  CholeskyArgs* pa = (CholeskyArgs*)ptr;

  // Initial synchronization before starting computations.
  pthread_barrier_wait(pa->barrier);

  cholesky(pa->matrix_size, pa->matrix, pa->diagonal, pa->workspace,
           pa->block_size, pa->thread_id, pa->total_threads, pa->barrier,
           pa->error);

  // Report individual thread CPU time.
  printf("Thread %d CPU time: %.2lf\n", pa->thread_id,
         (get_time_pthread() - timer) / (1000.0 * 1000.0 * 1000.0));

  // Final synchronization before exit.
  pthread_barrier_wait(pa->barrier);

  return 0;
}

// Parallel block Cholesky implementation.
//
// Each thread is responsible for updating a specific set of blocks in each
// iteration of the outer loop. Barriers are used to ensure data consistency
// between block updates and diagonal decomposition.
int cholesky(int matrix_size, double* matrix, double* diagonal,
             double* workspace, int block_size, int thread_id,
             int total_threads, pthread_barrier_t* barrier, int* error) {
  int i, j, k, t;
  int pij_n, pij_m;
  int pki_n, pki_m;
  int pkj_n, pkj_m;

  double *ma, *mb, *mc, *md, *me;
  me = workspace;
  // Each thread uses a private portion of the workspace for block operations.
  ma = workspace + thread_id * 4 * block_size * block_size +
       block_size * block_size;
  mb = ma + block_size * block_size;
  mc = mb + block_size * block_size;
  md = mc + block_size * block_size;

  for (i = 0; i < matrix_size; i += block_size) {
    // Stage 1: Update blocks in the current row.
    for (j = i + thread_id * block_size; j < matrix_size;
         j += total_threads * block_size) {
      pij_n = (i + block_size < matrix_size ? block_size : matrix_size - i);
      pij_m = (j + block_size < matrix_size ? block_size : matrix_size - j);

      if (j != i)
        cpy_matrix_block_to_block(matrix, i, j, matrix_size, pij_n, pij_m, mc);
      else
        cpy_diagonal_block_to_block(matrix, i, matrix_size, pij_n, mc);

      k = 0;
      for (t = 0; t < i; t += block_size) {
        if (k >= i) k = 0;

        pki_n = (k + block_size < matrix_size ? block_size : matrix_size - k);
        pki_m = (i + block_size < matrix_size ? block_size : matrix_size - i);
        pkj_n = (k + block_size < matrix_size ? block_size : matrix_size - k);
        pkj_m = (j + block_size < matrix_size ? block_size : matrix_size - j);

        cpy_matrix_block_to_block(matrix, k, i, matrix_size, pki_n, pki_m, ma);
        cpy_matrix_block_to_block(matrix, k, j, matrix_size, pkj_n, pkj_m, mb);

        main_blocks_diagonal_multiply(pki_n, pki_m, pkj_m, ma, mb, diagonal + k,
                                      mc);
        k += block_size;
      }

      if (j != i)
        cpy_block_to_matrix_block(matrix, i, j, matrix_size, pij_n, pij_m, mc);
      else
        cpy_block_to_diagonal_block(matrix, i, matrix_size, pij_n, mc);
    }

    pij_n = (i + block_size < matrix_size ? block_size : matrix_size - i);

    // Stage 2: Thread 0 handles the diagonal block decomposition and inversion.
    if (thread_id == 0) {
      cpy_diagonal_block_to_block(matrix, i, matrix_size, pij_n, mb);

      if (cholesky_for_block(pij_n, mb, diagonal + i)) {
        printf("Cholesky method with this block size cannot be applied\n");
        *error = 1;
      }

      cpy_block_to_diagonal_block(matrix, i, matrix_size, pij_n, mb);

      if (!(*error) && inverse_upper_triangle_block_and_diagonal(
                           pij_n, mb, diagonal + i, me)) {
        printf("Cholesky method with this block size cannot be applied\n");
        *error = 2;
      }
    }

    pthread_barrier_wait(barrier);
    if (*error) return -1;

    // Copy the inverted diagonal block to thread-local storage for multiplication.
    memcpy(md, me, pij_n * pij_n * sizeof(double));

    // Stage 3: Update remaining off-diagonal blocks using the inverted diagonal.
    for (j = i + block_size + thread_id * block_size; j < matrix_size;
         j += total_threads * block_size) {
      pij_n = (i + block_size < matrix_size ? block_size : matrix_size - i);
      pij_m = (j + block_size < matrix_size ? block_size : matrix_size - j);

      cpy_matrix_block_to_block(matrix, i, j, matrix_size, pij_n, pij_m, mb);
      main_blocks_multiply(pij_n, pij_n, pij_m, md, mb, mc);
      cpy_block_to_matrix_block(matrix, i, j, matrix_size, pij_n, pij_m, mc);
    }

    pthread_barrier_wait(barrier);
  }

  return 0;
}
