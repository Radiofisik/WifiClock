#ifndef SettingsService
#define SettingsService

#include <Logger.h>
#include <HttpEndpoint.h>
#include <FSPersistence.h>
#include <time.h>
#include "MAX7219.h"
#include "RF24.h"

#define SETTINGS_PATH "/rest/settings"
#define APP_SETTINGS_FILE "/config/appSettings.json"

typedef struct 
{
	float Temp;
	float Humidity;
} TransmitInterface;

class SettingsState {
 public:
  String welcomeMessage;
  int dayBrightness;
  int nightBrightness;
  int startNightHour;
  int endNightHour;

  static void read(SettingsState& settings, JsonObject& root) {
    root["welcomeMessage"] = settings.welcomeMessage;
    root["dayBrightness"] = settings.dayBrightness;
    root["nightBrightness"] = settings.nightBrightness;
    root["startNightHour"] = settings.startNightHour;
    root["endNightHour"] = settings.endNightHour;
  }

  static StateUpdateResult update(JsonObject& root, SettingsState& state) {
    state.welcomeMessage = root["welcomeMessage"].as<String>();
    state.dayBrightness = root["dayBrightness"];
    state.nightBrightness = root["nightBrightness"];
    state.startNightHour = root["startNightHour"];
    state.endNightHour = root["endNightHour"];
    return StateUpdateResult::CHANGED;
  }
};

class SettingsStateService : public StatefulService<SettingsState> {
 public:
  SettingsStateService(AsyncWebServer* server, FS* fs, SecurityManager* securityManager);
  void begin();
  void onConfigUpdated();

 private:
  char str[12];
  uint32_t t;
  time_t now;
  tm* tmtime;
  static SPIClass vspi;
  MAX7219<GPIO_NUM_5, 4, vspi> display;

  TransmitInterface payload;
  RF24 radio;

  HttpEndpoint<SettingsState> _httpEndpoint;
  FSPersistence<SettingsState> _fsPersistence;

  static void displayTaskImpl(void* _this);
  void displayTask();
  void updateBrightness();
};

#endif