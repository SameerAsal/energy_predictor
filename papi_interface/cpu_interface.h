#include "papi_interface.h"

// CPU events:
int  cpu_num_events;       // Number of events registered. 
int  *cpu_events;          // Events we need to subscribe to.
int  cpu_native_event_set; // The event set we will use
long_long *cpu_values;     // Values for the performance counters for the events we need to subscribe to.
                           // used with both native 

void init_cpu_counters();
void start_cpu_counting();
void stop_cpu_counting();


// Private methods:
void register_flop_events();
void register_mem_events();
