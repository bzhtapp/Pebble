#include <pebble.h>

static GFont s_battery_font;
static int s_battery_level;
static TextLayer *s_battery_layer;

static void battery_callback(BatteryChargeState state) {
  // Record the new battery level
  
  s_battery_level = state.charge_percent;
  
  static char s_buffer[5];
  snprintf(s_buffer, sizeof(s_buffer), "%d%%", s_battery_level);
  
  // Display this time on the TextLayer
  text_layer_set_text(s_battery_layer, s_buffer);
  
}
