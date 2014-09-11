PAPI_LIB=/home/sabuas1/svn/installations/papi_MIC_5.3_mic_host/lib
PAPI_INC=/home/sabuas1/svn/installations/papi_MIC_5.3_mic_host/include
MIC_INC=-I/usr/include/CommonMic -I/usr/include/API/public  
MIC_LIB=-L/usr/lib64/  -lMicAccessSDK -lscif -lODMDebug -lpthread



all: libpapi_interface.a 

libpapi_interface.a: papi_interface.c papi_interface.h  
	icc -c papi_interface.c -Wall -I$(PAPI_INC) $(MIC_INC) -L$(PAPI_LIB) $(MIC_LIB) -lpapi -o papi_interface.o
	ar rcs libpapi_interface.a papi_interface.o
	
test: papi_interface.c papi_interface.h
	@rm -f micpower+rapl+cpu.txt
	icc  -DTEST papi_interface.c -Wall -I$(PAPI_INC) $(MIC_INC) -L$(PAPI_LIB) $(MIC_LIB) -lpapi -o test.exe
	#sudo setcap cap_sys_rawio=ep ./papi_interface.exe

clean:
	rm -f libpapi_interface.a papi_interface.exe
