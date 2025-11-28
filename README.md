# RBSearch with OMP/MPI and OMP/CUDA

## CREDIT FOR RED BLACK TREE IMPLEMENTATION IN C 
Copyright (c) 2019 xieqing
https://github.com/xieqing/red-black-tree

## Dependencies

* MPI library
* an NVIDIA graphic card with Cuda setup
* Python
* Pandas pyhton library (that can be installed with pip install pandas)

## How to run 
IMPORTANT: CHANGE THE MAKEFILE VARIABLES 

1. Change the Makefile COMPILERS variables to fit the machine where the code has to be run if necessary (default is Windows 10 with ms-mpi library and compute capabilty 8.6).
2. Enther the command 'make clean' to remove previous results
3. Enter the command 'make all' to compile the project
4. Enter the command 'make test' to run the tests, you can change the test parameters (e.g number of iteratation) in the Makefile

The raw results of the tests (time measurements) can be viewed in the Results folder.
The Cuda results will be in the Cuda folder, the Mpi in the Mpi folder and sequential in the Sequential folder.
The csv file name follow this rule OPTIMIZATION_NODES_COMPARETYPE.csv (cuda does not have comparetype)

If the Pandas library has been installed, then in the Tables folder divided by optimization and tree size there will be the processed CSVs, with avarages values, minimum values, sorted by optimization and size. 

The test present in the report where obtained with 50 iterations.

## Design choices

Check the [Design Report](Report_RBSearch.pdf)