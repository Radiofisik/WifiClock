#include "SettingsStateService.h"

SPIClass SettingsStateService::vspi = SPIClass(VSPI);

SettingsStateService::SettingsStateService(AsyncWebServer* server, FS* fs, SecurityManager* securityManager) :
    _httpEndpoint(SettingsState::read,
                  SettingsState::update,
                  this,
                  server,
                  SETTINGS_PATH,
                  securityManager,
                  AuthenticationPredicates::IS_AUTHENTICATED),
    _fsPersistence(SettingsState::read, SettingsState::update, this, fs, APP_SETTINGS_FILE) {
  addUpdateHandler([&](const String& originId) { onConfigUpdated(); }, false);
}

void SettingsStateService::begin() {
  _fsPersistence.readFromFS();

  radio = RF24(GPIO_NUM_2, GPIO_NUM_4);  // using pin D2 for the CE pin, and pin D4 for the CSN pin)

  // initialize the transceiver on the SPI bus
  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1) {
    }  // hold in infinite loop
  }

  uint64_t address = 0x11223344AA;

  radio.setAutoAck(true);
  radio.setChannel(52);
  radio.setDataRate(RF24_2MBPS);
  radio.setPayloadSize(20);
  radio.enableDynamicPayloads();
  radio.enableAckPayload();

  // radio.openWritingPipe(address);
  // set the RX address of the TX node into a RX pipe
  radio.openReadingPipe(0, address);  // using pipe 0
  radio.startListening();             // put radio in RX mode

  display.init();
  display.begin(_state.dayBrightness);
  display.scroll(_state.welcomeMessage.c_str(), 50);
  vTaskDelay(3000 / portTICK_PERIOD_MS);
  display.noScroll();
  display.clear();

  xTaskCreate(displayTaskImpl,  // Function that should be called
              "display",        // Name of the task (for debugging)
              5000,             // Stack size (bytes)
              this,             // Parameter to pass
              10,               // Task priority
              NULL              // Task handle
  );
}

void SettingsStateService::onConfigUpdated() {
  updateBrightness();
}

void SettingsStateService::displayTaskImpl(void* _this) {
  ((SettingsStateService*)_this)->displayTask();
}

void SettingsStateService::updateBrightness() {
  if (tmtime->tm_hour >= _state.endNightHour && tmtime->tm_hour < _state.startNightHour)
    display.setBrightness(_state.dayBrightness);
  else
    display.setBrightness(_state.nightBrightness);
}

void SettingsStateService::displayTask() {
  for (;;) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    now = time(nullptr);
    tmtime = localtime(&now);

    if ((tmtime->tm_sec <= 1)) {  // Beginning of hour
      updateBrightness();
    }

    sprintf_P(str, PSTR("%2u:%02u"), tmtime->tm_hour, tmtime->tm_min);
    t = (display.width() - display.strWidth(str)) / 2;
    display.beginUpdate();
    display.clear();
    display.printStr(t, 0, str);
    if (tmtime->tm_sec & 0x01) {
      str[2] = '\0';
      display.drawPattern(
          t + display.strWidth(str) + display.FONT_GAP, 0, display.charWidth(':'), display.FONT_HEIGHT, (uint8_t)0);
    }
    display.endUpdate();

    uint8_t pipe;
    if (radio.available(&pipe)) {                       // is there a payload? get the pipe number that recieved it
      uint8_t bytes = radio.getPayloadSize();           // get the size of the payload
      radio.read(&payload, sizeof(TransmitInterface));  // fetch payload from FIFO
      radio.flush_rx();
      vTaskDelay(100 / portTICK_PERIOD_MS);

      // logger.print(F("Received "));
      // logger.print(bytes);  // print the size of the payload
      // logger.print(F(" bytes on pipe "));
      // logger.print(pipe);  // print the pipe number
      // logger.print(F(": "));
      // logger.println(payload.Temp);  // print the payload's value

      if (payload.Humidity > 0) {
        // sprintf_P(str, PSTR("%0.1f\xB0 %0.1f%%"), payload.Temp, payload.Humidity);
        sprintf_P(str, PSTR("%0.1f\xB0"), payload.Temp);
        logger.println(str);
        // sprintf_P(str, PSTR("%2.1f"), payload.Temp);

        display.clear();
        display.scroll(str, 50);
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        display.noScroll();
        // display.clear();
      }
      else logger.println("err");
    }

    // payload.Temp = 1;
    // payload.Humidity = 1;
    // radio.write(&payload, sizeof(TransmitInterface), true);
  }

  vTaskDelete(NULL);
}