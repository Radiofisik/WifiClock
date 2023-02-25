#pragma once

#include <MqttPubSub.h>
#include <SettingsService.h>

#include <time.h>
#include "MAX7219.h"
#include "RF24.h"
#include "MHZ19.h"

#define RXD2 16
#define TXD2 17

typedef struct {
  float Temp;
  float Humidity;
} TransmitInterface;

class ClockState {
 public:
  float temperature;
  float humidity;
  float co2;
  int sensorTtl;

  static void haRead(ClockState& state, JsonObject& root) {
    root["temperature"] = state.temperature;
    root["humidity"] = state.humidity;
    root["co2"] = state.co2;
  }

  static StateUpdateResult haUpdate(JsonObject& root, ClockState& state) {
    return StateUpdateResult::UNCHANGED;
  }

  static StateUpdateResult updateTempHum(JsonObject& root, ClockState& state) {
    state.temperature = root["temperature"];
    state.humidity = root["humidity"];
    state.sensorTtl = 10 * 60;
    return StateUpdateResult::CHANGED;
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
  MqttSub<ClockState> _mqttTempHumSub;
  SettingsService* _settingsService;

  char str[12];
  uint32_t t;
  time_t now;
  tm* tmtime;
  static SPIClass vspi;
  MAX7219<GPIO_NUM_5, 4, vspi> display;
  int errorCount = 0;

  TransmitInterface payload;
  RF24 radio;
  MHZ19 myMHZ19;     

  static void displayTaskImpl(void* _this);
  void displayTask();
  void updateBrightness();
  void displayTemperature();
  void getCO2();
  void initRemoteSensor();
  void getRemoteSensorData();
  void displayCO2();
  void displayTime();
};