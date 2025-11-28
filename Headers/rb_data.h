/*
 * Copyright (c) 2019 xieqing. https://github.com/xieqing
 * May be freely redistributed, but copyright notice must be retained.
 */

/*
 * Final Assignment of the Course: High Performance Computing 2023/2024 (Unisa) 
 *
 *  Student : Antonio Sessa
 *  Matricola: 0622702305
 *  Email: a.sessa108@studenti.unisa.it
 *  The Name of the assignment: RBTREE SEARCH
 *  Lecturer: Francesco Moscato	fmoscato@unisa.it
 *
 * Requirement:  Student shall provide a parallel version of an algorithm (RBTREE SEARCH) with both "OpenMP + MPI" and "OpenMP + Cuda" approaches,
 *  comparing results with a known solution on single-processing node. Results and differences shall be discussed for different inputs (type and size).
 *  The parallel algorithm used in "OpenMP + MPI" solution could not be the same of the "OpenMP + CUDA" approach.
 *
 * Purpose of the file: This file is the header for the red-black tree, credit for implementation to: xieqing. https://github.com/xieqing  .
 * The declarations marked by ADDITIONS are for function that I added.
 */

#ifndef _RB_DATA_HEADER
#define _RB_DATA_HEADER


typedef struct {
    int key;
    int value;
} mydata;



mydata *makedata(int key);
int compare_func(const void *d1, const void *d2);
void destroy_func(void *d);
void print_func(void *d);
void print_char_func(void *d);

/* ADDITIONS */
int long_compare_func(const void *d1, const void *d2);
void simulate_compare();

#endif /* _RB_DATA_HEADER */

