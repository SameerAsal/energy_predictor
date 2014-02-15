#include <stdlib.h>
#include <stdio.h>
#include <papi.h>

int num_events;      // Number of events registered. 
int        *Events;  // Events we need to subscribe to.
int        native_event_set; // The event set we will use
long_long  *Values;  // Values for the performance counters for the events we need to subscribe to.
                     // used with both native a

// Value for timers.
long_long start_usec, end_usec, total_usec;

// Users should only use those !
void start_counting();
void stop_counting();

int  get_num_events();
void start_papi();
void stop_papi();
void register_events();
void add_event(int*, int, int* index);
void print_counters();
void accumlate_counters();
void finalize();
void init_counters();
void fill_event_set();
void create_event_set();
void zero_fill();
void init_library();
// Native  event specific.
void register_flop_events();
void register_mem_events();
void finalize_native();

// Utility functions:
void handle_error(int); 
void CHECK (int retval, char* error_message);
void print_event_info(int); 

// Printing functions:
void print_counters();
void print_counters_to_file(char*);

// Deprecated: 
// Can be used to read counters without stopping the counters.Can be only used when the high level API is used, now that 
// We are using multiplexing and other complex features for Multiplexing we can't use it anymore ! 
//void read_counters();
