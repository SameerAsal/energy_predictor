#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

#include "decls.h"
#include "util.h"

#define  PERFCTR
#include "papi_defs.h"

void trisolv(long N, char** argv) {
    long i,j,k;

  #ifdef PERFCTR
    PERF_INIT(); 
  #endif
  #pragma scop
    for (i=0;i<=N-1;i++) {
        for (j=0;j<=N-1;j++) {
            for (k=0;k<=j-1;k++) {
                B[j][i]=B[j][i]-L[j][k]*B[k][i];  //S1 ;

            }
            B[j][i]=B[j][i]/L[j][j]; // S2 ;
        } // for j
    } // for i
  #pragma endscop
  #ifdef PERFCTR
    PERF_EXIT(argv[0]);
  #endif
}


int main(int argc, char** argv)
{
    long N=NMAX;
    int i,j;
    double t_start, t_end;

    IF_TIME(t_start = rtclock());
    trisolv(N, argv);
    IF_TIME(t_end = rtclock());
    IF_TIME(fprintf(stderr, "%0.6lfs\n", t_end - t_start));

    if (fopen(".test", "r")) {
        for (i = 0; i < N; i++) {
            for (j = 0; j < N; j++) {
                fprintf(stdout, "%lf ", B[i][j]);
            }
            fprintf(stdout, "\n");
        }
    }
    return 0;
}
