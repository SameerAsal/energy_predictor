#include "papi_interface.h"

// http://icl.cs.utk.edu/projects/papi/wiki/PAPIC:High_Level#High_Level_Code_Example
void start_counting() { 
  // Decide weather we want to use 
  // start_couters OR start_papi
  // TODO: Smarter way to choose based on numnber of counters we are trying to read !
  start_papi();
  // start_counters();  
  start_usec =  PAPI_get_real_usec();
}

void stop_counting() {
  // Decide weather we want to use: 
  // stop_couters OR stop_papi
  end_usec = PAPI_get_real_usec();
  // TODO: Smarter way to choose based on number of counters we are trying to read !
  // If we are using Multiplexing (because we need to count more events that the number of
  // counting registers we have). 
  stop_papi();
  total_usec = end_usec - start_usec;
}

void start_counters() {
  CHECK (PAPI_start_counters(Events, num_events), "Error in  start_counters");
}

void stop_counters() {
  CHECK (PAPI_stop_counters(Values, num_events), "Stop Counters failed !");
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
  if(PAPI_accum_counters(Values, num_events) != PAPI_OK)
    handle_error(1);
}

// Reads the current values from the CPU register counters and 
// resets the values in theese registers.
void read_counters() {
  end_usec = PAPI_get_real_usec();
  total_usec = end_usec - start_usec;
  int retval = PAPI_read_counters(Values, num_events);
  if (retval != PAPI_OK) {
    handle_error(retval);
  }
}

void  print_counters_to_file(char* file_name) {
  int idx=0;
  FILE* out_file = fopen(file_name, "w");
  if (out_file == NULL) { 
    printf ("Error opening file %s for writing \n", file_name);
    exit(-1);
  } 
  char event_name[PAPI_MAX_STR_LEN];
  for (idx=0; idx < num_events; idx++) {
    PAPI_event_code_to_name(Events[idx], event_name);
    fprintf(out_file,"%s:\t%lld\n", event_name, Values[idx]);
  }
  fprintf(out_file,"%s:\t%f\n", "EXEC_TIME", total_usec/1000.0);
  fclose(out_file);
}

void print_counters() {
  int idx=0;
  char event_name[PAPI_MAX_STR_LEN];
  printf("Values of performance counters: \n");
  for (idx=0; idx < num_events; idx++) {
    PAPI_event_code_to_name(Events[idx], event_name);
    printf("%s :\t%lld\n", event_name, Values[idx]);
  } 
  printf("%s:\t%f\n", "EXEC_TIME", total_usec/1000.0);
}

// Utility Functions: 
void handle_error (int retval) {
  fprintf(stderr,"handle_error %d: %s\n", retval, PAPI_strerror(retval));
  exit(1);
}

void CHECK (int retval, char* error_message) {
  if (retval != PAPI_OK) { 
    fprintf(stderr,"%s  %d: %s\n", error_message, retval, PAPI_strerror(retval));
    exit(1);
  }
}

void show_all_events() {
  // Show all the native events in the system we are in. 
  // (I think these are the ones that are not mapped to preset events).
  int EventCode, retval;
  char EventCodeStr[PAPI_MAX_STR_LEN];
  EventCode = 0 | PAPI_NATIVE_MASK;
  retval = PAPI_library_init(PAPI_VER_CURRENT);
  if  (retval != PAPI_VER_CURRENT) {
    fprintf(stderr, "PAPI library init error!\n");
    exit(1); 
  }

  do {
   /* Translate the integer code to a string */
   if (PAPI_event_code_to_name(EventCode, EventCodeStr) == PAPI_OK) {
      /* Print all the native events for this platform */
      printf("Name: %s\nCode: %x\n", EventCodeStr, EventCode);
   } else {
     printf("Failed to translate event name \n");
   }

  } while (PAPI_enum_event(&EventCode, 0) == PAPI_OK);
}

void add_event(int * events, int papi_event_code, int* idx) {
  char event_name[PAPI_MAX_STR_LEN];
  int retval;
  // Works with both native and preset events.
  retval = PAPI_event_code_to_name(papi_event_code, event_name);
  if (retval != PAPI_OK) {
   printf("Can't find name for event %i\n", papi_event_code);
  }
  // Checks if the PAPI preset event can be counted, can also be used to check if  
  retval = PAPI_query_event(papi_event_code);
  if (retval != PAPI_OK) {        
    printf ("Event %s not spported\n", event_name);
    return;
  } else {
    events[*idx] = papi_event_code;
    printf ("Adding %s to list of events\n", event_name);
    (*idx)++;
  }
}

void register_events() {
  // add_event(Events,PAPI_VEC_SP, &num_events);
  // add_event(Events,PAPI_DP_OPS, &num_events); 
  // counts all the double precision operations. 
  add_event(Events,PAPI_VEC_DP, &num_events);
  add_event(Events,PAPI_L1_DCM, &num_events);
  zero_fill();
}

void finalize() {
  free(Events);
  free(Values);
  num_events = 0;
}

void test_papi() {
  // TODO: Make this into a test case to check for correctness when 
  //   trying it on new architectures.
  init_counters();
  //start_papi();
  start_counting();
  int i = 90; 
  double vvv = 9.7;
  double* arr = (double*)(malloc(sizeof(double)*1000));
  for (i = 90; i < 999 ; i++) {
    if (i < 90)
      printf("i = %i\n", i);
    if (i==999)
      break;
    vvv *= i*0.54;
    arr[i] = vvv;
  }
  printf("i = %i , vv = %f \n", i, vvv);
  stop_counting();
  //stop_papi();
  print_counters();  
  print_counters_to_file("native.txt");
  finalize_native();
  finalize();
}
  
void zero_fill() {
  int idx_;
  for (idx_=0; idx_ < num_events; idx_++) {
    Values[idx_] = 0;
  }
}
 
void init_library() {
  int retval = PAPI_library_init(PAPI_VER_CURRENT);  
  if (retval != PAPI_VER_CURRENT) {
    fprintf(stderr,"PAPI library init error!\n");
    handle_error(retval);
  }
}

void init_counters() {
  printf ("Start init_counters\n");
  int max_num_events = get_num_events();
  // Allocate memory for events.
  Events = malloc(sizeof(int)*max_num_events);
  Values = malloc(sizeof(long_long)*max_num_events);

  init_library();
  create_event_set();
  // Bind our EventSet to the cpu component
  // Look at the Multiplexing example in the ctests folder.
  CHECK (PAPI_assign_eventset_component(native_event_set, 0), "assign_event_set_component to 0 (cpu)");
  // Enable and initialize multiplex support
  CHECK (PAPI_multiplex_init(), "Error Initialzing multiplexing !");
  CHECK (PAPI_set_multiplex(native_event_set), "PAPI_set_multiplex(native_event_set)");
  register_flop_events();
  register_mem_events();
  fill_event_set();
  zero_fill();
  printf ("Done init_counters\n");
}

void create_event_set() {
  // Create the event set. 
  native_event_set = PAPI_NULL;
  CHECK (PAPI_create_eventset(&native_event_set), "PAPI_create_event_set failed !");
}

void register_flop_events() {
  // Find the first available native even
  // Figure out how to find the evebt code for 
  // SSE, AVX , x87 (then the ones related to memory).
  int native;
  // Avx. 
  // CHECK(PAPI_event_name_to_code("SIMD_FP_256:PACKED_SINGLE", &native), "Error translating event name to code\n");
  // print_event_info(native);
  // add_event(Events, native, &num_events);

  CHECK(PAPI_event_name_to_code("SIMD_FP_256:PACKED_DOUBLE", &native), "Error translating event name to code\n");
  print_event_info(native);
  add_event(Events, native, &num_events);
  
  // SSE + x87.
  // CHECK(PAPI_event_name_to_code("FP_COMP_OPS_EXE:X87", &native), "Error translating FP_COMP_OS_EXE  event name to code\n");
  // print_event_info(native);
  // add_event(Events, native, &num_events);
  
  CHECK(PAPI_event_name_to_code("FP_COMP_OPS_EXE:SSE_FP_PACKED_DOUBLE", &native), "Error translating FP_COMP_OPS_EXE:SSE_FP_PACKED_DOUBLE event name to code\n");
  print_event_info(native);
  add_event(Events, native, &num_events);
  
  // CHECK(PAPI_event_name_to_code("FP_COMP_OPS_EXE:SSE_FP_SCALAR_SINGLE", &native), "Error translating FP_COMP_OPS_EXE:SSE_FP_SCALAR_SINGLE event name to code\n");
  // print_event_info(native);
  // add_event(Events, native, &num_events);
  
  // CHECK(PAPI_event_name_to_code("FP_COMP_OPS_EXE:SSE_PACKED_SINGLE", &native), "Error translating FP_COMP_OPS_EXE:SSE_PACKED_SINGLE event name to code\n");
  // print_event_info(native);
  // add_event(Events, native, &num_events);
  
//  CHECK(PAPI_event_name_to_code("FP_COMP_OPS_EXE:SSE_SCALAR_DOUBLE", &native), "Error translating FP_COMP_OPS_EXE:SSE_SCALAR_DOUBLE  event name to code\n");
//  print_event_info(native);
//  add_event(Events, native, &num_events);
}

void register_mem_events() {
  int native;
  // Events related to cache and memory accesses: 
  //  iCHECK(PAPI_event_name_to_code("L1D:REPLACEMENT", &native), "Error translating L1D:REPLACEMENT event name to code\n");
  // print_event_info(native);
  // add_event(Events, native, &num_events);
  
  // CHECK(PAPI_event_name_to_code("L1D:M_EVICT", &native), "Error translating L1D:M_EVICT event name to code\n");
  // print_event_info(native);
  // add_event(Events, native, &num_events);
 
  // CHECK(PAPI_event_name_to_code("perf::PERF_COUNT_HW_CACHE_L1D", &native), "Error translating perf::PERF_COUNT_HW_CACHE_L1D:MISS event name to code\n");
  // print_event_info(native);
  // add_event(Events, native, &num_events);
  //
  CHECK(PAPI_event_name_to_code("PAPI_L1_DCM", &native), "Error translating PAPI_L1_DCM event name to code\n");
  print_event_info(native);
  add_event(Events, native, &num_events);
  
  CHECK(PAPI_event_name_to_code("PAPI_L2_DCM", &native), "Error translating PAPI_L2_DCM event name to code\n");
  print_event_info(native);
  add_event(Events, native, &num_events);

  CHECK(PAPI_event_name_to_code("PAPI_L1_LDM", &native), "Error translating PAPI_L1_LDM event name to code\n");
  print_event_info(native);
  add_event(Events, native, &num_events);
  
  CHECK(PAPI_event_name_to_code("PAPI_L1_STM", &native), "Error translating PAPI_L1_STM  event name to code\n");
  print_event_info(native);
  add_event(Events, native, &num_events);
  
  CHECK(PAPI_event_name_to_code("PAPI_L2_STM", &native), "Error translating PAPI_L2_STM  event name to code\n");
  print_event_info(native);
  add_event(Events, native, &num_events);
}

void print_event_info(int event_code) { 
  PAPI_event_info_t info;
  int retval = PAPI_get_event_info(event_code, &info);

  if (retval != PAPI_OK) {
    printf("PAPI_get_event_info failed !\n");
    if (PAPI_enum_event(&event_code, 0) != PAPI_OK) {
      fprintf(stderr,"PAPI_enum_event failed !\n");
    }
    handle_error(retval);
  } else { 
    // Print out the event info !
    printf("------------------------------------\n");
    printf("Event code: %i\n", event_code);
    printf("Name: %s\n", info.symbol);
    printf("Description: %s\n", info.long_descr);
    printf("------------------------------------\n");
  }
}

void fill_event_set() {  
  int retval = PAPI_add_events(native_event_set, Events, num_events); 
  if (retval != PAPI_OK) {
    printf ("Error adding events to event set\n");
    handle_error(retval);    
  }
}

// Used when the non high level Apis are used. 
void start_papi() {
  printf ("Start PAPI events !!\n");
  CHECK (PAPI_start(native_event_set), "Error start_papi()\n");
  printf ("PAPI events stopped !!\n");
}


// Used when the non high level Apis are used. 
void stop_papi() {
  printf ("Stop PAPI events!!\n");
  CHECK (PAPI_stop(native_event_set, Values), "Error stop_papi\n");
  printf ("Stop PAPI events done !!\n");
}

void finalize_native() {
  CHECK (PAPI_cleanup_eventset(native_event_set) , "Error cleaning up events !!\n");
  CHECK (PAPI_destroy_eventset(&native_event_set), "Error destroying events !!\n");
}

#ifdef TEST 
int main() {
  test_papi();
  return 0;
}
#endif

