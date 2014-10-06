#ifdef PERFCTR
void PERF_INIT() { 
  init_counters("../interface.cfg");  
  start_counting();
}

void PERF_EXIT(char* name) { 
  char file_name[64];
  sprintf(file_name, "%s_timings.txt", name);
  stop_counting();
  print_counters();
  print_counters_to_file(file_name);
  finalize();
}
#endif
