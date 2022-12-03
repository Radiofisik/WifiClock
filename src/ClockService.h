#pragma once

#include <MqttPubSub.h>
#include <SettingsService.h>

#include <time.h>
#include "MAX7219.h"
#include "RF24.h"

typedef struct {
  float Temp;
  float Humidity;
} TransmitInterface;

class ClockState {
 public:
  float temperature;
  float humidity;
  float co2;

  static void haRead(ClockState& state, JsonObject& root) {
    root["temperature"] = state.temperature;
    root["humidity"] = state.temperature;
    root["co2"] = state.co2;
  }

  static StateUpdateResult haUpdate(JsonObject& root, ClockState& lightState) {
    return StateUpdateResult::UNCHANGED;
  }

 private:
};

class ClockService : public StatefulService<ClockState> {
 public:
  ClockService(AsyncMqttClient* mqttClient, SettingsService* settingsService);
  void begin();
  void registerConfig();
  void onConfigUpdated();

 private:
  AsyncMqttClient* _mqttClient;
  MqttPubSub<ClockState> _mqttPubSub;
  SettingsService* _settingsService;

  char str[12];
  uint32_t t;
  time_t now;
  tm* tmtime;
  static SPIClass vspi;
  MAX7219<GPIO_NUM_5, 4, vspi> display;

  TransmitInterface payload;
  RF24 radio;

  static void displayTaskImpl(void* _this);
  void displayTask();
  void updateBrightness();
};