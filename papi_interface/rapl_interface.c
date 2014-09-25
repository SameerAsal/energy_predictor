#include "papi_interface.h"
#include "rapl_interface.h"


void list_rapl_events() {
  int code = PAPI_NATIVE_MASK;
  int    r = PAPI_enum_cmp_event(&code, PAPI_ENUM_EVENTS, rapl_cid);
  rapl_events_count = 0;
  CHECK(r, " PAPI_enum_cmp_events failed\n");
  // printf("listing RAPL events\n");
  while (r == PAPI_OK) {
    rapl_events_count++;
    // print_event_info (code); 
    r = PAPI_enum_cmp_event(&code, PAPI_ENUM_EVENTS, rapl_cid);
  }
}


void init_rapl_counters() {
  CHECK_BOOL (find_cmp("rapl", &rapl_cid), "RAPL component not found in the system !!");
  rapl_event_set = PAPI_NULL;  
  list_rapl_events();
  CHECK (PAPI_create_eventset(&rapl_event_set), "PAPI_create_event_set for RAPL failed !");
  CHECK (PAPI_assign_eventset_component(rapl_event_set, rapl_cid),"Assigning rapl_event_set to RAPL");

  // This breaks things when trying to register events in event set. the call reyurns with PAPI_OK though !!
  // CHECK (PAPI_set_multiplex(rapl_event_set), "PAPI_set_multiplex(rapl_event_set) Failed !!!");
  rapl_values = (long_long*)calloc(rapl_events_count, sizeof(long_long));
  rapl_events = (int*)calloc(rapl_events_count, sizeof(int));

  register_rapl_energy_events();
  printf("RAPL counters initialized \n");
}

void start_rapl_counting() {
  //printf ("Start PAPI RAPL events !!\n");
  CHECK (PAPI_start(rapl_event_set), "Error start_rapl_counting()\n");
  printf ("PAPI RAPL events started !!\n");
}

void stop_rapl_counting() {
  printf ("Stop PAPI RAPL events!!\n");
  CHECK (PAPI_stop(rapl_event_set, rapl_values), "Error stop_papi\n");
  printf ("Stop PAPI RAPl events done !!\n");
}


void register_rapl_energy_events() {
  int native;
  rapl_num_registered_events = 0;
  
  CHECK(PAPI_event_name_to_code("rapl:::PACKAGE_ENERGY:PACKAGE0", &native), "Error translating event name to code: rapl:::PACKAGE_ENERGY:PACKAGE0 \n");
  add_event(rapl_events, native, &rapl_num_registered_events);

  CHECK(PAPI_event_name_to_code("rapl:::PP1_ENERGY:PACKAGE0", &native), "Error translating event name to code: rapl:::PP1_ENERGY:PACKAGE0\n");
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


