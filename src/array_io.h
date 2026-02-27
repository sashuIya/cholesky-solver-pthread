#ifndef ARRAY_IO_H
#define ARRAY_IO_H

// Fills the matrix with test values and generates the corresponding RHS.
// The matrix is stored in a packed upper triangular format.
int fill_matrix(int n, double* matrix, double* vector_answer, double* rhs);

// Legacy/simple matrix filling routine.
int stupid_fill_matrix(int n, double* matrix);

// Reads matrix elements from a file and generates the corresponding RHS.
int read_matrix(int matrix_size, double** p_a, double* vector_answer,
                double* rhs, char* input_file_name);

// Legacy/simple matrix reading routine.
int stupid_read_matrix(int matrix_size, double** p_a, char* input_file_name);

// Prints the matrix to standard output.
void printf_matrix(int n, double* matrix);

// Legacy/simple matrix printing routine.
void stupid_printf_matrix(int n, double* matrix);

// Fills the answer vector with a known pattern for verification.
void fill_vector_answer(int n, double* vector_answer);

#endif  // ARRAY_IO_H
