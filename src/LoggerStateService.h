#ifndef LoggerService
#define LoggerService

#include <HttpEndpoint.h>
#include "Logger.h"

#define LOGGER_PATH "/rest/logger"

class LoggerState {
 public:
  Logger* logger;
  static void read(LoggerState& settings, JsonObject& root);
  static StateUpdateResult update(JsonObject& root, LoggerState& state);
};

class LoggerStateService : public StatefulService<LoggerState> {
 public:
  LoggerStateService(AsyncWebServer* server, SecurityManager* securityManager);
  void begin();
  void onConfigUpdated();

 private:
  HttpEndpoint<LoggerState> _httpEndpoint;
};

#endif