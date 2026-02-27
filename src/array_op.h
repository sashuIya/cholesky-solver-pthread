#ifndef ARRAY_OP_H
#define ARRAY_OP_H

// Inverts an upper triangular matrix block considering a diagonal scaling.
//
// n: Size of the block (n x n).
// a: Pointer to the upper triangular matrix block.
// d: Pointer to the diagonal elements.
// b: Output buffer for the inverted block.
// Returns: 0 on success, -1 if the matrix is singular.
int inverse_upper_triangle_block_and_diagonal(int n, double* a, double* d, double* b);

// Applies Cholesky decomposition to a single matrix block.
//
// n: Size of the block.
// a: The matrix block to decompose (modified in-place).
// d: Output buffer for the diagonal scaling elements.
// Returns: 0 on success, -1 if decomposition cannot be applied.
int cholesky_for_block(int n, double* a, double* d);

// Inverts an upper triangular block and applies it to a right-hand side vector.
int inverse_upper_triangle_block_and_diagonal_rhs(int n, double* a, double* d, double* rhs);

// Inverts a lower triangular block and applies it to a right-hand side vector.
int inverse_lower_triangle_block_rhs(int n, double* a, double* rhs);

// Multiplies a matrix block by a vector.
void matrix_block_vector_multiply(int n, int m, double* a, double* b, double* c);

// Multiplies a transposed matrix block by a vector.
void matrix_block_transposed_vector_multiply(int n, int m, double* a, double* b, double* c);

// Performs C = C - A^T * D * B multiplication for matrix blocks.
void main_blocks_diagonal_multiply(int n, int m, int l, double* a, double* b, double* d, double* c);

// Performs C = A * B multiplication for matrix blocks.
void main_blocks_multiply(int n, int m, int l, double* a, double* b, double* c);

// Copies a diagonal block from the packed matrix to a square buffer.
void cpy_diagonal_block_to_block(double* a, int t, int matrix_size, int m, double* b);

// Copies a square buffer back to a diagonal block in the packed matrix.
void cpy_block_to_diagonal_block(double* a, int t, int matrix_size, int m, double* b);

// Copies a square buffer back to an off-diagonal block in the packed matrix.
void cpy_block_to_matrix_block(double* a, int row, int column, int matrix_size, int n, int m,
                               double* b);

// Copies an off-diagonal block from the packed matrix to a square buffer.
void cpy_matrix_block_to_block(double* a, int row, int column, int matrix_size, int n, int m,
                               double* b);

// Solves the R^T * y = b system using forward substitution.
int solve_lower_triangle_matrix_system(int matrix_size, double* matrix, double* rhs,
                                       double* workspace, int block_size);

// Solves the D * R * x = y system using backward substitution.
int solve_upper_triangle_matrix_diagonal_system(int matrix_size, double* matrix, double* diagonal,
                                                double* rhs, double* workspace, int block_size);

#endif  // ARRAY_OP_H
