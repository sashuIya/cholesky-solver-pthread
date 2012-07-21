#ifndef ARRAY_OP_H
#define ARRAY_OP_H

int inverse_upper_triangle_block_and_diagonal(int n, double *a, double *d, double *b);
int cholesky_for_block(int n, double *a, double *d);

int inverse_upper_triangle_block_and_diagonal_rhs(int n, double *a, double *d, double *rhs);
int inverse_lower_triangle_block_rhs(int n, double *a, double *rhs);

void matrix_block_vector_multiply(int n, int m, double *a, double *b, double *c);
void matrix_block_transposed_vector_multiply(int n, int m, double *a, double *b, double *c);

void main_blocks_diagonal_multiply(int n, int m, int l, double *a, double *b, double *d, double *c);
void main_blocks_multiply(int n, int m, int l, double *a, double *b, double *c);
void cpy_diagonal_block_to_block(double *a, int t, int matrix_size, int m, double *b);
void cpy_block_to_diagonal_block(double *a, int t, int matrix_size, int m, double *b);
void cpy_block_to_matrix_block(double *a, int row, int column, int matrix_size, int n, int m, double *b);
void cpy_matrix_block_to_block(double *a, int row, int column, int matrix_size, int n, int m, double *b);

/*
int cholesky(
        int matrix_size, 
        double *matrix, 
        double *diagonal,
        double *workspace,
        int block_size
);
*/

int solve_lower_triangle_matrix_system(
        int matrix_size,
        double *matrix,
        double *rhs,
        double *workspace,
        int block_size
);

int solve_upper_triangle_matrix_diagonal_system(
        int matrix_size,
        double *matrix,
        double *diagonal,
        double *rhs,
        double *workspace,
        int block_size
);


#endif
