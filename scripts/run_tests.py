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
import sqlite3
from os.path import expanduser

# Read those from config file.
# Create a config struct for these variables 
PAPI_LIB     = "PAPI_LIB=" + expanduser("~") + "/svn/installations/papi_MIC_5.3_mic_host/lib"
POLYCC_LIB   = "PLC="      + expanduser("~") + "/svn/installations/pluto/polycc"
base         = "../benchmarks/"
dbname       = "NONAME"
mic_run_data = " total0 REAL, total1 REAL, pcie REAL, EXEC_TIME REAL"
cpu_run_data = ""
conn = ""
run_on = "cpu"
papi_interface_conf = ""

#Not from config file:
bench_list = ["adi", "lu", "matmul", "jacobi-1d-imper", "jacobi-2d-imper"]  
tokens     = [ "%N_VAL%", "%M_VAL%", "%K_VAL%"]
varients   = ["par", \
              "orig_par",\
              "orig",\
              "tiled"]

data_files=["orig_par_timings.txt",\
           "orig_timings.txt", \
            "par_timings.txt",\
            "tiled_timings.txt" ]

thread_sizes_mic = [2, 4 , 8 , 10, 12, 24, 32, 36, 40, 80, 100, 128, 160, 180, 200, 220 ]
thread_sizes_cpu = [2, 4 , 8 , 10, 12, 24]

# Dictionary of key value data followed by 
def insert_mic_run_info(kernel_config_id , output_file_path, num_threads):
 #open the file and read last two lines 
 output = open(output_file_path, "r")
 lines  = output.readlines()
 keys   = lines[-2].strip().split("\t")
 values = lines[-1].strip().split("\t")
 output.close()
 print " keys: "
 print keys
 print " values: "
 print values
 
 # insert_query = "insert into  kernel_config values  (\"" + kernel_name + "\"," + str(sizez[0]) + "," + size_m + "," + size_k + ")"
 #INSERT INTO table_name (column1,column2,column3,...)
 # VALUES (value1,value2,value3,...);
 col_names = "("
 col_values= "("

 for key_val in zip(keys, values):
  col_names = col_names + str(key_val[0]) + ","
  col_values= col_values+ str(key_val[1]) + ","
 
 col_names   = col_names + "num_threads,"
 col_values = col_values + str(num_threads) + ","
 
 col_names  = col_names  + "kernel_config_id)" 
 col_values = col_values + str(kernel_config_id) + ")"

 insert = "INSERT INTO  mic_run " + col_names + " values " + col_values + ";"
 print insert 
 cur = conn.cursor()
 cur.execute(insert)
 conn.commit()

 #quit()
 return  



def insert_cpu_run_info(kernel_config_id , output_file_path, num_threads):
 print "inserting cpu run "
 #open the file and read last two lines 
 try:
   output = open (output_file_path, "r")
   lines  = output.readlines()
   keys   = lines[-2].strip().split("\t")
   values = lines[-1].strip().split("\t")
   output.close()
   print "error happened "

   print " keys: "
   print keys
   print " values: "
   print values
  
   # insert_query = "insert into  kernel_config values  (\"" + kernel_name + "\"," + str(sizez[0]) + "," + size_m + "," + size_k + ")"
   #INSERT INTO table_name (column1,column2,column3,...)
   # VALUES (value1,value2,value3,...);
   col_names = "("
   col_values= "("
  
   for key_val in zip(keys, values):
    col_names = col_names + str(key_val[0]) + ","
    col_values= col_values+ str(key_val[1]) + ","
  
   print "adding number of threads" 
   col_names   = col_names  + "num_threads,"
   col_values  = col_values + str(num_threads) + ","

   col_names  = col_names  + "kernel_config_id)" 
   col_values = col_values + str(kernel_config_id) + ")"
  
   print "column names : " + col_names
   print "column values: " + col_values

   insert = "INSERT INTO  cpu_run " + col_names + " values " + col_values + ";"
   print insert
   import re
   #Need to replace all the "::" with an underscrores.  
   insert = re.sub('::*', "_" ,insert)
   #Need to replace all the "(J)" with empty spaces:
   insert = insert.replace('(J)', "" )
   print "After removing all the colons we get:"
   print insert
  
   cur = conn.cursor()
   cur.execute(insert)
   conn.commit()
 except ValueError as err:
   print "error in insert cpu run" + str(err)
   quit()
   return
 
 print "retirnining from insert cpu run safely"
 return  

def insert_kernel_config(kernel_name, sizez):
  size_m = "-1"
  size_k = "-1"

  #First query the connection for this same config
  query = "select rowid from kernel_config where kernel_name = \"" + kernel_name + "\"  and size_n = " + str(sizez[0])
  if (len(sizez) > 1):
    query  = query + " AND  size_m = " + str(sizez[1])
    size_m = str(sizez[1])
  if (len(sizez) > 2):
    query = query + " AND  size_k = " + str(sizez[2])
    size_k = str(sizez[2])
  query = query + ";" 
  #print query

  cur = conn.cursor()
  cur.execute(query)
  data = cur.fetchall();
 
  if (len(data) > 0):
    print  "Entry found for this configuration, exiting !"
    print int(data[0][0])
    return data[0][0] 
  else: 
    print "NOT Found Data. Inserting "
  
  insert_query = "insert into  kernel_config values  (\"" + kernel_name + "\"," + str(sizez[0]) + "," + size_m + "," + size_k + ")"
  cur.execute(insert_query)
  conn.commit()
  #Now do the same query to get the ID ( I know stupid right !!)
  cur.execute(query)
  data = cur.fetchall()
  print data[0][0]
  return data[0][0]


def table_exists(conn, table_name):
  exists = "select * from  sqlite_master WHERE type='table' AND name = '" + table_name + "'"
  cur    = conn.cursor()
  cur.execute(exists)
  return (not (len(cur.fetchall()) == 0))

def create_db():
 global dbname

 dbpath = "./" + dbname + ".db"
 if os.path.exists(dbpath):
   print "database exists  !"
   #os.remove(dbpath)
 
 #Create a connection and create the database
 create_query = ""
 conn   = sqlite3.connect(dbpath)

 if (not table_exists(conn, "kernel_config")):
   print "Will create kernel_config"
   create_query = "create table kernel_config (kernel_name TEXT ," \
                  "size_n  INTEGER, size_m  INTEGER, size_k  INTEGER);\n"
   conn.execute(create_query)
   conn.commit()
 else:
   print "kernel_config already exists"

 if (not table_exists(conn, "mic_run")):
   print "Will create mic_run"
   create_query  = "create table mic_run  (" + mic_run_data + "," \
              "num_threads INTEGER, kernel_config_id INTEGER, FOREIGN KEY(kernel_config_id) REFERENCES kernel_config(ROWID));\n"
   conn.execute(create_query)
   conn.commit()
 else:
   print "mic_rn already exists"

 if (not table_exists(conn, "cpu_run")):
   print "Will create cpu_run"
   create_query  = "create table cpu_run  (" + cpu_run_data + "," \
              "num_threads INTEGER, kernel_config_id INTEGER, FOREIGN KEY(kernel_config_id) REFERENCES kernel_config(ROWID));\n"
   print "Will execute: " + create_query
   conn.execute(create_query)
   conn.commit()
 else: 
  print "cpu run alredy exists"


 return conn

def read_config(): 
  config = ConfigParser.ConfigParser()
  print "Config file: " + config.read("../config/runs.cfg")[0] #check which file was opemn, in the return value.

  global PAPI_LIB
  global POLYCC_LIB 
  global base
  global dbname
  global mic_run_data
  global cpu_run_data
  global run_on
  global papi_interface_conf

  PAPI_LIB   = "PAPI_LIB=" + config.get("LIBS", "PAPI_LIB")
  POLYCC_LIB = "PLC="      + config.get("LIBS", "POLYCC_LIB")

  base        = config.get("BENCHMARKS", "base")
  run_on      = config.get("BENCHMARKS", "run_on")
  papi_interface_conf = config.get("BENCHMARKS","papi_interface_conf")


  #if ( not (len(base) == len(run_on))):
  #  print "error, bases abd run_on dont have same length"
  #  quit()

  dbname        = config.get("DATA_BASE", "dbname")
  mic_run_data  = config.get("DATA_BASE","mic_run_data")
  cpu_run_data  = config.get("DATA_BASE", "cpu_run_data")

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
      subprocess.check_call(["make", "-C", base + bench, "perf", PAPI_LIB, POLYCC_LIB], stderr=std_err, stdout=std_out)
    except ValueError:
      print "Error compiling " + bench + " sizes: " + str(e)
      print ValueError

    print "Make finished" 
    #Now run each varient of the benchmark separetly, wait 10 escs between each run !
    for exe in varients:
      kernel_config_id  = insert_kernel_config(bench + "_" + exe, e)
      exe_file          = base + bench + "/" + exe
      output_file_path  = exe_file + "_timings" + ".txt"
      thread_sizes = []

      if ("par" in exe):
        if (run_on == "mic"):
          thread_sizes = thread_sizes_mic
        else:
          thread_sizes = thread_sizes_cpu
      else:
        thread_sizes = [1]

      #Run each 3 times
      for i in range(0,3):
       for num_thread in thread_sizes:
        try:
          if ("par" in exe):
            os.environ['OMP_NUM_THREADS'] =  str(num_thread)
            if (run_on == "mic"):
              os.environ["OFFLOAD_REPORT"] = "3"
  

          print "Now executing " + exe_file + " for the " + str(i) + " time "
          subprocess.check_call([exe_file], stderr=std_err, stdout=std_out, env=os.environ)

          if (run_on == "mic"):
            insert_mic_run_info(kernel_config_id, output_file_path, num_thread)
          if (run_on == "cpu"):
            insert_cpu_run_info(kernel_config_id, output_file_path, num_thread)

          print "now sleeping !"
          time.sleep(3)
        except IOError as err:
          print "ERRRRRRRRRR "  + "happened ! \n"
          print err
        except OSError as err:
          print "OSError ERRRRRRRRRR "  + "happened ! \n"
          print err
          print exe_file
          exit(0)
        except ValueError as err:
          print "Generic error, what was that !"
          print err
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
  std_err = open("../output/std.err","w")
  std_out = open("../output/std.out","w")
  for bench in benchmarks:
    to_compose = []
    name       = base  + bench + "/" + bench + ".template" + ".c"
    template   = open(name, 'r')
    text       = template.read()
    template.close()
    text = text.replace("%PAPI_INTERFACE_CONF%", "\"" +  papi_interface_conf + "\"");
 
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

    test_all_versions(to_compose, tokens, text, bench, std_err, std_out)
  
#close the output and error files.( I am using only one foe now).
  std_err.close() 
  std_out.close()

def clean_results():
  output_directory = base + "Results//" + datetime.datetime.now().strftime('%b-%d-%I%M%p-%G')
  
  if not os.path.exists(output_directory):
    os.makedirs(output_directory)

  for benchmark in bench_list: 
    move_files(benchmark, output_directory)

def main(): 
  global conn
  read_config()
  conn = create_db()
  clean_results()
  run_tests()
 
if __name__ == "__main__":
    main()

