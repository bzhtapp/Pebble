#include <pebble.h>

static TextLayer *s_forecastHr3_layer;
static TextLayer *s_forecastDay2_layer;
static TextLayer *s_forecastDay3_layer;
static TextLayer *s_forecastCity_layer;
static TextLayer *s_weather_layer;
static uint16_t s_forecast_check;
static GFont s_weather_font;

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {

  // Store incoming information
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char weather_layer_buffer[32];
  static char forecastHr3_layer_buffer[32];
  static char forecastHr3_temperature_buffer[8];
  static char forecastHr3_conditions_buffer[32];
  static char forecastDay2_layer_buffer[32];
  static char forecastDay2_temperature_buffer[8];
  static char forecastDay2_conditions_buffer[32];
  static char forecastDay3_layer_buffer[32];
  static char forecastDay3_temperature_buffer[8];
  static char forecastDay3_conditions_buffer[32];
  static char forecastCity_layer_buffer[32];
  static char forecastCity_buffer[32];
  
  static char time_buffer[8];
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  // Read Weather tuples for data
  Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_TEMPERATURE);
  Tuple *conditions_tuple = dict_find(iterator, MESSAGE_KEY_CONDITIONS);

  // If all Weather data is available, use it
  if(temp_tuple && conditions_tuple) {
    snprintf(temperature_buffer, sizeof(temperature_buffer), "%dF", (int)temp_tuple->value->int32);
    snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_tuple->value->cstring);
    
    //Set time weather was last successfully update
    strftime(time_buffer, sizeof(time_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);
    
    //Update weather layer
    snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s %s @ %s", temperature_buffer, conditions_buffer, time_buffer);
    text_layer_set_text(s_weather_layer, weather_layer_buffer);
  }
  
  // Read ForeCast tuples for data
  Tuple *forecastHr3_temp_tuple = dict_find(iterator, MESSAGE_KEY_FORECAST_HR3_TEMPERATURE);
  Tuple *forecastHr3_conditions_tuple = dict_find(iterator, MESSAGE_KEY_FORECAST_HR3_CONDITIONS);

  // If all forecast data is available, use it
  if(forecastHr3_temp_tuple && forecastHr3_conditions_tuple) {
    snprintf(forecastHr3_temperature_buffer, sizeof(forecastHr3_temperature_buffer), "%dF", (int)forecastHr3_temp_tuple->value->int32);
    snprintf(forecastHr3_conditions_buffer, sizeof(forecastHr3_conditions_buffer), "%s", forecastHr3_conditions_tuple->value->cstring);
    
    //Update forecast hour 3 layer
    snprintf(forecastHr3_layer_buffer, sizeof(forecastHr3_layer_buffer), "%s, %s in 4 hrs", forecastHr3_temperature_buffer, forecastHr3_conditions_buffer);
    text_layer_set_text(s_forecastHr3_layer, forecastHr3_layer_buffer);
  }
  
  // Read ForeCast tuples for data
  Tuple *forecastDay2_temp_tuple = dict_find(iterator, MESSAGE_KEY_FORECAST_DAY2_TEMPERATURE);
  Tuple *forecastDay2_conditions_tuple = dict_find(iterator, MESSAGE_KEY_FORECAST_DAY2_CONDITIONS);

  // If all forecast data is available, use it
  if(forecastDay2_temp_tuple && forecastDay2_conditions_tuple) {
    snprintf(forecastDay2_temperature_buffer, sizeof(forecastDay2_temperature_buffer), "%dF", (int)forecastDay2_temp_tuple->value->int32);
    snprintf(forecastDay2_conditions_buffer, sizeof(forecastDay2_conditions_buffer), "%s", forecastDay2_conditions_tuple->value->cstring);
    
    //Update forecast hour 3 layer
    snprintf(forecastDay2_layer_buffer, sizeof(forecastDay2_layer_buffer), "%s %s nxt morning", forecastDay2_conditions_buffer, forecastDay2_temperature_buffer);
    text_layer_set_text(s_forecastDay2_layer, forecastDay2_layer_buffer);
  }
  
  // Read ForeCast Day3 tuples for data
  Tuple *forecastDay3_temp_tuple = dict_find(iterator, MESSAGE_KEY_FORECAST_DAY3_TEMPERATURE);
  Tuple *forecastDay3_conditions_tuple = dict_find(iterator, MESSAGE_KEY_FORECAST_DAY3_CONDITIONS);
  
  // If all forecast data is available, use it
  if(forecastDay3_temp_tuple && forecastDay3_conditions_tuple) {
    snprintf(forecastDay3_temperature_buffer, sizeof(forecastDay3_temperature_buffer), "%dF", (int)forecastDay3_temp_tuple->value->int32);
    snprintf(forecastDay3_conditions_buffer, sizeof(forecastDay3_conditions_buffer), "%s", forecastDay3_conditions_tuple->value->cstring);
    
    //Update forecast hour 3 layer
    snprintf(forecastDay3_layer_buffer, sizeof(forecastDay3_layer_buffer), "%s %s nxt evening", forecastDay3_conditions_buffer, forecastDay3_temperature_buffer);
    text_layer_set_text(s_forecastDay3_layer, forecastDay3_layer_buffer);
  }

  // Read ForeCast City tuples for data
  Tuple *forecastCity_tuple = dict_find(iterator, MESSAGE_KEY_FORECAST_CITY);
  
  // If all forecast data is available, use it
  if(forecastCity_tuple) {
    snprintf(forecastCity_buffer, sizeof(forecastCity_buffer), "%s", forecastCity_tuple->value->cstring);
    
    //Update forecast hour 3 layer
    snprintf(forecastCity_layer_buffer, sizeof(forecastCity_layer_buffer), "Forecast for %s", forecastCity_buffer);
    text_layer_set_text(s_forecastCity_layer, forecastCity_layer_buffer);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

