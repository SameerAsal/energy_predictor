#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#define M 512
#define N 5096
#define K 512

#define alpha 1
#define beta 1

#define PERFCTR

#pragma declarations
double A[M][K];
double B[K][N];
double C[M][N];
#pragma enddeclarations

#ifdef PERFCTR


#pragma offload_attribute (push,target(mic))
#include "papi.h"
#pragma offload_attribute (pop)
//#include <papi.h>
#include "papi_interface.h"
#include "papi_defs.h"

#endif

#include "util.h"

double t_start, t_end;

int main(int argc, char** argv) {

  int i, j, k;
  double* a = (double *) malloc( (size_t) N*K*sizeof(double) );
  double* b = (double *) malloc( (size_t) K*N*sizeof(double) );
  double* c = (double *) malloc( (size_t) N*M*sizeof(double) );
  init_array();
  char size_string[100];

  #ifdef PERFCTR
  PERF_INIT(); 
  #endif

  IF_TIME(t_start = rtclock());

  #ifdef PERFCTR
  // reset_count_registers(); 
  #endif

  #pragma scop
  //#pragma offload target(mic:0)  inout(C: length(morder*morder)) , in(A: length(M*K)) , in(B: length(K*N))
  #pragma offload target(mic:0)  inout(c: length(M*K)) , in(a: length(M*K)) , in(b: length(K*N))
  {
    for(i=0; i<M; i++)
        for(j=0; j<N; j++)  
            for(k=0; k<K; k++) {
//                C[i][j] = C[i][j] + A[i][k] * B[k][j];
    a[i] += b[i] * c[i];
//    A[i][i] += B[i] * C[i][i];
    }

  }
  #pragma endscop

  #ifdef PERFCTR
    sprintf(size_string,"%s_(%i,%i,%i)",argv[0],N,M,K);
    fprintf(stderr, "Now writing %s", size_string);
    PERF_EXIT(argv[0]); 
  #endif

    IF_TIME(t_end = rtclock());
    IF_TIME(fprintf(stdout, "%0.6lfs\n", t_end - t_start));


  if (fopen(".test", "r")) {
#ifdef MPI
      if (my_rank == 0) {
          print_array();
      }
#else
          print_array();
#endif
  }

  return 0;
}
