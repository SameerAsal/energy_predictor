#define MAX_DEVICES		(2)
#define MAX_READINGS    (1000)
#include "Types.h"
#include "MicAccessTypes.h"
#include "MicBasicTypes.h"
#include "MicAccessErrorTypes.h"
#include "MicAccessApi.h"
#include "MicPowerManagerAPI.h"

#include <pthread.h>

#include "papi_interface.h"

// Private members of the interface (no need to be exposed).
MicDeviceOnSystem adaptersList[MAX_DEVICES];
HANDLE accessHandle;
HANDLE accessHandles[MAX_DEVICES];
U32 nAdapters;
U32 adapterNum;
U32 retVal;
MicPwrUsage powerUsage;

// Threaded reads
BOOL read_running;
MicPwrUsage* power_readings[MAX_DEVICES];
size_t    idx_write;
pthread_t thread_id;
void* THREAD_read_power(void* args);


// Public functions:
void init_mic_access_sdk_counters();
void start_mic_access_sdk_counting();
void stop_mic_access_sdk_counting();

