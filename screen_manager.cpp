#include "screen_manager.h"
#include "mqtt_handler.h"
#include <Arduino.h>

extern MQTTHandler mqttHandler;

ScreenManager* ScreenManager::instance = nullptr;

void ScreenManager::init() {
    instance = this;
    
    screenContainer = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screenContainer, lv_color_hex(COLOR_BACKGROUND), 0);
    lv_obj_set_style_bg_opa(screenContainer, LV_OPA_COVER, 0);
    
    createAllScreens();
    setupGestureHandling();
    
    lv_scr_load(screenContainer);
    showScreen(SCREEN_LIGHT);
    
    Serial.println("Screen manager initialized");
}

void ScreenManager::createAllScreens() {
    for (int i = 0; i < SCREEN_COUNT; i++) {
        screens[i] = nullptr;
    }
    
    createLightScreen();
    createHVACScreen();
    
    screensCreated = true;
    Serial.println("All screens created");
}

void ScreenManager::createLightScreen() {
    screens[SCREEN_LIGHT] = lv_obj_create(screenContainer);
    lv_obj_set_size(screens[SCREEN_LIGHT], SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(screens[SCREEN_LIGHT], lv_color_hex(COLOR_BACKGROUND), 0);
    lv_obj_set_style_border_width(screens[SCREEN_LIGHT], 0, 0);
    lv_obj_set_style_pad_all(screens[SCREEN_LIGHT], 10, 0);
    lv_obj_clear_flag(screens[SCREEN_LIGHT], LV_OBJ_FLAG_SCROLLABLE);
    
    lv_obj_t* title = createLabel(screens[SCREEN_LIGHT], "LIGHT CONTROL", 0, 10);
    lv_obj_set_style_text_color(title, lv_color_hex(COLOR_ON_SURFACE), 0);
    lv_obj_center(title);
    lv_obj_set_y(title, 20);
    
    lightElements.statusLabel = createLabel(screens[SCREEN_LIGHT], "Connecting...", 0, 50);
    lv_obj_set_style_text_color(lightElements.statusLabel, lv_color_hex(COLOR_WARNING), 0);
    lv_obj_center(lightElements.statusLabel);
    lv_obj_set_y(lightElements.statusLabel, 50);
    
    lightElements.powerButton = createButton(screens[SCREEN_LIGHT], "OFF", 20, 80, 100, 50);
    lv_obj_set_style_bg_color(lightElements.powerButton, lv_color_hex(COLOR_ERROR), 0);
    lv_obj_add_event_cb(lightElements.powerButton, lightPowerButtonEvent, LV_EVENT_CLICKED, this);
    
    lightElements.brightnessLabel = createLabel(screens[SCREEN_LIGHT], "Brightness", 150, 85);
    lv_obj_set_style_text_color(lightElements.brightnessLabel, lv_color_hex(COLOR_ON_SURFACE), 0);
    
    lightElements.brightnessValueLabel = createLabel(screens[SCREEN_LIGHT], "50%", 420, 85);
    lv_obj_set_style_text_color(lightElements.brightnessValueLabel, lv_color_hex(COLOR_SECONDARY), 0);
    
    lightElements.brightnessSlider = createSlider(screens[SCREEN_LIGHT], 150, 110, 280, MIN_BRIGHTNESS, MAX_BRIGHTNESS, 50);
    lv_obj_add_event_cb(lightElements.brightnessSlider, lightBrightnessSliderEvent, LV_EVENT_VALUE_CHANGED, this);
    
    lightElements.colorTempLabel = createLabel(screens[SCREEN_LIGHT], "Color Temperature", 150, 145);
    lv_obj_set_style_text_color(lightElements.colorTempLabel, lv_color_hex(COLOR_ON_SURFACE), 0);
    
    lightElements.colorTempValueLabel = createLabel(screens[SCREEN_LIGHT], "4000K", 420, 145);
    lv_obj_set_style_text_color(lightElements.colorTempValueLabel, lv_color_hex(COLOR_SECONDARY), 0);
    
    lightElements.colorTempSlider = createSlider(screens[SCREEN_LIGHT], 150, 170, 280, MIN_COLOR_TEMP, MAX_COLOR_TEMP, 4000);
    lv_obj_add_event_cb(lightElements.colorTempSlider, lightColorTempSliderEvent, LV_EVENT_VALUE_CHANGED, this);
    
    lv_obj_t* navHint = createLabel(screens[SCREEN_LIGHT], "Swipe for HVAC", 0, 210);
    lv_obj_set_style_text_color(navHint, lv_color_hex(0x888888), 0);
    lv_obj_center(navHint);
    lv_obj_set_y(navHint, 210);
    
    Serial.println("Light screen created");
}

void ScreenManager::createHVACScreen() {
    screens[SCREEN_HVAC] = lv_obj_create(screenContainer);
    lv_obj_set_size(screens[SCREEN_HVAC], SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(screens[SCREEN_HVAC], lv_color_hex(COLOR_BACKGROUND), 0);
    lv_obj_set_style_border_width(screens[SCREEN_HVAC], 0, 0);
    lv_obj_set_style_pad_all(screens[SCREEN_HVAC], 10, 0);
    lv_obj_clear_flag(screens[SCREEN_HVAC], LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(screens[SCREEN_HVAC], LV_OBJ_FLAG_HIDDEN);
    
    lv_obj_t* title = createLabel(screens[SCREEN_HVAC], "HVAC CONTROL", 0, 10);
    lv_obj_set_style_text_color(title, lv_color_hex(COLOR_ON_SURFACE), 0);
    lv_obj_center(title);
    lv_obj_set_y(title, 20);
    
    hvacElements.statusLabel = createLabel(screens[SCREEN_HVAC], "Connecting...", 0, 50);
    lv_obj_set_style_text_color(hvacElements.statusLabel, lv_color_hex(COLOR_WARNING), 0);
    lv_obj_center(hvacElements.statusLabel);
    lv_obj_set_y(hvacElements.statusLabel, 50);
    
    hvacElements.powerButton = createButton(screens[SCREEN_HVAC], "OFF", 20, 80, 100, 50);
    lv_obj_set_style_bg_color(hvacElements.powerButton, lv_color_hex(COLOR_ERROR), 0);
    lv_obj_add_event_cb(hvacElements.powerButton, hvacPowerButtonEvent, LV_EVENT_CLICKED, this);
    
    hvacElements.currentTempLabel = createLabel(screens[SCREEN_HVAC], "Current: --°C", 150, 85);
    lv_obj_set_style_text_color(hvacElements.currentTempLabel, lv_color_hex(COLOR_ON_SURFACE), 0);
    
    hvacElements.targetTempLabel = createLabel(screens[SCREEN_HVAC], "Target Temperature", 150, 115);
    lv_obj_set_style_text_color(hvacElements.targetTempLabel, lv_color_hex(COLOR_ON_SURFACE), 0);
    
    hvacElements.targetTempValueLabel = createLabel(screens[SCREEN_HVAC], "22°C", 270, 140);
    lv_obj_set_style_text_color(hvacElements.targetTempValueLabel, lv_color_hex(COLOR_PRIMARY), 0);
    
    hvacElements.tempDownButton = createButton(screens[SCREEN_HVAC], "-", 200, 140, 50, 50);
    lv_obj_set_style_bg_color(hvacElements.tempDownButton, lv_color_hex(COLOR_PRIMARY), 0);
    lv_obj_add_event_cb(hvacElements.tempDownButton, hvacTempDownButtonEvent, LV_EVENT_CLICKED, this);
    
    hvacElements.tempUpButton = createButton(screens[SCREEN_HVAC], "+", 350, 140, 50, 50);
    lv_obj_set_style_bg_color(hvacElements.tempUpButton, lv_color_hex(COLOR_PRIMARY), 0);
    lv_obj_add_event_cb(hvacElements.tempUpButton, hvacTempUpButtonEvent, LV_EVENT_CLICKED, this);
    
    hvacElements.modeButton = createButton(screens[SCREEN_HVAC], "Heat", 450, 80, 80, 50);
    lv_obj_set_style_bg_color(hvacElements.modeButton, lv_color_hex(COLOR_SECONDARY), 0);
    lv_obj_add_event_cb(hvacElements.modeButton, hvacModeButtonEvent, LV_EVENT_CLICKED, this);
    
    lv_obj_t* navHint = createLabel(screens[SCREEN_HVAC], "Swipe for Light", 0, 210);
    lv_obj_set_style_text_color(navHint, lv_color_hex(0x888888), 0);
    lv_obj_center(navHint);
    lv_obj_set_y(navHint, 210);
    
    Serial.println("HVAC screen created");
}

lv_obj_t* ScreenManager::createButton(lv_obj_t* parent, const char* text, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h) {
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_size(btn, w, h);
    
    lv_obj_t* label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_center(label);
    
    return btn;
}

lv_obj_t* ScreenManager::createLabel(lv_obj_t* parent, const char* text, lv_coord_t x, lv_coord_t y) {
    lv_obj_t* label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_pos(label, x, y);
    return label;
}

lv_obj_t* ScreenManager::createSlider(lv_obj_t* parent, lv_coord_t x, lv_coord_t y, lv_coord_t w, int min_val, int max_val, int default_val) {
    lv_obj_t* slider = lv_slider_create(parent);
    lv_obj_set_pos(slider, x, y);
    lv_obj_set_width(slider, w);
    lv_slider_set_range(slider, min_val, max_val);
    lv_slider_set_value(slider, default_val, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(slider, lv_color_hex(COLOR_SURFACE), LV_PART_MAIN);
    lv_obj_set_style_bg_color(slider, lv_color_hex(COLOR_PRIMARY), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider, lv_color_hex(COLOR_SECONDARY), LV_PART_KNOB);
    return slider;
}

void ScreenManager::showScreen(ScreenType screen) {
    if (screen >= SCREEN_COUNT || !screensCreated) {
        return;
    }
    
    for (int i = 0; i < SCREEN_COUNT; i++) {
        if (screens[i]) {
            lv_obj_add_flag(screens[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
    
    if (screens[screen]) {
        lv_obj_clear_flag(screens[screen], LV_OBJ_FLAG_HIDDEN);
        currentScreen = screen;
        
        Serial.print("Switched to screen: ");
        Serial.println(SCREEN_NAMES[screen]);
        
        if (screen == SCREEN_LIGHT) {
            updateLightStatus();
        } else if (screen == SCREEN_HVAC) {
            updateHVACStatus();
        }
    }
}

void ScreenManager::nextScreen() {
    ScreenType nextScreen = (ScreenType)((currentScreen + 1) % SCREEN_COUNT);
    showScreen(nextScreen);
}

void ScreenManager::previousScreen() {
    ScreenType prevScreen = (ScreenType)((currentScreen - 1 + SCREEN_COUNT) % SCREEN_COUNT);
    showScreen(prevScreen);
}

void ScreenManager::setupGestureHandling() {
    lv_obj_add_event_cb(screenContainer, gestureEventHandler, LV_EVENT_GESTURE, this);
    lv_obj_clear_flag(screenContainer, LV_OBJ_FLAG_GESTURE_BUBBLE);
}

void ScreenManager::gestureEventHandler(lv_event_t* e) {
    ScreenManager* mgr = (ScreenManager*)lv_event_get_user_data(e);
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    
    if (dir == LV_DIR_LEFT) {
        mgr->nextScreen();
    } else if (dir == LV_DIR_RIGHT) {
        mgr->previousScreen();
    }
}

void ScreenManager::updateLightStatus() {
    if (!lightElements.statusLabel) return;
    
    if (lightState.available) {
        lv_label_set_text(lightElements.statusLabel, "Online");
        lv_obj_set_style_text_color(lightElements.statusLabel, lv_color_hex(COLOR_SUCCESS), 0);
        
        lv_label_set_text(lv_obj_get_child(lightElements.powerButton, 0), lightState.isOn ? "ON" : "OFF");
        lv_obj_set_style_bg_color(lightElements.powerButton, lv_color_hex(lightState.isOn ? COLOR_SUCCESS : COLOR_ERROR), 0);
        
        lv_slider_set_value(lightElements.brightnessSlider, lightState.brightness, LV_ANIM_OFF);
        char brightStr[10];
        snprintf(brightStr, sizeof(brightStr), "%d%%", lightState.brightness);
        lv_label_set_text(lightElements.brightnessValueLabel, brightStr);
        
        lv_slider_set_value(lightElements.colorTempSlider, lightState.colorTemp, LV_ANIM_OFF);
        char tempStr[10];
        snprintf(tempStr, sizeof(tempStr), "%dK", lightState.colorTemp);
        lv_label_set_text(lightElements.colorTempValueLabel, tempStr);
        
    } else {
        lv_label_set_text(lightElements.statusLabel, "Offline");
        lv_obj_set_style_text_color(lightElements.statusLabel, lv_color_hex(COLOR_ERROR), 0);
    }
}

void ScreenManager::updateHVACStatus() {
    if (!hvacElements.statusLabel) return;
    
    if (hvacState.available) {
        lv_label_set_text(hvacElements.statusLabel, "Online");
        lv_obj_set_style_text_color(hvacElements.statusLabel, lv_color_hex(COLOR_SUCCESS), 0);
        
        lv_label_set_text(lv_obj_get_child(hvacElements.powerButton, 0), hvacState.isOn ? "ON" : "OFF");
        lv_obj_set_style_bg_color(hvacElements.powerButton, lv_color_hex(hvacState.isOn ? COLOR_SUCCESS : COLOR_ERROR), 0);
        
        char currentTempStr[20];
        snprintf(currentTempStr, sizeof(currentTempStr), "Current: %.1f°C", hvacState.currentTemp);
        lv_label_set_text(hvacElements.currentTempLabel, currentTempStr);
        
        char targetTempStr[10];
        snprintf(targetTempStr, sizeof(targetTempStr), "%.0f°C", hvacState.targetTemp);
        lv_label_set_text(hvacElements.targetTempValueLabel, targetTempStr);
        
        lv_label_set_text(lv_obj_get_child(hvacElements.modeButton, 0), hvacState.mode.c_str());
        
    } else {
        lv_label_set_text(hvacElements.statusLabel, "Offline");
        lv_obj_set_style_text_color(hvacElements.statusLabel, lv_color_hex(COLOR_ERROR), 0);
    }
}

void ScreenManager::update() {
    // Handle any periodic updates here
}

void ScreenManager::lightPowerButtonEvent(lv_event_t* e) {
    mqttHandler.setLightState(!lightState.isOn);
}

void ScreenManager::lightBrightnessSliderEvent(lv_event_t* e) {
    lv_obj_t* slider = (lv_obj_t*)lv_event_get_target(e);
    int brightness = lv_slider_get_value(slider);
    
    char brightStr[10];
    snprintf(brightStr, sizeof(brightStr), "%d%%", brightness);
    lv_label_set_text(instance->lightElements.brightnessValueLabel, brightStr);
    
    mqttHandler.setLightBrightness(brightness);
}

void ScreenManager::lightColorTempSliderEvent(lv_event_t* e) {
    lv_obj_t* slider = (lv_obj_t*)lv_event_get_target(e);
    int colorTemp = lv_slider_get_value(slider);
    
    char tempStr[10];
    snprintf(tempStr, sizeof(tempStr), "%dK", colorTemp);
    lv_label_set_text(instance->lightElements.colorTempValueLabel, tempStr);
    
    mqttHandler.setLightColorTemp(colorTemp);
}

void ScreenManager::hvacPowerButtonEvent(lv_event_t* e) {
    mqttHandler.setHVACState(!hvacState.isOn);
}

void ScreenManager::hvacTempUpButtonEvent(lv_event_t* e) {
    float newTemp = hvacState.targetTemp + 1.0f;
    if (newTemp <= MAX_TEMPERATURE) {
        mqttHandler.setHVACTemperature(newTemp);
        
        char tempStr[10];
        snprintf(tempStr, sizeof(tempStr), "%.0f°C", newTemp);
        lv_label_set_text(instance->hvacElements.targetTempValueLabel, tempStr);
    }
}

void ScreenManager::hvacTempDownButtonEvent(lv_event_t* e) {
    float newTemp = hvacState.targetTemp - 1.0f;
    if (newTemp >= MIN_TEMPERATURE) {
        mqttHandler.setHVACTemperature(newTemp);
        
        char tempStr[10];
        snprintf(tempStr, sizeof(tempStr), "%.0f°C", newTemp);
        lv_label_set_text(instance->hvacElements.targetTempValueLabel, tempStr);
    }
}

void ScreenManager::hvacModeButtonEvent(lv_event_t* e) {
    String currentMode = hvacState.mode;
    int currentIndex = 0;
    
    for (int i = 0; i < HVAC_MODE_COUNT; i++) {
        if (currentMode == HVAC_MODES[i]) {
            currentIndex = i;
            break;
        }
    }
    
    int nextIndex = (currentIndex + 1) % HVAC_MODE_COUNT;
    const char* nextMode = HVAC_MODES[nextIndex];
    
    mqttHandler.setHVACMode(nextMode);
    
    lv_label_set_text(lv_obj_get_child(instance->hvacElements.modeButton, 0), nextMode);
}