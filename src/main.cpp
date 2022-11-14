#include <ESP8266React.h>
#include <LightMqttSettingsService.h>
#include <LightStateService.h>
#include "SettingsStateService.h"
#include "LoggerStateService.h"

#define SERIAL_BAUD_RATE 115200

Logger logger;
AsyncWebServer server(80);
ESP8266React esp8266React(&server);
LoggerStateService loggerStateService = LoggerStateService(&server, esp8266React.getSecurityManager());
LightMqttSettingsService lightMqttSettingsService =
    LightMqttSettingsService(&server, esp8266React.getFS(), esp8266React.getSecurityManager());
LightStateService lightStateService = LightStateService(&server,
                                                        esp8266React.getSecurityManager(),
                                                        esp8266React.getMqttClient(),
                                                        &lightMqttSettingsService);

SettingsStateService settingsStateService =
    SettingsStateService(&server, esp8266React.getFS(), esp8266React.getSecurityManager());

void setup() {
  // start serial and filesystem
  Serial.begin(SERIAL_BAUD_RATE);
  logger = Logger(&Serial);
  logger.begin();

  // start the framework and demo project
  esp8266React.begin();

  // load the initial light settings
  lightStateService.begin();

  // start the light service
  lightMqttSettingsService.begin();

  settingsStateService.begin();

  loggerStateService.begin();

  // start the server
  server.begin();
}

void loop() {
  // run the framework's loop function
  esp8266React.loop();
}
