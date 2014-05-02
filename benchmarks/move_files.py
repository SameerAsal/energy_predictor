import datetime 
import os
import shutil
import math
#
#

#*****************************************************************************************************************************************#
def move_files(kernel_name, directory):
  
  data_files=["orig_par_timings.txt",\
              "orig_timings.txt", \
              "par_timings.txt",\
              "tiled_timings.txt" ]

  file_paths = map(lambda file : "./" + kernel_name + "/" +file, data_files)
  dest_paths = map(lambda file : directory + "/" + kernel_name + "_"  + file, data_files)
  for file_path,dest_path  in zip(file_paths, dest_paths):
    print "Moving " + file_path 
    shutil.move(file_path,dest_path)


def main():
  bench_list       = ["adi", "lu", "matmul", "jacobi-1d-imper", "jacobi-2d-imper"]
  output_directory = "Results//" + datetime.datetime.now().strftime('%b-%d-%I%M%p-%G')
  
  if not os.path.exists(output_directory):
    os.makedirs(output_directory)

  for benchmark in bench_list: 
    move_files(benchmark, output_directory)

if __name__ == "__main__":
    main()
