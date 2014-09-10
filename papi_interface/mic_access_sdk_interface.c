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
 // Iterate through the list of available cards (referred to as adapters
 // below), initialize them. Following this, call the API to get uOS
 // version and then close the adapter.
 printf ("THREAD_read_power starting\n");
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
      printf("Current Power Usage total0:\t%u\n", powerUsage.total0.prr);
      printf("Current Power Usage total1:\t%u\n", powerUsage.total1.prr);
      printf("Current Power Usage inst:\t%u\n", powerUsage.inst.prr);
      printf("Current Power Usage pcie:\t%u\n", powerUsage.pcie.prr);


      power_readings[adapterNum][idx_write++] = powerUsage;
      if (idx_write > MAX_READINGS) 
      	idx_write = 0;
    }
    // Sleep for 50 ms.
    usleep(READ_PERIOD);
  }

 printf ("THREAD_read_power exiting\n");
 return NULL;
}

void start_mic_access_sdk_counting() {
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
  integrate_pwr();
}


void integrate_pwr() {
  size_t reading_idx;
  size_t device_idx;
  uint32_t _total0_energy = 0;
  uint32_t _total1_energy = 0;
  uint32_t _pcie_energy   = 0;

  for (device_idx = 0; device_idx < nAdapters; device_idx++) {

    for (reading_idx = 0; reading_idx < idx_write; reading_idx++) {
      _total0_energy += power_readings[device_idx][reading_idx].total0.prr;
      _total1_energy += power_readings[device_idx][reading_idx].total1.prr;
      _pcie_energy   += power_readings[device_idx][reading_idx].pcie.prr;
    }
    // Power trported is in Micro Watts.
    total0_energy = (READ_PERIOD*1.0e-6)*(_total0_energy*1.0e-6);
    total1_energy = (READ_PERIOD*1.0e-6)*(_total1_energy*1.0e-6);
    pcie_energy   = (READ_PERIOD*1.0e-6)*(_pcie_energy*1.0e-6);

    // printf ("Device %i:\n", device_idx);    
    // printf ("\ttotal0_energy =\t%f\n",   (READ_PERIOD*1.0e-6)*total0_energy*1.0e-6);
    // printf ("\ttotal1_energy =\t%f\n",   (READ_PERIOD*1.0e-6)*total1_energy*1.0e-6);
    // printf ("\tpcie_energy   =\t%f\n\n", (READ_PERIOD*1.0e-6)*pcie_energy*1.0e-6);


    printf ("\ttotal0_energy =\t%f\n", total0_energy);
    printf ("\ttotal1_energy =\t%f\n", total1_energy);
    printf ("\tpcie_energy   =\t%f\n"  , pcie_energy);
  }

}

#ifdef TEST
int main() {
  init_mic_access_sdk_counters();
  start_mic_access_sdk_counting();
  usleep(4000000);
  stop_mic_access_sdk_counting();
}
#endif