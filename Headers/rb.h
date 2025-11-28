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

#ifndef _RB_HEADER
#define _RB_HEADER

// #define RB_DUP 1
#define RB_MIN 1

#define RED 0
#define BLACK 1

#include "../Headers/rb_data.h"

enum rbtraversal
{
	PREORDER,
	INORDER,
	POSTORDER
};

typedef struct rbnode
{
	struct rbnode *left;
	struct rbnode *right;
	struct rbnode *parent;
	char color;
	int count;
	void *data;
} rbnode;

typedef struct
{
	int (*compare)(const void *, const void *);
	void (*print)(void *);
	void (*destroy)(void *);

	rbnode root;
	rbnode nil;
	/*ADDITION*/
	int count;

#ifdef RB_MIN
	rbnode *min;
#endif
} rbtree;

#define RB_ROOT(rbt) (&(rbt)->root)
#define RB_NIL(rbt) (&(rbt)->nil)
#define RB_FIRST(rbt) ((rbt)->root.left)
#define RB_MINIMAL(rbt) ((rbt)->min)

#define RB_ISEMPTY(rbt) ((rbt)->root.left == &(rbt)->nil && (rbt)->root.right == &(rbt)->nil)
#define RB_APPLY(rbt, f, c, o) rbapply_node((rbt), (rbt)->root.left, (f), (c), (o))

rbtree *rb_create(int (*compare_func)(const void *, const void *), void (*destroy_func)(void *));
void rb_destroy(rbtree *rbt);

rbnode *rb_find(rbtree *rbt, void *data);
rbnode *rb_successor(rbtree *rbt, rbnode *node);

int rb_apply_node(rbtree *rbt, rbnode *node, int (*func)(void *, void *), void *cookie, enum rbtraversal order);
void rb_print(rbtree *rbt, void (*print_func)(void *));

rbnode *rb_insert(rbtree *rbt, void *data);
void *rb_delete(rbtree *rbt, rbnode *node, int keep);

int rb_check_order(rbtree *rbt, void *min, void *max);
int rb_check_black_height(rbtree *rbt);

/*ADDITION*/
rbnode **rb_node_array(rbtree *rbt, int threads, mydata *data_array);
void set_counts(rbtree *rbt, rbnode *node, int level, int currentLevel);
void traversal(rbtree *nil, rbnode *node, rbnode **node_array, mydata *, int *index);
int count_nodes(rbtree *rbt, rbnode *node);
void set_roots(rbtree *rbt, rbnode *node, int level, int currentLevel, rbnode **node_array, rbnode **excluded, int *included_index, int *excluded_index);
void sort_node_array(rbtree *rbt, rbnode **, int);
void swap(rbnode **a, rbnode **b);
mydata *biggest_data(rbtree *rbt);

#endif /* _RB_HEADER */
