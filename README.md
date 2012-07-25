Overview
--------

This program is a multi-threaded solver for symmetric linear systems. It can
be used to obtain both the answer and residual. It's based on the block
Cholesky decomposition method. This version uses the `pthread` library.

Several features were implemented to speed up the program:

1. Block matrix format that maximises the cache usage. For example, a matrix
   of the form

        a11 a12 ... a1n
        a21 a22 ... a2n
        ...
        an1 an2 ... ann

    is represented as a block matrix

        A11 A12 ... A1m
        A21 A22 ... A2m
        ...
        Am1 Am2 ... Amm

    where each block `Aij` is a matrix of `(block_size, block_size)` sizes.

    Some docs about the block algorithm can be found in the `doc` folder.

2. Loops are unrolled by 8 elements.

The matrix can be generated or read from a file. The right-hand side is
generated automatically by calculating the answer and then multiplying the
matrix by the answer vector.

Usage
-----

1. Makefile in `src` directory
2. `./a (matrix_size) (block_size) (threads count) [matrix input file]`

License
-------

Copyright 2011-2012 Alexander Lapin

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Contacts
--------
Alexander Lapin, <lapinra@gmail.com>
