#include "screen_manager.h"
#include "mqtt_handler.h"
#include <Arduino.h>

extern MQTTHandler mqttHandler;

ScreenManager* ScreenManager::instance = nullptr;

void ScreenManager::init() {
    instance = this;
    
    screenContainer = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screenContainer, lv_color_black(), 0);  // Force black background
    lv_obj_set_style_bg_opa(screenContainer, LV_OPA_COVER, 0);
    
    createAllScreens();
    // setupGestureHandling is now called in createAllScreens()
    
    lv_scr_load(screenContainer);
    showScreen(SCREEN_LIGHT);
    
    Serial.println("Screen manager initialized with gesture support");
}

void ScreenManager::createAllScreens() {
    for (int i = 0; i < SCREEN_COUNT; i++) {
        screens[i] = nullptr;
    }
    
    createLightScreen();
    createHVACScreen();
    
    screensCreated = true;
    
    // Set up gestures after all screens are created
    setupGestureHandling();
    
    Serial.println("All screens created with gesture handling");
}

void ScreenManager::createLightScreen() {
    screens[SCREEN_LIGHT] = lv_obj_create(screenContainer);
    lv_obj_set_size(screens[SCREEN_LIGHT], SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(screens[SCREEN_LIGHT], lv_color_black(), 0);
    lv_obj_set_style_border_width(screens[SCREEN_LIGHT], 0, 0);
    lv_obj_set_style_pad_all(screens[SCREEN_LIGHT], 0, 0);
    lv_obj_clear_flag(screens[SCREEN_LIGHT], LV_OBJ_FLAG_SCROLLABLE);
    
    // Title - HUGE and centered
    lv_obj_t* title = createLabel(screens[SCREEN_LIGHT], "LIGHT", 268, 20);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_22, 0);
    
    // Power button - MUCH BIGGER, left side
    lightElements.powerButton = createButton(screens[SCREEN_LIGHT], "OFF", 30, 70, 120, 60);
    lv_obj_set_style_bg_color(lightElements.powerButton, lv_color_hex(COLOR_ERROR), 0);
    lv_obj_set_style_text_font(lv_obj_get_child(lightElements.powerButton, 0), &lv_font_montserrat_22, 0);
    lv_obj_add_event_cb(lightElements.powerButton, lightPowerButtonEvent, LV_EVENT_CLICKED, this);
    
    // Brightness section - BIGGER TEXT, spread across full width
    lightElements.brightnessLabel = createLabel(screens[SCREEN_LIGHT], "Bright: 50%", 180, 75);
    lv_obj_set_style_text_color(lightElements.brightnessLabel, lv_color_white(), 0);
    lv_obj_set_style_text_font(lightElements.brightnessLabel, &lv_font_montserrat_22, 0);
    
    // MUCH BIGGER brightness bar - full width
    lightElements.brightnessBar = createBar(screens[SCREEN_LIGHT], 180, 110, 320, MIN_BRIGHTNESS, MAX_BRIGHTNESS, 50);
    
    // Color temp section - BIGGER TEXT, full width
    lightElements.colorTempLabel = createLabel(screens[SCREEN_LIGHT], "Color: 4000K", 180, 150);
    lv_obj_set_style_text_color(lightElements.colorTempLabel, lv_color_white(), 0);
    lv_obj_set_style_text_font(lightElements.colorTempLabel, &lv_font_montserrat_22, 0);
    
    // MUCH BIGGER color temp bar - full width
    lightElements.colorTempBar = createBar(screens[SCREEN_LIGHT], 180, 185, 320, MIN_COLOR_TEMP, MAX_COLOR_TEMP, 4000);
    
    Serial.println("Light screen - HUGE LAYOUT");
}

void ScreenManager::createHVACScreen() {
    screens[SCREEN_HVAC] = lv_obj_create(screenContainer);
    lv_obj_set_size(screens[SCREEN_HVAC], SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(screens[SCREEN_HVAC], lv_color_black(), 0);
    lv_obj_set_style_border_width(screens[SCREEN_HVAC], 0, 0);
    lv_obj_set_style_pad_all(screens[SCREEN_HVAC], 0, 0);
    lv_obj_clear_flag(screens[SCREEN_HVAC], LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(screens[SCREEN_HVAC], LV_OBJ_FLAG_HIDDEN);
    
    // Title - HUGE and centered
    lv_obj_t* title = createLabel(screens[SCREEN_HVAC], "HVAC", 268, 20);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_22, 0);
    
    // Mode buttons - MUCH BIGGER, spread across screen
    hvacElements.offButton = createButton(screens[SCREEN_HVAC], "OFF", 60, 80, 120, 70);
    lv_obj_set_style_bg_color(hvacElements.offButton, lv_color_hex(COLOR_SUCCESS), 0);
    lv_obj_set_style_text_font(lv_obj_get_child(hvacElements.offButton, 0), &lv_font_montserrat_22, 0);
    lv_obj_add_event_cb(hvacElements.offButton, hvacOffButtonEvent, LV_EVENT_CLICKED, this);
    
    hvacElements.coolButton = createButton(screens[SCREEN_HVAC], "COOL", 200, 80, 120, 70);
    lv_obj_set_style_bg_color(hvacElements.coolButton, lv_color_hex(COLOR_SURFACE), 0);
    lv_obj_set_style_text_font(lv_obj_get_child(hvacElements.coolButton, 0), &lv_font_montserrat_22, 0);
    lv_obj_add_event_cb(hvacElements.coolButton, hvacCoolButtonEvent, LV_EVENT_CLICKED, this);
    
    // Temperature controls - MUCH BIGGER buttons, spread across bottom
    hvacElements.tempDownButton = createButton(screens[SCREEN_HVAC], "-", 100, 170, 80, 60);
    lv_obj_set_style_bg_color(hvacElements.tempDownButton, lv_color_hex(COLOR_SECONDARY), 0);
    lv_obj_set_style_text_font(lv_obj_get_child(hvacElements.tempDownButton, 0), &lv_font_montserrat_22, 0);
    lv_obj_add_event_cb(hvacElements.tempDownButton, hvacTempDownButtonEvent, LV_EVENT_CLICKED, this);
    
    // HUGE temperature display
    hvacElements.targetTempValueLabel = createLabel(screens[SCREEN_HVAC], "22", 268, 180);
    lv_obj_set_style_text_color(hvacElements.targetTempValueLabel, lv_color_hex(COLOR_PRIMARY), 0);
    lv_obj_set_style_text_font(hvacElements.targetTempValueLabel, &lv_font_montserrat_22, 0);
    
    hvacElements.tempUpButton = createButton(screens[SCREEN_HVAC], "+", 360, 170, 80, 60);
    lv_obj_set_style_bg_color(hvacElements.tempUpButton, lv_color_hex(COLOR_SECONDARY), 0);
    lv_obj_set_style_text_font(lv_obj_get_child(hvacElements.tempUpButton, 0), &lv_font_montserrat_22, 0);
    lv_obj_add_event_cb(hvacElements.tempUpButton, hvacTempUpButtonEvent, LV_EVENT_CLICKED, this);
    
    Serial.println("HVAC screen - HUGE LAYOUT");
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

lv_obj_t* ScreenManager::createBar(lv_obj_t* parent, lv_coord_t x, lv_coord_t y, lv_coord_t w, int min_val, int max_val, int default_val) {
    lv_obj_t* bar = lv_bar_create(parent);
    lv_obj_set_pos(bar, x, y);
    lv_obj_set_width(bar, w);
    lv_obj_set_height(bar, 25);  // MUCH THICKER bars - was 15
    lv_bar_set_range(bar, min_val, max_val);
    lv_bar_set_value(bar, default_val, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0x333333), LV_PART_MAIN);  // Dark gray background
    lv_obj_set_style_bg_color(bar, lv_color_hex(COLOR_PRIMARY), LV_PART_INDICATOR);  // Blue indicator
    
    // Make bars non-clickable to prevent gesture interference
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(bar, LV_OBJ_FLAG_GESTURE_BUBBLE);
    
    return bar;
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
    // Set up gesture handling on the screen container
    lv_obj_add_event_cb(screenContainer, gestureEventHandler, LV_EVENT_GESTURE, this);
    
    // Enable gestures on all screens
    for (int i = 0; i < SCREEN_COUNT; i++) {
        if (screens[i]) {
            lv_obj_add_event_cb(screens[i], gestureEventHandler, LV_EVENT_GESTURE, this);
        }
    }
}

void ScreenManager::gestureEventHandler(lv_event_t* e) {
    ScreenManager* mgr = (ScreenManager*)lv_event_get_user_data(e);
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    
    Serial.print("Gesture detected: ");
    Serial.println(dir);
    
    // Try both sets of constants to see which work
    if (dir == LV_DIR_TOP) {
        Serial.println("Swipe UP detected - next screen");
        mgr->nextScreen();
    } else if (dir == LV_DIR_BOTTOM) {
        Serial.println("Swipe DOWN detected - previous screen");  
        mgr->previousScreen();
    } else if (dir == LV_DIR_LEFT) {
        Serial.println("Swipe LEFT detected - next screen");
        mgr->nextScreen();
    } else if (dir == LV_DIR_RIGHT) {
        Serial.println("Swipe RIGHT detected - previous screen");
        mgr->previousScreen();
    }
}

void ScreenManager::updateLightStatus() {
    if (!lightElements.powerButton) return;
    
    if (lightState.available) {
        // Update power button
        lv_label_set_text(lv_obj_get_child(lightElements.powerButton, 0), lightState.isOn ? "ON" : "OFF");
        lv_obj_set_style_bg_color(lightElements.powerButton, lv_color_hex(lightState.isOn ? COLOR_SUCCESS : COLOR_ERROR), 0);
        
        // Update brightness - shorter label to prevent overlap
        lv_bar_set_value(lightElements.brightnessBar, lightState.brightness, LV_ANIM_OFF);
        char brightStr[20];
        snprintf(brightStr, sizeof(brightStr), "Bright: %d%%", lightState.brightness);
        lv_label_set_text(lightElements.brightnessLabel, brightStr);
        
        // Update color temp - shorter label
        lv_bar_set_value(lightElements.colorTempBar, lightState.colorTemp, LV_ANIM_OFF);
        char tempStr[20];
        snprintf(tempStr, sizeof(tempStr), "Color: %dK", lightState.colorTemp);
        lv_label_set_text(lightElements.colorTempLabel, tempStr);
    }
}

void ScreenManager::updateHVACStatus() {
    if (!hvacElements.offButton) return;
    
    if (hvacState.available) {
        // Update button colors based on mode
        if (hvacState.mode == "off" || hvacState.mode == "" || !hvacState.isOn) {
            lv_obj_set_style_bg_color(hvacElements.offButton, lv_color_hex(COLOR_SUCCESS), 0);
            lv_obj_set_style_bg_color(hvacElements.coolButton, lv_color_hex(COLOR_SURFACE), 0);
        } else if (hvacState.mode == "cool") {
            lv_obj_set_style_bg_color(hvacElements.offButton, lv_color_hex(COLOR_SURFACE), 0);
            lv_obj_set_style_bg_color(hvacElements.coolButton, lv_color_hex(COLOR_SUCCESS), 0);
        } else {
            lv_obj_set_style_bg_color(hvacElements.offButton, lv_color_hex(COLOR_SUCCESS), 0);
            lv_obj_set_style_bg_color(hvacElements.coolButton, lv_color_hex(COLOR_SURFACE), 0);
        }
        
        // Update target temperature - just the number, no "°C"
        char targetTempStr[5];
        snprintf(targetTempStr, sizeof(targetTempStr), "%.0f", hvacState.targetTemp);
        lv_label_set_text(hvacElements.targetTempValueLabel, targetTempStr);
    }
}

void ScreenManager::update() {
    // Handle any periodic updates here
}

void ScreenManager::lightPowerButtonEvent(lv_event_t* e) {
    mqttHandler.setLightState(!lightState.isOn);
}

void ScreenManager::hvacOffButtonEvent(lv_event_t* e) {
    mqttHandler.setHVACMode("off");
}

void ScreenManager::hvacCoolButtonEvent(lv_event_t* e) {
    mqttHandler.setHVACMode("cool");
}

void ScreenManager::hvacTempUpButtonEvent(lv_event_t* e) {
    float newTemp = hvacState.targetTemp + 1.0f;
    if (newTemp <= MAX_TEMPERATURE) {
        mqttHandler.setHVACTemperature(newTemp);
        
        char tempStr[5];
        snprintf(tempStr, sizeof(tempStr), "%.0f", newTemp);
        lv_label_set_text(instance->hvacElements.targetTempValueLabel, tempStr);
    }
}

void ScreenManager::hvacTempDownButtonEvent(lv_event_t* e) {
    float newTemp = hvacState.targetTemp - 1.0f;
    if (newTemp >= MIN_TEMPERATURE) {
        mqttHandler.setHVACTemperature(newTemp);
        
        char tempStr[5];
        snprintf(tempStr, sizeof(tempStr), "%.0f", newTemp);
        lv_label_set_text(instance->hvacElements.targetTempValueLabel, tempStr);
    }
}