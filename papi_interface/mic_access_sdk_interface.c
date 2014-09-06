#include "mic_access_sdk_interface.h"



void init_mic_access_sdk_counters() {

  retVal = MIC_ACCESS_API_ERROR_UNKNOWN;
  accessHandle = NULL;
  nAdapters = MAX_DEVICES;
  adapterNum = 0;

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
    accessHandles[adapterNum] = accessHandle;
  }
}



void start_mic_access_sdk_counting() {

  // Iterate through the list of available cards (referred to as adapters
  // below), initialize them. Following this, call the API to get uOS
  // version and then close the adapter.
  for (adapterNum = 0; adapterNum < nAdapters; adapterNum++) {
    // fork a thread instead !!
    while (1) {
      // API call example: get and display the power usage.
      retVal = MicGetPowerUsage(accessHandles[adapterNum], &powerUsage);
      if (retVal != MIC_ACCESS_API_SUCCESS) {
	    printf("%s\n", MicGetErrorString(retVal));
	    MicCloseAdapter(accessHandle);
	    MicCloseAPI(&accessHandle);
	    return ;//retVal;
      }
      // Write to an array instead !
      printf("Current Power Usage: %u\n", powerUsage.total0.prr);
      usleep(1000);
    }
  }
}



void stop_mic_access_sdk_counting() {

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