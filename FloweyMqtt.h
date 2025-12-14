#ifndef FloweyMqtt_h
#define FloweyMqtt_h

#include "Arduino.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Client.h>

#define FLOWEY_MQTT_SERVER "rasp.etamarov.me"
#define FLOWEY_MQTT_PORT 1883

// 1. สร้าง Struct เพื่อห่อหุ้มข้อมูล
struct FloweyPlantData {
  String plantNickname;
  String userNickname;
  String species;
  String birth;
};

typedef void (*FloweyCommandCallback)(String command);
typedef void (*FloweySensorRequestCallback)();
// 2. สร้าง type สำหรับ callback รับ data
typedef void (*FloweyDataCallback)(FloweyPlantData data);

class FloweyMqtt {
  public:
    FloweyMqtt(Client& client);

    void begin(const char* token);
    void loop();

    bool publishSensors(float soil, float temp, float humid, float light);
    bool sendNotification(String message);

    void setCommandCallback(FloweyCommandCallback callback);
    void setSensorRequestCallback(FloweySensorRequestCallback callback);
    
    // 3. ฟังก์ชัน setter สำหรับ Data Callback
    void setDataCallback(FloweyDataCallback callback);

    bool isConnected();

  private:
    PubSubClient _client;
    String _uuid;
    String _jwt;
    
    FloweyCommandCallback _commandCallback;
    FloweySensorRequestCallback _sensorRequestCallback;
    FloweyDataCallback _dataCallback; // เก็บ pointer ของ callback

    String _commandTopic;
    String _sensorTopic;
    String _notifyTopic;
    String _statusTopic;
    String _dataTopic; // เพิ่มตัวแปรนี้ (ของเดิมใน .h ขาดไป)

    void reconnect();
    static void callback(char* topic, byte* payload, unsigned int length);
    static FloweyMqtt* _instance;
    void handleMessage(char* topic, byte* payload, unsigned int length);
};

#endif