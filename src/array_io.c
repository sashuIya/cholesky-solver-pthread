#include "array_io.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int fill_matrix(int n, double* matrix, double* vector_answer, double* rhs) {
  int i, j, k, t;

  for (i = 0; i < n; ++i) rhs[i] = 0;

  for (i = 0; i < n; ++i) {
    k = n * i - (i * (i - 1)) / 2;

    for (j = 0; j < i; j++) {
      t = n * j - (j * (j - 1)) / 2;

      rhs[i] += matrix[t + i - j] * vector_answer[j];
    }

    for (j = i; j < n; j++) {
      matrix[k + j - i] = fabs(n - j);
      //            matrix[k + j-i] = 1.0 / (i + j + 1.0);

      rhs[i] += matrix[k + j - i] * vector_answer[j];
    }
  }

  return 0;
}

int stupid_fill_matrix(int n, double* matrix) {
  int i, j;

  for (i = 0; i < n; ++i)
    for (j = 0; j < n; ++j) matrix[i * n + j] = 1.0 / (i + j + 1.0);

  return 0;
}

int read_matrix(int matrix_size, double** p_a, double* vector_answer,
                double* rhs, char* input_file_name) {
  int i, j, k;
  FILE* input_file;
  double* matrix;
  double tmp;
  matrix = (*p_a);

  input_file = fopen(input_file_name, "r");
  if (input_file == NULL) {
    printf("Error: cannot open input file\n");
    return -1;
  }

  for (i = 0; i < matrix_size; ++i) rhs[i] = 0;

  k = 0;
  for (i = 0; i < matrix_size; i++) {
    for (j = 0; j < i; ++j) {
      if (fscanf(input_file, "%lf", &tmp) != 1) {
        printf("Cannot read matrix\n");
        return -2;
      }

      rhs[i] += tmp * vector_answer[j];
    }

    k = i * matrix_size - (i * (i - 1)) / 2;
    for (j = i; j < matrix_size; j++) {
      if (fscanf(input_file, "%lf", matrix + k + j - i) != 1) {
        printf("Cannot read matrix\n");
        return -3;
      }

      rhs[i] += matrix[k + j - i] * vector_answer[j];
    }
  }

  fclose(input_file);
  return 0;
}

int stupid_read_matrix(int matrix_size, double** p_a, char* input_file_name) {
  int i, j;
  FILE* input_file;
  double* matrix;
  matrix = (*p_a);

  input_file = fopen(input_file_name, "r");
  if (input_file == NULL) {
    printf("Error: cannot open input file\n");
    return -1;
  }

  for (i = 0; i < matrix_size; i++) {
    for (j = 0; j < matrix_size; j++)
      if (fscanf(input_file, "%lf", matrix + i * matrix_size + j) != 1) {
        printf("Cannot read matrix\n");
        return -3;
      }
  }

  fclose(input_file);
  return 0;
}

void printf_matrix(int n, double* matrix) {
  int i, j;
  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++)
      if (j >= i)
        printf("%A ", matrix[n * i - (i * (i - 1)) / 2 + j - i]);
      else
        printf("%A ", matrix[n * j - (j * (j - 1)) / 2 + i - j]);

    printf("\n");
  }
}

void stupid_printf_matrix(int n, double* matrix) {
  int i, j;
  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++) printf("%.3lf ", matrix[n * i + j]);

    printf("\n");
  }
}

void fill_vector_answer(int n, double* vector_answer) {
  int i;
  memset(vector_answer, 0, n * sizeof(double));

  for (i = 0; i < n; i += 2) vector_answer[i] = 1;
}
