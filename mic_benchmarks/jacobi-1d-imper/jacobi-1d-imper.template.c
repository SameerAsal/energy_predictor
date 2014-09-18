#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "util.h"

#define N %N_VAL%
#define T 1000


#define PERFCTR

#ifdef PERFCTR
#include "papi_defs.h" 
#endif


//#pragma declarations
__attribute__ ((target(mic))) double a[N];
__attribute__ ((target(mic))) double b[N];
//#pragma enddeclarations

double t_start, t_end;

void init_array()
{
    int j;

    for (j=0; j<N; j++) {
        a[j] = ((double)j)/N;
    }
}


void print_array()
{
    int j;

    for (j=0; j<N; j++) {
        fprintf(stderr, "%lf ", a[j]);
        if (j%80 == 20) fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
}


int main(int argc,char** argv )
{
    int t, i, j;

    init_array();

    IF_TIME(t_start = rtclock());
#ifdef PERFCTR
   PERF_INIT();
#endif
#pragma offload target(mic) inout(a,b)
{
#pragma scop
    for (t = 0; t < T; t++) {
        for (i = 2; i < N - 1; i++) {
            b[i] = 0.33333 * (a[i-1] + a[i] + a[i + 1]);
        }
        for (j = 2; j < N - 1; j++) {
            a[j] = b[j];
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
