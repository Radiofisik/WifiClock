#include "SettingsService.h"

SettingsService::SettingsService(AsyncWebServer* server, FS* fs, SecurityManager* securityManager) :
    _httpEndpoint(SettingsState::read,
                  SettingsState::update,
                  this,
                  server,
                  SETTINGS_PATH,
                  securityManager,
                  AuthenticationPredicates::IS_AUTHENTICATED),
    _fsPersistence(SettingsState::read, SettingsState::update, this, fs, APP_SETTINGS_FILE) {
}

void SettingsService::begin() {
  _fsPersistence.readFromFS();
}