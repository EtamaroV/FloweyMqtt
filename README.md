# FloweyMqtt Library

Library Arduino ที่ออกแบบมาเฉพาะสำหรับโปรเจกต์ **Flowey Plant Monitoring System** ช่วยจัดการการเชื่อมต่อ MQTT ให้เป็นเรื่องง่าย โดยจัดการเรื่องระบบความปลอดภัย (Authentication), การเชื่อมต่อซ้ำ (Reconnection), ระบบเช็คสถานะ (Heartbeat), และการแปลงข้อมูล Sensor เป็น JSON ให้อัตโนมัติ

## ฟีเจอร์หลัก (Features)

  * **Single Token Authentication:** ใช้งานง่ายเพียงแค่ใส่ Token ชุดเดียว Library จะทำการแยกและจัดการให้เอง
  * **Auto-Connection Management:** มีระบบเชื่อมต่อใหม่อัตโนมัติเมื่อเน็ตหลุด (Reconnection)
  * **Built-in Heartbeat:** มีระบบตอบกลับอัตโนมัติ เมื่อ Server ส่ง `PING` มา Device จะตอบ `PONG` กลับไปทันทีเพื่อยืนยันว่ายังออนไลน์อยู่
  * **Simplified Sensor Publishing:** แปลงค่า Sensor ดิบๆ (ดิน, อุณหภูมิ, ความชื้น, แสง) ให้เป็น JSON พร้อมส่งทันที
  * **Event-Driven Callbacks:** แยกฟังก์ชันตอบกลับชัดเจนระหว่าง "คำสั่งทั่วไป" กับ "คำขออ่านค่า Sensor" (`GET_SENSORS`)
  * **Zero-Configuration:** ค่า Server และ Port ถูกตั้งค่ามาใน Library แล้ว ผู้ใช้ไม่ต้องตั้งค่าเองในโค้ดหลัก

## สิ่งที่ต้องติดตั้งเพิ่ม (Dependencies)

ก่อนใช้งาน Library นี้ คุณต้องติดตั้ง Library ต่อไปนี้ผ่าน Arduino Library Manager:

1.  **PubSubClient** โดย Nick O'Leary
2.  **ArduinoJson** โดย Benoit Blanchon (รองรับทั้ง v6 และ v7)

## วิธีการใช้งาน (Usage)

### 1\. การตั้งค่าเบื้องต้น (Basic Setup)

```cpp
#include <WiFi.h>
#include "FloweyMqtt.h"

// 1. ประกาศตัวแปร WiFiClient (บุรุษไปรษณีย์)
WiFiClient espClient;

// 2. สร้าง Object FloweyMqtt โดยส่ง espClient เข้าไป
FloweyMqtt flowey(espClient);

// 3. ใส่ Token ของอุปกรณ์
const char* DEVICE_TOKEN = "TOKENNNNNNNNN"; 

void setup() {
  // เชื่อมต่อ WiFi ให้เรียบร้อยก่อน...
  
  // เริ่มต้นใช้งาน Flowey
  flowey.begin(DEVICE_TOKEN);
}

void loop() {
  // จำเป็นต้องเรียก loop() ตลอดเวลา เพื่อรักษาการเชื่อมต่อ
  flowey.loop();
}
```

### 2\. การรับคำสั่งและส่งค่า Sensor (Handling Commands & Sensor Requests)

```cpp
// Callback สำหรับคำสั่งทั่วไป (เช่น สั่งเปิดน้ำ)
void onCommand(String cmd) {
  if (cmd == "WATER_ON") {
    digitalWrite(PUMP_PIN, HIGH);
    // ส่งแจ้งเตือนกลับไป
    flowey.sendNotification("Pump Activated");
  }
}

// Callback เมื่อ Server ต้องการค่า Sensor เดี๋ยวนี้ (GET_SENSORS)
void onSensorRequest() {
  // อ่านค่าจาก Sensor จริง
  float soil = analogRead(34);
  float temp = 25.0;
  float humid = 60.0;
  float light = 1024;
  
  // ส่งข้อมูลกลับทันที (Library จะแปลงเป็น JSON ให้เอง)
  flowey.publishSensors(soil, temp, humid, light);
}

void setup() {
  // ... setup wifi ...
  flowey.begin(DEVICE_TOKEN);
  
  // ลงทะเบียนฟังก์ชัน Callback
  flowey.setCommandCallback(onCommand);       // สำหรับคำสั่งทั่วไป
  flowey.setSensorRequestCallback(onSensorRequest); // สำหรับขอค่า Sensor
}
```

## โครงสร้าง MQTT Topic (Protocol Reference)

Library จะสร้าง Topic อัตโนมัติโดยอิงจาก UUID ใน Token

| การกระทำ (Action) | Topic | Payload (ข้อมูล) | ทิศทาง |
| :--- | :--- | :--- | :--- |
| **รับคำสั่ง** | `flowey/{uuid}/command` | `WATER_ON`, `RESET`, ฯลฯ | Cloud -\> Device |
| **ขอค่า Sensor** | `flowey/{uuid}/command` | `GET_SENSORS` | Cloud -\> Device |
| **เช็คสถานะ (Ping)** | `flowey/{uuid}/command` | `PING` | Cloud -\> Device |
| **ตอบสถานะ (Pong)** | `flowey/{uuid}/status` | `PONG` (ตอบกลับอัตโนมัติ) | Device -\> Cloud |
| **ส่งค่า Sensor** | `flowey/{uuid}/sensors` | `{ "soil": 50, "temp": 25 ... }` | Device -\> Cloud |
| **ส่งแจ้งเตือน** | `flowey/{uuid}/notify` | `ข้อความแจ้งเตือน` | Device -\> Cloud |

## คำอธิบายฟังก์ชัน (API Documentation)

  * **`void begin(const char* token)`**
      * เริ่มต้นเชื่อมต่อ MQTT โดยใส่ Token ในรูปแบบ `"UUID:JWT"`
  * **`void loop()`**
      * ต้องเรียกใน `loop()` ของ Arduino เสมอ เพื่อจัดการการรับส่งข้อมูลและรักษาการเชื่อมต่อ
  * **`bool publishSensors(float soil, float temp, float humid, float light)`**
      * ส่งค่า Sensor โดยจะแปลงเป็น JSON ให้อัตโนมัติ คืนค่า `true` ถ้าส่งสำเร็จ
  * **`bool sendNotification(String message)`**
      * ส่งข้อความแจ้งเตือนไปยัง Topic notify
  * **`void setCommandCallback(void (*callback)(String))`**
      * กำหนดฟังก์ชันที่จะเรียกเมื่อได้รับคำสั่งข้อความทั่วไป
  * **`void setSensorRequestCallback(void (*callback)())`**
      * กำหนดฟังก์ชันที่จะเรียกเมื่อได้รับคำสั่ง `GET_SENSORS` โดยเฉพาะ