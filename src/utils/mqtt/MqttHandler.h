#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <AsyncMqttClient.h>
#include <WiFi.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

class MqttHandler {
public:
  MqttHandler(const char* ssid, const char* password, const IPAddress& mqttHost, 
              uint16_t mqttPort, const char* mqttUsername, const char* mqttPassword, 
              const char* subscribeTopic, uint8_t qos);
  void begin();
  String getLastPayload();

private:
  // Wi-Fi
  const char* ssid;
  const char* password;

  // MQTT
  IPAddress mqttHost;
  uint16_t mqttPort;
  const char* mqttUsername;
  const char* mqttPassword;
  const char* subscribeTopic;
  uint8_t qos;

  AsyncMqttClient mqttClient;
  TimerHandle_t mqttReconnectTimer;
  TimerHandle_t wifiReconnectTimer;
  TimerHandle_t payloadTimer;

  String lastPayload;

  void connectToWifi();
  void connectToMqtt();

  // Callbacks
  static void wifiEvent(WiFiEvent_t event, MqttHandler* handler);
  static void mqttConnect(bool sessionPresent, MqttHandler* handler);
  static void mqttDisconnect(AsyncMqttClientDisconnectReason reason, MqttHandler* handler);
  static void mqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total, MqttHandler* handler);
  static void payloadTimerCallback(TimerHandle_t xTimer);

  // Helper methods
  void handleWifiEvent(WiFiEvent_t event);
  void handleMqttConnect(bool sessionPresent);
  void handleMqttDisconnect(AsyncMqttClientDisconnectReason reason);
  void handleMqttMessage(char* topic, char* payload, size_t len);
  void handlePayloadTimer();
};

#endif // MQTT_HANDLER_H
