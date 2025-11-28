
/*
 * Final Assignment of the Course: High Performance Computing 2023/2024 (Unisa) * Copyright (C) 2024 Antonio Sessa
 * Copyright (C) 2024 Antonio Sessa
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not, see https://www.gnu.org/licenses/.
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
 * Purpose of the file: This file uses the serial solution of the RBTREE SEARCH provided by the GitHub user xieqing (https://github.com/xieqing) .
 */

/**
 * @file sequential.c
 * @author Antonio Sessa
 * @brief Serial RBSearch, using xieqing. https://github.com/xieqing code
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include "../Headers/rb.h"
#include "../Headers/rb_data.h"
#include <time.h>
#include <assert.h>
#include <omp.h>
#include <string.h>

/* forward declarations */
void randomInit(rbtree *, int);
void linearInit(rbtree *, int);
int long_compare_flag = 0;

int main(int argc, char *argv[])
{

  if (argc != 4)
  {
    fprintf(stderr, "Usage: %s <number_of_nodes> <optimization> <longcompareflag>\n", argv[0]);
    return 1;
  }
  /* number-nodes passed as argoument */
  int number_of_nodes = atoi(argv[1]) + 1;
  /* just for writing in the correct csv files*/
  char *optimization = argv[2];
  /* variable to choose wich kind of compare should be executed */
  long_compare_flag = atoi(argv[3]);
  /* time variables */
  clock_t start_time, end_time, total_start_time;
  total_start_time = clock();
  /* rb-tree cration */
  start_time = clock();
  rbtree *rbt;
  if ((rbt = rb_create(compare_func, destroy_func)) == NULL)
  {
    fprintf(stderr, "create red-black tree failed\n");
    return 1;
  }
  /* initializing rbt (the input file has 10 milion unique integers, so if num_nodes it's bigger we use linearInit)*/
  if(number_of_nodes <= 10000000)
    randomInit(rbt, number_of_nodes);
  else{
    linearInit(rbt, number_of_nodes);
  }

  end_time = clock();

  /* inserting searched data */
  mydata query;
  mydata * biggest = biggest_data(rbt);
  query.key = biggest->key + 1;
  query.value = 0;
  rb_insert(rbt, (void *)&query);
  double tree_creation_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

  // Execute the rb_find function
  if (long_compare_flag == 1)
    rbt->compare = long_compare_func;
  start_time = clock();
  rbnode *h_answer = rb_find(rbt, &query);
  end_time = clock();
  assert(h_answer != NULL);
  
  // End measuring time
  end_time = clock();

  double binary_search_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
  double total_time = ((double)(end_time - total_start_time)) / CLOCKS_PER_SEC;

  
  /* writing results to file */
  char ftopen[30] = "./Results/Sequential/";

  strcat(ftopen, optimization);

  strcat(ftopen, "_");
  strcat(ftopen, argv[1]);
  strcat(ftopen, "_");

  if (long_compare_flag == 1)
    strcat(ftopen, "lc");
  else
    strcat(ftopen, "sc");

  strcat(ftopen, ".csv");

  FILE *fp = fopen(ftopen, "a");
  fseek(fp, 0, SEEK_END);
  if (fp == NULL)
  {
    fprintf(stderr, "Failed to open file for writing.\n");
    return 1;
  }
  if (ftell(fp) == 0)
  {
    fprintf(fp, "total_time,binary_search_time,tree_creation_time\n");
  }
  fprintf(fp, "%lf,%lf,%lf\n", total_time, binary_search_time, tree_creation_time);

  fclose(fp);
  free(rbt);
  return 0;
}

/**
 * @brief Initializes the red-black-tree with random values
 * @param rbt        pointer to the red-black-tree
 * @param num_nodes  number of nodes to insert into the rbt
 */
void randomInit(rbtree *rbt, int num_nodes)
{
  FILE *fp = fopen("./Input/maxsize.txt", "r");
  
  int value;

  while(rbt->count < num_nodes){
    fscanf(fp, "%d", &value);
    rb_insert(rbt, makedata(value));
  }

}

/**
 * @brief Initializes the red-black-tree with values from 0 to num_nodes - 1
 * @param rbt        pointer to the red-black-tree
 * @param num_nodes  number of nodes to insert into the rbt
 */
void linearInit(rbtree *rbt, int num_nodes)
{
  for(int i = 0; i < num_nodes;i++){
    rb_insert(rbt,makedata(i));
  }
}