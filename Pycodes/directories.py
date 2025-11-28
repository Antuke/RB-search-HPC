"""
/**
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
 * Purpose of the file: Create the folder structure for the project
 */
"""

import os


def create_directory_structure(base_dir, sub_dirs):
    """
    Create directory structure based on the given base directory and subdirectories.
    
    Args:
    - base_dir (str): The base directory path.
    - sub_dirs (list): List of subdirectories to be created inside the base directory.
    """
    for sub_dir in sub_dirs:
        dir_path = os.path.join(base_dir, sub_dir)
        os.makedirs(dir_path, exist_ok=True)  # Using exist_ok=True will not raise an error if directory exists
        
        # Create size subdirectories inside each opt directory
        if base_dir == 'Tables':
            size_sub_dirs = [f"size{i}" for i in range(1, 5)]  # size1, size2, size3, size4
            for size_sub_dir in size_sub_dirs:
                size_dir_path = os.path.join(dir_path, size_sub_dir)
                os.makedirs(size_dir_path, exist_ok=True)

if __name__ == "__main__":
    # Create the base directories
    base_directories = ['Results', 'Tables']
    for base_dir in base_directories:
        os.makedirs(base_dir, exist_ok=True)
        
    sub_directories = [f"opt{i}" for i in range(4)]  # opt0, opt1, opt2, opt3
    sub_directories_results = ['Cuda','Mpi','Sequential']
    create_directory_structure('Results', sub_directories_results)
    create_directory_structure('Tables', sub_directories)

    print("Directory structure created successfully!")
