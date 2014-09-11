#define MAX_DEVICES		(2)
#define MAX_READINGS    (1000)
// Read period in Micro seconds.
#define READ_PERIOD     (50*1000)

#include "Types.h"
#include "MicAccessTypes.h"
#include "MicBasicTypes.h"
#include "MicAccessErrorTypes.h"
#include "MicAccessApi.h"
#include "MicPowerManagerAPI.h"

#include <pthread.h>

#include <unistd.h>

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
// Integrated Energy values:
void integrate_pwr();

double mic_total0_energy;
double mic_total1_energy;
double mic_pcie_energy;

// uint32_t total0_energy;
// uint32_t total1_energy;
// uint32_t pcie_energy;

/* 
/// \brief Power and senor readings.
typedef struct _MicPwrPws
{
    uint32_t    prr;                    ///< Current 50mSec Average Power reading in uWatts.
    uint8_t     sts;                    ///< Current sensor status as per the SMC Spec.
} MicPwrPws;

/// \brief Voltage regulator status.
typedef struct _MicPwrVrr
{
    uint32_t    pwr;                    ///< Power reading in uWatts.
    uint32_t    cur;                    ///< Current reading in mA.
    uint32_t    volt;                   ///< Voltage reading in linear format uVolts.
    uint8_t     pwr_sts;                ///< Power sensor status as per the SMC Spec.
    uint8_t     cur_sts;                ///< Current sensor status as per the SMC Spec.
    uint8_t     volt_sts;               ///< Voltage sensor status as per the SMC Spec.
} MicPwrVrr;

/// \brief Power Management Usage.
typedef struct _MicPwrUsage {
    MicPwrPws   total0;                 ///< Total power utilization by Intel® Xeon Phi™ product codenamed “Knights Corner” device, Averaged over Time Window 0 (uWatts).
    MicPwrPws   total1;                 ///< Total power utilization by Intel® Xeon Phi™ product codenamed “Knights Corner” device, Averaged over Time Window 1 (uWatts).
    MicPwrPws   inst;                   ///< Instantaneous power (uWatts).
    MicPwrPws   imax;                   ///< Max instantaneous power (uWatts).
    MicPwrPws   pcie;                   ///< PCI-E connector power (uWatts).
    MicPwrPws   c2x3;                   ///< 2x3 connector power (uWatts).
    MicPwrPws   c2x4;                   ///< 2x4 connector power (uWatts).
    MicPwrVrr   vccp;                   ///< Core rail (uVolts).
    MicPwrVrr   vddg;                   ///< Uncore rail (uVolts).
    MicPwrVrr   vddq;                   ///< Memory subsystem rail (uVolts).
} MicPwrUsage;
*/

// Public functions:
void init_mic_access_sdk_counters();
void start_mic_access_sdk_counting();
void stop_mic_access_sdk_counting();

