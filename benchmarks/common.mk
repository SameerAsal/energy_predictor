#
# common Makefile
#
# Gets included after the local Makefile in an example sub-directory
#
BASEDIR=$(dir $(lastword $(MAKEFILE_LIST)))

override CC = gcc 

NPROCS=4
NTHREADS=4
PLC=$(BASEDIR)../polycc

# Intel MKL and AMD ACML library paths
MKL=/opt/intel/mkl
ACML=/usr/local/acml

#OPT_FLAGS_ICC:= -O3 -fp-model precise -xavx 
#PAR_FLAGS_ICC:= -parallel
#OMP_FLAGS_ICC:= -openmp

#OPT_FLAGS_GCC:=-O3 -ftree-vectorize -msse3 
#PAR_FLAGS_GCC:=-ftree-parallelize-loops=4
#OMP_FLAGS_GCC:=-fopenmp

ifeq ($(CC), icc)
	OPT_FLAGS :=-O3 -fp-model precise -xavx
	PAR_FLAGS := -parallel
	OMP_FLAGS := -openmp
else
	OPT_FLAGS := -O3 -ftree-vectorize -msse3 
	PAR_FLAGS := -ftree-parallelize-loops=4
	OMP_FLAGS := -fopenmp
endif


CFLAGS+=-DTIME
PAPI_INSTALL=
PAPI_LIB=
PAPI_INC=$(PAPI_INSTALL)/include
PAPI_INTERFACE=../../papi_interface
#LDFLAGS += -lm -lpapi -lpapi_interface -L$(PAPI_LIB) -L$(PAPI_INTERFACE)
LDFLAGS +=  -lpapi_interface -L$(PAPI_INTERFACE) -lpapi -lm
INC_FLAGS = -I$(PAPI_INC) -I$(PAPI_INTERFACE)
PLCFLAGS +=
TILEFLAGS += 

#PERFCTR=perfctr

ifdef PERFCTR
	CFLAGS += -DPERFCTR -L/usr/local/lib64 -lpapi
endif

# Path to pluto instalation:
PLC=/home/sameer/svn/installations/pluto/bin/polycc

all: orig tiled par

$(SRC).opt.c:  $(SRC).c
	$(PLC) $(SRC).c $(INC_FLAGS) $(PLCFLAGS)  -o $@

$(SRC).tiled.c:  $(SRC).c
	$(PLC) $(SRC).c --tile $(TILEFLAGS) $(PLCFLAGS)  -o $@

$(SRC).par.c:  $(SRC).c 
	$(PLC) $(SRC).c --tile --parallel $(TILEFLAGS) $(PLCFLAGS)  -o $@

orig: $(SRC).c papi_defs.h perf_interface
	$(CC) $(OPT_FLAGS) $(INC_FLAGS) $(CFLAGS) $(SRC).c -o $@ $(LDFLAGS)

orig_par: $(SRC).c papi_defs.h perf_interface
	$(CC) $(OPT_FLAGS) $(INC_FLAGS) $(CFLAGS) $(PAR_FLAGS) $(SRC).c -o $@ $(LDFLAGS)

opt: $(SRC).opt.c papi_defs.h perf_interface
	$(CC) $(OPT_FLAGS) $(CFLAGS) $(SRC).opt.c -o $@ $(LDFLAGS)

tiled: $(SRC).tiled.c papi_defs.h perf_interface
	$(CC) $(OPT_FLAGS) $(INC_FLAGS) $(CFLAGS) $(SRC).tiled.c -o $@ $(LDFLAGS)

par: $(SRC).par.c papi_defs.h perf_interface
	$(CC) $(OPT_FLAGS) $(INC_FLAGS) $(CFLAGS) $(OMP_FLAGS) $(SRC).par.c -o $@  $(LDFLAGS)

perf_interface: $(PAPI_INTERFACE)/papi_interface.h $(PAPI_INTERFACE)/papi_interface.c
	make -C $(PAPI_INTERFACE) libpapi_interface.a

perf: orig tiled par orig_par
	rm -f .test
	./orig
	OMP_NUM_THREADS=4 ./orig_par
	./tiled
	OMP_NUM_THREADS=4 ./par 


test: orig tiled par
	touch .test
	./orig 2> out_orig
	./tiled 2> out_tiled
	diff -q out_orig out_tiled
	OMP_NUM_THREADS=$(NTHREADS) ./par 2> out_par4
	rm -f .test
	diff -q out_orig out_par4
	@echo Success!

opt-test: orig opt
	touch .test
	./orig > out_orig
	./opt > out_opt
	rm -f .test
	diff -q out_orig out_opt
	@echo Success!
	rm -f .test

clean:
	rm -f out_* *.tiled.c *.opt.c *.par.c orig opt tiled par sched orig_par \
		hopt hopt *.par2d.c *.out.* \
		*.kernel.* a.out $(EXTRA_CLEAN) tags tmp* gmon.out *~ .unroll \
	   	.vectorize par2d parsetab.py *.body.c *.pluto.c *.par.cloog *.tiled.cloog

exec-clean:
	rm -f out_* opt orig tiled  sched sched hopt hopt par orig_par *.out.* *.kernel.* a.out \
		$(EXTRA_CLEAN) tags tmp* gmon.out *~ par2d
