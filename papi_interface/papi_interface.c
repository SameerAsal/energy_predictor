#include <time.h>
#include <string.h>
#include "papi_interface.h"


// http://icl.cs.utk.edu/projects/papi/wiki/PAPIC:High_Level#High_Level_Code_Example
void start_cpu_counting() { 
  // Decide weather we want to use 
  // start_couters OR start_papi
  // TODO: Smarter way to choose based on numnber of counters we are trying to read !
  start_papi();
  // start_counters();  
  start_usec =  PAPI_get_real_usec();
}

void stop_cpu_counting() {
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

void  print_counters_to_file(char* file_name) {
  int idx=0;
  FILE* out_file = fopen(file_name, "a");
  char event_unit[PAPI_MAX_STR_LEN];
  char event_name[PAPI_MAX_STR_LEN];
  char time_now[100];
  strcpy(event_unit, "NN");   
  // Add the time before appendig the entry for the new measurement. 
  get_time(time_now);
  fprintf(out_file,"\n%s-----------------------------------------------------\n", time_now);
  
  if (cpu_enabled) {
    for (idx=0; idx < cpu_num_events; idx++) {
      PAPI_event_code_to_name(cpu_events[idx], event_name);
      fprintf(out_file,"%s:\t%lld\n", event_name, cpu_values[idx]);
    }
  }

  if (rapl_enabled) {
    for (idx=0; idx < rapl_num_registered_events; idx++) {
      get_event_unit(rapl_events[idx], event_unit);
      PAPI_event_code_to_name(rapl_events[idx], event_name);
      if (strstr(event_unit,"nJ")) {
        fprintf(out_file,"%s :\t%f J\n", event_name, rapl_values[idx]/1.0e09);
      } else {
        fprintf(out_file,"%s :\t%lld \t %s\n", event_name, rapl_values[idx], event_unit);
      }
    } 
  }

  fprintf(out_file,"%s:\t%f\n", "EXEC_TIME", total_usec/1000.0);
  fclose(out_file);
}

void print_counters() {
  int idx=0;
  char event_name[PAPI_MAX_STR_LEN];
  char event_unit[PAPI_MAX_STR_LEN];
      
  if (cpu_enabled) {
    printf("CPU performance counters: \n");
    for (idx=0; idx < cpu_num_events; idx++) {
      PAPI_event_code_to_name(cpu_events[idx], event_name);
      printf("%s :\t%lld\n", event_name, cpu_values[idx]);
    } 
  }

  if (rapl_enabled) {
    for (idx=0; idx < rapl_num_registered_events; idx++) {
      get_event_unit(rapl_events[idx], event_unit);
      PAPI_event_code_to_name(rapl_events[idx], event_name);
      if (strstr(event_unit,"nJ")) {
        printf("%s :\t%f J\n", event_name, rapl_values[idx]/1.0e09);
      } else {
        printf("%s :\t%lld \t %s\n", event_name, rapl_values[idx], event_unit);
      }
    } 
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

void CHECK_BOOL (int retval, char* error_message) {
  if (retval != TRUE) { 
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
  if (retval != PAPI_VER_CURRENT) {
    fprintf(stderr, "PAPI library init error!\n");
    exit(1); 
  }

  do {
   // Translate the integer code to a string.
   if (PAPI_event_code_to_name(EventCode, EventCodeStr) == PAPI_OK) {
      // Print all the native events for this platform.
      printf("Name: %s\nCode: %x\n", EventCodeStr, EventCode);
   } else {
     printf("Failed to translate event name \n");
   }

  } while (PAPI_enum_event(&EventCode, 0) == PAPI_OK);
}

// Adds event to a list of events. 
// Can be used with both RAPL and CPU events. 
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

// void register_events() {
  // add_event(cpu_events,PAPI_VEC_SP, &cpu_num_events);
  // add_event(cpu_events,PAPI_DP_OPS, &cpu_num_events); 
  // counts all the double precision operations. 
//  add_event(cpu_events,PAPI_VEC_DP, &cpu_num_events);
//  add_event(cpu_events,PAPI_L1_DCM, &cpu_num_events);
//  zero_fill_values();
//}

void finalize() {
  free(cpu_events);
  free(cpu_values);
  cpu_num_events = 0;
}

  
void zero_fill_values() {
  int idx_;
  for (idx_=0; idx_ < cpu_num_events; idx_++) {
    cpu_values[idx_] = 0;
  }
}
 
void init_library() {
  int retval = PAPI_library_init(PAPI_VER_CURRENT);  
  if (retval != PAPI_VER_CURRENT) {
    fprintf(stderr,"PAPI library init error!\n");
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
  // CHECK(PAPI_event_name_to_code("SIMD_FP_256:PACKED_SINGLE", &native), "Error translating event name to code\n");
  // print_event_info(native);
  // add_event(cpu_events, native, &cpu_num_events);

  CHECK(PAPI_event_name_to_code("SIMD_FP_256:PACKED_DOUBLE", &native), "Error translating event name to code\n");
  print_event_info(native);
  add_event(cpu_events, native, &cpu_num_events);
  
  // SSE + x87.
  // CHECK(PAPI_event_name_to_code("FP_COMP_OPS_EXE:X87", &native), "Error translating FP_COMP_OS_EXE  event name to code\n");
  // print_event_info(native);
  // add_event(cpu_events, native, &cpu_num_events);
  
  CHECK(PAPI_event_name_to_code("FP_COMP_OPS_EXE:SSE_FP_PACKED_DOUBLE", &native), 
        "Error translating FP_COMP_OPS_EXE:SSE_FP_PACKED_DOUBLE event name to code\n");
  print_event_info(native);
  add_event(cpu_events, native, &cpu_num_events);
  
  // CHECK(PAPI_event_name_to_code("FP_COMP_OPS_EXE:SSE_FP_SCALAR_SINGLE", &native), "Error translating FP_COMP_OPS_EXE:SSE_FP_SCALAR_SINGLE event name to code\n");
  // print_event_info(native);
  // add_event(cpu_events, native, &cpu_num_events);
  
  // CHECK(PAPI_event_name_to_code("FP_COMP_OPS_EXE:SSE_PACKED_SINGLE", &native), "Error translating FP_COMP_OPS_EXE:SSE_PACKED_SINGLE event name to code\n");
  // print_event_info(native);
  // add_event(cpu_events, native, &cpu_num_events);
  
//  CHECK(PAPI_event_name_to_code("FP_COMP_OPS_EXE:SSE_SCALAR_DOUBLE", &native), "Error translating FP_COMP_OPS_EXE:SSE_SCALAR_DOUBLE  event name to code\n");
//  print_event_info(native);
//  add_event(cpu_events, native, &cpu_num_events);
}

void register_mem_events() {
  int native;
  // cpu_events related to cache and memory accesses: 
  //  iCHECK(PAPI_event_name_to_code("L1D:REPLACEMENT", &native),
  //  "Error translating L1D:REPLACEMENT event name to code\n");
  // print_event_info(native);
  // add_event(cpu_events, native, &cpu_num_events);
  
  // CHECK(PAPI_event_name_to_code("L1D:M_EVICT", &native), "Error translating L1D:M_EVICT event name to code\n");
  // print_event_info(native);
  // add_event(cpu_events, native, &cpu_num_events);
 
  // CHECK(PAPI_event_name_to_code("perf::PERF_COUNT_HW_CACHE_L1D", &native), 
  // "Error translating perf::PERF_COUNT_HW_CACHE_L1D:MISS event name to code\n");

  // print_event_info(native);
  // add_event(cpu_events, native, &cpu_num_events);
  //
  CHECK(PAPI_event_name_to_code("PAPI_L1_DCM", &native), "Error translating PAPI_L1_DCM event name to code\n");
  print_event_info(native);
  add_event(cpu_events, native, &cpu_num_events);
  
  CHECK(PAPI_event_name_to_code("PAPI_L2_DCM", &native), "Error translating PAPI_L2_DCM event name to code\n");
  print_event_info(native);
  add_event(cpu_events, native, &cpu_num_events);

  CHECK(PAPI_event_name_to_code("PAPI_L1_LDM", &native), "Error translating PAPI_L1_LDM event name to code\n");
  print_event_info(native);
  add_event(cpu_events, native, &cpu_num_events);
  
  CHECK(PAPI_event_name_to_code("PAPI_L1_STM", &native), "Error translating PAPI_L1_STM  event name to code\n");
  print_event_info(native);
  add_event(cpu_events, native, &cpu_num_events);
  
  CHECK(PAPI_event_name_to_code("PAPI_L2_STM", &native), "Error translating PAPI_L2_STM  event name to code\n");
  print_event_info(native);
  add_event(cpu_events, native, &cpu_num_events);
}


void get_event_unit(int event_code, char *unit) { 
  PAPI_event_info_t info;
  strcpy(unit, "II");
  int retval = PAPI_get_event_info(event_code, &info);

  if (retval != PAPI_OK) {
    printf("PAPI_get_event_info failed !\n");
    if (PAPI_enum_event(&event_code, 0) != PAPI_OK) {
      fprintf(stderr,"PAPI_enum_event failed !\n");
    }
    strcpy(unit, "Unknown");
  } else {
    strcpy(unit, info.units);
  }
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
    printf("Units: %s\n", info.units);
    printf("Description: %s\n", info.long_descr);
    printf("------------------------------------\n");
  }
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


// Used when the non high level Apis are used. 
void stop_papi() {
  printf ("Stop PAPI events!!\n");
  CHECK (PAPI_stop(cpu_native_event_set, cpu_values), "Error stop_papi\n");
  printf ("Stop PAPI events done !!\n");
}

void finalize_native() {
  CHECK (PAPI_cleanup_eventset(cpu_native_event_set) , "Error cleaning up events !!\n");
  CHECK (PAPI_destroy_eventset(&cpu_native_event_set), "Error destroying events !!\n");
}


void print_comp_details(const PAPI_component_info_t * cmp) {
  printf ("Name: %s\n", cmp->name);
  printf ("Description: %s\n", cmp->description);
  printf ("Version: %s\n", cmp->version);
  printf ("Support Version: %s\n", cmp->support_version);
  printf ("Kernel Version: %s\n", cmp->kernel_version);
}

BOOL find_rapl() {
  int num_comps; 
  int cid;
  // First, look for the RAPL component:
  num_comps = PAPI_num_components();
  for (cid=0; cid<num_comps; cid++) {
    cmpinfo = PAPI_get_component_info(cid);
    if (cmpinfo == NULL) {
       printf("PAPI_get_component_info failed\n");
       exit(-1);
    }

    if (strstr(cmpinfo->name,"rapl")) {
      rapl_cid=cid;
      printf("Found rapl component at cid %d\n",rapl_cid);
      if (cmpinfo->disabled) { 
        printf("RAPL component disabled: %s\n", cmpinfo->disabled_reason);
      } else {
        print_comp_details(cmpinfo); 
        return TRUE;
      }
    }
  } 
  return FALSE;
}


void list_rapl_events() {
  int code = PAPI_NATIVE_MASK;
  int    r = PAPI_enum_cmp_event(&code, PAPI_ENUM_EVENTS, rapl_cid);
  rapl_events_count = 0;
  CHECK(r, " PAPI_enum_cmp_events failed\n");
  
  printf("listing RAPL events\n");
  while (r == PAPI_OK) {
    rapl_events_count++;
    print_event_info (code); 
    r = PAPI_enum_cmp_event(&code, PAPI_ENUM_EVENTS, rapl_cid);
  }
}


void read_config() {
  // Supposedly reading sone hypotheitcal config file that will hopefully 
  // set the values for settings for an illusion of organized code.
  rapl_enabled = TRUE; 
  cpu_enabled  = TRUE;
}

void register_energy_events() {
  int native;
  rapl_num_registered_events = 0;
  
  CHECK(PAPI_event_name_to_code("rapl:::PACKAGE_ENERGY:PACKAGE0", &native), "Error translating event name to code\n");
  add_event(rapl_events, native, &rapl_num_registered_events);

  CHECK(PAPI_event_name_to_code("rapl:::PP1_ENERGY:PACKAGE0", &native), "Error translating event name to code\n");
  add_event(rapl_events, native, &rapl_num_registered_events);
  
  CHECK(PAPI_event_name_to_code("rapl:::PP0_ENERGY:PACKAGE0", &native), "Error translating rapl:::PP0_ENERGY_CNT:PACKAGE0\n");
  add_event(rapl_events, native, &rapl_num_registered_events);

  // CHECK(PAPI_event_name_to_code("THERMAL_SPEC:PACKAGE0", &native), "Error translating event name to code\n");
  // add_event(rapl_events, native, &rapl_num_registered_events);
  
  //CHECK(PAPI_event_name_to_code("MSR_PKG_ENERGY_STATUS", &native), "Error translating MSR_PKG_ENERGY_STATUS to numeric code\n");
  //add_event(rapl_events, native, &rapl_num_registered_events);
 
  // Now add events to the event set !
  CHECK (PAPI_add_events(rapl_event_set, rapl_events, rapl_num_registered_events), "Error adding events to RAPL EventSet");
  printf("Events added to event set successfully !!!\n"); 
}
 

// Initialize everything
void init_counters() {

  printf("Inside init_counters()\n");
  read_config();
  init_library();
  if (rapl_enabled) {
    init_rapl_counters();
  }

  if (cpu_enabled) {
    init_cpu_counters();
  }
}

// Start counting.
void start_counting() {
  if (rapl_enabled) {
    start_rapl_counting();
  }
  if (cpu_enabled) {
    start_cpu_counting();
  }
}
// Stop counting.
void stop_counting() {
  if (rapl_enabled) {
    stop_rapl_counting();
  }
  if (cpu_enabled) {
    stop_cpu_counting();
  }
}

void init_rapl_counters() {
  CHECK_BOOL (find_rapl(), "RAPL component not found in the system !!");
  rapl_event_set = PAPI_NULL;  
  list_rapl_events();
  CHECK (PAPI_create_eventset(&rapl_event_set), "PAPI_create_event_set for RAPL failed !");
  CHECK (PAPI_assign_eventset_component(rapl_event_set, rapl_cid),"Assigning rapl_event_set to RAPL");
  // This breaks things when trying to register events in event set. the call reyurns with PAPI_OK though !!
  // CHECK (PAPI_set_multiplex(rapl_event_set), "PAPI_set_multiplex(rapl_event_set) Failed !!!");
  
  rapl_values = (long_long*)calloc(rapl_events_count, sizeof(long_long));
  rapl_events = (int*)calloc(rapl_events_count, sizeof(int));
  
  register_energy_events();
  printf("RAPL counters initialized \n");
}

void start_rapl_counting() {
  printf ("Start PAPI RAPL events !!\n");
  CHECK (PAPI_start(rapl_event_set), "Error start_rapl_counting()\n");
  printf ("PAPI RAPL events started !!\n");
}

void stop_rapl_counting() {
  printf ("Stop PAPI RAPL events!!\n");
  CHECK (PAPI_stop(rapl_event_set, rapl_values), "Error stop_papi\n");
  printf ("Stop PAPI RAPl events done !!\n");
}

void test() {    
  init_counters();
  start_counting();
  test_papi(); 
  stop_counting();
  print_counters();  
  print_counters_to_file("rapl+cpu.res");  
  
  finalize_native();
  finalize();
}


void test_papi() {
#define MATRIX_SIZE 1024
 static double a[MATRIX_SIZE][MATRIX_SIZE];
 static double b[MATRIX_SIZE][MATRIX_SIZE];
 static double c[MATRIX_SIZE][MATRIX_SIZE];
    /* Naive matrix multiply */
 double s;
 int i,j,k;


  for(i=0;i<MATRIX_SIZE;i++) {
    for(j=0;j<MATRIX_SIZE;j++) {
      a[i][j]=(double)i*(double)j;
        b[i][j]=(double)i/(double)(j+5);
    }
  }

  for(j=0;j<MATRIX_SIZE;j++) {
    for(i=0;i<MATRIX_SIZE;i++) {
      s=0;
      for(k=0;k<MATRIX_SIZE;k++) {
        s+=a[i][k]*b[k][j];
      }
      c[i][j] = s;
     }
  }

  s=0.0;
  for(i=0;i<MATRIX_SIZE;i++) {
    for(j=0;j<MATRIX_SIZE;j++) {
      s+=c[i][j];
     }
   }
}


void get_time(char* now) {
  time_t current_time;
  char* c_time_string; 
  /* Obtain current time as seconds elapsed since the Epoch. */
  current_time = time(NULL);  
  /* Convert to local time format. */
  c_time_string = ctime(&current_time); 
  strcpy(now, c_time_string);
}

#ifdef TEST 
int main() {
  read_config();
  test();
  return 0;
}
#endif

