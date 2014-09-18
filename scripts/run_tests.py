# loops over sover all the benchmarks templates, write a kernel that with a specific size
# for each benchmarks then run make perf. 
# next loop get a different size and run the tests. 
from itertools import product
import os
import subprocess
import datetime
import shutil
import time 
import pdb
import ConfigParser
from os.path import expanduser

#Passs these arguments to the make perf script !
#Read those from config file.
PAPI_LIB   = "PAPI_LIB=" + expanduser("~") + "/svn/installations/papi_MIC_5.3_mic_host/lib"
POLYCC_LIB = "PLC="      + expanduser("~") + "/svn/installations/pluto/polycc"
#base       = "../mic_benchmarks/"
base       = "../benchmarks/"
#
bench_list = ["adi", "lu", "matmul", "jacobi-1d-imper", "jacobi-2d-imper"]  
tokens     = [ "%N_VAL%", "%M_VAL%", "%K_VAL%"]

varients   = ["orig_par",\
              "orig",\
              "par",\
              "tiled"]

data_files=["orig_par_timings.txt",\
            "orig_timings.txt", \
            "par_timings.txt",\
            "tiled_timings.txt" ]

def read_config(): 
  config = ConfigParser.ConfigParser()
  print "Config file: " + config.read("./runs.cfg")[0] #check which file was opemn, in the return value.
  #print config.sections()
  global PAPI_LIB
  global POLYCC_LIB 
  global base 

  PAPI_LIB   = "PAPI_LIB=" + config.get('LIBS', "PAPI_LIB")
  POLYCC_LIB = "PLC="      + config.get("LIBS", "POLYCC_LIB")
  base       = config.get("BEMNCHMARKS", "base")
  
  print PAPI_LIB 
  print POLYCC_LIB
  print "Base: " + base 

def move_files(kernel_name, directory):
  file_paths = map(lambda file : base  + kernel_name + "/" +file, data_files)
  dest_paths = map(lambda file : directory + "/" + kernel_name + "_"  + file, data_files)
  for file_path,dest_path  in zip(file_paths, dest_paths):
    if os.path.exists(file_path):
      print "Moving " + file_path 
      shutil.move(file_path,dest_path)


def test_all_versions(to_compose, tokens, text, bench, std_err, std_out):
  name = base + bench + "/" + bench  + ".c"
  #pdb.set_trace()
  for e in product(*to_compose):
    solid = text
    for i in range (0 , len(e)):       
      solid = solid.replace(tokens[i], str(e[i]))

    f = open(name, "w")
    f.write(solid)
    #Write the filled in template !
    f.close()
    #Now make perf
    print "now making: " + str(e) + " for benchmark " + bench
    try:
      print subprocess.check_call(["make", "-C", base + bench, "perf", PAPI_LIB, POLYCC_LIB], stderr=std_err, stdout=std_out)
      #print subprocess.check_call(["make", "-C", bench, "perf"], stderr=std_err, stdout=std_out)
    except ValueError:
      print "Error compiling " + bench + " sizes: " + str(e)
      print ValueError
      quit()


    #Now run each varient of the benchmark separetly, wait 10 escs between each run !
    print "in test all versions, Base is: " + base 
    for exe in varients:
      exe_file =  base + bench + "/" + exe
    #Run each 3 times
      for i in range(0,3):
        try:
          print "Now executing " + exe_file + " for the " + str(i) + " time !"
          subprocess.call([exe_file], stderr=std_err, stdout=std_out)
          print "now sleeping !"
          time.sleep(3)
        except IOError as err:
          print "ERRRRRRRRRR "  + "happened ! \n"
        except OSError as err:
          print "OSError ERRRRRRRRRR "  + "happened ! \n"
          print err
          print exe_file
          exit(0)
        except:
          print "Generic error, what was that !"
          exit(0)
           #{0}): {1}".format(e.errno, e.strerror)
      

    #make -C ./bench  perf

  

def run_tests():
  benchmarks = ["matmul", "jacobi-1d-imper","jacobi-2d-imper", "adi","lu"]

  n_Values = [128,256,512,1024,2048,3096,5096]
  m_Values = [128,256,348,512]
  k_Values = [128,256,348,512]

  
  to_compose = []

  # open to forward output and erro to.
  err_out = open("./std.err.out","w")
  for bench in benchmarks:
    to_compose = []
    name       = base  + bench + "/" + bench + ".template" + ".c"
    template   = open(name, 'r')
    text       = template.read()
    template.close()
    print "wrote " + name

    if not(-1 == text.find(tokens[0])):
      print bench + " " + tokens[0]
      to_compose.append(n_Values)

    if not(-1 == text.find(tokens[1])):
      print bench + " " +  tokens[1]
      to_compose.append(m_Values)

    if not(-1 == text.find(tokens[2])):
      print bench + " " + tokens[2]
      to_compose.append(k_Values)

    test_all_versions(to_compose, tokens, text, bench, err_out, err_out)
  
#close the output and error files.( I am using only one foe now).
  err_out.close() 

def clean_results():
  output_directory = base + "Results//" + datetime.datetime.now().strftime('%b-%d-%I%M%p-%G')
  
  if not os.path.exists(output_directory):
    os.makedirs(output_directory)

  for benchmark in bench_list: 
    move_files(benchmark, output_directory)

def main(): 
  read_config()
  clean_results()
  print "done with clean results"
  run_tests()
 
if __name__ == "__main__":
    main()

