
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
import pandas as pd
#import dataframe_image as dfi

sizes = ["10000", "100000", "1000000"]
#cuda_clms = ["number_of_threads_per_block","number_of_blocks","total_time","gpu_time_sec","tree_creation_time","tree_linearization_time"]
#sequential_clms = ["total_time","binary_search_time","tree_creation_time"]
#mpi_clms = ["number_of_processes","omp_num_threads","total_time","communication_time","binary_search_time","tree_creation_time","tree_linearization_time"]


def get_resultfn(file_name,info,type):
    """
    Parses the file_name to obtain the path where to write the results
    """

    size = file_name.split('_')[1]
    Path = './Tables/opt'
    if "O0" in file_name:
        Path+="0"
        info+="0_"
    if "O1" in file_name:
        Path+="1"
        info+="1_"
    if "O2" in file_name:  
        Path+="2"
        info+="2_"
    if "O3" in file_name:
        Path+="3"
        info+="3_"
    if sizes[2] == size:
        Path+="/size3/"
        info+="size3_"
    if sizes[1] == size:
        Path+="/size2/"
        info+="size2_"
    if sizes[0] == size:
        Path+="/size1/"
        info+="size1_"
    

    if "lc" in file_name:
        Path+="lc_"
        info+="lc"
    if "sc" in file_name:
        Path+="sc_"
        info+="sc"

    Path+=type
    info+=type
    
    return Path



def process_csv_files(directory):
    """
    Process the csv files and create csv with mean values and jpgs of those csv
    """
    
    for root, dirs, files in os.walk(directory):
        for file in files:
            print('processing '+root+file)

            file_path = os.path.join(root, file)  
            df = pd.read_csv(file_path)
            
            sequential_flag = 0
            cuda = 0
            columns = df.columns.tolist()
            filter_df = pd.DataFrame()
            info = ""

            # filter out rows with values that are too big too small ( if abs(row[clm] - mean) >= std)
            for index, row in df.iterrows():
                include_row = True  
                count = 0
                for clm in columns:
                    if 'time' in clm:
                        mean = df[clm].mean()
                        std = df[clm].std()
                        if mean != 0.0 and std != 0.0:
                            if abs(row[clm] - mean) >= 2 * std:
                                include_row = False
                                break  
                        else:
                            count += 1

                if include_row:
                    filter_df = pd.concat([filter_df, df.loc[index:index]], ignore_index=True)

            if "Sequential" in root:
                resultpath = get_resultfn(file_path,info,"Serial")
                sequential_flag = 1

                min_df = df.min()
                min_df.to_csv(resultpath+info+"_min.csv")

                filter_df = filter_df.mean()
                resultpath = get_resultfn(file_path,info,"Serial")

            if "Cuda" in root:
                cuda = 1 
                resultpath = get_resultfn(file_path,info,"Cuda")
                filter_df = filter_df.groupby(['threads','blocks','omp_threads']).mean().reset_index()

                min_df = df.groupby(['threads','blocks','omp_threads']).min().reset_index()
                min_df.to_csv(resultpath+info+"_min.csv")

            if "Mpi" in root:
                filter_df = filter_df.groupby(['processes','omp_threads']).mean().reset_index()
                resultpath = get_resultfn(file_path,info,"Mpi")

                min_df = df.groupby(['processes','omp_threads']).min().reset_index()
                min_df.to_csv(resultpath+info+"_min.csv")
                
            if sequential_flag == 1:
                rounded_agg = filter_df.round(6)
                result_df = rounded_agg.to_frame().T
                result_df.to_csv(resultpath+".csv")
                #dfi.export(result_df,resultpath+".jpg")
            else:
                rounded_agg = filter_df.round(6)
                if cuda == 1:
                    sorted_agg = rounded_agg.sort_values(['threads','blocks','omp_threads'])
                else:
                    sorted_agg = rounded_agg.sort_values(['processes','omp_threads'])

                sorted_agg.to_csv(resultpath+".csv",float_format='%.6f')  
                #dfi.export(sorted_agg,resultpath+".jpg")


#MPI : number_of_processes, omp_num_threads, total_time, communication_time, binary_search_time, tree_creation_time
#CUDA :  number_of_threads_per_block, number_of_blocks, total_time, gpu_time_sec, tree_creation_time
#SEQUENTIAL : total_time, binary_search_time, tree_creation_time



# Specify the path to the Results directory
results_directory = './Results/'

# Iterate over the mpi, cuda, and sequential subdirectories
subdirectories = ["Sequential/","Mpi/", "Cuda/"]

for subdir in subdirectories:
    full_path = os.path.join(results_directory, subdir)
    if os.path.exists(full_path):
        process_csv_files(full_path)
    

