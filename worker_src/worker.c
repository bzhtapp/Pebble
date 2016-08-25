#include <pebble_worker.h>
#include <pedometer.h>

static uint16_t s_step_gForce;

static void update_step(int steps){
  int step_count = persist_read_int(s_step_key);
    
  step_count += steps; //increment step
  
  persist_write_int(s_step_key, step_count);
  
}

static void accel_data_handler(AccelData *data, uint32_t num_samples){
    // Read first sample gForce
  int16_t gForce = data[0].y;
  bool did_vibrate = data[0].did_vibrate;
  
  //default number of credited steps per event
  int steps = 1;
  
  //Count as a step if difference between last stored value and current value exceed minimum difference
  //and current gForce above threshold
  //and watch did not vibrate
  if ( abs(gForce - s_step_gForce) > s_step_min_diff
      && abs(gForce) > s_step_threshold 
     && !did_vibrate) 
  {
    update_step(steps); 
  }
  
  //store new current value
  s_step_gForce = gForce; 
  
}

static void prv_init() {
  // Initialize the worker here
    
  //change accelerameter sampling rate
  accel_service_set_sampling_rate(s_acelSamplingRate);

  // Subscribe to batched data events (for step count, etc)
  accel_data_service_subscribe(s_samples_per_update, accel_data_handler);
}

static void prv_deinit() {
  // Deinitialize the worker here
}

int main(void) {
  prv_init();
  worker_event_loop();
  prv_deinit();
}
