#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config.h"

class ScreenManager;

class MQTTHandler {
public:
    void init(PubSubClient* client, ScreenManager* screenMgr);
    void subscribeToTopics();
    void requestStatusUpdate();
    
    void setLightState(bool state);
    void setLightBrightness(int brightness);
    void setLightColorTemp(int colorTemp);
    
    void setHVACTemperature(float temperature);
    void setHVACMode(const char* mode);
    void setHVACState(bool state);
    
    static void messageCallback(char* topic, byte* payload, unsigned int length);
    
private:
    PubSubClient* mqttClient;
    ScreenManager* screenManager;
    
    void processMessage(const char* topic, const char* payload);
    void processLightUpdate(const char* payload);
    void processHVACUpdate(const char* payload);
    
    void sendLightCommand(JsonDocument& doc);
    void sendHVACCommand(JsonDocument& doc);
    
    static MQTTHandler* instance;
};

struct LightState {
    bool isOn = false;
    int brightness = 50;
    int colorTemp = 4000;
    bool available = false;
};

struct HVACState {
    bool isOn = false;
    float currentTemp = 20.0;
    float targetTemp = 22.0;
    String mode = "off";
    bool available = false;
};

extern LightState lightState;
extern HVACState hvacState;

#endif