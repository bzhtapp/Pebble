#include <pebble.h>
#include <pedometer.h>

#include "bluetooth.c"
#include "battery.c"
#include "weather.c"

static Window *s_main_window;
static TextLayer *s_date_layer;
static TextLayer *s_time_layer;
static TextLayer *s_step_layer;
static GFont s_step_font;
static uint16_t s_step_gForce;
static time_t s_timeStamp;
static int s_timeStamp_diff;

static void update_step(int steps) {
  
  int step_count = persist_read_int(s_step_key);
    
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
  
static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
}

static void update_date() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current date in like "Aug 08"
  static char s_buffer[15];
  
  strftime(s_buffer, sizeof(s_buffer), "%a %b %d", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_date_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  update_date();
  
  // Reset step count at midnight 23:59
  if (tick_time->tm_hour == 23
      && tick_time->tm_min == 59) {
    persist_write_int(s_step_key, 0);
  }
  
  //Prevent duplicate weather api call when watchface initially loads by moving forecast check minute marker by one minute
  if (!s_forecast_check)
  {
    if (tick_time->tm_min == 0){
      s_forecast_check = 59;
    }
    else {
      s_forecast_check = tick_time->tm_min - 1;
    }
  }
  
  // Get weather update every half hour from the last check
  if( tick_time->tm_min % 30 == s_forecast_check % 30) { 
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);

    // Send the message!
    app_message_outbox_send();
  }
  
}


static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  //Separate row pedometer is in into two columns
  int dividerStep = bounds.size.w * 3 / 4;
  
  //Track current Y position of bottom most layer as layers are being stack top to bottom
  int currentY = 0;
  
  //defines height of Step layer
  int heightStep = 14;
  
  // Create Pedometer Layer, GRect is (X,Y,Width,Height)
  s_step_layer = text_layer_create(
    GRect(0, currentY, dividerStep, heightStep));  
  //  GRect(0, PBL_IF_ROUND_ELSE(5, currentY), dividerStep, heightStep));
  
  s_step_font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  text_layer_set_font(s_step_layer, s_step_font);
  
  text_layer_set_background_color(s_step_layer, GColorWhite);
  text_layer_set_text_color(s_step_layer, GColorBlack);
  text_layer_set_text_alignment(s_step_layer, GTextAlignmentLeft);
  
  layer_add_child(window_layer, text_layer_get_layer(s_step_layer));
  
    // Create battery meter Layer
  s_battery_layer = text_layer_create(
    GRect(dividerStep, currentY, bounds.size.w - dividerStep, heightStep));
  
  s_battery_font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  text_layer_set_font(s_battery_layer, s_battery_font);
  
  text_layer_set_background_color(s_battery_layer, GColorWhite);
  text_layer_set_text_color(s_battery_layer, GColorBlack);
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentRight);

  layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));
  currentY += heightStep; //current Y position is now at bottom of height Layer
  
  /*** End Step row ***/
  
  
  int heightDate = 30;
  
    // Create Date layer
  s_date_layer = text_layer_create(
      GRect(0, currentY, bounds.size.w, heightDate));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_date_layer, GColorWhite);
  text_layer_set_text_color(s_date_layer, GColorBlack);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

  // Create the Bluetooth icon GBitmap
  s_bt_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT_ICON);

  // Create the BitmapLayer to display the GBitmap
  int btWidth = 30;
  int btX = bounds.size.w - btWidth; //place bluetooth bitmap on right side
  
  s_bt_icon_layer = bitmap_layer_create(GRect(btX, 15, btWidth, 30));
  bitmap_layer_set_bitmap(s_bt_icon_layer, s_bt_icon_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bt_icon_layer));

  // Show the correct state of the BT connection from the start
  bluetooth_callback(connection_service_peek_pebble_app_connection());
  currentY += heightDate;

  /*** End Date row ***/
 
  // Create Time TextLayer with specific bounds
  int heightTime = 44;
  s_time_layer = text_layer_create(
      GRect(0, currentY, bounds.size.w, heightTime));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorWhite);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  currentY += heightTime;
  
  /*** End Time Row ***/

  // Create weather Layer
  int heightWeather = 14;
  s_weather_layer = text_layer_create(
    GRect(0, currentY, bounds.size.w, heightWeather));

  // Style the text
  text_layer_set_background_color(s_weather_layer, GColorWhite);
  text_layer_set_text_color(s_weather_layer, GColorBlack);
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
  text_layer_set_text(s_weather_layer, "Loading Weather ...");
  
  // Create second custom font, apply it and add to Window
  s_weather_font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  text_layer_set_font(s_weather_layer, s_weather_font);
  
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));
  currentY += heightWeather;
  
  /*** End Weather Row ***/
  
  
  // Create forecast hour 3 row ***/
  int heightforecastHr3 = 14;
  s_forecastHr3_layer = text_layer_create(
    GRect(0, currentY, bounds.size.w, heightforecastHr3));

  // Style the text
  text_layer_set_background_color(s_forecastHr3_layer, GColorWhite);
  text_layer_set_text_color(s_forecastHr3_layer, GColorBlack);
  text_layer_set_text_alignment(s_forecastHr3_layer, GTextAlignmentCenter);
  text_layer_set_text(s_forecastHr3_layer, "Loading 3-hr Forecast...");
  text_layer_set_font(s_forecastHr3_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecastHr3_layer));
  currentY += heightforecastHr3;
  
  /*** End ForeCast 3 Hour row ***/
  
  // Create forecast Day 2 layer
  int heightforecastDay2 = 14;
  s_forecastDay2_layer = text_layer_create(
    GRect(0, currentY, bounds.size.w, heightforecastDay2));

  // Style the text
  text_layer_set_background_color(s_forecastDay2_layer, GColorWhite);
  text_layer_set_text_color(s_forecastDay2_layer, GColorBlack);
  text_layer_set_text_alignment(s_forecastDay2_layer, GTextAlignmentCenter);
  text_layer_set_text(s_forecastDay2_layer, "Loading morning forecast...");
  text_layer_set_font(s_forecastDay2_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecastDay2_layer));
  currentY += heightforecastDay2;
  
  // Create forecast Day 3 layer
  int heightforecastDay3 = 14;
  s_forecastDay3_layer = text_layer_create(
    GRect(0, currentY, bounds.size.w, heightforecastDay3));

  // Style the text
  text_layer_set_background_color(s_forecastDay3_layer, GColorWhite);
  text_layer_set_text_color(s_forecastDay3_layer, GColorBlack);
  text_layer_set_text_alignment(s_forecastDay3_layer, GTextAlignmentCenter);
  text_layer_set_text(s_forecastDay3_layer, "Loading evening forecast...");
  text_layer_set_font(s_forecastDay3_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecastDay3_layer));
  currentY += heightforecastDay3;
  
  int heightforecastCity = 14;
  s_forecastCity_layer = text_layer_create(
    GRect(0, currentY, bounds.size.w, heightforecastDay3));

  // Style the text
  text_layer_set_background_color(s_forecastCity_layer, GColorWhite);
  text_layer_set_text_color(s_forecastCity_layer, GColorBlack);
  text_layer_set_text_alignment(s_forecastCity_layer, GTextAlignmentCenter);
  text_layer_set_text(s_forecastCity_layer, "Loading city...");
  text_layer_set_font(s_forecastCity_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecastCity_layer));
  currentY += heightforecastCity;
}

static void main_window_unload(Window *window) {

  // Destroy Step Layer
  text_layer_destroy(s_step_layer);
  
  // Destroy battery layer
  text_layer_destroy(s_battery_layer);
  
  //destory bluetooth  layer and bit map
  gbitmap_destroy(s_bt_icon_bitmap);
  bitmap_layer_destroy(s_bt_icon_layer);
  
  //Destroy Date Layer
  text_layer_destroy(s_date_layer);
    
  // Destroy Time Layer
  text_layer_destroy(s_time_layer);
  
  // Destory WeatherLayer
  text_layer_destroy(s_weather_layer);
  
  // Destory ForecastHour3 Layer
  text_layer_destroy(s_forecastHr3_layer);
  
  // Destory ForecastDay2 Layer
  text_layer_destroy(s_forecastDay2_layer);
  
  // Destory ForecastDay3 Layer
  text_layer_destroy(s_forecastDay3_layer);
  
  // Destory ForecastCity Layer
  text_layer_destroy(s_forecastCity_layer);
  
}


static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  
  // Check to see if the worker is currently active
  bool running = app_worker_is_running();
  
  // Stop worker if it is running
  if (running) {
    app_worker_kill();
  }
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
  const int inbox_size = 128;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Make sure the time is displayed from the start
  update_time();
  
  //Make sure the date is displayed from the start
  update_date();
  
  //change accelerameter sampling rate
  accel_service_set_sampling_rate(s_acelSamplingRate);

  // Subscribe to batched data events (for step count, etc)
  accel_data_service_subscribe(s_samples_per_update, accel_data_handler);
  
  //Make sure the step count is displayed from the start
  update_step(0);
 
  // Ensure battery level is displayed from the start
  battery_callback(battery_state_service_peek());
  
  // Register for battery level updates
  battery_state_service_subscribe(battery_callback);
  
  // Register for Bluetooth connection updates
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback
  });
  
  
}

static void deinit() {
  
  //launch background work process before this window is closed
  AppWorkerResult result = app_worker_launch();
  
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}