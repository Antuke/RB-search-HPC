
/*
 * Final Assignment of the Course: High Performance Computing 2023/2024 (Unisa)
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
 * Purpose of the file: This file provides an OpenMP + MPI solution for the RBTREE SEARCH algorithm.
 */

/**
 * @file find_mpi.c
 * @author Antonio Sessa (a.sessa108@studenti.unisa.it)
 * @brief MPI implementation of RBSearch
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#include <omp.h>
#include "../Headers/rb.h"
#include "../Headers/rb_data.h"
#include <assert.h>
#include <string.h>
#include <math.h>


/* rank of the process */ 
int rank;

/* flag for compare operation to use (long/short) */
int long_compare_flag = 0;


/*! Max number of procs, to define the arrays later */ 
#define MAX_PROC 16

/**
 * @brief binary search
 * @param data_array sorted array of mydata, from where to search
 * @param data searched data
 * @param begin start index
 * @param end end index
 */
int binarySearch(mydata *data_array, mydata data, int begin, int end)
{

  int mid_point = (begin + end) / 2;
  int cmp_result;

  if (long_compare_flag != 0)
    cmp_result = long_compare_func(&data_array[mid_point], &data);
  else
  {
    cmp_result = compare_func(&data_array[mid_point], &data);
  }

  if (cmp_result == 0)
    return mid_point;
  else if (abs(begin - end) == 1)
    return -1;
  else if (cmp_result < 1)
    return binarySearch(data_array, data, mid_point + 1, end);
  else
    return binarySearch(data_array, data, begin, mid_point - 1);

  return -1;
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
 * @brief Set the values for the count and displacements arrays for the MPI_scatterv
 * @param count is the array that indicates how many elements will the i-rank process, so count[2] = 10, means that the rank 2 will receive 10 elements
 * @param displacements is the array that indicates from where to pick the elements in the array that has to be scattared
 * @param num_proc is the number of processes
 * @param num_nodes is the number of elements in the array
 */
void set_count_displacements(int *count, int *displacements, int num_proc, int num_nodes)
{
  // how many each process should get 
  int elements_per_proc = num_nodes / num_proc;

  int remainder = num_nodes % num_proc;

  for (int i = 0; i < num_proc; i++)
  {
    // assign to process i at least num_nodes/num_proc nodes
    count[i] = elements_per_proc;

    // if we have a remainder we add another node (the remainder will be less then num_proc)
    if (remainder > 0)
    {
      count[i]++;
      remainder--;
    }
    if (i == 0)
    {
      // process 0 starts from 0
      displacements[i] = 0;
    }
    else
    {
      // process i starts from the displecement of the last process + the number of nodes that that process handles
      displacements[i] = displacements[i - 1] + count[i - 1];
    }
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

int main(int argc, char *argv[])
{
  if (argc != 5)
  {
    fprintf(stderr, "Usage: %s <number_of_nodes> <number_of_omp_threads> <optimization> <longcompareflag>\n", argv[0]);
    return 1;
  }

  /* variable to choose wich kind of compare should be executed */
  long_compare_flag = atoi(argv[4]);
  /* number of threads to use in openMP and setting*/
  int omp_num_threads = atoi(argv[2]);
  omp_set_num_threads(omp_num_threads);
  /* just for writing the result*/
  char *optimization = argv[3];

  /*time variables*/
  double start_time, end_time, total_start, tree_linearization_time;
  double total_time, communication_time = 0.0, binary_search_time = 0.0, tree_creation_time = 0.0;
  /* MPI Initiliazation */
  MPI_Init(NULL, NULL);
  total_start = MPI_Wtime();

  /* number of nodes of the rbt  */
  int number_of_nodes = atoi(argv[1]);
  /* query is the searched data */
  mydata query;
  /* array where data of the linearized tree will be put*/
  mydata *data_array;
  /* array where of the pointer to the nodes of the linearized tree will be put*/
  rbnode **node_array;
  /* the rbt */
  rbtree *rbt;

  /* MPI init */
  // MPI_Status status;
  int number_of_processes;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &number_of_processes);

  /* MPI datatype definition for mydata */
  /* Reference code: https://rookiehpc.org/mpi/docs/mpi_type_create_struct/index.html*/
  MPI_Datatype MPI_MYDATA;
  int lengths[2] = {1, 1};
  MPI_Aint displacements[2];
  mydata dummy_data;
  dummy_data.key = 0;
  dummy_data.value = 0;
  MPI_Aint base_address;
  MPI_Get_address(&dummy_data, &base_address);
  MPI_Get_address(&dummy_data.key, &displacements[0]);
  MPI_Get_address(&dummy_data.value, &displacements[1]);
  displacements[0] = MPI_Aint_diff(displacements[0], base_address);
  displacements[1] = MPI_Aint_diff(displacements[1], base_address);
  MPI_Datatype types[2] = {MPI_INT, MPI_INT};
  MPI_Type_create_struct(2, lengths, displacements, types, &MPI_MYDATA);
  MPI_Type_commit(&MPI_MYDATA);

  /* rank 0 creates the rbt and linearizes it */
  start_time = MPI_Wtime();
  if (rank == 0)
  {

    if ((rbt = rb_create(compare_func, destroy_func)) == NULL)
    {
      fprintf(stderr, "create red-black tree failed\n");
      return 1;
    }

    /* initializing rbt (the input file has 10 milion unique integers, so if num_nodes it's bigger we use linearInit)*/
    if(number_of_nodes <= 10000000)
      randomInit(rbt, number_of_nodes);
    else
      linearInit(rbt,number_of_nodes);
    /* insertion of the searched data */
    mydata *biggest =  biggest_data(rbt);
    /* I make sure that the query goes to the last process*/
    query.key = biggest->key + 1;
    rb_insert(rbt, (void *)&query);

    end_time = MPI_Wtime();
    tree_creation_time = end_time - start_time;

    /* linearize the red-black tree*/
    /* we set the count variable of the nodes at log2(omp_num_threads),
    the rbt tree implementation could be changed so that this is an operation done during the insert, but 
    this was not done, so I have to call this function. I choose to not time it in the tree linearization */
    set_counts(rbt, RB_FIRST(rbt), log2(omp_num_threads), 0);
    start_time = MPI_Wtime();


    /* only rank0 does the malloc of the data_array and node_array*/
    data_array = (mydata *)malloc(sizeof(mydata) * rbt->count);
    node_array = rb_node_array(rbt, omp_num_threads, data_array);
    end_time = MPI_Wtime();
    tree_linearization_time = end_time - start_time;
  }
  /* variables for scatterv function, scatterv instead of scatter because not all processes will receive the same amount of data*/
  /* how many elements each process gets*/
  int count[MAX_PROC];
  /* index from where each process start receiving the array 
  [process i, receives count[i] elements, and this elements are taken starting from index displacement[i]] */
  int displacementsv[MAX_PROC];
  /* array where to hold the result of each binary search */
  int result_array[MAX_PROC];
  /* setting the count and displacement arrays */
  set_count_displacements(count, displacementsv, number_of_processes, number_of_nodes + 1);

  /* how many element each process will receive */
  int elem_per_process = number_of_nodes / number_of_processes;
  /* plus one because we distribuite the remainder between process*/
  mydata *mypart = (mydata *)malloc(sizeof(mydata) * elem_per_process + 1);

  if (rank == 0)
    start_time = MPI_Wtime();
  /* this are collective operations, so each process has to call this functions */
  /* broadcast the searched data */
  MPI_Bcast(&query, 1, MPI_MYDATA, 0, MPI_COMM_WORLD);
  /* scatter the data_array, each process receives only part of the data_array */
  MPI_Scatterv(data_array, count, displacementsv, MPI_MYDATA, mypart, count[rank], MPI_MYDATA, 0, MPI_COMM_WORLD);

  if (rank == 0)
  {
    end_time = MPI_Wtime();
    communication_time += end_time - start_time;
    free(data_array);
  }

  start_time = MPI_Wtime();
  // the binary search returns the index where the searched data was found
  int index = binarySearch(mypart, query, 0, count[rank]);
  end_time = MPI_Wtime();
  binary_search_time = end_time - start_time;

  // rank0 is the master process, so it will check wich process has found the data
  // in our case is always the last one, so that we can time the binary search for the worst case where the data is not found
  // (worst case does not mean a lot in MPI, because each process has to wait for the slowest process to finish to terminate the execution)
  if (rank == 0)
  {
    start_time = MPI_Wtime();
    MPI_Gather(&index, 1, MPI_INT, result_array, 1, MPI_INT, 0, MPI_COMM_WORLD);
  }
  else
  {
    MPI_Gather(&index, 1, MPI_INT, NULL, 0, MPI_INT, 0, MPI_COMM_WORLD);
    // the processes that are not rank0 can terminate now
    free(mypart);
    MPI_Finalize();
    return 0;
  }
  //int founded_flag = -1;
  if (rank == 0)
  {
    end_time = MPI_Wtime();
    communication_time += end_time - start_time;
    for (int i = 0; i < number_of_processes; i++)
    {
      // iterate over the result array, if a process has not found the element it sends -1
      // when the element is different than -1 it means the element was found
      if (result_array[i] != -1)
      {
        //printf("FOUND BY %d",i);
        // Accounting for displacements
        index = result_array[i] + displacementsv[i];

        end_time = MPI_Wtime();

        /* verify the result with sequential implementation */
        total_time = end_time - total_start;
        rbnode *to_assert = rb_find(rbt, &query);
        assert(to_assert == node_array[index]);
        //founded_flag = 1;
        break;
      }
    }
  }
  /* process 0 writes the times to a csv file */
  if (rank == 0)
  {
    char ftopen[50] = "./Results/Mpi/";
    // i know there is a better way to do this
    strcat(ftopen, optimization);

    strcat(ftopen, "_");
    strcat(ftopen, argv[1]);
    strcat(ftopen, "_");

    if (long_compare_flag == 1)
      strcat(ftopen, "lc");
    else
      strcat(ftopen, "sc");

    strcat(ftopen, ".csv");

    FILE *fp = fopen(ftopen, "a+");
    fseek(fp, 0, SEEK_END);
    if (ftell(fp) == 0)
    {
      fprintf(fp, "processes,omp_threads,total time,comm time,binary search,tree creation,tree linearization\n");
    }

    if (fp == NULL)
    {
      fprintf(stderr, "Failed to open file for writing.\n");
      return 1;
    }
    fprintf(fp, "%d,%d,%lf,%lf,%lf,%lf,%lf\n", number_of_processes, omp_num_threads, total_time, communication_time, binary_search_time, tree_creation_time, tree_linearization_time);
    /*
    if (founded_flag == 1)
    {
      fprintf(fp, "%d,%d,%lf,%lf,%lf,%lf,%lf\n", number_of_processes, omp_num_threads, total_time, communication_time, binary_search_time, tree_creation_time, tree_linearization_time);
    }
    else
    {
      fprintf(fp, "ERRORE\n");
    }
    */
    fclose(fp);
  }
  if (rank == 0)
  {
    //frees
    free(rbt);
    free(mypart);
    free(node_array);
  }

  MPI_Finalize();
}