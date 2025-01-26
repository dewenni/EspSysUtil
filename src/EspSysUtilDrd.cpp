
#include <EspSysUtil.h>

#define DRD_FLAG_SET 0xD0D01234
#define DRD_FLAG_CLEAR 0xD0D04321

#define DRD_FILENAME "/drd.dat"

static const char *TAG = "ESU-DRD"; // LOG TAG

EspSysUtil::DRD32::DRD32(int timeout) {
  // LittleFS
  if (!LittleFS.begin()) {
    ESP_LOGE(TAG, "LittleFS failed!");
  }

  this->timeout = timeout * 1000;
  doubleResetDetected = false;
  waitingForDoubleReset = false;
}

bool EspSysUtil::DRD32::detectDoubleReset() {
  doubleResetDetected = detectRecentlyResetFlag();

  if (doubleResetDetected) {
    ESP_LOGI(TAG, "doubleResetDetected");
    clearRecentlyResetFlag();
  } else {
    ESP_LOGD(TAG, "No doubleResetDetected");

    setRecentlyResetFlag();
    waitingForDoubleReset = true;
  }

  return doubleResetDetected;
}

bool EspSysUtil::DRD32::waitingForDRD() { return waitingForDoubleReset; }

void EspSysUtil::DRD32::loop() {
  if (waitingForDoubleReset && millis() > timeout) {
    ESP_LOGD(TAG, "Stop doubleResetDetecting");
    stop();
  }
}

void EspSysUtil::DRD32::stop() {
  clearRecentlyResetFlag();
  waitingForDoubleReset = false;
}

bool EspSysUtil::DRD32::detectRecentlyResetFlag() {
  if (LittleFS.exists(DRD_FILENAME)) {
    // if config file exists, load
    File file = LittleFS.open(DRD_FILENAME, "r");
    if (!file) {
      ESP_LOGE(TAG, "Loading DRD file failed");
    }

    file.readBytes((char *)&DRD_FLAG, sizeof(DRD_FLAG));
    doubleResetDetectorFlag = DRD_FLAG;
    ESP_LOGD(TAG, "LittleFS Flag read = 0x%lX\n", DRD_FLAG);
    file.close();
  }

  doubleResetDetected = (doubleResetDetectorFlag == DRD_FLAG_SET);
  return doubleResetDetected;
}

void EspSysUtil::DRD32::setRecentlyResetFlag() {
  doubleResetDetectorFlag = DRD_FLAG_SET;
  DRD_FLAG = DRD_FLAG_SET;
  File file = LittleFS.open(DRD_FILENAME, "w");
  ESP_LOGD(TAG, "Saving DRD file...");
  if (file) {
    file.write((uint8_t *)&DRD_FLAG, sizeof(DRD_FLAG));
    file.close();
    ESP_LOGD(TAG, "Saving DRD file OK");
  } else {
    ESP_LOGD(TAG, "Saving DRD file failed");
  }
}

void EspSysUtil::DRD32::clearRecentlyResetFlag() {
  doubleResetDetectorFlag = DRD_FLAG_CLEAR;
  DRD_FLAG = DRD_FLAG_CLEAR;

  // LittleFS / SPIFFS code
  File file = LittleFS.open(DRD_FILENAME, "w");
  ESP_LOGD(TAG, "Saving DRD file...");

  if (file) {
    file.write((uint8_t *)&DRD_FLAG, sizeof(DRD_FLAG));
    file.close();
    ESP_LOGD(TAG, "Saving DRD file OK");
  } else {
    ESP_LOGD(TAG, "Saving DRD file failed");
  }
}