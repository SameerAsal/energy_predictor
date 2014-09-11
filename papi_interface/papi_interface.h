#ifndef __PAPI_INTERFACE_H__ 
#define __PAPI_INTERFACE_H__

#include <stdlib.h>
#include <stdio.h>
#include <papi.h>
#define BOOL int 
#define TRUE 1
#define FALSE 0


//micpower library settings: 
BOOL rapl_enabled; 
BOOL cpu_enabled;
BOOL mic_enabled;     // Using the  papi ports for wrapping MICAccessSDK
BOOL mic_access_sdk_enabled; // Using the MICAcessSDK directly, based on the example: example-powerusage.c


// Value for timers.
long_long start_usec, end_usec, total_usec;
const PAPI_component_info_t *cmpinfo;

// Initialize everything
void init_counters();
// Start counting.
void start_counting();
// Stop counting.
void stop_counting();

// CPU counters Users should only use those !
void start_cpu_counting();
void stop_cpu_counting();

// private function. 
// user don't touch those.
int  get_num_events();
void start_papi();
void stop_papi();
void register_energy_events();
void add_event(int*, int, int* index);
void print_counters();
void accumlate_counters();
void finalize();
void init_cpu_counters();
void fill_event_set();
void create_event_set();
void zero_fill_values();
// Init PAPI Library. 
void init_library();
// Native  event specific.
void register_flop_events();
void register_mem_events();
void finalize_native();

// Utility functions:
void read_config();
void get_event_unit(int event_code, char *unit); 
void handle_error(int); 
void CHECK (int retval, char* error_message);
void CHECK_BOOL (int retval, char* error_message);
void print_event_info(int); 
void print_comp_details(const PAPI_component_info_t*);
void get_time(char* now);
BOOL file_exists(char* name); 

// Printing functions:
void print_counters();

void print_counters_to_file(char*);

void test_papi();


// Common functionality:
BOOL find_cmp(char *cmp_name, int* cmp_id);

// Deprecated: 
// Can be used to read counters without stopping the counters.Can be only used when the high level API is used, now that 
// We are using multiplexing and other complex features for Multiplexing we can't use it anymore ! 
//void read_counters();
#endif