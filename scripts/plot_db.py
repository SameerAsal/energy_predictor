import matplotlib as mpl
import matplotlib.pyplot as plt 
from matplotlib.backends.backend_pdf import PdfPages
from   pylab import *
import datetime 
import sqlite3
import ConfigParser
import os
import pdb
import inspect
from inspect import currentframe, getframeinfo


# Ploting the graphs for the data I am getting.
# for data picked up from the mic runs, pic the ones with the smallest energy/time among all the threads  and compare oit to the
# cpu runs data.



# View mic_run_agg: 

# CREATE VIEW mic_run_agg AS select num_threads, kernel_config_id , 
# avg(exec_time) as avg_exec_time, min(exec_time) as min_exec_time, max(exec_time) as max_exec_time , 
# avg(total0) as avg_total0 , max(total0) as max_total0, min(total0) as min_total0 
# FROM mic_run group by kernel_config_id, num_threads

# Query selecting  all the runs that are available with reasonable readings:

# select  (max_total0 - min_total0)/(avg_total0) , num_threads ,kernel_conifg_id  from mic_run_agg where kernel_config_id in 
# (select rowid from kernel_config where kernel_name like "%par%") AND 
# (max_total0 - min_total0)/(avg_total0) < 0.1;

# we shouldn't take the matmul_orig_par


#Pick only the accepted reading, the ones that have very little erros in the 


# CREATE VIEW mic_run_agg_accepted AS 
  #select  num_threads , avg_total0 , avg_exec_time , kernel_config_id  from mic_run_agg where kernel_config_id in
  #(select rowid from kernel_config where kernel_name like "%par%" AND not kernel_name like "%orig%") 
  #AND (max_total0 - min_total0)/(avg_total0) < 0.1 AND avg_exec_time >  1000 ;




#Pick kernel names that  
#select distinct kernel_name from kernel_config where rowid in     (select kernel_config_id  from mic_run_agg_accepted where kernel_config_id in  (select rowid from kernel_config));

# module_name, package_name, ClassName, method_name, ExceptionName, function_name, 
# GLOBAL_CONSTANT_NAME, global_var_name, instance_var_name, function_parameter_name, local_var_name
#


class Plotter:
  create_view_agg ='''Drop view if exists mic_run_agg; 
                    CREATE VIEW if not exists mic_run_agg AS select 
                    num_threads AS num_threads, 
                    kernel_config_id AS kernel_config_id,
                    avg(exec_time)   AS avg_exec_time, min(exec_time) AS min_exec_time, max(exec_time) AS max_exec_time,
                    avg(total0)      AS avg_total0,       max(total0) AS max_total0,       min(total0) AS min_total0,

                    (max(total0)    - min(total0))/avg(total0)        AS relative_err_energy,
                    (max(exec_time) - min(exec_time))/avg(exec_time)  AS relative_err_exec_time

                    FROM mic_run 
                    group by mic_run.kernel_config_id, mic_run.num_threads;'''

  create_view_accepted = """ Drop view if exists mic_run_agg_accepted; 
                            CREATE VIEW if not exists mic_run_agg_accepted AS select 
                            num_threads      AS num_threads, 
                            kernel_config_id AS kernel_config_id, 
                            avg_total0       AS avg_total0, 
                            avg_exec_time    AS avg_exec_time,
                            relative_err_energy    AS relative_err_energy,
                            relative_err_exec_time AS relative_err_exec_time
                            from mic_run_agg where kernel_config_id in 
                            (select rowid from kernel_config where kernel_name like \"%par%\" AND not kernel_name like \"%orig%\")
                            AND relative_err_energy < 0.1 AND relative_err_exec_time < 0.1 """
                            ##AND avg_exec_time >  1000; """

  select_kernel_names   = """ select distinct kernel_name from kernel_config, mic_run_agg_accepted 
                             where mic_run_agg_accepted.kernel_config_id = kernel_config.rowid """

  #select_kernel_ids      = """ select distinct kernel_config.rowid  from kernel_config, mic_run_agg_accepted 
  #                           where mic_run_agg_accepted.kernel_config_id = kernel_config.rowid """


  select_kernel_ids      = """ select distinct kernel_config_id from mic_run_agg_accepted"""

  select_kernels_run_data= """select  kernel_config.rowid, kernel_name, size_n, size_m, size_k, 
                             mic_run_agg_accepted.num_threads,mic_run_agg_accepted.avg_exec_time , mic_run_agg_accepted.avg_total0  
                             from kernel_config, mic_run_agg_accepted 
                             where mic_run_agg_accepted.kernel_config_id = kernel_config.rowid"""                         
                             

  select_kernel_run_template= """select  kernel_config.rowid, kernel_name, size_n, size_m, size_k, 
                             mic_run_agg_accepted.num_threads,   
                             mic_run_agg_accepted.avg_exec_time, 
                             mic_run_agg_accepted.avg_total0,
                             relative_err_energy, 
                             relative_err_exec_time 
                             from kernel_config, mic_run_agg_accepted 
                             where mic_run_agg_accepted.kernel_config_id = kernel_config.rowid 
                             AND kernel_config.rowid = KER_ID"""


                             

  def __init__(self, conf):
    config      = ConfigParser.ConfigParser()
    config.read(conf)

    self.threshold_energy_err=config.get("DATA", "energy_err")
    self.threshold_energy_val=config.get("DATA", "energy_val")
    self.threshold_time_err  =config.get("DATA", "time_err")
    self.threshold_time_val  =config.get("DATA", "time_val")
    self.output_path         =config.get("OUTPUT", "path")


    print self.threshold_energy_err
    print self.threshold_energy_val
    print self.threshold_time_err  
    print self.threshold_time_val  


    self.dbname = "./" + config.get("DATA_BASE", "dbname") + ".db"
    if not os.path.exists(self.dbname):
      err_msg  = "Data base file " + self.dbname + " doesn't exist !"
      raise Exception(err_msg)
    try:
      self.dbconn = sqlite3.connect(self.dbname)
      self.cursor = self.dbconn.cursor()
      self.create_views()
    except Exception as Err:
      raise Err

  def create_views(self):
    try:      
      self.cursor.executescript(Plotter.create_view_agg)
      self.cursor.executescript(Plotter.create_view_accepted)      
      self.dbconn.commit()
    except Exception as Err:
      err_str =  "An error occured while trying to create views:" + str(Err)
      raise Exception (err_str)
    return

  def plot_kernel_runs(self,runs, ker_name):
    #Plot the runs for threads Vs evergy and time
    threads= map(lambda item: float(item[5]), runs)
    time   = map(lambda item: float(item[6]), runs)
    energy = map(lambda item: float(item[7]), runs)

    subplot = plt.subplot(2,1,1)
    plt.title(ker_name)
    plt.xlabel("#Threads")
    plt.ylabel("#Energy")
    plot(threads, energy, 'b--')

    subplot = plt.subplot(2,1,2)
    plt.xlabel("#Threads")
    plt.ylabel("#Time")
    plot(threads, time, 'r--')
    savefig(self.output_path + "/" + ker_name + ".png")   
    plt.close()
    return

  def plot_all(self):
    try:      
      #Pick all the kernel names
      accepted_kernel_ids = self.dbconn.execute (Plotter.select_kernel_ids).fetchall()
      for ID in accepted_kernel_ids:
        select_str = Plotter.select_kernel_run_template.replace("KER_ID", str(ID[0]))
        results    = self.dbconn.execute(select_str).fetchall()
        ker_name   = results[0][1] + "(" + str(results[0][2]) + "," + str(results[0][3]) + "," + str(results[0][4]) + ")"
        msg        = ker_name + " with " + str(len(results)) + " thread runs"
        if (len(results) > 8):
          print "Accepted: " + msg
          self.plot_kernel_runs(results, ker_name)
    except Exception as Err:
      err_str = "An eror occurred while trying to read kernel names in plot_all\n" + str(Err) 
      raise Exception(err_str)

#(kernel Name ,Sizes) --> title
# Number of threads --> X
# Energy \ Time consumed --> Y axis.

def main():
  try:
    pp = Plotter("plots.cfg")
    pp.plot_all()
  except Exception as Err:
    print Err



  
 
if __name__ == "__main__":
    main()
