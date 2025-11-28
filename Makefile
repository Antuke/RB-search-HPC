# -----------------------------------COMPILERS-----------------------------------#
# THIS HAS TO BE CHANGED TO FIT THE MACHINE WHERE THE CODE HAS TO BE RUNNED
# c compiler
CC = gcc
# mpi wrapper if present
MPICC = gcc
# mpi flags if needed, default is default installation of ms-mpi. 
MPICC_FLAGS = -I"C:\\Program Files (x86)\\Microsoft SDKs\\MPI\\Include\\" -L"C:\\Program Files (x86)\\Microsoft SDKs\\MPI\\Lib\\x64\\" -lmsmpi
# mpi command to execute
EXECUTE = mpiexec -n 
# nvcc 
NVCC = nvcc
# compute capabilty flags, this has to be changed if the GPU has not compute capability 8.6
NVCC_FLAGS = -arch=compute_86 -code=sm_86
# -------------------------------------------------------------------------------#

# -----------------------------------TEST CONFIGURATIONS-----------------------------------#
NODES = 10000 100000 1000000 
PROCESSES = 1 2 4 6 8
OMP_THREADS = 1 4 8 16  # THIS HAVE TO BE POWER OF TWO (the parallel linearizations pretends it) 
ITERATION = 20
OPTIMIZATIONS = O0 O1 O2 O3
THREADS_PER_BLOCK = 256 512 1024 # 1024 is the MAX
# -----------------------------------------------------------------------------------------#


# Create the directories structures of the project
directories:
	python ./Pycodes/directories.py

# Create the aggregate csvs
statis:
	python ./Pycodes/aggregate_raws.py

# Compiles and link the projects (adds the prefix compile to the values of OPTIMITAZIONS) 
all: directories $(addprefix compile, $(OPTIMIZATIONS)) data

# creates the data file if not already present ( 10 milion unique integers)
data:
	python ./Pycodes/generate_big.py

# cleans the project
clean:
	rm -rf ./Build/*
	rm -rf ./Results/*
	rm -rf ./Tables/*

#run the test with different optimization and sizes and run python code to create tables
test: runmpi runcuda statis

runmpi:  mpitestO0 mpitestO1 mpitestO2 mpitestO3
runcuda: cudatestO0 cudatestO1 cudatestO2 cudatestO3


#mpitest runs also the serial version
mpitest%:
	@echo "Starting tests for optimization: $*"
	@for i in $$(seq 1 ${ITERATION}); do \
		echo "  Running Iteration: $$i"; \
		for n in $(NODES); do \
			for omp in $(OMP_THREADS); do \
				./Build/sequential_$*.exe $$n  $* 0; \
				./Build/sequential_$*.exe $$n  $* 1; \
				for proc in $(PROCESSES); do \
					$(EXECUTE) $$proc ./Build/find_mpi_$*.exe $$n $$omp $* 0; \
					$(EXECUTE) $$proc ./Build/find_mpi_$*.exe $$n $$omp $* 1; \
					echo "    Parameters: n=$$n, processes=$$proc, omp_threads=$$omp"; \
				done; \
			done; \
		done; \
	done



cudatest%:
	@for i in $$(seq 1 ${ITERATION}); do \
	echo "  Running Iteration: $$i"; \
		for n in $(NODES); do \
			for threads in $(THREADS_PER_BLOCK); do \
				for omp_threads in $(OMP_THREADS); do \
					./Build/find_cuda_$*.exe $$n $$threads $$omp_threads $*; \
					echo "    Parameters: n=$$n, threads=$$threads, omp_threads=$$omp_threads"; \
				done; \
			done; \
		done; \
	done; 


compile%:
	${CC} -c -o ./Build/rb.o ./Source/rb.c -$* -fopenmp
	${CC} -c -o ./Build/rb_data.o ./Source/rb_data.c -$* -fopenmp
	
	${CC} -c -o ./Build/sequential_$*.o ./Source/sequential.c -fopenmp -$* 
	${CC} -o ./Build/sequential_$*.exe ./Build/rb_data.o ./Build/rb.o ./Build/sequential_$*.o -fopenmp -$* 

	${MPICC} -c -o ./Build/find_mpi_$*.o ./Source/find_mpi.c ${MPICC_FLAGS} -fopenmp -$*  
	${MPICC} -o ./Build/find_mpi_$*.exe ./Build/rb_data.o ./Build/rb.o ./Build/find_mpi_$*.o ${MPICC_FLAGS} -fopenmp -$* 
	
# -Xptxas to optimize kernel also
	${NVCC} -o ./Build/find_cuda_$*.exe ./Source/rb.c ./Source/rb_data.c ./Source/find_cuda.cu -$* -Xptxas -$* -Xcompiler "-openmp" -diag-suppress 2464 ${NVCC_FLAGS}


## single test, ignore
mytestsize = 100000 # every node is 4 (left,right,parent,data) pointer  + 1 char (color) + an int(count) so 9 bytes + data is two int so 8 bytes
mytestproc = 16
singlempi%:
	${CC} -g -c -o ./Build/rb.o ./Source/rb.c -$* -fopenmp
	${CC} -g -c -o ./Build/rb_data.o ./Source/rb_data.c -$* -fopenmp
	${MPICC} -g -c -o ./Build/find_mpi_$*.o ./Source/find_mpi.c ${MPICC_FLAGS} -fopenmp -$* 
	${MPICC} -g -o ./Build/find_mpi_$*.exe ./Build/rb_data.o ./Build/rb.o ./Build/find_mpi_$*.o ${MPICC_FLAGS} -fopenmp -$*
	$(EXECUTE) -n ${mytestproc} ./Build/find_mpi_$*.exe  ${mytestsize} 4 $* 1
	

singleserial%:
	${CC} -g -c -o ./Build/rb.o ./Source/rb.c -$* -fopenmp
	${CC} -g -c -o ./Build/rb_data.o ./Source/rb_data.c -$* -fopenmp
	
	${CC} -g -c -o ./Build/sequential_$*.o ./Source/sequential.c -fopenmp -$* 
	${CC} -g -o ./Build/sequential_$*.exe ./Build/rb_data.o ./Build/rb.o ./Build/sequential_$*.o -fopenmp -$* 

	./Build/sequential_$*.exe ${mytestsize} $* 1
	
# -Xptxas="-v" to find out registers per kernel
# -Xptxas -dlcm=cg disable L1 cache
singlecuda%:
	${NVCC} -o ./Build/find_cuda_$*.exe ./Source/rb.c ./Source/rb_data.c ./Source/find_cuda.cu -$* -Xcompiler "-openmp" -diag-suppress 2464 ${NVCC_FLAGS}
	./Build/find_cuda_$*.exe ${mytestsize} 512 4 $* 


bigtest:
	@for _ in $$(seq 1 10);do\
		./Build/find_cuda_O3.exe ${mytestsize} 256 4 O3;\
		./Build/sequential_O3.exe ${mytestsize} O3 1;\
	done;

