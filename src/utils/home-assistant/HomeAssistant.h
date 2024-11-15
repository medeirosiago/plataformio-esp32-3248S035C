#ifndef HOME_ASSISTANT_H
#define HOME_ASSISTANT_H

#include <ArduinoHA.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Configuração Wi-Fi e MQTT
void setupWiFi(const char* ssid, const char* password);
void setupMQTT(const char* mqttServer, int mqttPort, const char* mqttUser, const char* mqttPassword);
void reconnectMQTT();

// Inicialização do Home Assistant
void setupHA();
void updateTemperature(float temperature);

#endif // HOME_ASSISTANT_H
