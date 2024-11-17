#include "MqttHandler.h"
#include <Arduino.h> // Para funções de string

MqttHandler::MqttHandler(const char *ssid, const char *password, const IPAddress &mqttHost,
                         uint16_t mqttPort, const char *mqttUsername, const char *mqttPassword,
                         const char *subscribeTopic, uint8_t qos)
    : ssid(ssid), password(password), mqttHost(mqttHost), mqttPort(mqttPort),
      mqttUsername(mqttUsername), mqttPassword(mqttPassword),
      subscribeTopic(subscribeTopic), qos(qos)
{
    // Inicializando timers
    mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, this, MqttHandler::payloadTimerCallback);
    wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, this, MqttHandler::payloadTimerCallback);
    payloadTimer = xTimerCreate("payloadTimer", pdMS_TO_TICKS(60000), pdTRUE, this, MqttHandler::payloadTimerCallback);
}

void MqttHandler::begin()
{
    Serial.println("[MQTT] Iniciando o MQTT Handler");

    // Vinculando eventos WiFi
    WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info)
                 { handleWifiEvent(event); });

    // Vinculando os callbacks MQTT
    mqttClient.onConnect([this](bool sessionPresent)
                         { handleMqttConnect(sessionPresent); });
    mqttClient.onDisconnect([this](AsyncMqttClientDisconnectReason reason)
                            { handleMqttDisconnect(reason); });
    mqttClient.onMessage([this](char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
                         { handleMqttMessage(topic, payload, len); });

    // Configurando o broker e as credenciais MQTT
    mqttClient.setServer(mqttHost, mqttPort);
    mqttClient.setCredentials(mqttUsername, mqttPassword);

    Serial.println("[MQTT] Conectando ao WiFi...");
    connectToWifi(); // Inicia a conexão WiFi
    xTimerStart(payloadTimer, 0);
}

void MqttHandler::handleMqttMessage(char *topic, char *payload, size_t len)
{
    Serial.println("[MQTT] Mensagem recebida:");
    Serial.printf("Tópico: %s\n", topic);
    Serial.printf("Payload: %.*s\n", len, payload);

    String payloadString = String(payload).substring(0, len);

    // Verifica se o payload contém "climate.ar" para garantir que estamos parseando o conteúdo certo
    if (payloadString.indexOf("climate.ar") == -1)
    {
        Serial.println("[PARSE] Chave 'climate.ar' não encontrada no payload");
        climateStatus.valid = false;
        return;
    }

    // Inicializa todos os valores como inválidos antes de começar o parsing
    climateStatus.valid = true;

    // Parse manual dos valores
    if (payloadString.indexOf("\"power\"") != -1)
    {
        climateStatus.power = extractValue(payloadString, "power");
    }
    else
    {
        Serial.println("[PARSE] Chave 'power' ausente");
        climateStatus.valid = false;
    }

    if (payloadString.indexOf("\"set_temperature\"") != -1)
    {
        climateStatus.setTemperature = extractFloatValue(payloadString, "set_temperature");
    }
    else
    {
        Serial.println("[PARSE] Chave 'set_temperature' ausente");
        climateStatus.valid = false;
    }

    if (payloadString.indexOf("\"current_temperature\"") != -1)
    {
        climateStatus.currentTemperature = extractFloatValue(payloadString, "current_temperature");
    }
    else
    {
        Serial.println("[PARSE] Chave 'current_temperature' ausente");
        climateStatus.valid = false;
    }

    if (payloadString.indexOf("\"fan_mode\"") != -1)
    {
        climateStatus.fanMode = extractValue(payloadString, "fan_mode");
    }
    else
    {
        Serial.println("[PARSE] Chave 'fan_mode' ausente");
        climateStatus.valid = false;
    }

    if (payloadString.indexOf("\"tornado\"") != -1 || payloadString.indexOf("\"torando\"") != -1)
    {
        String key = payloadString.indexOf("\"tornado\"") != -1 ? "tornado" : "torando";
        String tornadoValue = extractValue(payloadString, key);
        climateStatus.tornado = (tornadoValue.equalsIgnoreCase("true") || tornadoValue == "1");
    }
    else
    {
        Serial.println("[PARSE] Chave 'tornado' ou 'torando' ausente");
        climateStatus.valid = false;
    }

    // Log do status atualizado
    if (climateStatus.valid)
    {
        Serial.printf("[PARSE] Status atualizado: Power: %s, Set Temp: %.1f, Current Temp: %.1f, Fan Mode: %s, Tornado: %s\n",
                      climateStatus.power.c_str(),
                      climateStatus.setTemperature,
                      climateStatus.currentTemperature,
                      climateStatus.fanMode.c_str(),
                      climateStatus.tornado ? "true" : "false");
    }
}

String MqttHandler::extractValue(const String &payload, const String &key)
{
    int keyIndex = payload.indexOf("\"" + key + "\"");
    if (keyIndex == -1)
    {
        return "";
    }
    int valueStart = payload.indexOf(":", keyIndex) + 1;
    int valueEnd = payload.indexOf(",", valueStart);
    if (valueEnd == -1)
    {
        valueEnd = payload.indexOf("}", valueStart);
    }
    String value = payload.substring(valueStart, valueEnd);
    value.trim();
    value.replace("\"", ""); // Remover aspas, se houver
    return value;
}

float MqttHandler::extractFloatValue(const String &payload, const String &key)
{
    String valueString = extractValue(payload, key);
    return valueString.toFloat();
}

ClimateStatus MqttHandler::getClimateStatus() const
{
    return climateStatus;
}

void MqttHandler::connectToWifi()
{
    Serial.printf("[WiFi] Tentando conectar ao SSID: %s\n", ssid);
    WiFi.begin(ssid, password);
}

void MqttHandler::connectToMqtt()
{
    Serial.println("[MQTT] Tentando conectar ao broker MQTT...");
    mqttClient.connect();
}

void MqttHandler::handleWifiEvent(WiFiEvent_t event)
{
    Serial.print("[WiFi] Evento WiFi disparado: ");
    Serial.println(event);

    if (event == ARDUINO_EVENT_WIFI_STA_GOT_IP)
    {
        Serial.println("[WiFi] Conectado ao WiFi. Obtendo endereço IP, agora conectando ao MQTT...");
        connectToMqtt();
    }
    else if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED)
    {
        Serial.println("[WiFi] Desconectado do WiFi. Tentando reconectar...");
        xTimerStop(mqttReconnectTimer, 0);
        xTimerStart(wifiReconnectTimer, 0);
    }
}

void MqttHandler::handleMqttConnect(bool sessionPresent)
{
    Serial.println("[MQTT] Conexão ao MQTT bem-sucedida. Inscrevendo-se no tópico...");
    mqttClient.subscribe(subscribeTopic, qos);
    Serial.printf("[MQTT] Inscrito no tópico: %s\n", subscribeTopic);
}

void MqttHandler::handleMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
    Serial.println("[MQTT] Desconexão do MQTT detectada, tentando reconectar...");
    xTimerStart(mqttReconnectTimer, 0);
}

void MqttHandler::payloadTimerCallback(TimerHandle_t xTimer)
{
    auto *handler = static_cast<MqttHandler *>(pvTimerGetTimerID(xTimer));
    handler->handlePayloadTimer();
}

void MqttHandler::handlePayloadTimer()
{
    Serial.println("[MQTT] Executando ação (timer de payload)");
    if (climateStatus.valid)
    {
        Serial.printf("[MQTT] Último estado do clima: Power: %s, Set Temp: %.1f, Current Temp: %.1f, Fan Mode: %s, Tornado: %s\n",
                      climateStatus.power.c_str(),
                      climateStatus.setTemperature,
                      climateStatus.currentTemperature,
                      climateStatus.fanMode.c_str(),
                      climateStatus.tornado ? "true" : "false");
    }
}
