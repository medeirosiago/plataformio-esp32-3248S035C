#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <ArduinoJson.h>
#include <AsyncMqttClient.h>
#include <WiFi.h>

// Estrutura para armazenar o status do clima
struct ClimateStatus {
    bool valid = false;
    String power;
    float setTemperature = 0.0;
    float currentTemperature = 0.0;
    String fanMode;
    bool tornado = false;
};

class MqttHandler {
public:
    MqttHandler(const char *ssid, const char *password, const IPAddress &mqttHost,
                uint16_t mqttPort, const char *mqttUsername, const char *mqttPassword,
                const char *subscribeTopic, uint8_t qos);

    void begin();
    ClimateStatus getClimateStatus() const;

private:
    void connectToWifi();
    void connectToMqtt();
    void handleWifiEvent(WiFiEvent_t event);
    void handleMqttConnect(bool sessionPresent);
    void handleMqttDisconnect(AsyncMqttClientDisconnectReason reason);
    void handleMqttMessage(char *topic, char *payload, size_t len);
    void handlePayloadTimer();
    static void payloadTimerCallback(TimerHandle_t xTimer);

    // Funções para extração de valores do payload
    String extractValue(const String &payload, const String &key);
    float extractFloatValue(const String &payload, const String &key);

    // Parâmetros de conexão WiFi e MQTT
    const char *ssid;
    const char *password;
    IPAddress mqttHost;
    uint16_t mqttPort;
    const char *mqttUsername;
    const char *mqttPassword;
    const char *subscribeTopic;
    uint8_t qos;

    // Status do clima
    ClimateStatus climateStatus;

    // Timers
    TimerHandle_t mqttReconnectTimer;
    TimerHandle_t wifiReconnectTimer;
    TimerHandle_t payloadTimer;
    
    // MQTT Client
    AsyncMqttClient mqttClient;
};

#endif // MQTT_HANDLER_H
