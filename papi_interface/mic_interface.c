#include "papi_interface.h"
#include "mic_interface.h"

void init_mic_counters() {
  CHECK_BOOL (find_cmp("host_micpower", &mic_cid), "host_micpower component not found in the system !!");
  printf ("mic_cid = %i\n", mic_cid);
  mic_event_set = PAPI_NULL;  
  list_mic_events();
  CHECK (PAPI_create_eventset(&mic_event_set), "PAPI_create_event_set for mic_event_set failed !");
  CHECK (PAPI_assign_eventset_component(mic_event_set, mic_cid),"Assigning mic_event_set to RAPL");
  // This breaks things when trying to register events in event set. the call returns with PAPI_OK though !!
  // CHECK (PAPI_set_multiplex(rapl_event_set), "PAPI_set_multiplex(rapl_event_set) Failed !!!");
  
  mic_values = (long_long*)calloc(mic_events_count, sizeof(long_long));
  mic_events = (int*)calloc(mic_events_count, sizeof(int));
  
  register_mic_energy_events();
  printf("mic counters initialized \n");
}

void register_mic_energy_events() {
  int native;
  rapl_num_registered_events = 0;
  
  CHECK(PAPI_event_name_to_code("host_micpower:::mic0:tot0", &native), "Error translating host_micpower:::mic0:tot0 to code\n");
  add_event(mic_events, native, &mic_num_registered_events);

  CHECK(PAPI_event_name_to_code("host_micpower:::mic0:pcie", &native), "Error translating host_micpower:::mic0:pcie to code\n");
  add_event(mic_events, native, &mic_num_registered_events);

  // Now add events to the event set !
  CHECK (PAPI_add_events(mic_event_set, mic_events, mic_num_registered_events), "Error adding events to RAPL EventSet");
  printf("Events added to event set successfully !!!\n"); 
}

void start_mic_counting() {
  //printf ("Start PAPI RAPL events !!\n");
  CHECK (PAPI_start(mic_event_set), "Error start_mic_counting()\n");
  printf ("PAPI host_micpower events started !!\n");
}

void stop_mic_counting() {
  printf ("Stop PAPI host_micpower events!!\n");
  CHECK (PAPI_stop(mic_event_set, mic_values), "Error stop_papi\n");
  printf ("Stop PAPI host_micpower events done !!\n");
}

void list_mic_events() {
  int code = PAPI_NATIVE_MASK;
  int    r = PAPI_enum_cmp_event(&code, PAPI_ENUM_EVENTS, mic_cid);
  mic_events_count = 0;
  CHECK(r, " PAPI_enum_cmp_events failed\n");
  
  // printf("listing RAPL events\n");
  while (r == PAPI_OK) {
    mic_events_count++;
    // print_event_info (code); 
    r = PAPI_enum_cmp_event(&code, PAPI_ENUM_EVENTS, mic_cid);
  }
}

 
