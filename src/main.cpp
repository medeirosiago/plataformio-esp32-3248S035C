#include <WiFi.h>
extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/timers.h"
}
#include <AsyncMqttClient.h>

// Configurações de Wi-Fi
#define WIFI_SSID "Tatakae"
#define WIFI_PASSWORD "Gael060515"

// Configurações do MQTT
#define MQTT_HOST IPAddress(192, 168, 0, 123) // Altere para o IP do seu broker
#define MQTT_PORT 1883
#define MQTT_USERNAME "magoia" // Substitua pelo seu username
#define MQTT_PASSWORD "8191323@@Magoiamon"   // Substitua pela sua senha
#define MQTT_SUBSCRIBE_TOPIC "home/ar/status" // Tópico que você quer capturar
#define MQTT_QOS 1 // Nível de Qualidade de Serviço (QoS)

// Objetos globais
AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;
TimerHandle_t printPayloadTimer;

// Variável para armazenar a última mensagem recebida
String lastPayload = "";

// Funções de conexão
void connectToWifi() {
  Serial.println("[WiFi] Tentando conectar ao Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connectToMqtt() {
  Serial.println("[MQTT] Tentando conectar ao broker MQTT...");
  mqttClient.connect();
}

// Eventos de Wi-Fi
void WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] Event ID: %d\n", event);
  switch (event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("[WiFi] Conectado com sucesso!");
      Serial.print("[WiFi] Endereço IP: ");
      Serial.println(WiFi.localIP());
      connectToMqtt();
      break;

    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("[WiFi] Conexão perdida. Tentando reconectar...");
      xTimerStop(mqttReconnectTimer, 0);
      xTimerStart(wifiReconnectTimer, 0);
      break;

    default:
      Serial.println("[WiFi] Evento não tratado.");
      break;
  }
}

// Eventos de conexão MQTT
void onMqttConnect(bool sessionPresent) {
  Serial.println("[MQTT] Conectado ao broker MQTT.");
  Serial.printf("[MQTT] Sessão presente: %d\n", sessionPresent);

  uint16_t packetIdSub = mqttClient.subscribe(MQTT_SUBSCRIBE_TOPIC, MQTT_QOS); // Assinar o tópico
  Serial.printf("[MQTT] Inscrição enviada para o tópico '%s' (QoS %d), Packet ID: %d\n", MQTT_SUBSCRIBE_TOPIC, MQTT_QOS, packetIdSub);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.printf("[MQTT] Desconectado. Motivo: %d\n", (int)reason);

  if (WiFi.isConnected()) {
    Serial.println("[MQTT] Reagendando conexão ao MQTT...");
    xTimerStart(mqttReconnectTimer, 0);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.printf("[MQTT] Inscrição confirmada (Packet ID: %d, QoS: %d)\n", packetId, qos);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.println("[MQTT] Mensagem recebida:");
  Serial.printf("  Tópico: %s\n", topic);
  Serial.printf("  QoS: %d | Retain: %d | Comprimento: %d\n", properties.qos, properties.retain, len);

  // Salvar o payload na variável global
  lastPayload = "";
  for (size_t i = 0; i < len; i++) {
    lastPayload += payload[i];
  }

  Serial.printf("  Payload: %s\n", lastPayload.c_str());
}

void onMqttPublish(uint16_t packetId) {
  Serial.printf("[MQTT] Publicação confirmada (Packet ID: %d)\n", packetId);
}

// Função do timer para imprimir o payload a cada 5 segundos
void printPayloadCallback(TimerHandle_t xTimer) {
  if (lastPayload != "") {
    Serial.println("[Timer] Última mensagem recebida:");
    Serial.println(lastPayload);
  } else {
    Serial.println("[Timer] Nenhuma mensagem recebida ainda.");
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n[Setup] Inicializando...");

  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

  // Timer para imprimir o payload a cada 5 segundos
  printPayloadTimer = xTimerCreate("printPayloadTimer", pdMS_TO_TICKS(5000), pdTRUE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(printPayloadCallback));

  xTimerStart(printPayloadTimer, 0);

  WiFi.onEvent(WiFiEvent);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);

  // Configurar servidor MQTT
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCredentials(MQTT_USERNAME, MQTT_PASSWORD); // Adiciona autenticação

  connectToWifi();
}

void loop() {
}
