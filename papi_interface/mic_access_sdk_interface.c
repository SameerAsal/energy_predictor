#include "mic_access_sdk_interface.h"

void init_mic_access_sdk_counters() {

  retVal       = MIC_ACCESS_API_ERROR_UNKNOWN;
  accessHandle = NULL;
  nAdapters    = MAX_DEVICES;
  adapterNum   = 0;
  idx_write    = 0;

  // Initialize the API class. Use eTARGET_SCIF_DRIVER target for Linux.
  retVal = MicInitAPI(&accessHandle, eTARGET_SCIF_DRIVER, adaptersList, &nAdapters);

  if (retVal != MIC_ACCESS_API_SUCCESS) {
    printf("%s\n", MicGetErrorString(retVal));
	MicCloseAPI(&accessHandle);
	return ;//retVal;
  }

  if (nAdapters < 0 || nAdapters >= MAX_DEVICES) {
	printf("%s\n",	MicGetErrorString(MIC_ACCESS_API_ERROR_UNKNOWN));
	MicCloseAPI(&accessHandle);
	return ;//retVal;
  }

  // Initialize the adapters we have installed in the system.
  for (adapterNum = 0; adapterNum < nAdapters; adapterNum++) {
    // Initialize adapter
    retVal =   MicInitAdapter(&accessHandle, &adaptersList[adapterNum]);
    if (retVal != MIC_ACCESS_API_SUCCESS) {
	  MicCloseAPI(&accessHandle);
	  printf("%s\n", MicGetErrorString(retVal));
	  return ;//retVal;
    } else {
      printf("%s%i%s", "Adapter ", adapterNum , "initialized successfully !\n");      
    }
    // Save the access habdle for this device:
    accessHandles[adapterNum] = accessHandle;
    // Create an array for power readings:
    power_readings[adapterNum] = (MicPwrUsage*)malloc( sizeof(MicPwrUsage) * MAX_READINGS);
  }
}

void* THREAD_read_power(void* args) {
// Fork a thread instead !!
 printf ("thread starting\n");
while (read_running) {
  for (adapterNum = 0; adapterNum < nAdapters; adapterNum++) {
      // API call example: get and display the power usage.
      retVal = MicGetPowerUsage(accessHandles[adapterNum], &powerUsage);
      if (retVal != MIC_ACCESS_API_SUCCESS) {
	    printf("%s\n", MicGetErrorString(retVal));
	    MicCloseAdapter(accessHandle);
	    MicCloseAPI(&accessHandle);
	    return NULL;//retVal;
      }
      // Write to an array instead !
      printf("Current Power Usage: %u\n", powerUsage.total0.prr);
      power_readings[adapterNum][idx_write] = powerUsage;
      if (idx_write > MAX_READINGS) 
      	idx_write = 0;
    }
    // Sleep for 50 ms.
    usleep(50000);
  }

  printf ("thread exiting\n");
  return NULL;
}

void start_mic_access_sdk_counting() {
  // Iterate through the list of available cards (referred to as adapters
  // below), initialize them. Following this, call the API to get uOS
  // version and then close the adapter.
  read_running = TRUE;
  pthread_create(&thread_id, NULL, THREAD_read_power, NULL);
}

void stop_mic_access_sdk_counting() {
  read_running = FALSE;
  pthread_join(thread_id, NULL);

  for (adapterNum = 0; adapterNum < nAdapters; adapterNum++) {
    // Close adapter !
    retVal = MicCloseAdapter(accessHandle);
    if (retVal != MIC_ACCESS_API_SUCCESS) {
  	  printf("%s\n", MicGetErrorString(retVal));
	  MicCloseAPI(&accessHandle);
	  return ;//retVal;
    }
  }
}

#ifdef TEST
int main() {
  init_mic_access_sdk_counters();
  start_mic_access_sdk_counting();
  usleep(800000);
  stop_mic_access_sdk_counting();
}
#endif