import matplotlib as mpl
mpl.use('Agg')
import os
import matplotlib.pyplot as plt 
from matplotlib.backends.backend_pdf import PdfPages
import pylab
from   pylab import *
import pdb
import numpy as np 
import datetime 
import shutil
import math
import Image
#
#This section generates gaphs from HFy benchmarking.
#

#***************************************************************************************************************************************************#

def get_counters(kernel_name):
  counters = []
  lines      = open("./" + kernel_name + "/orig_par_timings.txt").read().split('\n')
  for line in lines:
    if line: # check line is not empty
      counters.append((line.strip()).split('\t')[0])

  return counters

def plot_graphs(kernel_name, counters, directory):
  
  data = []
  data_files=["orig_par_timings.txt",\
              "orig_timings.txt", \
              "par_timings.txt",\
              "tiled_timings.txt" ]

  x_label = map(lambda file : file[0:len(file) - len("_timings.txt")], data_files)
  file_paths = map(lambda file : "./" + kernel_name + "/" +file, data_files)
  #Also copy the data files to the data folder.
  dest_paths = map(lambda file : directory + "/" + kernel_name + "_"  + file, data_files)
  # Read the data from each data file. Data resides in the second column, first column for the counter name.
  for file_path,dest_path  in zip(file_paths, dest_paths):
    data.append(np.loadtxt(file_path , usecols=[1]))
    shutil.copyfile(file_path,dest_path)
  data = np.asmatrix(data)
  fig = plt.figure(1)
  fig.subplots_adjust(wspace=0.4, hspace = 0.3)
  fig.suptitle(kernel_name, fontsize = 20)
  # Create a graph for each hardware counter separetly!!
  from matplotlib.backends.backend_pdf import PdfPages
  pdf_file = PdfPages(str(directory +"//" + kernel_name + "_perf.pdf"))

  for counter in range(0,len(counters)):
    #http://matplotlib.org/api/pyplot_api.html#matplotlib.pyplot.subplot
    #           (num_rows, num_cols, plot_index)
    #plt.subplot(ceil(len(counters)/2.0),2,counter + 1)
    # It is simple, one page one figure: http://blog.marmakoide.org/?p=94
    if counter%2 == 0:
      fig = plt.figure()

    subplot = plt.subplot(2,1,counter%2 + 1)
    # Had to do this because the table was getting read as table cells instead of integers\floats. 
    l = map(lambda item: float(item), data[:,counter])
    # Ploto bar charts, first parameter is a range of numbers with length same as number of bars !!
    #TODO: Add the problem size to the graph !!
    #TODO: Add the actual number to the bar chart !!
    plt.bar(np.arange(len(l)),l)
    plt.title(counters[counter], fontsize=10)
    plt.xticks(np.arange(len(l)) + 0.5, x_label)
    if counter%2 == 1: #Close file\plot after two plots were drawn.
      pdf_file.savefig()
      plt.close()

  if len(counters)%2 == 1:
    pdf_file.savefig()
    plt.close()
    
  pdf_file.close()


def plot_graphs_separate(kernel_name, counters, directory):
  
  data = []
  data_files=["orig_par_timings.txt",\
              "orig_timings.txt", \
              "par_timings.txt",\
              "tiled_timings.txt" ]

  x_label = map(lambda file : file[0:len(file) - len("_timings.txt")], data_files)
  file_paths = map(lambda file : "./" + kernel_name + "/" +file, data_files)
  #Also copy the data files to the data folder.
  dest_paths = map(lambda file : directory + "/" + kernel_name + "_"  + file, data_files)
  # Read the data from each data file. Data resides in the second column, first column for the counter name.
  for file_path,dest_path  in zip(file_paths, dest_paths):
    data.append(np.loadtxt(file_path , usecols=[1]))
    shutil.copyfile(file_path,dest_path)

  data = np.asmatrix(data)
  # Create a graph for each hardware counter separetly!!
  # friom matplotlib.backends.backend_pdf import PdfPages
  # pdf_file = PdfPages(str(directory +"//" + kernel_name + "_perf.pdf"))

  from matplotlib.backends.backend_pdf import PdfPages
  for counter in range(0,len(counters)):
    #http://matplotlib.org/api/pyplot_api.html#matplotlib.pyplot.subplot
    #           (num_rows, num_cols, plot_index)
    #plt.subplot(ceil(len(counters)/2.0),2,counter + 1)
    # It is simple, one page one figure: http://blog.marmakoide.org/?p=94
    #fig.suptitle(kernel_name + "_" + counters[counter], fontsize = 20)
    #pdf_file = PdfPages(str(directory +"//" + kernel_name + "_" + str(counter) + "_perf.pdf"))
    fig      = plt.figure(1)
    subplot  = plt.subplot(111)
    values   = map(lambda item: float(item), data[:,counter])
    idx      = np.arange(len(values))
    plt.bar(idx, values, label =counters[counter])
    # That is how east it is to show the legend, just use legend()
    legend()
    plt.title(counters[counter], fontsize=10)
    plt.xticks(np.arange(len(values)) + 0.5, x_label)
    
    #pdf_file.savefig()
    fig_name = str(directory +"//" + kernel_name + "_" + counters[counter] + "_perf")
    pylab.savefig(fig_name + ".png")
    plt.close()
    #pdf_file.close()
    img = Image.open(fig_name + ".png")
    img.save(fig_name + ".jpeg")
 
def main():
  bench_list       = ["adi", "lu", "matmul", "jacobi-1d-imper", "jacobi-2d-imper"]
  output_directory = "Results//" + datetime.datetime.now().strftime('%b-%d-%I%M%p-%G')
  output_directory = "Results//" + "test_directory" 
  
  if not os.path.exists(output_directory):
    os.makedirs(output_directory)

  for benchmark in bench_list: 
    counters_list =  get_counters(benchmark)
    plot_graphs_separate(benchmark, counters_list, output_directory)

if __name__ == "__main__":
    main()
