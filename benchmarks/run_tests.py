# loops over sover all the benchmarks templates, write a kernel that with a specific size
# for each benchmarks then run make perf. 
# next loop get a different size and run the tests. 

from itertools import product
import os
import subprocess

tokens = [ "%N_VAL%", "%M_VAL%", "%K_VAL%"]

def test_all_versions(to_compose, tokens, text, bench, std_err, std_out):
  name = "./" + bench + "/" + bench  + ".c"

  for e in product(*to_compose):
    solid = text
    for i in range (0 , len(e)):       
      solid = solid.replace(tokens[i], str(e[i]))

    f = open(name, "w")
    f.write(solid)
    #Write the filled in template !
    f.close()
    #Now make perf
    print "now running: " + str(e)
    subprocess.call(["make", "-C", bench, "perf"], stderr=std_err, stdout=std_out)
    #make -C ./bench  perf

  

def run_tests():
  benchmarks = {"matmul", "jacobi-1d-imper","jacobi-2d-imper", "adi","lu"}

  n_Values = [128,256,512,1024,2048,3096]
  m_Values = [128,256,348,512]
  k_Values = [128,256,348,512]

  #m_Values = [128,256,512,1024,2048,3096]
  #k_Values = [128,256,512,1024,2048,3096]
  
  to_compose = []

  # open to forward output and erro to.
  err_out = open("./std.err.out","w")
  for bench in benchmarks:
    to_compose = []
    name = "./" + bench + "/" + bench + ".template" + ".c"
    template = open(name, 'r')
    text = template.read()
    template.close()

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


def main(): 
  run_tests()
 
if __name__ == "__main__":
    main()
