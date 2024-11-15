#include "HomeAssistant.h"

// Wi-Fi e MQTT
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Configuração do Home Assistant
HADevice device("ar_condicionado");
HASensor tempSensor("temperatura");

void setupWiFi(const char* ssid, const char* password) {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Conectando ao WiFi...");
    }
    Serial.println("WiFi conectado!");
}

// void setupMQTT(const char* mqttServer, int mqttPort, const char* mqttUser, const char* mqttPassword) {
//     mqttClient.setServer(mqttServer, mqttPort);

//     while (!mqttClient.connected()) {
//         Serial.println("Conectando ao broker MQTT...");
//         if (mqttClient.connect("ArCondicionado", mqttUser, mqttPassword)) {
//             Serial.println("Conectado ao MQTT!");
//         } else {
//             Serial.print("Falha, rc=");
//             Serial.print(mqttClient.state());
//             Serial.println(". Tentando novamente em 5 segundos...");
//             delay(5000);
//         }
//     }
// }

// void reconnectMQTT() {
//     if (!mqttClient.connected()) {
//         setupMQTT("IP_DO_BROKER_MQTT", 1883, "USUARIO_MQTT", "SENHA_MQTT");
//     }
//     mqttClient.loop();
// }

// void setupHA() {
//     device.setName("Ar Condicionado");
//     device.setSoftwareVersion("1.0");

//     tempSensor.setName("Temperatura AC");
//     tempSensor.setDeviceClass("temperature");
//     tempSensor.setUnitOfMeasurement("°C");
//     tempSensor.setIcon("mdi:thermometer");

//     HADiscovery discovery(mqttClient, device);
//     discovery.begin();
// }

// void updateTemperature(float temperature) {
//     tempSensor.setValue(temperature);
//     if (tempSensor.AvailabilityOnline()) {
//         tempSensor.PrecisionP0;
//     }
// }
