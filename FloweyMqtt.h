#ifndef FloweyMqtt_h
#define FloweyMqtt_h

#include "Arduino.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Client.h>

#define FLOWEY_MQTT_SERVER "rasp.etamarov.me"
#define FLOWEY_MQTT_PORT 1883

typedef void (*FloweyCommandCallback)(String command);
typedef void (*FloweySensorRequestCallback)();

class FloweyMqtt {
  public:
    FloweyMqtt(Client& client);

    void begin(const char* token);
    void loop();

    bool publishSensors(float soil, float temp, float humid, float light);
    
    bool sendNotification(String message);

    void setCommandCallback(FloweyCommandCallback callback);
    void setSensorRequestCallback(FloweySensorRequestCallback callback); // ใหม่

    bool isConnected();

  private:
    PubSubClient _client;
    String _uuid;
    String _jwt;
    
    FloweyCommandCallback _commandCallback;
    FloweySensorRequestCallback _sensorRequestCallback;

    String _commandTopic;
    String _sensorTopic;
    String _notifyTopic;
    String _statusTopic;

    void reconnect();
    static void callback(char* topic, byte* payload, unsigned int length);
    static FloweyMqtt* _instance;
    void handleMessage(char* topic, byte* payload, unsigned int length);
};

#endif