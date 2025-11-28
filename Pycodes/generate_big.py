
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
 * Purpose of the file: Create the input data if not present
 */
"""
import random
import os

def generate_unique_random_numbers(n, min_val, max_val):
    """Generate n unique random numbers between min_val and max_val."""
    return random.sample(range(min_val, max_val + 1), n)

def write_to_file(numbers, filename):
    """Write numbers to a text file."""
    with open(filename, 'w') as file:
        for num in numbers:
            file.write(f"{num}\n")

if __name__ == "__main__":
    num_unique_integers = 10000000 
    min_value = 1  
    max_value = 2147483640  
    output_file = '.\\input\\maxsize.txt'  # Output file name.
    
    if os.path.exists(output_file):
        exit()
    
    print("\n\nTEST\n\n")
    random_numbers = generate_unique_random_numbers(num_unique_integers, min_value, max_value)

    write_to_file(random_numbers, output_file)

    print(f"Generated {num_unique_integers} unique random integers between {min_value} and {max_value}.")
    print(f"Saved to {output_file}")