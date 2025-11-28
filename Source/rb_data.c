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
 * Purpose of the file: This file is the source for the mydata, credit for implementation to: xieqing. https://github.com/xieqing  .
 * The functions marked by ADDITIONS  are for function what I added.
 */


/**
 * @file rb_data.c
 * @author xieqing. https://github.com/xieqing & some additions by Antonio Sessa
 * @brief Rb_data implementation
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../Headers/rb_data.h"

mydata *makedata(int key)
{
	mydata *p;

	p = (mydata *)malloc(sizeof(mydata));
	if (p != NULL)
		p->key = key;

	return p;
}

int compare_func(const void *d1, const void *d2)
{
	mydata *p1, *p2;

	assert(d1 != NULL);
	assert(d2 != NULL);

	p1 = (mydata *)d1;
	p2 = (mydata *)d2;
	if (p1->key == p2->key)
		return 0;
	else if (p1->key > p2->key)
	{
		return 1;
	}
	else
		return -1;
}

/*ADDITIONS */
/**
 * @brief The functions writes the result of the sum of 100000 random values to a txt files, so that optimizations
 * are sure to run this piece of code that would be otherwise ignored (if an optimization see that the variable sum was not used in some
 * meaningful way the for loop would not even be done)
 */
void simulate_long_compare()
{
	
	int sum = 0;
	for (int i = 0; i < 100000; i++)
	{
		int randomValue = rand() % 100 + 1;
		sum += randomValue;
	}
	FILE *fp;
	fp = fopen("./Build/trash.txt", "w");
	if (fp == NULL)
	{
		return;
	}
	fprintf(fp, "%d", sum);
	fclose(fp);
}


/*ADDITIONS */
/**
 * @brief Edit of the default compare_func to make it take more time
 * @param d1 memory address of data to be compared
 * @param d2 memory address of data to be compared
 */
int long_compare_func(const void *d1, const void *d2)
{
	mydata *p1, *p2;

	assert(d1 != NULL);
	assert(d2 != NULL);

	p1 = (mydata *)d1;
	p2 = (mydata *)d2;
	if (p1->key == p2->key)
		return 0;
	else if (p1->key > p2->key)
	{
		simulate_long_compare();
		return 1;
	}
	else
	{
		simulate_long_compare();
		return -1;
	}
}

void destroy_func(void *d)
{
	mydata *p;

	assert(d != NULL);

	p = (mydata *)d;
	free(p);
}

void print_func(void *d)
{
	mydata *p;

	assert(d != NULL);

	p = (mydata *)d;
	printf("%d", p->key);
}

void print_char_func(void *d)
{
	mydata *p;

	assert(d != NULL);

	p = (mydata *)d;
	printf("%c", p->key & 127);
}