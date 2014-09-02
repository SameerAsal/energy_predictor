// host_micpowr related events:
int 	  mic_cid;
int 	  mic_events_count; 
int 	  mic_event_set;
int 	  *mic_events;    // Energy related events we need to subscribe to. 
long_long *mic_values;
int mic_num_registered_events;

// host_micpower:
void list_mic_events();
void init_mic_counters();
void start_mic_counting();
void stop_mic_counting();

// private mthods:
void register_mic_energy_events();
