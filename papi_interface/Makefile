PAPI_LIB=/home/sabuas1/svn/installations/papi_MIC_5.3_mic_host/lib
PAPI_INC=/home/sabuas1/svn/installations/papi_MIC_5.3_mic_host/include
MIC_INC=-I/usr/include/CommonMic -I/usr/include/API/public  
MIC_LIB=-L/usr/lib64/  -lMicAccessSDK -lscif -lODMDebug -lpthread

FLAGS=-DDISABLE_MIC
CC=gcc

all: libpapi_interface.a test
nomic_all: libpapi_interface_nomic test_nomic


libpapi_interface.a: papi_interface.c papi_interface.h  
	$(CC) -c papi_interface.c -Wall -I$(PAPI_INC) $(MIC_INC) -L$(PAPI_LIB) $(MIC_LIB) $(FLAGS) -lpapi -o papi_interface.o
	ar rcs libpapi_interface.a papi_interface.o
	

test: papi_interface.c papi_interface.h
	@rm -f micpower+rapl+cpu.txt
	 $(CC)  -DTEST $(FLAGS) papi_interface.c -Wall -I$(PAPI_INC) -L$(PAPI_LIB) $(MIC_INC) $(MIC_LIB)  -lpapi -o test.exe


test_nomic: papi_interface.c papi_interface.h
	@rm -f micpower+rapl+cpu.txt
	$(CC)  -DTEST $(FLAGS) papi_interface.c -Wall -lpapi -o test.exe


libpapi_interface_nomic: papi_interface.c papi_interface.h  
	$(CC) -c papi_interface.c -Wall $(FLAGS) -lpapi -o papi_interface.o
	ar rcs libpapi_interface.a papi_interface.o


clean:
	rm -f libpapi_interface.a papi_interface.exe
