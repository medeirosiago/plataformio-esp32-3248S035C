#include "MqttHandler.h"

MqttHandler::MqttHandler(const char* ssid, const char* password, const IPAddress& mqttHost, 
                         uint16_t mqttPort, const char* mqttUsername, const char* mqttPassword, 
                         const char* subscribeTopic, uint8_t qos)
  : ssid(ssid), password(password), mqttHost(mqttHost), mqttPort(mqttPort), 
    mqttUsername(mqttUsername), mqttPassword(mqttPassword), 
    subscribeTopic(subscribeTopic), qos(qos), lastPayload("") {
  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, this, [](TimerHandle_t xTimer) {
    static_cast<MqttHandler*>(pvTimerGetTimerID(xTimer))->connectToMqtt();
  });

  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, this, [](TimerHandle_t xTimer) {
    static_cast<MqttHandler*>(pvTimerGetTimerID(xTimer))->connectToWifi();
  });

  payloadTimer = xTimerCreate("payloadTimer", pdMS_TO_TICKS(5000), pdTRUE, this, payloadTimerCallback);
}

void MqttHandler::begin() {
  WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
    Serial.println("WiFi Event Triggered");
  });

  mqttClient.onConnect([this](bool sessionPresent) { mqttConnect(sessionPresent, this); });
  mqttClient.onDisconnect([this](AsyncMqttClientDisconnectReason reason) { mqttDisconnect(reason, this); });
  mqttClient.onMessage([this](char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
    mqttMessage(topic, payload, properties, len, index, total, this);
  });

  mqttClient.setServer(mqttHost, mqttPort);
  mqttClient.setCredentials(mqttUsername, mqttPassword);

  connectToWifi();
  xTimerStart(payloadTimer, 0);
}


String MqttHandler::getLastPayload() {
  return lastPayload;
}

void MqttHandler::connectToWifi() {
  WiFi.begin(ssid, password);
}

void MqttHandler::connectToMqtt() {
  mqttClient.connect();
}

void MqttHandler::wifiEvent(WiFiEvent_t event, MqttHandler* handler) {
  handler->handleWifiEvent(event);
}

void MqttHandler::mqttConnect(bool sessionPresent, MqttHandler* handler) {
  handler->handleMqttConnect(sessionPresent);
}

void MqttHandler::mqttDisconnect(AsyncMqttClientDisconnectReason reason, MqttHandler* handler) {
  handler->handleMqttDisconnect(reason);
}

void MqttHandler::mqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total, MqttHandler* handler) {
  handler->handleMqttMessage(topic, payload, len);
}

void MqttHandler::payloadTimerCallback(TimerHandle_t xTimer) {
  auto* handler = static_cast<MqttHandler*>(pvTimerGetTimerID(xTimer));
  handler->handlePayloadTimer();
}

void MqttHandler::handleWifiEvent(WiFiEvent_t event) {
  if (event == SYSTEM_EVENT_STA_GOT_IP) {
    connectToMqtt();
  } else if (event == SYSTEM_EVENT_STA_DISCONNECTED) {
    xTimerStop(mqttReconnectTimer, 0);
    xTimerStart(wifiReconnectTimer, 0);
  }
}

void MqttHandler::handleMqttConnect(bool sessionPresent) {
  mqttClient.subscribe(subscribeTopic, qos);
}

void MqttHandler::handleMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  xTimerStart(mqttReconnectTimer, 0);
}

void MqttHandler::handleMqttMessage(char* topic, char* payload, size_t len) {
  lastPayload = String(payload).substring(0, len);
}

void MqttHandler::handlePayloadTimer() {
  // Placeholder for additional actions
}
