#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#define M 32
#define N 16
#define K 32

#define alpha 1
#define beta 1

#define PERFCTR

#pragma declarations
double A[M][K];
double B[K][N];
double C[M][N];
#pragma enddeclarations

#ifdef PERFCTR
#include <papi.h>
#include "papi_interface.h"
#include "papi_defs.h"
#endif

#include "util.h"

double t_start, t_end;

int main(int argc, char** argv)
{
    int i, j, k;

    init_array();

#ifdef PERFCTR
    PERF_INIT(); 
#endif

    IF_TIME(t_start = rtclock());

#ifdef PERFCTR
    // reset_count_registers(); 
#endif

#pragma scop
    for(i=0; i<M; i++)
        for(j=0; j<N; j++)  
            for(k=0; k<K; k++)
                C[i][j] = C[i][j] + A[i][k] * B[k][j];
#pragma endscop

#ifdef PERFCTR
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
