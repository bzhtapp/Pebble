#include <pebble.h>

static TextLayer *s_step_layer;
static GFont s_step_font;
const uint32_t s_samples_per_update = 5;
static uint16_t s_step_gForce;
const uint16_t s_step_min_diff = 400;
const uint16_t s_step_timeStamp_diff_min = 150;
const uint16_t s_step_threshold = 550;
const AccelSamplingRate s_acelSamplingRate = ACCEL_SAMPLING_10HZ;
uint32_t s_step_key = 1;
static time_t s_timeStamp;
static int s_timeStamp_diff;

static void update_step(int steps) {
  
  int step_count = persist_read_int(s_step_key);
    
  // Check to see if step variable has been initialized
  step_count += steps; //increment step
  
  persist_write_int(s_step_key, step_count);
  
  static char s_buffer[15];
  
  snprintf(s_buffer, sizeof(s_buffer), "Steps: %d", step_count);
  
  // Display this time on the TextLayer
  text_layer_set_text(s_step_layer, s_buffer);
}

static void accel_data_handler(AccelData *data, uint32_t num_samples) {
  
  // Read first sample gForce
  int16_t gForce = data[0].y;
  bool did_vibrate = data[0].did_vibrate;
  time_t timeStamp = data[0].timestamp;
  
  //default number of credited steps per event
  int steps = 1;
  
  //initialize static timeStamp
  if (!s_timeStamp) {
    s_timeStamp = timeStamp;
  }
  
  s_timeStamp_diff = timeStamp - s_timeStamp;
  
  //Count as a step if difference between last stored value and current value exceed minimum difference
  //and current gForce above threshold
  //and watch did not vibrate
  //and time different between previous sample more than 200 millisecond
  if ( abs(gForce - s_step_gForce) > s_step_min_diff
      && abs(gForce) > s_step_threshold 
     && !did_vibrate
     && s_timeStamp_diff > s_step_timeStamp_diff_min) 
  {
    update_step(steps); 
    s_timeStamp = timeStamp;
  }
  
  //store new current value
  s_step_gForce = gForce;

  
}
