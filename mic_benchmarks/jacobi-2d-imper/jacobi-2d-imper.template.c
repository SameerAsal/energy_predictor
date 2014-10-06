#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <assert.h>

#define N %N_VAL%
#define T 1000
#define PERFCTR

#ifdef PERFCTR
#include "papi_defs.h"
#endif 


//#pragma declarations
double a[N][N];
double b[N][N];
//#pragma enddeclarations

__attribute__ ((target(mic))) double a[N][N];
__attribute__ ((target(mic))) double b[N][N];

#include "util.h"

int main(int argc, char** argv)
{
    int t, i, j;
    double t_start, t_end;

    init_array();

    IF_TIME(t_start = rtclock());

#ifdef PERFCTR
  PERF_INIT();
#endif 

#pragma offload target(mic) inout(a,b)
{
  #pragma scop
    for (t=0; t<T; t++) {
        for (i=2; i<N-1; i++) {
            for (j=2; j<N-1; j++) {
                b[i][j]= 0.2*(a[i][j]+a[i][j-1]+a[i][1+j]+a[1+i][j]+a[i-1][j]);
            }
        }
        for (i=2; i<N-1; i++) {
            for (j=2; j<N-1; j++)   {
                a[i][j]=b[i][j];
            }
        }
    }
  #pragma endscop
}

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
