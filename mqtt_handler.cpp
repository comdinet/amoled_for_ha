#include "mqtt_handler.h"
#include "screen_manager.h"
#include <Arduino.h>

LightState lightState;
HVACState hvacState;

MQTTHandler* MQTTHandler::instance = nullptr;

void MQTTHandler::init(PubSubClient* client, ScreenManager* screenMgr) {
    mqttClient = client;
    screenManager = screenMgr;
    instance = this;
    
    mqttClient->setCallback(messageCallback);
    
    Serial.println("MQTT Handler initialized");
}

void MQTTHandler::subscribeToTopics() {
    if (!mqttClient->connected()) {
        return;
    }
    
    if (mqttClient->subscribe(LIGHT_STATE_TOPIC)) {
        Serial.print("Subscribed to light topic: ");
        Serial.println(LIGHT_STATE_TOPIC);
    }
    
    if (mqttClient->subscribe(HVAC_STATE_TOPIC)) {
        Serial.print("Subscribed to HVAC topic: ");
        Serial.println(HVAC_STATE_TOPIC);
    }
    
    mqttClient->subscribe("homeassistant/status");
}

void MQTTHandler::requestStatusUpdate() {
    if (!mqttClient->connected()) {
        return;
    }
    
    StaticJsonDocument<50> lightRequest;
    lightRequest["state"] = "";
    sendLightCommand(lightRequest);
    
    StaticJsonDocument<50> hvacRequest;
    hvacRequest["state"] = "";
    sendHVACCommand(hvacRequest);
    
    Serial.println("Status update requested");
}

void MQTTHandler::setLightState(bool state) {
    StaticJsonDocument<100> doc;
    doc["state"] = state ? "ON" : "OFF";
    sendLightCommand(doc);
    
    Serial.print("Light state: ");
    Serial.println(state ? "ON" : "OFF");
}

void MQTTHandler::setLightBrightness(int brightness) {
    brightness = constrain(brightness, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
    
    StaticJsonDocument<100> doc;
    doc["state"] = "ON";
    doc["brightness"] = map(brightness, 0, 100, 0, 255);
    sendLightCommand(doc);
    
    Serial.print("Light brightness: ");
    Serial.println(brightness);
}

void MQTTHandler::setLightColorTemp(int colorTemp) {
    colorTemp = constrain(colorTemp, MIN_COLOR_TEMP, MAX_COLOR_TEMP);
    
    StaticJsonDocument<150> doc;
    doc["state"] = "ON";
    doc["color_temp"] = round(1000000.0 / colorTemp);
    sendLightCommand(doc);
    
    Serial.print("Light color temp: ");
    Serial.println(colorTemp);
}

void MQTTHandler::setHVACTemperature(float temperature) {
    temperature = constrain(temperature, MIN_TEMPERATURE, MAX_TEMPERATURE);
    
    StaticJsonDocument<100> doc;
    doc["temperature"] = temperature;
    sendHVACCommand(doc);
    
    Serial.print("HVAC temperature: ");
    Serial.println(temperature);
}

void MQTTHandler::setHVACMode(const char* mode) {
    StaticJsonDocument<100> doc;
    doc["hvac_mode"] = mode;
    sendHVACCommand(doc);
    
    Serial.print("HVAC mode: ");
    Serial.println(mode);
}

void MQTTHandler::setHVACState(bool state) {
    if (state) {
        setHVACMode("heat");
    } else {
        setHVACMode("off");
    }
    
    Serial.print("HVAC state: ");
    Serial.println(state ? "ON" : "OFF");
}

void MQTTHandler::sendLightCommand(JsonDocument& doc) {
    if (!mqttClient->connected()) {
        Serial.println("MQTT not connected");
        return;
    }
    
    String payload;
    serializeJson(doc, payload);
    
    if (mqttClient->publish(LIGHT_COMMAND_TOPIC, payload.c_str())) {
        Serial.print("Light command: ");
        Serial.println(payload);
    } else {
        Serial.println("Failed to publish light command");
    }
}

void MQTTHandler::sendHVACCommand(JsonDocument& doc) {
    if (!mqttClient->connected()) {
        Serial.println("MQTT not connected");
        return;
    }
    
    String payload;
    serializeJson(doc, payload);
    
    if (mqttClient->publish(HVAC_COMMAND_TOPIC, payload.c_str())) {
        Serial.print("HVAC command: ");
        Serial.println(payload);
    } else {
        Serial.println("Failed to publish HVAC command");
    }
}

void MQTTHandler::messageCallback(char* topic, byte* payload, unsigned int length) {
    if (instance) {
        char message[length + 1];
        memcpy(message, payload, length);
        message[length] = '\0';
        
        instance->processMessage(topic, message);
    }
}

void MQTTHandler::processMessage(const char* topic, const char* payload) {
    Serial.print("MQTT received - Topic: ");
    Serial.print(topic);
    Serial.print(", Payload: ");
    Serial.println(payload);
    
    if (strcmp(topic, LIGHT_STATE_TOPIC) == 0) {
        processLightUpdate(payload);
    } else if (strcmp(topic, HVAC_STATE_TOPIC) == 0) {
        processHVACUpdate(payload);
    } else if (strcmp(topic, "homeassistant/status") == 0) {
        if (strcmp(payload, "online") == 0) {
            Serial.println("HA online, requesting status");
            requestStatusUpdate();
        }
    }
}

void MQTTHandler::processLightUpdate(const char* payload) {
    StaticJsonDocument<500> doc;
    DeserializationError error = deserializeJson(doc, payload);
    
    if (error) {
        Serial.print("Light JSON parse error: ");
        Serial.println(error.c_str());
        return;
    }
    
    if (doc.containsKey("state")) {
        lightState.isOn = (doc["state"] == "ON");
        lightState.available = true;
    }
    
    if (doc.containsKey("brightness")) {
        lightState.brightness = map(doc["brightness"].as<int>(), 0, 255, 0, 100);
    }
    
    if (doc.containsKey("color_temp")) {
        int mireds = doc["color_temp"];
        lightState.colorTemp = constrain(round(1000000.0 / mireds), MIN_COLOR_TEMP, MAX_COLOR_TEMP);
    }
    
    if (screenManager) {
        screenManager->updateLightStatus();
    }
    
    Serial.println("Light state updated");
}

void MQTTHandler::processHVACUpdate(const char* payload) {
    StaticJsonDocument<500> doc;
    DeserializationError error = deserializeJson(doc, payload);
    
    if (error) {
        Serial.print("HVAC JSON parse error: ");
        Serial.println(error.c_str());
        return;
    }
    
    if (doc.containsKey("hvac_mode")) {
        hvacState.mode = doc["hvac_mode"].as<String>();
        hvacState.isOn = (hvacState.mode != "off");
        hvacState.available = true;
    }
    
    if (doc.containsKey("current_temperature")) {
        hvacState.currentTemp = doc["current_temperature"];
    }
    
    if (doc.containsKey("temperature")) {
        hvacState.targetTemp = doc["temperature"];
    }
    
    if (screenManager) {
        screenManager->updateHVACStatus();
    }
    
    Serial.println("HVAC state updated");
}