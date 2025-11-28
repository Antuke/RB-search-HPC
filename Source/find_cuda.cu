
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
 * Purpose of the file: This file provides an OpenMP + CUDA solution for the RBTREE SEARCH algorithm.
 */


/**
 * @file find_cuda.cu
 * @author Antonio Sessa (a.sessa108@studenti.unisa.it)
 * @brief CUDA implementation of RBSearch
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "rb.c"
#include "rb_data.c"

/*credit for red-black tree c implementation: https://github.com/xieqing/red-black-tree */
#define SUCCESS 7
#define cudaCheckError()                                                               \
  {                                                                                    \
    cudaError_t e = cudaGetLastError();                                                \
    if (e != cudaSuccess)                                                              \
    {                                                                                  \
      printf("Cuda failure %s:%d: '%s'\n", __FILE__, __LINE__, cudaGetErrorString(e)); \
      exit(0);                                                                         \
    }                                                                                  \
  }
/* credit for cudaCheckError: https://gist.github.com/jefflarkin/5390993 */

/*
 * Forward declarations
 */
__device__ int d_memcmp(const void *, const void *, size_t);
__global__ void d_find(const mydata *, const mydata);
/* where the thread that finds the elements will write its id, there are no duplicate so no race conditions */
__device__ int d_found_index = -1;
void linearInit(rbtree *,int);
void randomInit(rbtree *, int);
rbnode *d_rb_find(rbtree *, void *);
/* time variables */
double gpu_time_sec, cpu_time_sec, tree_linearization_time, cuda_search_start;
/* parameters */
int number_of_threads_per_block, number_of_blocks, omp_threads;

int main(int argc, char *argv[])
{
  /* clock variables */
  clock_t total_start_time, end_time, start_time;
  /* start timing program */
  total_start_time = clock();

  if (argc != 5)
  {
    fprintf(stderr, "Usage: %s <number_of_nodes> <number_of_threads> <number_of_omp_threads> <optimization>\n", argv[0]);
    return 1;
  }
  /* number-nodes passed as argoument */
  int number_of_nodes = atoi(argv[1]);
  /* number of threads to use per block */
  number_of_threads_per_block = atoi(argv[2]);
  /* number of OpenMP threads*/
  omp_threads = atoi(argv[3]);
  /* just for writing the result*/
  char *optimization = argv[4];
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
  else
    linearInit(rbt, number_of_nodes);
  /* inserting of the searched data */
  mydata query;
  mydata *biggest =  biggest_data(rbt);
  query.key = biggest->key + 1;
  rb_insert(rbt, (void *)&query);

  /* finished creating the tree*/
  end_time = clock();
  double tree_creation_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

  /* device rb find call */
  rbnode *d_answer = d_rb_find(rbt, &query);
  end_time = clock();
  double cuda_search = ((double)(end_time - cuda_search_start)) / CLOCKS_PER_SEC;

  /* the program is done, the rest of the code is to verify the result with serial implementation of find*/
  end_time = clock();
  double total_time = ((double)(end_time - total_start_time)) / CLOCKS_PER_SEC;

  /* host rb_find call */
  rbnode *h_answer = rb_find(rbt, &query);

  /* check if the answers are equal */
  assert(d_answer == h_answer);

  /*
  printf("\nHOST ANSWER %p\t KEY: ",h_answer);
  print_func(h_answer->data);
  printf("\nDEVICE ANSWER %p\t KEY: ",d_answer);
  print_func(d_answer->data);
  */

  /* writing result to file */
  char ftopen[50] = "./Results/Cuda/";
  strcat(ftopen, optimization);
  strcat(ftopen, "_");
  strcat(ftopen, argv[1]);
  strcat(ftopen, "_");
  strcat(ftopen, ".csv");
  FILE *fp = fopen(ftopen, "a+");

  fseek(fp, 0, SEEK_END);
  if (fp == NULL)
  {
    fprintf(stderr, "Failed to open file for writing.\n");
    return 1;
  }
  if (ftell(fp) == 0)
  {
    fprintf(fp, "threads,blocks,omp_threads,total_time,gpu_time,cuda search,tree_creation,tree_linearization\n");
  }

  fprintf(fp, "%d,%d,%d,%lf,%lf,%lf,%lf,%lf\n", number_of_threads_per_block, number_of_blocks, omp_threads, total_time, gpu_time_sec,cuda_search, tree_creation_time, tree_linearization_time);

  fclose(fp);
  return 0;
}

/**
 * @brief Initializes the red-black-tree with random values, reading it from a file
 * @param rbt        pointer to the red-black-tree
 * @param num_nodes  number of nodes to insert into the rbt
 * @note make sure that the maxsize.txt exist in the input folder, it's generated by generate_big.py.
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

/**
 * @brief Cuda version of rb_find
 * @param rbt pointer to the red-black-tree
 * @param data pointer to the searched data
 * @return Returns pointer to the node that contains the searched data, NULL if not found
 */
rbnode *d_rb_find(rbtree *rbt, void *data)
{
  /* time variables */
  cudaEvent_t start, stop;
  clock_t start_d, end_d;

  /* we set the count variable of the nodes at log2(omp_num_threads),
    the rbt tree implementation could be changed so that this is an operation done during the insert, but 
    this was not done, so I have to call this function. I choose to not time it in the tree linearization */
  set_counts(rbt,RB_FIRST(rbt),log2(omp_threads),0);
  start_d = clock();
  cuda_search_start = clock();
  //mydata *h_data_array = (mydata*)malloc(sizeof(mydata) * rbt->count );
  // there seems not to be a difference between malloc/cudaMalloc at this size
  mydata *h_data_array;
  cudaMallocHost((void **)&h_data_array, ( sizeof(mydata) * rbt->count ));
  cudaCheckError();
  //linearizing tree
  rbnode **h_node_array = rb_node_array(rbt, omp_threads,h_data_array);

  end_d = clock();
  tree_linearization_time =  ((double)(end_d - start_d)) / CLOCKS_PER_SEC;

  /*
   * the node h_node_array[i] contains data that has the same value as
   * the data h_data_array[i]
   * mydata * test =(mydata*) h_node_array[3]->data;
   * printf("h_node_array[3] contiene : %d\n",test->key);
   * printf("h_data_array[3] contiene : %d\n",h_data_array[3].key);
   */

  /* device variables */
  mydata *d_data_array;

  /* start timing the search */
  cudaEventCreate(&start);
  cudaEventCreate(&stop);
  cudaEventRecord(start);

  /* device allocations */
  cudaMalloc((void **)&d_data_array, rbt->count * sizeof(mydata));
  cudaCheckError();

  /* copying the data from host to device */
  cudaMemcpy(d_data_array, h_data_array, rbt->count * sizeof(mydata), cudaMemcpyHostToDevice);
  cudaCheckError();

  /* setting up the kernel */
  /* number of threads for each block, we try 256,512,1024*/
  int threadsPerBlock = number_of_threads_per_block;
  /* we are working with a one dimensional array, so the blocks will be one dimension also */
  int blocksPerGrid = (rbt->count + threadsPerBlock - 1) / threadsPerBlock;
  /* to write the result to file later*/
  number_of_blocks = blocksPerGrid;

  /* invoking kernel */
  /* the device where to code has been tested has 8GB of memory and enough thread to handle every input test size,
  so only one execution of the kernel is necessary */
  d_find<<<blocksPerGrid, threadsPerBlock>>>(d_data_array, (*(mydata *)data));
  cudaDeviceSynchronize();
  cudaCheckError();

  /* copying the result on the host result array*/
  int found_index;
  rbnode *ret = NULL;
  /* copying from the device symbol (like a global variable for the device)*/
  cudaMemcpyFromSymbol(&found_index, d_found_index, sizeof(int));
  cudaCheckError();
 
  if (found_index != -1)
    ret = h_node_array[found_index];

  /* ret points to the searched node (if found), at this point d_rb_find is done the rest is just timing and frees*/
  /* end timing */
  cudaEventRecord(stop);
  cudaEventSynchronize(stop);

  float gpu_time;
  cudaEventElapsedTime(&gpu_time, start, stop);
  gpu_time_sec = gpu_time / 1000.0;

  cudaEventDestroy(start);
  cudaEventDestroy(stop);
  cudaFree(d_data_array);
  cudaFree(h_data_array);
  free(h_node_array);

  return ret;
}

/**
 * @brief Kernel, Every thread checks a node doing a memcmp, bypassing the normal compare function
 * @param data_array array of the data contained in the red-black tree.
 * @param searched_data the searched data.
 * @note Every thread checks an element of the data array based on his index with memcmp, if it correspond to the searched data
 * it changes the value of the device symbol d_found_index to its index, no need to worry about race conditions because the element is unique in the tree
 */
__global__ void d_find(const mydata *data_array, const mydata searched_data)
{
  int index = blockIdx.x * blockDim.x + threadIdx.x;
  

  if (d_memcmp(&searched_data, &data_array[index], sizeof(mydata)) == SUCCESS)
  {
    d_found_index = index; //no need for atomExch, each element is unique
  }
  
  // this is faster, but less generic
  // if(data_array[index].key == searched_data.key) d_found_index = index;
}

/**
 * @brief memcmp implementation for the device
 * @param s2 Pointer to the first memory block to be compared
 * @param s1 Pointer to the second memory block to be compared
 * @param n Number of bytes to be compared.
 * @note memcmp is public domain
 */
__device__ int d_memcmp(const void *s1, const void *s2, size_t n)
{
  const unsigned char *us1 = (const unsigned char *)s1;
  const unsigned char *us2 = (const unsigned char *)s2;
  while (n-- != 0)
  {
    if (*us1 != *us2)
    {
      return -1; // we do not care about order so i avoided the if that was here
    }
    us1++;
    us2++;
  }
  return SUCCESS;
}

