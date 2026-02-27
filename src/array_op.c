#include "array_op.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const double EPS = 1e-16;

// Copies an off-diagonal block from packed symmetric storage to a square block.
inline void cpy_matrix_block_to_block(double* a, int row, int column, int matrix_size, int n, int m,
                                      double* b) {
  int i, j, k;
  memset(b, 0, n * m * sizeof(double));

  // Map 2D block coordinates to packed 1D index.
  k = ((row * ((matrix_size << 1) - row + 1)) >> 1) + column - row;

  for (i = row; i < row + n; i++) {
    // Manually unrolled loop for performance.
    for (j = 0; j < m - 7; j += 8) {
      b[(i - row) * m + j] = a[k + j];
      b[(i - row) * m + j + 1] = a[k + j + 1];
      b[(i - row) * m + j + 2] = a[k + j + 2];
      b[(i - row) * m + j + 3] = a[k + j + 3];
      b[(i - row) * m + j + 4] = a[k + j + 4];
      b[(i - row) * m + j + 5] = a[k + j + 5];
      b[(i - row) * m + j + 6] = a[k + j + 6];
      b[(i - row) * m + j + 7] = a[k + j + 7];
    }
    for (; j < m; ++j) {
      b[(i - row) * m + j] = a[k + j];
    }
    k += matrix_size - i - 1;
  }
}

// Copies a square block back to an off-diagonal position in packed storage.
inline void cpy_block_to_matrix_block(double* a, int row, int column, int matrix_size, int n, int m,
                                      double* b) {
  int i, j, k;
  k = ((row * ((matrix_size << 1) - row + 1)) >> 1) + column - row;

  for (i = row; i < row + n; i++) {
    for (j = 0; j < m - 7; j += 8) {
      a[k + j] = b[(i - row) * m + j];
      a[k + j + 1] = b[(i - row) * m + j + 1];
      a[k + j + 2] = b[(i - row) * m + j + 2];
      a[k + j + 3] = b[(i - row) * m + j + 3];
      a[k + j + 4] = b[(i - row) * m + j + 4];
      a[k + j + 5] = b[(i - row) * m + j + 5];
      a[k + j + 6] = b[(i - row) * m + j + 6];
      a[k + j + 7] = b[(i - row) * m + j + 7];
    }
    for (; j < m; ++j) {
      a[k + j] = b[(i - row) * m + j];
    }
    k += matrix_size - i - 1;
  }
}

// High-performance block multiplication: C = C - A^T * D * B.
// This is the core computational kernel of the block Cholesky method.
inline void main_blocks_diagonal_multiply(int n, int m, int l, double* a, double* b, double* d,
                                          double* c) {
  int i, j, k;
  double *pa, *pb, *pc, pd, ta;

  pa = a;
  pb = b;
  for (k = 0; k < n; ++k) {
    pd = d[k];
    pc = c;
    for (i = 0; i < m; ++i) {
      ta = pa[i] * pd;
      for (j = 0; j < l - 7; j += 8) {
        pc[j] -= pb[j] * ta;
        pc[j + 1] -= pb[j + 1] * ta;
        pc[j + 2] -= pb[j + 2] * ta;
        pc[j + 3] -= pb[j + 3] * ta;
        pc[j + 4] -= pb[j + 4] * ta;
        pc[j + 5] -= pb[j + 5] * ta;
        pc[j + 6] -= pb[j + 6] * ta;
        pc[j + 7] -= pb[j + 7] * ta;
      }
      for (; j < l; ++j) {
        pc[j] -= pb[j] * ta;
      }
      pc += l;
    }
    pa += m;
    pb += l;
  }
}

// Block multiplication: C = A * B.
inline void main_blocks_multiply(int n, int m, int l, double* a, double* b, double* c) {
  int i, j, k;
  double *pa, *pb, *pc, ta;

  memset(c, 0, m * l * sizeof(double));
  pa = a;
  pb = b;
  for (k = 0; k < n; ++k) {
    pc = c;
    for (i = 0; i < m; ++i) {
      ta = pa[i];
      for (j = 0; j < l - 7; j += 8) {
        pc[j] += pb[j] * ta;
        pc[j + 1] += pb[j + 1] * ta;
        pc[j + 2] += pb[j + 2] * ta;
        pc[j + 3] += pb[j + 3] * ta;
        pc[j + 4] += pb[j + 4] * ta;
        pc[j + 5] += pb[j + 5] * ta;
        pc[j + 6] += pb[j + 6] * ta;
        pc[j + 7] += pb[j + 7] * ta;
      }
      for (; j < l; ++j) {
        pc[j] += pb[j] * ta;
      }
      pc += l;
    }
    pa += m;
    pb += l;
  }
}

// Copies a diagonal block from packed storage to a square block.
void cpy_diagonal_block_to_block(double* a, int t, int matrix_size, int m, double* b) {
  int i, j, k;
  memset(b, 0, m * m * sizeof(double));
  k = ((t * ((matrix_size << 1) - t + 1)) >> 1);

  for (i = t; i < t + m; i++) {
    for (j = 0; j < m - (i - t); j++) {
      b[(i - t) * m + i - t + j] = a[k + j];
    }
    k += matrix_size - i;
  }
}

// Copies a square block back to a diagonal position in packed storage.
void cpy_block_to_diagonal_block(double* a, int t, int matrix_size, int m, double* b) {
  int i, j, k;
  k = ((t * ((matrix_size << 1) - t + 1)) >> 1);

  for (i = t; i < t + m; i++) {
    for (j = 0; j < m - (i - t); j++) {
      a[k + j] = b[(i - t) * m + i - t + j];
    }
    k += matrix_size - i;
  }
}

// Inverts an upper triangular block and scales it by the diagonal elements.
int inverse_upper_triangle_block_and_diagonal(int n, double* a, double* d, double* b) {
  int i, j, k;
  double dt;
  double *pa, *pbi, *pbj;

  memset(b, 0, n * n * sizeof(double));
  for (i = 0; i < n; ++i) {
    b[i * n + i] = d[i];
  }

  pbi = b + (n - 1) * n;
  for (i = n - 1; i >= 0; --i) {
    if (fabs(a[i * n + i]) < EPS) {
      return -1;
    }
    dt = 1.0 / a[i * n + i];
    for (j = i; j < n; j++) {
      pbi[j] *= dt;
    }

    pbj = b;
    pa = a;
    for (j = 0; j < i; ++j) {
      for (k = i; k < n; ++k) {
        pbj[k] -= pbi[k] * pa[i];
      }
      pbj += n;
      pa += n;
    }
    pbi -= n;
  }
  return 0;
}

// Internal non-blocked Cholesky for a single block.
int cholesky_for_block(int n, double* a, double* d) {
  int i, j, k;
  double *pai, *pak, dt;

  for (i = 0; i < n; ++i) {
    d[i] = 1.0;
  }

  pai = a;
  for (i = 0; i < n; ++i) {
    pak = a;
    for (k = 0; k < i; ++k) {
      for (j = i; j < n; ++j) {
        pai[j] -= pak[i] * d[k] * pak[j];
      }
      pak += n;
    }

    if (pai[i] < 0.0) {
      d[i] = -1.0;
      pai[i] = -pai[i];
    }
    pai[i] = sqrt(pai[i]);

    if (fabs(pai[i]) < EPS) {
      return -1;
    }

    dt = 1.0 / pai[i];
    for (j = i + 1; j < n - 8; j += 8) {
      pai[j] *= dt;
      pai[j + 1] *= dt;
      pai[j + 2] *= dt;
      pai[j + 3] *= dt;
      pai[j + 4] *= dt;
      pai[j + 5] *= dt;
      pai[j + 6] *= dt;
      pai[j + 7] *= dt;
    }
    for (; j < n; ++j) {
      pai[j] *= dt;
    }
    pai += n;
  }
  return 0;
}

// Inverts upper triangular scaled system for RHS vector.
int inverse_upper_triangle_block_and_diagonal_rhs(int n, double* a, double* d, double* rhs) {
  int i, j;
  for (i = 0; i < n; ++i) {
    rhs[i] *= d[i];
  }
  for (i = n - 1; i >= 0; --i) {
    if (fabs(a[i * n + i]) < EPS) {
      return -1;
    }
    rhs[i] *= 1.0 / a[i * n + i];
    for (j = 0; j < i; ++j) {
      rhs[j] -= rhs[i] * a[j * n + i];
    }
  }
  return 0;
}

// Inverts lower triangular system for RHS vector.
int inverse_lower_triangle_block_rhs(int n, double* a, double* rhs) {
  int i, j;
  for (i = 0; i < n; ++i) {
    if (fabs(a[i * n + i]) < EPS) {
      return -1;
    }
    rhs[i] *= 1.0 / a[i * n + i];
    for (j = i + 1; j < n; ++j) {
      rhs[j] -= rhs[i] * a[i * n + j];
    }
  }
  return 0;
}

// Standard matrix-vector multiplication for a block.
void matrix_block_vector_multiply(int n, int m, double* a, double* b, double* c) {
  int i, j;
  double* pai = a;
  for (i = 0; i < n; i++) {
    for (j = 0; j < m; j++) {
      c[i] -= pai[j] * b[j];
    }
    pai += m;
  }
}

// Standard transposed matrix-vector multiplication for a block.
void matrix_block_transposed_vector_multiply(int n, int m, double* a, double* b, double* c) {
  int i, j;
  for (i = 0; i < m; i++) {
    for (j = 0; j < n; j++) {
      c[i] -= a[j * m + i] * b[j];
    }
  }
}

// Solves the forward substitution step for the whole system.
int solve_lower_triangle_matrix_system(int matrix_size, double* matrix, double* rhs,
                                       double* workspace, int block_size) {
  int i, j, pii_n, pij_n, pij_m;
  double* ma = workspace;

  for (i = 0; i < matrix_size; i += block_size) {
    pii_n = (i + block_size < matrix_size ? block_size : matrix_size - i);
    cpy_diagonal_block_to_block(matrix, i, matrix_size, pii_n, ma);
    if (inverse_lower_triangle_block_rhs(pii_n, ma, rhs + i)) {
      return -1;
    }

    for (j = i + block_size; j < matrix_size; j += block_size) {
      pij_n = (i + block_size < matrix_size ? block_size : matrix_size - i);
      pij_m = (j + block_size < matrix_size ? block_size : matrix_size - j);
      cpy_matrix_block_to_block(matrix, i, j, matrix_size, pij_n, pij_m, ma);
      matrix_block_transposed_vector_multiply(pij_n, pij_m, ma, rhs + i, rhs + j);
    }
  }
  return 0;
}

// Solves the backward substitution step for the whole system.
int solve_upper_triangle_matrix_diagonal_system(int matrix_size, double* matrix, double* diagonal,
                                                double* rhs, double* workspace, int block_size) {
  int i, j, pii_n, pij_n, pij_m;
  int residue;
  double* ma = workspace;

  residue = matrix_size - (matrix_size % block_size);
  if (residue == matrix_size) {
    residue -= block_size;
  }

  for (i = residue; i >= 0; i -= block_size) {
    for (j = residue; j > i; j -= block_size) {
      pij_n = (i + block_size < matrix_size ? block_size : matrix_size - i);
      pij_m = (j + block_size < matrix_size ? block_size : matrix_size - j);
      cpy_matrix_block_to_block(matrix, i, j, matrix_size, pij_n, pij_m, ma);
      matrix_block_vector_multiply(pij_n, pij_m, ma, rhs + j, rhs + i);
    }

    pii_n = (i + block_size < matrix_size ? block_size : matrix_size - i);
    cpy_diagonal_block_to_block(matrix, i, matrix_size, pii_n, ma);
    if (inverse_upper_triangle_block_and_diagonal_rhs(pii_n, ma, diagonal + i, rhs + i)) {
      return -1;
    }
  }
  return 0;
}
