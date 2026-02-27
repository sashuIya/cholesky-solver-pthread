# Multi-threaded Block Cholesky Solver

A high-performance, multi-threaded solver for symmetric linear systems ($Ax = b$) using the Block Cholesky decomposition method. This project was originally developed in 2011 and has been modernized to follow contemporary C engineering standards.

## Overview

This program implements the block Cholesky method to obtain both the solution vector $x$ and the residual. It leverages the `pthread` library for parallel execution and employs several optimization techniques:

1.  **Block Matrix Storage**: Maximizes cache locality by representing the matrix as a grid of blocks.
2.  **Loop Unrolling**: Critical paths in matrix operations are unrolled to improve instruction-level parallelism.
3.  **Modern Concurrency**: Uses `pthread_barrier_t` for efficient thread synchronization and ensures re-entrant execution.

## Theory

The Cholesky method decomposes a symmetric, positive-definite matrix $A$ into $A = R^T D R$, where:
-   $R$ is an upper triangular matrix with positive diagonal elements.
-   $D$ is a diagonal matrix with elements equal to $\pm 1$.

### Block Algorithm
For a block matrix $A = (A_{ij})$, the decomposition formulas are:
-   **Off-diagonal blocks**: $R_{ij} = D_i^{-1} (R_{ii}^T)^{-1} \cdot (A_{ij} - \sum_{k=1}^{i-1} R_{ki}^T D_k R_{kj})$
-   **Diagonal blocks**: $R_{ii}^T D_i R_{ii} = A_{ii} - \sum_{k=1}^{i-1} R_{ki}^T D_k R_{ki}$

After decomposition, the system is solved in two stages:
1.  Solve $D R y = b$ (Forward substitution)
2.  Solve $R^T x = y$ (Backward substitution)

## Parallel Strategy

The workload is distributed across $p$ threads by partitioning the block updates. On each step $i$ of the outer loop:
-   Threads calculate their assigned blocks $A_{ij}$ in parallel.
-   A synchronization barrier ensures the diagonal block $A_{ii}$ is fully updated before the inversion starts.
-   Thread 0 handles the Cholesky decomposition and inversion of the diagonal block.
-   Another barrier ensures all threads have access to the inverted diagonal block before proceeding to the next row update.

## Getting Started

### Prerequisites
- GCC or Clang
- POSIX-compliant environment (Linux/macOS)
- Python 3 (for benchmarking)

### Building
```bash
cd src
make
```
The executable `a` will be placed in the `build/` directory.

### Usage
```bash
./build/a <matrix_size> <block_size> <thread_count> [input_file]
```
-   `matrix_size`: Total dimension of the matrix ($N$).
-   `block_size`: Dimension of the sub-blocks ($M$).
-   `thread_count`: Number of worker threads.
-   `input_file`: (Optional) Path to a file containing the matrix elements. If omitted, a test matrix is generated automatically.

## Benchmarking
A Python tool is provided to verify correctness and measure performance:
```bash
python3 benchmark.py --save results.json
```
To check for regressions against the baseline:
```bash
python3 benchmark.py --compare baseline.json
```

## License
Copyright 2011-2012 Alexander Lapin. Released under the GNU General Public License v3.0.
