#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#define M %M_VAL%
#define N %N_VAL%
#define K %K_VAL%

#define alpha 1
#define beta 1

#define PERFCTR

//#pragma declarations
__attribute__ ((target(mic))) double A[M][K];
__attribute__ ((target(mic))) double B[K][N];
__attribute__ ((target(mic))) double C[M][N];
//#pragma enddeclarations

#ifdef PERFCTR
#include <papi.h>
#include "papi_interface.h"
#include "papi_defs.h"
#endif

#include "util.h"

double t_start, t_end;

int main(int argc, char** argv) {

  int i, j, k;
  init_array();
  char size_string[100];

  #ifdef PERFCTR
  PERF_INIT(%PAPI_INTERFACE_CONF%); 
  #endif

  IF_TIME(t_start = rtclock());

  #ifdef PERFCTR
  //reset_count_registers();
  #endif

#pragma offload target (mic) inout(A,B,C)
{
  #pragma scop
    for(i=0; i<M; i++)
        for(j=0; j<N; j++)  
            for(k=0; k<K; k++)
                C[i][j] = C[i][j] + A[i][k] * B[k][j];
  #pragma endscop
}

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
