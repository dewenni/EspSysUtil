#pragma once

#include <Arduino.h>
#include <LittleFS.h>
#include <cctype>
#include <cstring>
#include <esp_task_wdt.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdint.h>
#include <stdlib.h>

class EspSysUtil {

public:
  class RestartReason {
  public:
    static const char *get();
    static void saveLocal(const char *reason);
    static bool readLocal();
  };

  class Wdt {
  private:
    esp_task_wdt_config_t twdt_config;
    bool isInitialized;
    TaskHandle_t loopTaskHandle;

    Wdt(uint32_t timeout, uint8_t idle_core_mask, bool trigger_panic);

  public:
    static Wdt &getInstance(uint32_t timeout = 10000, uint8_t idle_core_mask = 0, bool trigger_panic = true);
    void enable();
    void disable();
    bool isActive() const;
  };

  class OTA {
  private:
    bool otaState; // OTA-State
    OTA() : otaState(false) {}

  public:
    static OTA &getInstance() {
      static OTA instance;
      return instance;
    }

    OTA(const OTA &) = delete;
    OTA &operator=(const OTA &) = delete;
    void setActive(bool active) { otaState = active; }
    bool isActive() const { return otaState; }
  };

  class DRD32 {
  public:
    DRD32(int timeout);

    bool detectDoubleReset();
    bool waitingForDRD();
    void loop();
    void stop();

    bool doubleResetDetected;

  private:
    const char *TAG = "DRD";
    uint32_t DRD_FLAG;
    unsigned long timeout;
    int address;
    bool waitingForDoubleReset;

    bool detectRecentlyResetFlag();
    void setRecentlyResetFlag();
    void clearRecentlyResetFlag();
    uint32_t doubleResetDetectorFlag;
  };
};
