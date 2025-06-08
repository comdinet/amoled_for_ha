#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <lvgl.h>
#include "config.h"
#include "display_init.h"
#include "mqtt_handler.h"
#include "screen_manager.h"

WiFiClient espClient;
PubSubClient mqttClient(espClient);
ScreenManager screenManager;
MQTTHandler mqttHandler;

unsigned long lastWiFiReconnectAttempt = 0;
unsigned long lastMQTTReconnectAttempt = 0;
unsigned long lastStatusUpdate = 0;
bool isConnected = false;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("ESP32-S3 Home Assistant Controller");
    Serial.println("Device: " DEVICE_FRIENDLY_NAME);
    
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, LOW);
    
    Serial.println("Initializing display...");
    if (!initDisplay()) {
        Serial.println("ERROR: Display initialization failed!");
        while(1) {
            digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
            delay(200);
        }
    }
    
    Serial.println("Initializing screens...");
    screenManager.init();
    
    Serial.println("Connecting to WiFi...");
    connectWiFi();
    
    Serial.println("Initializing MQTT...");
    mqttHandler.init(&mqttClient, &screenManager);
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    
    connectMQTT();
    screenManager.showScreen(SCREEN_LIGHT);
    
    Serial.println("Setup completed!");
    digitalWrite(STATUS_LED_PIN, HIGH);
}

void loop() {
    unsigned long currentTime = millis();
    
    lv_timer_handler();
    
    if (WiFi.status() != WL_CONNECTED) {
        isConnected = false;
        digitalWrite(STATUS_LED_PIN, LOW);
        
        if (currentTime - lastWiFiReconnectAttempt >= WIFI_RECONNECT_INTERVAL) {
            connectWiFi();
            lastWiFiReconnectAttempt = currentTime;
        }
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        if (!mqttClient.connected()) {
            isConnected = false;
            digitalWrite(STATUS_LED_PIN, LOW);
            
            if (currentTime - lastMQTTReconnectAttempt >= MQTT_RECONNECT_INTERVAL) {
                connectMQTT();
                lastMQTTReconnectAttempt = currentTime;
            }
        } else {
            mqttClient.loop();
            
            if (!isConnected) {
                isConnected = true;
                digitalWrite(STATUS_LED_PIN, HIGH);
                mqttHandler.subscribeToTopics();
                mqttHandler.requestStatusUpdate();
            }
        }
    }
    
    if (isConnected && currentTime - lastStatusUpdate >= STATUS_UPDATE_INTERVAL) {
        sendDeviceStatus();
        lastStatusUpdate = currentTime;
    }
    
    screenManager.update();
    delay(5);
}

void connectWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nWiFi connection failed!");
    }
}

void connectMQTT() {
    if (WiFi.status() != WL_CONNECTED) return;
    
    String clientId = DEVICE_NAME;
    clientId += "_";
    clientId += String(random(0xffff), HEX);
    
    bool connected;
    if (strlen(MQTT_USER) > 0) {
        connected = mqttClient.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD);
    } else {
        connected = mqttClient.connect(clientId.c_str());
    }
    
    if (connected) {
        Serial.println("MQTT connected!");
        mqttClient.publish(DEVICE_STATUS_TOPIC, "online", true);
    } else {
        Serial.print("MQTT failed! Error: ");
        Serial.println(mqttClient.state());
    }
}

void sendDeviceStatus() {
    StaticJsonDocument<200> doc;
    
    doc["device"] = DEVICE_NAME;
    doc["status"] = "online";
    doc["wifi_rssi"] = WiFi.RSSI();
    doc["free_heap"] = ESP.getFreeHeap();
    doc["uptime"] = millis() / 1000;
    doc["current_screen"] = SCREEN_NAMES[screenManager.getCurrentScreen()];
    
    String payload;
    serializeJson(doc, payload);
    
    mqttClient.publish(DEVICE_STATUS_TOPIC, payload.c_str(), true);
}