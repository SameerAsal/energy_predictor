import matplotlib as mpl
mpl.use('Agg')
import matplotlib.pyplot as plt 
from matplotlib.backends.backend_pdf import PdfPages
from   pylab import *
import datetime 
import sqlite3
import ConfigParser
import os

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
  create_view_agg = '''
                    Drop view if exists mic_run_agg; 
                    CREATE VIEW if not exists mic_run_agg AS select 
                    num_threads, kernel_config_id, avg(exec_time) as avg_exec_time, min(exec_time) as min_exec_time, max(exec_time) as max_exec_time, 
                    avg(total0) as avg_total0 , max(total0) as max_total0, min(total0) as min_total0 
                    FROM mic_run 
                    group by mic_run.kernel_config_id, mic_run.num_threads;'''

  create_view_accepted = """ Drop view if exists mic_run_agg_accepted; 
                            CREATE VIEW if not exists mic_run_agg_accepted AS select 
                            (max_total0 - min_total0)/(avg_total0) AS relative_err,avg_total0 AS avg_total0, avg_exec_time AS avg_exec_time
                             ,num_threads AS num_threads, kernel_config_id AS kernel_config_id  
                            from mic_run_agg where kernel_config_id in 
                            (select rowid from kernel_config where kernel_name like \"%par%\" AND not kernel_name like \"%orig%\") 
                            AND (max_total0 - min_total0)/(avg_total0) < 0.1 AND avg_exec_time >  1000; """

#  select_kernel_names = """ select distinct kernel_name from kernel_config where rowid in     
#                            (select kernel_config_id  from mic_run_agg_accepted where kernel_config_id in  
#                            (select rowid from kernel_config)); """ 

  select_kernel_names   = """ select distinct kernel_name from kernel_config, mic_run_agg_accepted 
                             where mic_run_agg_accepted.kernel_config_id = kernel_config.rowid """

  select_kernel_ids      = """ select distinct kernel_config.rowid  from kernel_config, mic_run_agg_accepted 
                             where mic_run_agg_accepted.kernel_config_id = kernel_config.rowid """

  select_kernel_run_data= """select  kernel_config.rowid, kernel_name, size_n, size_m, size_k, 
                             mic_run_agg_accepted.num_threads,mic_run_agg_accepted.avg_exec_time , mic_run_agg_accepted.avg_total0  
                             from kernel_config, mic_run_agg_accepted 
                             where mic_run_agg_accepted.kernel_config_id = kernel_config.rowid""" 
                             #AND where kernel_config.rowid = KER_ID"""
                             #group by kernel_config.rowid"""

  def __init__(self, conf):
    config      = ConfigParser.ConfigParser()
    config.read(conf)
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

  def plot_all(self):
    try:
      #Pick all the kernel names
      accepted_kernel_ids = self.dbconn.execute (Plotter.select_kernel_ids).fetchall()
      print "Accepted kernel IDs: " 
      for each t in accepted_kernel_ids:
        print t[0]

      #for row in self.dbconn.execute (Plotter.select_kernel_run_data):

    except Exception as Err:
      err_str = "An eror occurred while trying to read kernel names in plot_all:\n" + str(Err) 
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
