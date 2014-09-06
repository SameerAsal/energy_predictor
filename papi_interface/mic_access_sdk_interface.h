#define MAX_DEVICES		(2)

#include "Types.h"
#include "MicAccessTypes.h"
#include "MicBasicTypes.h"
#include "MicAccessErrorTypes.h"
#include "MicAccessApi.h"

#include "MicPowerManagerAPI.h"


/// For threading:
#include <pthread.h>


// Private members of the interface (no need to be exposed).
MicDeviceOnSystem adaptersList[MAX_DEVICES];
HANDLE accessHandle;
HANDLE accessHandles[MAX_DEVICES];
U32 nAdapters;
U32 adapterNum;
U32 retVal;
MicPwrUsage powerUsage;


// Public functions:
void init_mic_access_sdk_counters();
void start_mic_access_sdk_counting();
void stop_mic_access_sdk_counting();

