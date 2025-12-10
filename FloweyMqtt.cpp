#include "FloweyMqtt.h"

FloweyMqtt* FloweyMqtt::_instance = nullptr;

FloweyMqtt::FloweyMqtt(Client& client) {
  _client.setClient(client);
  
  _client.setBufferSize(1024); 

  _client.setServer(FLOWEY_MQTT_SERVER, FLOWEY_MQTT_PORT);
  _instance = this;
  _commandCallback = nullptr;
  _sensorRequestCallback = nullptr;
}

void FloweyMqtt::begin(const char* token) {
  String tokenStr = String(token);
  int separatorIndex = tokenStr.indexOf(':');

  if (separatorIndex != -1) {
    _uuid = tokenStr.substring(0, separatorIndex);
    _jwt = tokenStr.substring(separatorIndex + 1);
  } else {
    _uuid = tokenStr;
    _jwt = "";
  }

  _commandTopic = String("/flowey/") + _uuid + "/command";
  _sensorTopic  = String("/flowey/") + _uuid + "/sensors";
  _notifyTopic  = String("/flowey/") + _uuid + "/notify";
  _statusTopic  = String("/flowey/") + _uuid + "/status";

  _client.setCallback(FloweyMqtt::callback);
}

void FloweyMqtt::callback(char* topic, byte* payload, unsigned int length) {
  if (_instance) {
    _instance->handleMessage(topic, payload, length);
  }
}

// *** หัวใจสำคัญอยู่ที่นี่ ***
void FloweyMqtt::handleMessage(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  if (String(topic) == _commandTopic) {
    
    if (message == "PING") {
      _client.publish(_statusTopic.c_str(), "PONG");
    }

    else if (message == "GET_SENSORS") {
      if (_sensorRequestCallback) {
        _sensorRequestCallback(); 
      }
    }
    else {
      if (_commandCallback) {
        _commandCallback(message);
      }
    }
  }
}

void FloweyMqtt::setCommandCallback(FloweyCommandCallback callback) {
  _commandCallback = callback;
}

void FloweyMqtt::setSensorRequestCallback(FloweySensorRequestCallback callback) {
  _sensorRequestCallback = callback;
}

void FloweyMqtt::loop() {
  if (!_client.connected()) {
    reconnect();
  }
  _client.loop();
}

void FloweyMqtt::reconnect() {
  while (!_client.connected()) {
    String clientId = "flowey-" + _uuid;
    if (_client.connect(clientId.c_str(), _uuid.c_str(), _jwt.c_str())) {
      _client.subscribe(_commandTopic.c_str());
      
      _client.publish(_statusTopic.c_str(), "ONLINE");
    } else {
      delay(5000);
    }
  }
}

bool FloweyMqtt::publishSensors(float soil, float temp, float humid, float light) {
  if (!_client.connected()) return false;

  JsonDocument doc;
  doc["soil"] = soil;
  doc["temp"] = temp;
  doc["humid"] = humid;
  doc["light"] = light;

  String jsonOutput;
  serializeJson(doc, jsonOutput);

  return _client.publish(_sensorTopic.c_str(), jsonOutput.c_str());
}

bool FloweyMqtt::sendNotification(String message) {
  if (!_client.connected()) return false;
  return _client.publish(_notifyTopic.c_str(), message.c_str());
}

bool FloweyMqtt::isConnected() {
  return _client.connected();
}