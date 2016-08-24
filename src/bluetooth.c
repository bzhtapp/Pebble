#include <pebble.h>

static BitmapLayer *s_bt_icon_layer;
static GBitmap *s_bt_icon_bitmap;

static void bluetooth_callback(bool connected) {
  // Show icon if disconnected
  layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_layer), connected);

  if(!connected) {
    // Issue a vibrating alert
    vibes_double_pulse();
  }
}
