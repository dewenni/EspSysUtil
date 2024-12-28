#include <EspSysUtil.h>

#ifndef MY_LOGGER_SUPPORT
#define MY_LOGGER_SUPPORT
#define MY_LOGE(tag, format, ...) esp_log_write(ESP_LOG_ERROR, tag, "E (APP-%s): " format "\n", tag, ##__VA_ARGS__)
#define MY_LOGI(tag, format, ...) esp_log_write(ESP_LOG_INFO, tag, "I (APP-%s): " format "\n", tag, ##__VA_ARGS__)
#define MY_LOGW(tag, format, ...) esp_log_write(ESP_LOG_WARN, tag, "W (APP-%s): " format "\n", tag, ##__VA_ARGS__)
#define MY_LOGD(tag, format, ...) esp_log_write(ESP_LOG_DEBUG, tag, "D (APP-%s): " format "\n", tag, ##__VA_ARGS__)
#endif

static const char *TAG = "EspSysUtil-WDT"; // LOG TAG

// Watchdog functions
EspSysUtil::Wdt::Wdt(uint32_t timeout, uint8_t idle_core_mask, bool trigger_panic) : isInitialized(false), loopTaskHandle(nullptr) {
  twdt_config = {timeout_ms : timeout, idle_core_mask : idle_core_mask, trigger_panic : trigger_panic};
}

EspSysUtil::Wdt &EspSysUtil::Wdt::getInstance(uint32_t timeout, uint8_t idle_core_mask, bool trigger_panic) {
  static Wdt instance(timeout, idle_core_mask, trigger_panic);
  return instance;
}

void EspSysUtil::Wdt::enable() {
  MY_LOGI("WDT", "enable cmd");

  if (isInitialized) {
    MY_LOGW("WDT", "Watchdog timer is already initialized");
    return;
  }

  if (loopTaskHandle == nullptr) {
    loopTaskHandle = xTaskGetCurrentTaskHandle();
  }

  if (esp_task_wdt_reconfigure(&twdt_config) == ESP_OK) {
    esp_task_wdt_add(loopTaskHandle);
    MY_LOGI("WDT", "Watchdog timer initialized for Loop Task");
    isInitialized = true;
  } else {
    MY_LOGE("WDT", "Failed to initialize Watchdog timer");
  }
}

void EspSysUtil::Wdt::disable() {
  MY_LOGI("WDT", "disable cmd");

  if (!isInitialized) {
    MY_LOGW("WDT", "Watchdog timer is not initialized");
    return;
  }

  if (loopTaskHandle == nullptr) {
    loopTaskHandle = xTaskGetCurrentTaskHandle();
  }

  esp_task_wdt_delete(loopTaskHandle);
  MY_LOGI("WDT", "Watchdog timer de-initialized");
  isInitialized = false;
}

bool EspSysUtil::Wdt::isActive() const { return isInitialized; }