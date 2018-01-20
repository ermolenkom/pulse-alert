#include <pebble_worker.h>
#include "worker.h"
#include "http.h"

ClaySettings settings;

// Initialize the default settings
static void prv_default_settings() {
  settings.Threshold = 130;
  settings.OverrideFreq = false;
  settings.BackgroundWorker = false;
  settings.SportsMode = false;
  settings.SportsModeFired = false;
  settings.Frequency = 300;
  settings.SnoozeUntil = 0;
  settings.Backoff = 1;
  settings.VibeTypeAbove = 4;
  settings.VibeTypeBelow = 3;
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

void snooze(time_t until) {
  prv_load_settings();
  settings.SnoozeUntil = until;
  prv_save_settings();
}

static void prv_on_health_data(HealthEventType type, void *context) {
  // If the update was from the Heart Rate Monitor, query it
  if (type == HealthEventHeartRateUpdate) {
    HealthValue value = health_service_peek_current_value(HealthMetricHeartRateBPM);
    // Check the heart rate
    APP_LOG(APP_LOG_LEVEL_DEBUG, "current heart rate: %lu", (uint32_t) value);
    prv_load_settings();

	http_get("http://alyssa.is");
	
    if ((value >= settings.Threshold) && (time(NULL) - settings.SnoozeUntil >= 0)) {
      snooze(time(NULL) + settings.Backoff * 6);
      worker_launch_app();
    }
  }
}

static void prv_init() {
  prv_load_settings();
  // Initialize the worker here
  // Subscribe to health event handler
  health_service_events_subscribe(prv_on_health_data, NULL);
}

static void prv_deinit() {
  // Deinitialize the worker here
  health_service_events_unsubscribe();
}

int main(void) {
  prv_init();
  worker_event_loop();
  prv_deinit();
}