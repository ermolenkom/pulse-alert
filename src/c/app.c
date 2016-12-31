#include <pebble.h>
#include "app.h"

static Window *s_window;
static GBitmap *s_res_zzz;
static GBitmap *s_res_gear;
static GBitmap *s_res_pencil;
static GBitmap *s_res_plus;
static GBitmap *s_res_minus;
static GBitmap *s_res_accept;
static GFont s_res_leco_42_numbers;
static GFont s_res_gothic_14;
static GBitmap *s_res_heart;
static GFont s_res_gothic_28;
static ActionBarLayer *s_actionbarlayer_1;
static TextLayer *s_hr_live_label;
static TextLayer *s_hr_live;
static TextLayer *s_alert_label;
static BitmapLayer *s_heart_icon;
static TextLayer *s_alert_threshold;
static Layer *s_graph;
ClaySettings settings;

// Initialize the default settings
static void prv_default_settings() {
  settings.Threshold = 130;
  settings.OverrideFreq = false;
  settings.Frequency = 300;
}

// Read settings from persistent storage
static void prv_load_settings() {
  // Load the default settings
  prv_default_settings();
  // Read settings from persistent storage, if they exist
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

// Save the settings to persistent storage
static void prv_save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static void update_threshold_hr_layer() {
  static char s_threshold_buffer[8];
  snprintf(s_threshold_buffer, sizeof(s_threshold_buffer), "%d BPM", settings.Threshold);
  text_layer_set_text(s_alert_threshold, s_threshold_buffer);
  prv_save_settings();
}

static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
  // Threshold
  Tuple *threshold_t = dict_find(iter, MESSAGE_KEY_Threshold);
  if (threshold_t) {
    settings.Threshold = threshold_t->value->int32;
    update_threshold_hr_layer();
  }

  bool success = true;

  // Override Frequency
  Tuple *override_freq_t = dict_find(iter, MESSAGE_KEY_OverrideFreq);
  if (override_freq_t) {
    settings.OverrideFreq = override_freq_t->value->int32 == 1;
    if (!settings.OverrideFreq) {
      success = health_service_set_heart_rate_sample_period(0);
    }
    else {
      success = health_service_set_heart_rate_sample_period(settings.Frequency);
    }
  }

  // Frequency
  Tuple *frequency_t = dict_find(iter, MESSAGE_KEY_Frequency);
  if (frequency_t) {
    settings.Frequency = frequency_t->value->int32;
    if (settings.OverrideFreq) {
      success = health_service_set_heart_rate_sample_period(settings.Frequency);
    }
  }

  if (!success) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Could not set sampling period");
  }

  //don't forget to save!
  prv_save_settings();
}

static void prv_on_health_data(HealthEventType type, void *context) {
  // If the update was from the Heart Rate Monitor, query it
  if (type == HealthEventHeartRateUpdate) {
    HealthValue value = health_service_peek_current_value(HealthMetricHeartRateBPM);
    // Check the heart rate
    APP_LOG(APP_LOG_LEVEL_DEBUG, "current heart rate: %lu", (uint32_t) value);

    static char s_hrm_buffer[8];
    snprintf(s_hrm_buffer, sizeof(s_hrm_buffer), "%lu", (uint32_t) value);
    text_layer_set_text(s_hr_live, s_hrm_buffer);
  }
}

/*static void snooze_click_handler() {
}*/

static void edit_click_handler() {
  action_bar_layer_set_icon(s_actionbarlayer_1, BUTTON_ID_UP, s_res_plus);
  action_bar_layer_set_icon(s_actionbarlayer_1, BUTTON_ID_SELECT, s_res_accept);
  action_bar_layer_set_icon(s_actionbarlayer_1, BUTTON_ID_DOWN, s_res_minus);
  action_bar_layer_set_click_config_provider(s_actionbarlayer_1,edit_click_config_provider);
}

static void plus_click_handler() {
  if (settings.Threshold < 190) {
    settings.Threshold += 10;
  }
  update_threshold_hr_layer();
}

static void minus_click_handler() {
  if (settings.Threshold > 40) {
    settings.Threshold -= 10;
  }
  update_threshold_hr_layer();
}

static void save_click_handler() {
  action_bar_layer_set_icon(s_actionbarlayer_1, BUTTON_ID_UP, s_res_zzz);
  action_bar_layer_set_icon(s_actionbarlayer_1, BUTTON_ID_SELECT, s_res_gear);
  action_bar_layer_set_icon(s_actionbarlayer_1, BUTTON_ID_DOWN, s_res_pencil);
  action_bar_layer_set_click_config_provider(s_actionbarlayer_1,click_config_provider);
}

static void click_config_provider(void *context) {
  //window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) snooze_click_handler);
  //window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler) menu_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) edit_click_handler);
}

static void edit_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) plus_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler) save_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) minus_click_handler);
}

static void initialise_ui(void) {
  s_window = window_create();
  #ifndef PBL_SDK_3
    window_set_fullscreen(s_window, false);
  #endif
  
  s_res_zzz = gbitmap_create_with_resource(RESOURCE_ID_ZZZ);
  s_res_gear = gbitmap_create_with_resource(RESOURCE_ID_GEAR);
  s_res_pencil = gbitmap_create_with_resource(RESOURCE_ID_PENCIL);
  s_res_plus = gbitmap_create_with_resource(RESOURCE_ID_PLUS);
  s_res_minus = gbitmap_create_with_resource(RESOURCE_ID_MINUS);
  s_res_accept = gbitmap_create_with_resource(RESOURCE_ID_ACCEPT);
  s_res_leco_42_numbers = fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS);
  s_res_gothic_14 = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  s_res_heart = gbitmap_create_with_resource(RESOURCE_ID_HEART);
  s_res_gothic_28 = fonts_get_system_font(FONT_KEY_GOTHIC_28);
  // s_actionbarlayer_1
  s_actionbarlayer_1 = action_bar_layer_create();
  action_bar_layer_add_to_window(s_actionbarlayer_1, s_window);
  action_bar_layer_set_background_color(s_actionbarlayer_1, GColorBlack);
  action_bar_layer_set_icon(s_actionbarlayer_1, BUTTON_ID_UP, s_res_zzz);
  action_bar_layer_set_icon(s_actionbarlayer_1, BUTTON_ID_SELECT, s_res_gear);
  action_bar_layer_set_icon(s_actionbarlayer_1, BUTTON_ID_DOWN, s_res_pencil);
  action_bar_layer_set_click_config_provider(s_actionbarlayer_1,click_config_provider);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_actionbarlayer_1);
  
  // s_hr_live_label
  s_hr_live_label = text_layer_create(GRect(85, 87, 28, 20));
  text_layer_set_background_color(s_hr_live_label, GColorClear);
  text_layer_set_text(s_hr_live_label, "BPM");
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_hr_live_label);
  
  // s_hr_live
  s_hr_live = text_layer_create(GRect(0, 58, 85, 45));
  text_layer_set_background_color(s_hr_live, GColorClear);
  text_layer_set_text_alignment(s_hr_live, GTextAlignmentRight);
  text_layer_set_font(s_hr_live, s_res_leco_42_numbers);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_hr_live);
  
  // s_alert_label
  s_alert_label = text_layer_create(GRect(0, 115, 110, 18));
  text_layer_set_background_color(s_alert_label, GColorClear);
  text_layer_set_text(s_alert_label, "ALERT AT");
  text_layer_set_text_alignment(s_alert_label, GTextAlignmentCenter);
  text_layer_set_font(s_alert_label, s_res_gothic_14);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_alert_label);
  
  // s_heart_icon
  s_heart_icon = bitmap_layer_create(GRect(85, 65, 24, 25));
  bitmap_layer_set_bitmap(s_heart_icon, s_res_heart);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_heart_icon);
  
  // s_alert_threshold
  s_alert_threshold = text_layer_create(GRect(0, 125, 110, 28));
  text_layer_set_background_color(s_alert_threshold, GColorClear);
  static char s_threshold_buffer[8];
  snprintf(s_threshold_buffer, sizeof(s_threshold_buffer), "%d BPM", settings.Threshold);
  text_layer_set_text(s_alert_threshold, s_threshold_buffer);
  text_layer_set_text_alignment(s_alert_threshold, GTextAlignmentCenter);
  text_layer_set_font(s_alert_threshold, s_res_gothic_28);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_alert_threshold);
  
  // s_graph
  s_graph = layer_create(GRect(5, 5, 105, 55));
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_graph);
}

static void destroy_ui(void) {
  window_destroy(s_window);
  action_bar_layer_destroy(s_actionbarlayer_1);
  text_layer_destroy(s_hr_live_label);
  text_layer_destroy(s_hr_live);
  text_layer_destroy(s_alert_label);
  bitmap_layer_destroy(s_heart_icon);
  text_layer_destroy(s_alert_threshold);
  layer_destroy(s_graph);
  gbitmap_destroy(s_res_zzz);
  gbitmap_destroy(s_res_gear);
  gbitmap_destroy(s_res_pencil);
  gbitmap_destroy(s_res_heart);
}

static void handle_window_unload(Window* window) {
  destroy_ui();
}

void show_main_window(void) {
  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_stack_push(s_window, true);
}

void hide_main_window(void) {
  window_stack_remove(s_window, true);
}

static void init(void) {
  prv_load_settings();

  show_main_window();
  
  //custom vibe pattern to really catch the user's attention
  static const uint32_t segments[] = { 100, 100, 100, 100, 100, 100, 800 };
  VibePattern pat = {
    .durations = segments,
    .num_segments = ARRAY_LENGTH(segments),
  };

  // Launch the background worker
  AppWorkerResult result = app_worker_launch();
  if (result == APP_WORKER_RESULT_NO_WORKER) {
    text_layer_set_text(s_alert_label, "Could not launch a worker.");
  }
  else {
    HealthValue value = health_service_peek_current_value(HealthMetricHeartRateBPM);
    // Check the heart rate
    APP_LOG(APP_LOG_LEVEL_DEBUG, "current heart rate: %lu", (uint32_t) value);
    
    prv_load_settings();
    static char s_hrm_buffer[8];
    snprintf(s_hrm_buffer, sizeof(s_hrm_buffer), "%lu", (uint32_t) value);
    text_layer_set_text(s_hr_live, s_hrm_buffer);
    if (value > settings.Threshold) {
      vibes_enqueue_custom_pattern(pat);
    }

    // Subscribe to health event handler
    health_service_events_subscribe(prv_on_health_data, NULL);
  }

  // App Logging!
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Just pushed a window!");

  // Listen for AppMessages
  app_message_register_inbox_received(prv_inbox_received_handler);
  app_message_open(128, 128);
}

static void deinit(void) {
  hide_main_window();

  //unsubscribe from healt data
  health_service_events_unsubscribe();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
