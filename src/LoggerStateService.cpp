#include "LoggerStateService.h"

LoggerStateService::LoggerStateService(AsyncWebServer* server, SecurityManager* securityManager) :
    _httpEndpoint(LoggerState::read,
                  LoggerState::update,
                  this,
                  server,
                  LOGGER_PATH,
                  securityManager,
                  AuthenticationPredicates::IS_AUTHENTICATED)
{
  addUpdateHandler([&](const String& originId) { onConfigUpdated(); }, false);
}

void LoggerState::read(LoggerState& state, JsonObject& root) {
  Logger& l = *state.logger;
  root["message"] =  l.getBuffer();
}

StateUpdateResult LoggerState::update(JsonObject& root, LoggerState& state) {
  state.logger->clear();
  return StateUpdateResult::CHANGED;
}

void LoggerStateService::begin() {
  logger.println("\xEF\xF0\xE8\xE2\xE5\xF2");
  _state.logger = &logger;
}

void LoggerStateService::onConfigUpdated() {
}
