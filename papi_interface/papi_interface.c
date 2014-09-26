#include <time.h>
#include <string.h>
#include <unistd.h>
#include "cpu_interface.c"
#include "rapl_interface.c"

#ifndef DISABLE_MIC
  #include "mic_interface.c"
  #include "mic_access_sdk_interface.c"
#endif

void init_library() {
  int retval = PAPI_library_init(PAPI_VER_CURRENT);
  if (retval != PAPI_VER_CURRENT) {
    fprintf(stderr,"PAPI library init error!\n");
    handle_error(retval);
  }
}

BOOL file_exists(char* name) {
  FILE* file = NULL;
  file = fopen(name,"r");
  if (NULL == file) {
    return FALSE;
  } else {
    fclose(file); 
    return TRUE;
  }  
}

void  print_counters_to_file(char* file_name) {

  int idx=0;
  FILE* out_file;
  char event_unit[PAPI_MAX_STR_LEN];
  char event_name[PAPI_MAX_STR_LEN];
  char time_now[100];
  BOOL exists = file_exists(file_name); 

 // if (!exists) {
 // if (TRUE) {
  if (exists) {
    fprintf(stdout, "file already exists open for append !!\n");
    out_file = fopen(file_name, "a");
  }  else {
    fprintf(stdout, "file doesn't exists open for first time !!\n");
    out_file = fopen(file_name, "w");
  }

  get_time(time_now);
  fprintf(out_file, "%s\t\n", time_now);
  // Add column for time stamp
  // fprintf(out_file,"%s", "Time_Stamp\t");

  // Add the header for cpu counters
  if (cpu_enabled) {
    for (idx=0; idx < cpu_num_events; idx++) {
      PAPI_event_code_to_name(cpu_events[idx], event_name);
      fprintf(out_file,"%s\t", event_name);
    }
  }

  // Add the header for RAPL counters:
  if (rapl_enabled) {
    for (idx=0; idx < rapl_num_registered_events; idx++) {
      get_event_unit(rapl_events[idx], event_unit);
      PAPI_event_code_to_name(rapl_events[idx], event_name);
      if (strstr(event_unit,"nJ")) {
        fprintf(out_file,"%s(J)\t", event_name);
      } else {
        fprintf(out_file,"%s\t", event_name);
      }
    } 
  }

#ifndef DISABLE_MIC
  if (mic_enabled) {
    for (idx=0; idx < mic_num_registered_events; idx++) {
      PAPI_event_code_to_name(mic_events[idx], event_name);
      fprintf(out_file,"%s\t", event_name);
    }
  }

  // double mic_total0_energy;
  // double mic_total1_energy;
  // double mic_pcie_energy;

  if (mic_access_sdk_enabled) {  
    fprintf(out_file,"total0\ttotal1\tpcie\t");
    //for (idx=0; idx < 3; idx++) {
    //}
  }
#endif

    // Add Column for exeution time 
  fprintf(out_file,"%s", "EXEC_TIME");
  fprintf(out_file, "\n");


  // Print out the data here !
  if (cpu_enabled) {
    for (idx=0; idx < cpu_num_events; idx++) {
      fprintf(out_file,"%lld\t", cpu_values[idx]);
    }
  }

  if (rapl_enabled) {
    for (idx=0; idx < rapl_num_registered_events; idx++) {
      get_event_unit(rapl_events[idx], event_unit);
      PAPI_event_code_to_name(rapl_events[idx], event_name);
      if (strstr(event_unit,"nJ")) {
        fprintf(out_file,"%f\t", rapl_values[idx]/1.0e09);
      } else {
        fprintf(out_file,"%lld\t",  rapl_values[idx]);
      }
    } 
  }

#ifndef DISABLE_MIC
  if (mic_enabled) {
    for (idx=0; idx < mic_num_registered_events; idx++) {
      get_event_unit(mic_events[idx], event_unit);
      PAPI_event_code_to_name(mic_events[idx], event_name);
      if (strstr(event_unit,"nJ")) {
        fprintf(out_file,"%f\t", mic_values[idx]/1.0e09);
      } else {
        fprintf(out_file,"%lld\t",  mic_values[idx]);
      }
    }
  }

 if (mic_access_sdk_enabled) {  
    //fprintf(out_file,"total0\ttotal1\tpcie");
     fprintf(out_file,"%f\t%f\t%f\t", mic_total0_energy, mic_total1_energy, mic_pcie_energy);
  }
#endif

  fprintf(out_file,"%f\n", total_usec/1000.0);
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

#ifndef DISABLE_MIC
  if (mic_enabled) {
    for (idx=0; idx < mic_num_registered_events; idx++) {
      get_event_unit(mic_events[idx], event_unit);
      PAPI_event_code_to_name(mic_events[idx], event_name);
      if (strstr(event_unit,"nJ")) {
        printf("%f\t", mic_values[idx]/1.0e09);
      } else {
        printf("%lld\t",  mic_values[idx]);
      }
    }
  }

  if (mic_access_sdk_enabled) {
    // Print out all the numbers, add the headers first !
  }
#endif

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
    // printf ("Adding %s to list of events\n", event_name);
    (*idx)++;
  }
}

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


void finalize_native() {
  if (cpu_enabled) {
    CHECK (PAPI_cleanup_eventset(cpu_native_event_set) , "Error cleaning up events !!\n");
    CHECK (PAPI_destroy_eventset(&cpu_native_event_set), "Error destroying events !!\n");
  }

  if (rapl_enabled) {
    CHECK (PAPI_cleanup_eventset(rapl_event_set) , "Error rapl cleaning up events !!\n");
    CHECK (PAPI_destroy_eventset(&rapl_event_set), "Error rapl destroying events !!\n");
  }

#ifndef DISABLE_MIC
  if (mic_enabled) {
    CHECK (PAPI_cleanup_eventset(mic_event_set) , "Error rapl cleaning up events !!\n");
    CHECK (PAPI_destroy_eventset(&mic_event_set), "Error mic destroying events !!\n"); 
  }
#endif

}


void print_comp_details(const PAPI_component_info_t * cmp) {
  printf ("Name: %s\n", cmp->name);
  printf ("Description: %s\n", cmp->description);
  printf ("Version: %s\n", cmp->version);
  printf ("Support Version: %s\n", cmp->support_version);
  printf ("Kernel Version: %s\n", cmp->kernel_version);
}

BOOL find_cmp(char *cmp_name, int* cmp_id) {
  int num_comps; 
  int cid;
  // First, look for the component:
  num_comps = PAPI_num_components();
  for (cid=0; cid<num_comps; cid++) {
    cmpinfo = PAPI_get_component_info(cid);
    if (cmpinfo == NULL) {
       printf("PAPI_get_component_info failed\n");
       exit(-1);
    }

    if (strstr(cmpinfo->name, cmp_name)) {
      *cmp_id = cid;
      printf("Found %s component at cid %d\n", cmp_name, *cmp_id);
      if (cmpinfo->disabled) { 
        printf("%s component disabled: %s\n", cmp_name, cmpinfo->disabled_reason);
      } else {
        print_comp_details(cmpinfo); 
        return TRUE;
      }
    }
  }
  return FALSE;
}

void read_config() {
  // Supposedly reading some hypotheitcal config file that will hopefully 
  // set the values for settings for an illusion of organized code.
  rapl_enabled    = TRUE; 
  cpu_enabled     = TRUE;

#ifdef DISABLE_MIC
  mic_enabled     = FALSE; 
  mic_access_sdk_enabled = FALSE;
#endif 
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

#ifndef DISABLE_MIC
  if (mic_enabled) { 
    init_mic_counters();   
  }

  if (mic_access_sdk_enabled) {
    init_mic_access_sdk_counters();
  }
#endif

}

// Start counting.
void start_counting() {
  if (rapl_enabled) {
    start_rapl_counting();
  }

  if (cpu_enabled) {
    start_cpu_counting();
  }

#ifndef DISABLE_MIC
  if (mic_enabled) {
    start_mic_counting();
  }

  if (mic_access_sdk_enabled) {
    start_mic_access_sdk_counting();
  }
#endif

  start_usec =  PAPI_get_real_usec();
}

// Stop counting.
void stop_counting() {
  if (rapl_enabled) {
    stop_rapl_counting();
  }

  if (cpu_enabled) {
    stop_cpu_counting();
  }

#ifndef DISABLE_MIC
  if (mic_enabled) {
    stop_mic_counting();
  }

  if (mic_access_sdk_enabled) {
    stop_mic_access_sdk_counting();
  }
#endif

  end_usec = PAPI_get_real_usec();
  total_usec = end_usec - start_usec;
}

void test() {    
  init_counters();
  start_counting();
  test_papi(); 
  stop_counting();
  print_counters();  
  print_counters_to_file("micpower+rapl+cpu.txt");  
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

  usleep(20.56*1e06);
   // Add a sample to offload the code to mic.
}


void get_time(char* now) {
  time_t current_time;
  char* c_time_string; 
  /* Obtain current time as seconds elapsed since the Epoch. */
  current_time = time(NULL);  
  /* Convert to local time format. */
  c_time_string = ctime(&current_time); 

  // Remnove the \n
  c_time_string[strlen(c_time_string) - 1] = 0;
  c_time_string[strlen(c_time_string)] = 0;
  strcpy(now, c_time_string);
}

#ifdef TEST 
int main() {
  read_config();
  test();
  return 0;
}
#endif
