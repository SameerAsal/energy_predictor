#include "cpu_interface.h"

// http://icl.cs.utk.edu/projects/papi/wiki/PAPIC:High_Level#High_Level_Code_Example
void start_cpu_counting() { 
  // Decide weather we want to use 
  // start_couters OR start_papi
  // TODO: Smarter way to choose based on numnber of counters we are trying to read !
  start_papi();
  // start_counters();  
  
}

void stop_cpu_counting() {
  // Decide weather we want to use: 
  // stop_couters OR stop_papi
  
  // TODO: Smarter way to choose based on number of counters we are trying to read !
  // If we are using Multiplexing (because we need to count more events that the number of
  // counting registers we have). 
  stop_papi();  
}

void start_counters() {
  CHECK (PAPI_start_counters(cpu_events, cpu_num_events), "Error in  start_counters");
}

void stop_counters() {
  CHECK (PAPI_stop_counters(cpu_values, cpu_num_events), "Stop Counters failed !");
}

int  get_num_events() {
  int count = PAPI_num_counters(); 
  if (count < 0) { 
    printf ("Number of counters (%i) returned is less than zero \n", count);
  }
  return count;  
}

// Reads the current values from the CPU register counters and 
// resets the values in theese registers. 
void accumlate_counters() {
  if(PAPI_accum_counters(cpu_values, cpu_num_events) != PAPI_OK)
    handle_error(1);
}

// Reads the current values from the CPU register counters and 
// resets the values in theese registers.
void read_counters() {
  end_usec = PAPI_get_real_usec();
  total_usec = end_usec - start_usec;
  int retval = PAPI_read_counters(cpu_values, cpu_num_events);
  if (retval != PAPI_OK) {
    handle_error(retval);
  }
}

void init_cpu_counters() {
  printf ("Start init_cpu_counters\n");
  int max_cpu_num_events = get_num_events();
  // Allocate memory for events.
  cpu_events = malloc(sizeof(int)*max_cpu_num_events);
  cpu_values = malloc(sizeof(long_long)*max_cpu_num_events);

  init_library();
  create_event_set();
  // Bind our EventSet to the cpu component
  // Look at the Multiplexing example in the ctests folder.
  CHECK (PAPI_assign_eventset_component(cpu_native_event_set, 0), "assign_event_set_component to 0 (cpu)");
  // Enable and initialize multiplex support
  CHECK (PAPI_multiplex_init(), "Error Initialzing multiplexing !");
  // To enable multiplexing we need to specify what componenet of PAPI we are exactly using. 
  // we don't need to do that with if multiplexing was not needed.
  CHECK (PAPI_set_multiplex(cpu_native_event_set), "PAPI_set_multiplex(native_event_set)");
  register_flop_events();
  register_mem_events();
  fill_event_set();
  zero_fill_values();
  printf ("Done init_cpu_counters\n");
}


void create_event_set() {
  // Create the event set. 
  cpu_native_event_set = PAPI_NULL;
  CHECK (PAPI_create_eventset(&cpu_native_event_set), "PAPI_create_event_set failed !");
}


void register_flop_events() {
  // Find the first available native even
  // Figure out how to find the evebt code for 
  // SSE, AVX , x87 (then the ones related to memory).
  int native;
  // Avx. 
  //CHECK(PAPI_event_name_to_code("SIMD_FP_256:PACKED_SINGLE", &native), "Error translating event name to code\n");
  //print_event_info(native);
  // add_event(cpu_events, native, &cpu_num_events);

  CHECK(PAPI_event_name_to_code("SIMD_FP_256:PACKED_DOUBLE", &native), "Error translating event name to code\n");
  // print_event_info(native);
  add_event(cpu_events, native, &cpu_num_events);
  
  // SSE + x87.
  CHECK(PAPI_event_name_to_code("FP_COMP_OPS_EXE:X87", &native), "Error translating FP_COMP_OS_EXE  event name to code\n");
  print_event_info(native);
  add_event(cpu_events, native, &cpu_num_events);
  
  CHECK(PAPI_event_name_to_code("FP_COMP_OPS_EXE:SSE_FP_PACKED_DOUBLE", &native), 
        "Error translating FP_COMP_OPS_EXE:SSE_FP_PACKED_DOUBLE event name to code\n");
  // print_event_info(native);
  add_event(cpu_events, native, &cpu_num_events);
  
  //CHECK(PAPI_event_name_to_code("FP_COMP_OPS_EXE:SSE_FP_SCALAR_SINGLE", &native), "Error translating FP_COMP_OPS_EXE:SSE_FP_SCALAR_SINGLE event name to code\n");
  // print_event_info(native);
  //add_event(cpu_events, native, &cpu_num_events);
  
  //CHECK(PAPI_event_name_to_code("FP_COMP_OPS_EXE:SSE_PACKED_SINGLE", &native), "Error translating FP_COMP_OPS_EXE:SSE_PACKED_SINGLE event name to code\n");
  // print_event_info(native);
  //add_event(cpu_events, native, &cpu_num_events);
  
  //CHECK(PAPI_event_name_to_code("FP_COMP_OPS_EXE:SSE_SCALAR_DOUBLE", &native), "Error translating FP_COMP_OPS_EXE:SSE_SCALAR_DOUBLE  event name to code\n");
  //print_event_info(native);
  //add_event(cpu_events, native, &cpu_num_events);

  //CHECK(PAPI_event_name_to_code("PAPI_SP_OPS", &native), "Error translating PAPI_SP_OPS event name to code\n");
  //print_event_info(native);
  //add_event(cpu_events, native, &cpu_num_events);

  //CHECK(PAPI_event_name_to_code("PAPI_DP_OPS", &native), "Error translating PAPI_DP_OPS event name to code\n");
  //print_event_info(native);
  //add_event(cpu_events, native, &cpu_num_events);
}

void register_mem_events() {
  int native;

  // cpu_events related to cache and memory accesses: 
  // CHECK(PAPI_event_name_to_code("L1D:REPLACEMENT", &native),
  // "Error translating L1D:REPLACEMENT event name to code\n");
  // print_event_info(native);
  // add_event(cpu_events, native, &cpu_num_events);
  
  // CHECK(PAPI_event_name_to_code("L1D:M_EVICT", &native), "Error translating L1D:M_EVICT event name to code\n");
  // print_event_info(native);
  // add_event(cpu_events, native, &cpu_num_events);
 
  // CHECK(PAPI_event_name_to_code("perf::PERF_COUNT_HW_CACHE_L1D", &native), 
  // "Error translating perf::PERF_COUNT_HW_CACHE_L1D:MISS event name to code\n");

  // print_event_info(native);
  // add_event(cpu_events, native, &cpu_num_events);
  CHECK(PAPI_event_name_to_code("PAPI_L1_DCM", &native), "Error translating PAPI_L1_DCM event name to code\n");
  //print_event_info(native);
  add_event(cpu_events, native, &cpu_num_events);
  
  CHECK(PAPI_event_name_to_code("PAPI_L2_DCM", &native), "Error translating PAPI_L2_DCM event name to code\n");
  //print_event_info(native);
  add_event(cpu_events, native, &cpu_num_events);

  //CHECK(PAPI_event_name_to_code("PAPI_L1_LDM", &native), "Error translating PAPI_L1_LDM event name to code\n");
  //print_event_info(native);
  //add_event(cpu_events, native, &cpu_num_events);
  
  //CHECK(PAPI_event_name_to_code("PAPI_L1_STM", &native), "Error translating PAPI_L1_STM  event name to code\n");
  //print_event_info(native);
  //add_event(cpu_events, native, &cpu_num_events);
  
  //CHECK(PAPI_event_name_to_code("PAPI_L2_STM", &native), "Error translating PAPI_L2_STM  event name to code\n");
  //print_event_info(native);
  //add_event(cpu_events, native, &cpu_num_events);
}



void fill_event_set() {  
  int retval = PAPI_add_events(cpu_native_event_set, cpu_events, cpu_num_events); 
  if (retval != PAPI_OK) {
    printf ("Error adding events to event set\n");
    handle_error(retval);    
  }
}

// Used when the non high level Apis are used. 
void start_papi() {
  printf ("Start PAPI events !!\n");
  CHECK (PAPI_start(cpu_native_event_set), "Error start_papi()\n");
  printf ("PAPI events started !!\n");
}

// Used when the non high level APIs are used. 
void stop_papi() {
  printf ("Stop PAPI events!!\n");
  CHECK (PAPI_stop(cpu_native_event_set, cpu_values), "Error stop_papi\n");
  printf ("Stop PAPI events done !!\n");
}
