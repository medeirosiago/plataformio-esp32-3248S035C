#include <Arduino.h>
#include "http-server.h"

#include <DNSServer.h>
#include <WiFiManager.h>

void conectarWifi()
{
  Serial.begin(9600);
  WiFiManager wifiManager;
  wifiManager.autoConnect("NodeMCU-Arduino-PlatformIO");
  Serial.println("Connected!");
}

// void loop()
// {
//   Serial.println("Idle...");
//   delay(1000);
// }