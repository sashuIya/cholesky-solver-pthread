#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array_io.h"
#include "array_op.h"
#include "cholesky_threaded.h"
#include "timer.h"

const int WORKSPACE_MATRIX_COUNT = 4;

// Entry point for the block Cholesky solver.
//
// Usage: ./a <matrix_size> <block_size> <thread_count> [matrix_file]
int main(int argc, char* argv[]) {
  int matrix_size, block_size, total_threads;
  int i;
  int len;

  CholeskyArgs* cholesky_args;
  pthread_t* threads;
  pthread_barrier_t barrier;
  int error_flag = 0;

  double* matrix;
  double* diagonal;
  double* vector_answer;
  double* vector;
  double* exact_rhs;
  double* rhs;
  double* workspace;

  double residual, rhs_norm, answer_error;

  timer_start();

  // Parse command line arguments.
  if (argc == 4 || argc == 5) {
    matrix_size = atoi(argv[1]);
    block_size = atoi(argv[2]);
    if (argc == 4)
      total_threads = atoi(argv[3]);
    else
      total_threads = atoi(argv[4]);

    if (matrix_size <= 0 || block_size <= 0 || total_threads <= 0 ||
        total_threads > 128 || block_size > matrix_size) {
      printf("Wrong input parameters\n");
      return -1;
    }

    // Allocate a single large buffer for all matrix-related arrays to
    // maximize memory contiguousness.
    len = total_threads * WORKSPACE_MATRIX_COUNT * block_size * block_size +
          2 * block_size * block_size + ((matrix_size * (matrix_size + 1)) / 2) +
          5 * matrix_size;
    len *= sizeof(double);

    if (!(matrix = (double*)malloc(len))) {
      printf("Not enough memory\n");
      return -2;
    }

    memset(matrix, 0, len);

    // Calculate offsets into the large memory buffer.
    diagonal = matrix + ((matrix_size * (matrix_size + 1)) / 2);
    vector_answer = diagonal + matrix_size;
    vector = vector_answer + matrix_size;
    exact_rhs = vector + matrix_size;
    rhs = exact_rhs + matrix_size;
    workspace = rhs + matrix_size;

    if (!(cholesky_args =
              (CholeskyArgs*)malloc(total_threads * sizeof(CholeskyArgs)))) {
      printf("Not enough memory\n");
      free(matrix);
      return -2;
    }

    if (!(threads = (pthread_t*)malloc(total_threads * sizeof(pthread_t)))) {
      printf("Not enough memory\n");
      free(matrix);
      free(cholesky_args);
      return -2;
    }

    if (pthread_barrier_init(&barrier, NULL, total_threads)) {
      printf("Cannot initialize barrier\n");
      free(matrix);
      free(cholesky_args);
      free(threads);
      return -2;
    }

    // Initialize thread arguments.
    for (i = 0; i < total_threads; ++i) {
      cholesky_args[i].matrix_size = matrix_size;
      cholesky_args[i].matrix = matrix;
      cholesky_args[i].diagonal = diagonal;
      cholesky_args[i].workspace = workspace;
      cholesky_args[i].block_size = block_size;
      cholesky_args[i].thread_id = i;
      cholesky_args[i].total_threads = total_threads;
      cholesky_args[i].barrier = &barrier;
      cholesky_args[i].error = &error_flag;
    }

    fill_vector_answer(matrix_size, vector_answer);

    // Load or generate matrix data.
    if (argc == 4) {
      if (fill_matrix(matrix_size, matrix, vector_answer, rhs)) {
        printf("Cannot fill matrix\n");
        goto cleanup;
      }
    } else if (argc == 5) {
      if (read_matrix(matrix_size, &matrix, vector_answer, rhs, argv[3])) {
        printf("Cannot read matrix\n");
        goto cleanup;
      }
    }

    for (i = 0; i < matrix_size; i++) {
      exact_rhs[i] = rhs[i];
      vector[i] = rhs[i];
    }
  } else {
    printf("Usage: %s <n> <m> <threads> [file]\n", argv[0]);
    return 0;
  }

  print_time("on initialization");

  if (matrix_size < 15) {
    printf("matrix A:\n");
    printf_matrix(matrix_size, matrix);
    printf("\nrhs:\n");
    for (i = 0; i < matrix_size; ++i) printf("%.10f ", rhs[i]);
    printf("\n\n");
  }

  // Spawn worker threads.
  for (i = 1; i < total_threads; ++i) {
    if (pthread_create(threads + i, 0, cholesky_threaded, cholesky_args + i)) {
      fprintf(stderr, "Cannot create thread #%d\n", i);
    }
  }

  // Thread 0 also performs work.
  cholesky_threaded(cholesky_args + 0);

  for (i = 1; i < total_threads; ++i) {
    if (pthread_join(threads[i], 0)) {
      fprintf(stderr, "Cannot wait for thread #%d\n", i);
    }
  }

  print_full_time("on cholesky decomposition");

  // Solve the resulting triangular systems.
  if (solve_lower_triangle_matrix_system(matrix_size, matrix, vector, workspace,
                                         block_size)) {
    printf("Cannot solve R^T y = b part\n");
    goto cleanup;
  }

  if (solve_upper_triangle_matrix_diagonal_system(
          matrix_size, matrix, diagonal, vector, workspace, block_size)) {
    printf("Cannot solve D R x = y part\n");
    goto cleanup;
  }

  if (matrix_size < 15) {
    printf("cholesky decomposition:\n");
    printf_matrix(matrix_size, matrix);
    printf("\ndiagonal:\n");
    for (i = 0; i < matrix_size; i++) printf("%.1f ", diagonal[i]);
    printf("\n\n");
  }

  print_time("on algorithm");

  // Verify results by calculating error and residual.
  residual = 0;
  rhs_norm = 0;
  answer_error = 0;

  if (argc == 4) {
    fill_matrix(matrix_size, matrix, vector, rhs);
  } else if (argc == 5) {
    read_matrix(matrix_size, &matrix, vector, rhs, argv[3]);
  }

  for (i = 0; i < matrix_size; ++i) {
    residual += (exact_rhs[i] - rhs[i]) * (exact_rhs[i] - rhs[i]);
    rhs_norm += exact_rhs[i] * exact_rhs[i];
    answer_error +=
        (vector_answer[i] - vector[i]) * (vector_answer[i] - vector[i]);
  }

  residual = sqrt(residual);
  rhs_norm = sqrt(rhs_norm);
  answer_error = sqrt(answer_error);

  printf("\n");
  printf("Error: %11.5le ; Residual: %11.5le (%11.5le)\n", answer_error,
         residual, residual / rhs_norm);
  printf("Total time in seconds: %.2f\n", WallTimerGet() / 100.0);
  printf("CPU time in seconds: %.2f\n", TimerGet() / 100.0);
  printf("\n");

cleanup:
  free(matrix);
  free(cholesky_args);
  free(threads);
  pthread_barrier_destroy(&barrier);

  return 0;
}
