#include <pebble.h>

#define SETTINGS_KEY 1

// A structure containing our settings
typedef struct ClaySettings {
  int16_t Threshold;
  bool OverrideFreq;
  uint16_t Frequency;
} __attribute__((__packed__)) ClaySettings;

static void prv_default_settings();
static void prv_load_settings();
static void prv_save_settings();
static void init();
static void deinit();
static void update_threshold_hr_layer();
static void edit_click_handler();
static void plus_click_handler();
static void minus_click_handler();
static void save_click_handler();
static void click_config_provider();
static void edit_click_config_provider();