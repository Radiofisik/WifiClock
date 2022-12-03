#include <ClockService.h>

SPIClass ClockService::vspi = SPIClass(VSPI);

ClockService::ClockService(AsyncMqttClient* mqttClient, SettingsService* settingsService) :
    _mqttClient(mqttClient),
    _mqttPubSub(ClockState::haRead, ClockState::haUpdate, this, mqttClient),
    _settingsService(settingsService) {
  // configure MQTT callback
  _mqttClient->onConnect(std::bind(&ClockService::registerConfig, this));

  // configure update handler for when the light settings change
  _settingsService->addUpdateHandler([&](const String& originId) { onConfigUpdated(); }, false);
}

void ClockService::onConfigUpdated() {
  updateBrightness();
}

void ClockService::begin() {
  _state.humidity = 0;
  _state.co2 = 0;
  _state.temperature = 0;

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
  _settingsService->read([&](SettingsState& settings) {
    display.begin(settings.dayBrightness);
    display.scroll(settings.welcomeMessage.c_str(), 50);
  });
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

void ClockService::displayTaskImpl(void* _this) {
  ((ClockService*)_this)->displayTask();
}

void ClockService::updateBrightness() {
  _settingsService->read([&](SettingsState& settings) {
    if (tmtime->tm_hour >= settings.endNightHour && tmtime->tm_hour < settings.startNightHour)
      display.setBrightness(settings.dayBrightness);
    else
      display.setBrightness(settings.nightBrightness);
  });
}

void ClockService::displayTask() {
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
      } else
        logger.println("err");
    }

    // payload.Temp = 1;
    // payload.Humidity = 1;
    // radio.write(&payload, sizeof(TransmitInterface), true);
  }

  vTaskDelete(NULL);
}

void ClockService::registerConfig() {
  //   if (!_mqttClient->connected()) {
  //     return;
  //   }
  //   String configTopic;
  //   String subTopic;
  //   String pubTopic;

  //   DynamicJsonDocument doc(256);
  //   _lightMqttSettingsService->read([&](LightMqttSettings& settings) {
  //     configTopic = settings.mqttPath + "/config";
  //     subTopic = settings.mqttPath + "/set";
  //     pubTopic = settings.mqttPath + "/state";
  //     doc["~"] = settings.mqttPath;
  //     doc["name"] = settings.name;
  //     doc["unique_id"] = settings.uniqueId;
  //   });
  //   doc["cmd_t"] = "~/set";
  //   doc["stat_t"] = "~/state";
  //   doc["schema"] = "json";
  //   doc["brightness"] = false;

  //   String payload;
  //   serializeJson(doc, payload);
  //   _mqttClient->publish(configTopic.c_str(), 0, false, payload.c_str());

  //   _mqttPubSub.configureTopics(pubTopic, subTopic);
}
