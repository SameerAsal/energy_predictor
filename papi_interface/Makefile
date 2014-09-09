PAPI_LIB=/home/sabuas1/svn/installations/papi_MIC_5.3_mic_host/lib
PAPI_INC=/home/sabuas1/svn/installations/papi_MIC_5.3_mic_host/include

all: libpapi_interface.a mic_access_sdk_interace

mic_access_sdk_interace: mic_access_sdk_interface.c mic_access_sdk_interface.h
	icc  -I/usr/include/CommonMic/ -I/usr/include/API/public  -L/usr/lib64/  -lMicAccessSDK -lscif -lODMDebug -lpthread mic_access_sdk_interface.c -o mic_access_sdk_interace

libpapi_interface.a: papi_interface.c papi_interface.h  
	gcc -c papi_interface.c -Wall -I$(PAPI_INC) -L$(PAPI_LIB)  -lpapi -o papi_interface.o
	ar rcs libpapi_interface.a papi_interface.o
	
test: papi_interface.c papi_interface.h
	gcc -g   papi_interface.c -Wall -I$(PAPI_INC) -L$(PAPI_LIB)  -lpapi -o papi_interface.exe -DTEST
	icc  -DTEST -I/usr/include/CommonMic/ -I/usr/include/API/public  -L/usr/lib64/  -lMicAccessSDK -lscif -lODMDebug -lpthread mic_access_sdk_interface.c -o mic_access_sdk_interace
	#sudo setcap cap_sys_rawio=ep ./papi_interface.exe

clean:
	rm -f libpapi_interface.a papi_interface.exe
