#include "papi_interface.h"

// RAPL related events:
int   rapl_cid;
int   rapl_events_count; 
int   rapl_event_set;
int   *rapl_events;    // Energy related events we need to subscribe to. 
long_long *rapl_values;
int rapl_num_registered_events;

// RAPL + Energy function:
void test_papi_rapl();
void init_rapl_counters();
void start_rapl_counting();
void stop_rapl_counting();
void register_rapl_energy_events();