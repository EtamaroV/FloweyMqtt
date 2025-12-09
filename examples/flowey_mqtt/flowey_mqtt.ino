#include <WiFi.h>
#include "FloweyMqtt.h"

const char* ssid = "WIFI_SSID";
const char* password = "WIFI_PASS";
const char* DEVICE_TOKEN = "TOKEN"; 

WiFiClient espClient;
FloweyMqtt flowey(espClient);

void onSensorRequest() {
  Serial.println("Server requested sensor data!");
  
  // อ่านค่าจาก Sensor
  float s = analogRead(34); // %
  float t = 25.0; // celcius
  float h = 60.0; // %
  float l = 1000; // lux

  // ส่งกลับ
  flowey.publishSensors(s, t, h, l);
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  flowey.begin(DEVICE_TOKEN);
  
  // Register Callbacks
  flowey.setSensorRequestCallback(onSensorRequest); // รับคำขอค่า Sensor
  
  flowey.sendNotification("Flowey Device Started!");
}

void loop() {
  flowey.loop();
}