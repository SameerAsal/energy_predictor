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

#Not from config file:
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

# Dictionary of key value data followed by 
def insert_mic_run_info(kernel_config_id , output_file_path):
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
 
 col_names  = col_names  + "kernel_config_id)" 
 col_values = col_values + str(kernel_config_id) + ")"

 insert = "INSERT INTO  mic_run " + col_names + " values " + col_values + ";"
 print insert 

 cur = conn.cursor()
 cur.execute(insert)
 conn.commit()

 #quit()
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
   print "database existed, deleting !"
   os.remove(dbpath)
 
 #Create a connection and create the database
 create_query = ""
 conn   = sqlite3.connect(dbpath)

 if (not table_exists(conn, "kernel_config")):
   print "Will create kernel_config"
   create_query = "create table kernel_config (kernel_name TEXT ," \
                  "size_n  INTEGER, size_m  INTEGER, size_k  INTEGER);\n"
   conn.execute(create_query)
   conn.commit()

 if (not table_exists(conn, "mic_run")):
   print "Will create mic_run"
   create_query  = "create table mic_run  (" + mic_run_data + "," \
              "kernel_config_id INTEGER, FOREIGN KEY(kernel_config_id) REFERENCES kernel_config(ROWID));\n"
   conn.execute(create_query)
   conn.commit()

 if (not table_exists(conn, "cpu_run")):
   print "Will create cou_run"
   create_query  = "create table cpu_run  (" + cpu_run_data + "," \
              "kernel_config_id INTEGER, FOREIGN KEY(kernel_config_id) REFERENCES kernel_config(ROWID));\n"
   conn.execute(create_query)
   conn.commit()


 print " hererererere "
 return conn

def read_config(): 
  config = ConfigParser.ConfigParser()
  print "Config file: " + config.read("./runs.cfg")[0] #check which file was opemn, in the return value.

  global PAPI_LIB
  global POLYCC_LIB 
  global base
  global dbname
  global mic_run_data
  global cpu_run_data


  PAPI_LIB   = "PAPI_LIB=" + config.get("LIBS", "PAPI_LIB")
  POLYCC_LIB = "PLC="      + config.get("LIBS", "POLYCC_LIB")
  base       = config.get("BEMNCHMARKS", "base")
  dbname     = config.get("DATA_BASE", "dbname")
  mic_run_data  = config.get("DATA_BASE","mic_run_data")

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
      print subprocess.check_call(["make", "-C", base + bench, "perf", PAPI_LIB, POLYCC_LIB], stderr=std_err, stdout=std_out)
    except ValueError:
      print "Error compiling " + bench + " sizes: " + str(e)
      print ValueError
      #quit()


    #Now run each varient of the benchmark separetly, wait 10 escs between each run !
    for exe in varients:
      kernel_config_id  = insert_kernel_config(bench + "_" + exe, e)
      exe_file          = base + bench + "/" + exe
      output_file_path  = exe_file + "_timings" + ".txt"
      #Run each 3 times
      for i in range(0,3):
        try:
          print "Now executing " + exe_file + " for the " + str(i) + " time "
          subprocess.check_call([exe_file], stderr=std_err, stdout=std_out)
          insert_mic_run_info(kernel_config_id, output_file_path)
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
  std_err = open("./std.err","w")
  std_out = open("./std.out","w")
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
  quit()
  clean_results()
  run_tests()
 
if __name__ == "__main__":
    main()

