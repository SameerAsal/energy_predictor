# Use the data from the matmul to train models and then run these dat
from sklearn import linear_model
import matplotlib.pyplot as plt
from itertools import product
import numpy
import pdb
import math


benchmarks  = ["matmul", 
               "jacobi-1d-imper",
               "jacobi-2d-imper",
               "adi",
               "lu"]

#benchmarks  = ["matmul", "adi", "lu"]
#benchmarks = ["matmul"]
benchmarks = ["jacobi-1d-imper","jacobi-2d-imper", "adi","lu"]

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
                   "FP_COMP_OPS_EXE:X87",
                   "FP_COMP_OPS_EXE:SSE_FP_PACKED_DOUBLE",
                   "FP_COMP_OPS_EXE:SSE_FP_SCALAR_SINGLE",
	           "FP_COMP_OPS_EXE:SSE_PACKED_SINGLE",
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

  def __init__(self):
    self.all_data = None
    for file_ in product(benchmarks, data_files):
      path = file_[0] + "/" + file_[1]
      if self.all_data is None:      
        self.all_data = numpy.loadtxt(path, skiprows=1)
      else: 
        new_data      = numpy.loadtxt(path, skiprows=1)
        self.all_data = numpy.concatenate((new_data, self.all_data))

    #Filter out all the data with execution time < 1000ms
    self.all_data = numpy.array(filter(lambda x: x[-1] > 1000, self.all_data))
    print "After filtering, number of data: " + str(len(self.all_data))

    # Shuffles the rows only: http://docs.scipy.org/doc/numpy/reference/generated/numpy.random.shuffle.html
    numpy.random.shuffle(self.all_data)
    self.features = self.all_data[:,0:6]
    self.results  = self.all_data[:,7:] # first 3 are rapl related and last one is time !
    #self.normalize_features() 

    # Now break data into training and valiation
    self.features_training = self.features[2:,:]
    self.features_testing  = self.features[0:1,:]

    self.energy_training   = self.results[2: ,-2]
    self.energy_testing    = self.results[0:1,-2]


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

    #print self.energy_testing

    plt.plot(linear_reg_ridge, "B",
             linear_reg_normalized, "R",
             self.energy_testing, "g--")


    #Compute the residual sum of squares:
    print "Residual sum of square: linear_reg: " + str(numpy.mean((linear_reg - self.energy_testing)**2))
    print "Residual sum of square: linear_reg_ridge: " + str(numpy.mean((linear_reg_ridge - self.energy_testing)**2))
    
    print "Average value for prediction error: linear_reg: " +       str(numpy.mean(numpy.absolute(linear_reg       - self.energy_testing)))
    print "Average value for prediction error: linear_reg_ridge: " + str(numpy.mean(numpy.absolute(linear_reg_ridge - self.energy_testing)))
    print "Values for energy error under testing !"

    print "expected\t\t\tactual\t\t\terror"
    zipped = zip(linear_reg, self.energy_testing)
    zipped.sort(key = lambda t:t[1])
    
    for val in zipped:
      print str(val[0]) + "\t\t\t" + str(val[1]) + "\t\t\t" + str(100*numpy.absolute(val[0] - val[1])/val[1]) + "%"


    #plt.show()
    pdb.set_trace()

    # Use Logintsic regression to learn the model: 
    # clf = linear_model.LogisticRegression()
    # clf.fit(features_training, energy_training)
    # Use Logistic regression to learn the model
    # print " Prediction with logistic regression:"   
    # print clf.predict(features_testing)



def main(): 
  mm = cpu_modeler()
  mm.run_regression()
  
 
if __name__ == "__main__":
    main()

