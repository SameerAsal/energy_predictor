#define PERFCTR
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include <omp.h>

#ifdef PERFCTR
#include "papi_interface.h"
#include "papi_defs.h"
#endif

//#include "decls.h"

#define T 500
#define N %N_VAL%

#define TMAX T
#define NMAX N

static double X[NMAX][NMAX+13], A[NMAX][NMAX+23], B[NMAX][NMAX+37];

#include "util.h"

int main(int argc, char** argv)
{
    int i, j, k, l, m, n, t;

    int i1, i2;

    double t_start, t_end;

    init_array();


    IF_TIME(t_start = rtclock());

#ifdef PERFCTR
//    reset_count_registers(); 
#endif

  #ifdef PERFCTR
  PERF_INIT(%PAPI_INTERFACE_CONF%);
  #endif

#pragma scop
    for (t = 0; t < T; t++) {

        for (i1=0; i1<N; i1++) {
            for (i2 = 1; i2 < N; i2++) {
                X[i1][i2] = X[i1][i2] - X[i1][i2-1] * A[i1][i2] / B[i1][i2-1]; // S1
                B[i1][i2] = B[i1][i2] - A[i1][i2] * A[i1][i2] / B[i1][i2-1];   // S2
            }
        }

        for (i1=1; i1<N; i1++) {
            for (i2 = 0; i2 < N; i2++) {
                X[i1][i2] = X[i1][i2] - X[i1-1][i2] * A[i1][i2] / B[i1-1][i2]; // S3
                B[i1][i2] = B[i1][i2] - A[i1][i2] * A[i1][i2] / B[i1-1][i2];   // S4
            }
        }
    }
#pragma endscop

#ifdef PERFCTR
    PERF_EXIT(argv[0]); 
#endif

    IF_TIME(t_end = rtclock());
    IF_TIME(fprintf(stderr, "%0.6lfs\n", t_end - t_start));


    if (fopen(".test", "r")) {
        print_array();
    }
    return 0;
}
