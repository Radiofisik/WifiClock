#include <ClockService.h>

// #define REMOTESENSOR

SPIClass ClockService::vspi = SPIClass(VSPI);

ClockService::ClockService(AsyncMqttClient* mqttClient, SettingsService* settingsService) :
    _mqttClient(mqttClient),
    _mqttPubSub(ClockState::haRead, ClockState::haUpdate, this, mqttClient),
    _mqttTempHumSub(ClockState::updateTempHum, this, mqttClient),
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

  _mqttTempHumSub.setSubTopic("zigbee2mqtt/outsideTempHum");

  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  myMHZ19.begin(Serial2);
  // myMHZ19.autoCalibration(false);

#ifdef REMOTESENSOR
  initRemoteSensor();
#endif

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
  registerConfig();
}

void ClockService::displayTaskImpl(void* _this) {
  ((ClockService*)_this)->displayTask();
}

void ClockService::displayTask() {
  for (;;) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    now = time(nullptr);
    tmtime = localtime(&now);

#ifdef REMOTESENSOR
    if(errorCount>10){
      initRemoteSensor();
    }
#endif

    if (_state.sensorTtl > 0)
      _state.sensorTtl--;
    else {
      _state.temperature = 0;
      _state.humidity = 0;
    }

    if (tmtime->tm_sec % 25 == 0) {
      updateBrightness();
    }

    if (tmtime->tm_sec % 10 == 0) {
      getCO2();
    }

    if ((tmtime->tm_sec + 5) % 15 == 0) {
      displayCO2();
    }

    if ((tmtime->tm_sec + 10) % 60 == 0) {
      callUpdateHandlers("rf");  // send mqtt update
    }

    if ((tmtime->tm_sec + 20) % 15 == 0) {
      if (_state.sensorTtl > 0)
        displayTemperature();
    }

    displayTime();
#ifdef REMOTESENSOR
    getRemoteSensorData();
#endif
  }

  vTaskDelete(NULL);
}

void ClockService::updateBrightness() {
  _settingsService->read([&](SettingsState& settings) {
    if (tmtime->tm_hour >= settings.endNightHour && tmtime->tm_hour < settings.startNightHour)
      display.setBrightness(settings.dayBrightness);
    else
      display.setBrightness(settings.nightBrightness);
  });
}

void ClockService::initRemoteSensor() {
  radio = RF24(GPIO_NUM_2, GPIO_NUM_4);  // using pin D2 for the CE pin, and pin D4 for the CSN pin)

  // initialize the transceiver on the SPI bus
  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1) {
    }  // hold in infinite loop
  }

  uint64_t address = 0x11223344AA;

  radio.setAutoAck(false);
  radio.setChannel(52);
  radio.setDataRate(RF24_2MBPS);
  radio.setPayloadSize(20);
  radio.enableDynamicPayloads();
  radio.disableAckPayload();

  // radio.openWritingPipe(address);
  // set the RX address of the TX node into a RX pipe
  radio.openReadingPipe(0, address);  // using pipe 0
  radio.startListening();             // put radio in RX mode
}

void ClockService::displayTemperature() {
  // sprintf_P(str, PSTR("%0.1f\xB0 %0.1f%%"), payload.Temp, payload.Humidity);
  sprintf_P(str, PSTR("%0.1f\xB0"), _state.temperature);
  display.clear();
  display.scroll(str, 50);
  vTaskDelay(3000 / portTICK_PERIOD_MS);
  display.noScroll();
  // display.clear();
}

void ClockService::getCO2() {
  _state.co2 = myMHZ19.getCO2();
  sprintf_P(str, PSTR("%.0f ppm"), _state.co2);
  logger.println(str);
  if (_state.co2 < 400) {
    sprintf_P(str, PSTR("err %d"), myMHZ19.errorCode);
    logger.println(str);
  }
}

void ClockService::displayCO2() {
  // sprintf_P(str, PSTR("%0.1f\xB0 %0.1f%%"), payload.Temp, payload.Humidity);
  sprintf_P(str, PSTR("%.0fp"), _state.co2);
  display.clear();
  display.scroll(str, 50);
  vTaskDelay(3000 / portTICK_PERIOD_MS);
  display.noScroll();
  // display.clear();
}

void ClockService::displayTime() {
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
}

void ClockService::getRemoteSensorData() {
  vTaskDelay(10 / portTICK_PERIOD_MS);
  uint8_t pipe;
  if (radio.available(&pipe)) {                       // is there a payload? get the pipe number that recieved it
    uint8_t bytes = radio.getPayloadSize();           // get the size of the payload
    radio.read(&payload, sizeof(TransmitInterface));  // fetch payload from FIFO
    radio.flush_rx();

    if (payload.Humidity > 0) {
      _state.humidity = payload.Humidity;
      _state.temperature = payload.Temp;
      sprintf_P(str, PSTR("%0.1f\xB0"), _state.temperature);
      logger.println(str);
      _state.sensorTtl = 5 * 60;
      errorCount = 0;
    } else
      logger.println("err");
      errorCount ++;

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }

  // payload.Temp = 1;
  // payload.Humidity = 1;
  // radio.write(&payload, sizeof(TransmitInterface), true);
}

void ClockService::registerConfig() {
  if (!_mqttClient->connected()) {
    return;
  }
  String mqttPath;
  String name;
  String uniqueId;

  String configTopic;
  String subTopic;
  String pubTopic;

  DynamicJsonDocument doc(256);

  mqttPath = "myclock";
  name = "clock";
  uniqueId = "rfclockuniqId";

  configTopic = mqttPath + "/config";
  subTopic = mqttPath + "/set";
  pubTopic = mqttPath + "/state";
  doc["~"] = mqttPath;
  doc["name"] = name;
  doc["unique_id"] = uniqueId;

  doc["cmd_t"] = "~/set";
  doc["stat_t"] = "~/state";
  doc["schema"] = "json";

  String payload;
  serializeJson(doc, payload);
  _mqttClient->publish(configTopic.c_str(), 0, false, payload.c_str());

  _mqttPubSub.configureTopics(pubTopic, subTopic);
}
