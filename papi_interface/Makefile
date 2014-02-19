

all: libpapi_interface.a

libpapi_interface.a: papi_interface.c papi_interface.h  
	gcc -c papi_interface.c -Wall -I/home/sabuas1/svn/installations/papi/include/ -L/home/sabuas1/svn/installations/papi/lib/  -lpapi -o papi_interface.o
	ar rcs libpapi_interface.a papi_interface.o

	
test: papi_interface.c papi_interface.h
	gcc -O3 papi_interface.c -Wall -I/home/sabuas1/svn/installations/papi/include/ -L/home/sabuas1/svn/installations/papi/lib/  -lpapi -o papi_interface.exe -DTEST
	sudo setcap cap_sys_rawio=ep ./papi_interface.exe

clean:
	rm -f libpapi_interface.a papi_interface.exe
 
