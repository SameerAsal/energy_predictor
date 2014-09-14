# Use the data from the matmul to train models and then run these dat
from sklearn import linear_model
from sklearn import svm
from sklearn import preprocessing
import matplotlib.pyplot as plt
from itertools import product
import numpy
import pdb
import math
import sys

benchmarks  = ["matmul", 
               "jacobi-1d-imper",
               "jacobi-2d-imper",
               "adi",
               "lu"]

#benchmarks  = ["matmul", "adi", "lu"]
#benchmarks = ["matmul"]
#benchmarks = ["jacobi-1d-imper","jacobi-2d-imper", "adi","lu"]

                   # Features here !
features_lables = [" SIMD_FP_256:PACKED_DOUBLE",
   	             "FP_COMP_OPS_EXE:SSE_FP_PACKED_DOUBLE",
                     "PAPI_L1_DCM",
                     "PAPI_L2_DCM",
                     "PAPI_L1_LDM",
                     "PAPI_L1_STM",
                     "PAPI_L2_STM",
                   # Results here ! 
                     "rapl:::PACKAGE_ENERGY:PACKAGE0(J)",
                     "rapl:::PP1_ENERGY:PACKAGE0(J)",
                     "rapl:::PP0_ENERGY:PACKAGE0(J)",
                     "EXEC_TIME"]

features_labels = ["SIMD_FP_256:PACKED_DOUBLE",
#                  "FP_COMP_OPS_EXE:X87",
                  "FP_COMP_OPS_EXE:SSE_FP_PACKED_DOUBLE",
#                  "FP_COMP_OPS_EXE:SSE_FP_SCALAR_SINGLE",
#	           "FP_COMP_OPS_EXE:SSE_PACKED_SINGLE",
	           "PAPI_L1_DCM",
             	   "PAPI_L2_DCM",

	           "rapl:::PACKAGE_ENERGY:PACKAGE0(J)",
            	   "rapl:::PP1_ENERGY:PACKAGE0(J)",
		   "rapl:::PP0_ENERGY:PACKAGE0(J)",
                   "EXEC_TIME"]

ff = ["SIMD_FP_256:PACKED_DOUBLE",        #0
"FP_COMP_OPS_EXE:SSE_FP_PACKED_DOUBLE",   #1
"FP_COMP_OPS_EXE:SSE_FP_SCALAR_SINGLE",   #2
"FP_COMP_OPS_EXE:SSE_PACKED_SINGLE",      #3
"PAPI_SP_OPS",                            #4
"PAPI_DP_OPS",                            #5
"PAPI_L1_DCM",
"PAPI_L2_DCM",
"PAPI_L1_LDM",
"PAPI_L1_STM",
"PAPI_L2_STM",
"rapl:::PACKAGE_ENERGY:PACKAGE0(J)",
"rapl:::PP1_ENERGY:PACKAGE0(J)",
"rapl:::PP0_ENERGY:PACKAGE0(J)",
"EXEC_TIME"]

data_files=["orig_timings.txt", \
            "tiled_timings.txt"]

class cpu_modeler:

  def __init__(self, data_path):


    #Read the catalogue
    #Determine which data are results and what are features. 
    #Print an ID for each run, telling what kernel , size and run index it is. 
    #Use this iD as a foreign key to the values in the Runs.
    #Read the files from an input folder, don't always assume it is in benchmarks.

    self.all_data = None
    self.num_features = 5
    self.num_runs     = 3 #Number of times each sample was ran.
    self.num_results  = 4 #rapl:::PACKAGE_ENERGY:PACKAGE0(J)      -4   
			  #rapl:::PP1_ENERGY:PACKAGE0(J)          -3
			  #rapl:::PP0_ENERGY:PACKAGE0(J)          -2 
			  #EXEC_TIME                              -1


    for file_ in product(benchmarks, data_files):
      path = data_path + "/" + file_[0] + "_" + file_[1]
      if self.all_data is None:      
        self.all_data = numpy.loadtxt(path, skiprows=1)
      else: 
        new_data      = numpy.loadtxt(path, skiprows=1)
        self.all_data = numpy.concatenate((new_data, self.all_data))

    self.mash_data()
    print "Before filtering, number of data: " + str(len(self.all_data))
    #Filter out all the data with execution time < 500ms
    self.all_data = numpy.array(filter(lambda x: x[-1] > 100, self.all_data))
    print "After filtering, number of data: " + str(len(self.all_data))

    #Shuffles the rows only: http://docs.scipy.org/doc/numpy/reference/generated/numpy.random.shuffle.html
    numpy.random.shuffle(self.all_data)
    self.features = self.all_data[:, 0:self.num_features ]
    self.features =  preprocessing.scale(self.features)
    self.results  = self.all_data[:, self.num_features: ] 
    pdb.set_trace();
    #self.normalize_features() 
    #self.prune_data()

    #Now break data into training and valiation
    self.features_training = self.features[10:,:]
    self.features_testing  = self.features[0:10,:]

    # In the paper Measuring Energy and power with PAPI:
    # "Energy usage for the total 
    # processor package and the total combined energy used by all the cores 
    # (referred to as Power-Plane 0 (PP0))."
    self.energy_training   = self.results[10: ,-2]
    self.energy_testing    = self.results[0:10,-2]

  def mash_data(self):
   #Each sample was run three times, we need to collapse each three coseceutive runs into 1 
   new_data = numpy.ndarray(self.all_data.shape)
   new_data = numpy.empty([0,(self.num_features + self.num_results)]) 
   for row in range(0, self.all_data.shape[0], 3): 
     new_data = numpy.vstack([new_data,sum(self.all_data[row:row+3])/3.0])
   
   self.all_data = new_data   
 

  def prune_data(self):
  #Remove all the Columns tha have 0 average. 
    cols_to_delete = []
    for col in range(0, self.features.shape[1]):
      avg    = sum(self.features[:,col])/self.features.shape[1]
      if avg == 0:
        print "Col " + str(col) + " has zero values"
        cols_to_delete.append(col)

    #delete columns: http://docs.scipy.org/doc/numpy/reference/generated/numpy.delete.html
    #self.features = numpy.delete(self.features,[1,2,3,4], 1) 
    self.features = numpy.delete(self.features,cols_to_delete, 1) 
  

  def normalize_features(self):

    # The first six columns are features, the rest are outpput (Exec_time and three energy values)
    for col in range(0, self.features.shape[1]):
      avg    = sum(self.features[:,col])/self.features.shape[1]
      if avg == 0:
        print "Col " + str(col) + " has zero values"
        continue
      range_ = max(self.features[:,col]) - min(self.features[:,col])
      self.features[:,col] = map((lambda val: (val-avg)/range_), self.features[:,col])


  def run_regression(self):

    #self.energy_testing   = self.energy_training
    #self.features_testing  = self.features_training

    clf = linear_model.LinearRegression(normalize=False)
    print "Prediction with linear regression"
    clf.fit(self.features_training, self.energy_training)
    linear_reg = clf.predict(self.features_testing)
    #print linear_reg
    
    #Feature-Scale the data before you learn the model. 
    print "Prediction with linear regression with normalization"
    clf = linear_model.LinearRegression(normalize=True)
    clf.fit(self.features_training, self.energy_training)
    linear_reg_normalized = clf.predict(self.features_testing)
    
    #Try Ridge regression:
    print "Prediction with Ridge regression"
    clf = linear_model.Ridge()
    clf.fit(self.features_training, self.energy_training)
    linear_reg_ridge = clf.predict(self.features_testing)
 
    #Try SVM regression:
    clf = svm.SVR() 
    clf.fit(self.features_training, self.energy_training)
    svm_reg  = clf.predict(self.features_testing)

  #  plt.plot(linear_reg_ridge, "B",
  #           linear_reg_normalized, "R",
  #           self.energy_testing, "g--")


    #Compute the residual sum of squares:
    print "Residual sum of square: linear_reg: " + str(numpy.mean((linear_reg - self.energy_testing)**2))
    print "Residual sum of square: linear_reg_ridge: " + str(numpy.mean((linear_reg_ridge - self.energy_testing)**2))
    
    print "Average value for prediction error: linear_reg: " +       str(numpy.mean(numpy.absolute(linear_reg       - self.energy_testing)))
    print "Average value for prediction error: svm_reg: " +       str(numpy.mean(numpy.absolute(svm_reg       - self.energy_testing)))
    print "Average value for prediction error: linear_reg_ridge: " + str(numpy.mean(numpy.absolute(linear_reg_ridge - self.energy_testing)))
    print "Values for energy error under testing !"

    print "Now for linear regression" 
    print "expected\t\t\tactual\t\t\terror"
    zipped = zip(linear_reg, self.energy_testing)
    zipped.sort(key = lambda t:t[1])
    
    for val in zipped:
      print str(val[0]) + "\t\t\t" + str(val[1]) + "\t\t\t" + str(100*numpy.absolute(val[0] - val[1])/val[1]) + "%"

    print "Now for SVM regression" 
    print "expected\t\t\tactual\t\t\terror"
    zipped = zip(svm_reg, self.energy_testing)
    zipped.sort(key = lambda t:t[1])
    
    for val in zipped:
      print str(val[0]) + "\t\t\t" + str(val[1]) + "\t\t\t" + str(100*numpy.absolute(val[0] - val[1])/val[1]) + "%"


    #plt.show()
    #pdb.set_trace()

    # Use Logintsic regression to learn the model: 
    # clf = linear_model.LogisticRegression()
    # clf.fit(features_training, energy_training)
    # Use Logistic regression to learn the model
    # print " Prediction with logistic regression:"   
    # print clf.predict(features_testing)



def main(): 

  if len(sys.argv) < 2 :
    print "Please pass data path"
    print "Number of arguments" , len(sys.argv) , " and they are ", str(sys.argv)
    return 

  mm = cpu_modeler(str(sys.argv[1]))
  mm.run_regression()
 
if __name__ == "__main__":
    main()

