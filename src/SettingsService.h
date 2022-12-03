#pragma once

#include <Logger.h>
#include <HttpEndpoint.h>
#include <FSPersistence.h>


#define SETTINGS_PATH "/rest/settings"
#define APP_SETTINGS_FILE "/config/appSettings.json"

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

class SettingsService : public StatefulService<SettingsState> {
 public:
  SettingsService(AsyncWebServer* server, FS* fs, SecurityManager* securityManager);
  void begin();
  void onConfigUpdated();

 private:
  HttpEndpoint<SettingsState> _httpEndpoint;
  FSPersistence<SettingsState> _fsPersistence;
};
