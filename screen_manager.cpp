#include "screen_manager.h"
#include "mqtt_handler.h"
#include <Arduino.h>

extern MQTTHandler mqttHandler;

ScreenManager* ScreenManager::instance = nullptr;

void ScreenManager::init() {
    instance = this;
    
    screenContainer = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screenContainer, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(screenContainer, LV_OPA_COVER, 0);
    
    createAllScreens();
    setupGestureHandling();
    
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
    
    Serial.println("All screens created with gesture handling");
}

void ScreenManager::createLightScreen() {
    screens[SCREEN_LIGHT] = lv_obj_create(screenContainer);
    lv_obj_set_size(screens[SCREEN_LIGHT], SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(screens[SCREEN_LIGHT], lv_color_black(), 0);
    lv_obj_set_style_border_width(screens[SCREEN_LIGHT], 0, 0);
    lv_obj_set_style_pad_all(screens[SCREEN_LIGHT], 0, 0);
    lv_obj_clear_flag(screens[SCREEN_LIGHT], LV_OBJ_FLAG_SCROLLABLE);
    
    // Create HUGE "B" using multiple labels to form ASCII art
    createLabel(screens[SCREEN_LIGHT], "BBB", 20, 40);
    createLabel(screens[SCREEN_LIGHT], "B  B", 20, 55);
    createLabel(screens[SCREEN_LIGHT], "BBB", 20, 70);
    createLabel(screens[SCREEN_LIGHT], "B  B", 20, 85);
    createLabel(screens[SCREEN_LIGHT], "BBB", 20, 100);
    
    // Long brightness bar
    lightElements.brightnessBar = createBar(screens[SCREEN_LIGHT], 90, 50, 430, MIN_BRIGHTNESS, MAX_BRIGHTNESS, 50);
    lv_obj_add_event_cb(lightElements.brightnessBar, brightnessBarEvent, LV_EVENT_CLICKED, this);
    lv_obj_add_event_cb(lightElements.brightnessBar, brightnessBarEvent, LV_EVENT_PRESSING, this);
    
    // Create HUGE "C" using multiple labels to form ASCII art
    createLabel(screens[SCREEN_LIGHT], "CCC", 20, 130);
    createLabel(screens[SCREEN_LIGHT], "C", 20, 145);
    createLabel(screens[SCREEN_LIGHT], "C", 20, 160);
    createLabel(screens[SCREEN_LIGHT], "C", 20, 175);
    createLabel(screens[SCREEN_LIGHT], "CCC", 20, 190);
    
    // Long color temp bar
    lightElements.colorTempBar = createBar(screens[SCREEN_LIGHT], 90, 140, 430, MIN_COLOR_TEMP, MAX_COLOR_TEMP, 4000);
    lv_obj_add_event_cb(lightElements.colorTempBar, colorTempBarEvent, LV_EVENT_CLICKED, this);
    lv_obj_add_event_cb(lightElements.colorTempBar, colorTempBarEvent, LV_EVENT_PRESSING, this);
    
    // Create hidden labels for status updates
    lightElements.brightnessLabel = createLabel(screens[SCREEN_LIGHT], "", 0, 0);
    lv_obj_add_flag(lightElements.brightnessLabel, LV_OBJ_FLAG_HIDDEN);
    lightElements.colorTempLabel = createLabel(screens[SCREEN_LIGHT], "", 0, 0);
    lv_obj_add_flag(lightElements.colorTempLabel, LV_OBJ_FLAG_HIDDEN);
    lightElements.powerButton = nullptr;
    
    Serial.println("Light screen created with HUGE ASCII art B and C");
}

void ScreenManager::createHVACScreen() {
    screens[SCREEN_HVAC] = lv_obj_create(screenContainer);
    lv_obj_set_size(screens[SCREEN_HVAC], SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(screens[SCREEN_HVAC], lv_color_black(), 0);
    lv_obj_set_style_border_width(screens[SCREEN_HVAC], 0, 0);
    lv_obj_set_style_pad_all(screens[SCREEN_HVAC], 0, 0);
    lv_obj_clear_flag(screens[SCREEN_HVAC], LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(screens[SCREEN_HVAC], LV_OBJ_FLAG_HIDDEN);
    
    // OFF button at TOP LEFT
    hvacElements.offButton = createButton(screens[SCREEN_HVAC], "OFF", 150, 10, 100, 50);
    lv_obj_set_style_bg_color(hvacElements.offButton, lv_color_hex(0x333333), 0);
    lv_obj_set_style_text_color(lv_obj_get_child(hvacElements.offButton, 0), lv_color_white(), 0);
    lv_obj_set_style_text_font(lv_obj_get_child(hvacElements.offButton, 0), &lv_font_montserrat_22, 0);
    lv_obj_add_event_cb(hvacElements.offButton, hvacOffButtonEvent, LV_EVENT_CLICKED, this);
    
    // COOL button at TOP RIGHT
    hvacElements.coolButton = createButton(screens[SCREEN_HVAC], "COOL", 286, 10, 100, 50);
    lv_obj_set_style_bg_color(hvacElements.coolButton, lv_color_hex(0x2196F3), 0);
    lv_obj_set_style_text_color(lv_obj_get_child(hvacElements.coolButton, 0), lv_color_white(), 0);
    lv_obj_set_style_text_font(lv_obj_get_child(hvacElements.coolButton, 0), &lv_font_montserrat_22, 0);
    lv_obj_add_event_cb(hvacElements.coolButton, hvacCoolButtonEvent, LV_EVENT_CLICKED, this);
    
    // MINUS button on LEFT SIDE - FULL HEIGHT with BIG minus
    hvacElements.tempDownButton = createButton(screens[SCREEN_HVAC], "---", 5, 70, 120, 165);
    lv_obj_set_style_bg_color(hvacElements.tempDownButton, lv_color_hex(0x404040), 0);
    lv_obj_set_style_text_color(lv_obj_get_child(hvacElements.tempDownButton, 0), lv_color_white(), 0);
    lv_obj_set_style_text_font(lv_obj_get_child(hvacElements.tempDownButton, 0), &lv_font_montserrat_22, 0);
    lv_obj_add_event_cb(hvacElements.tempDownButton, hvacTempDownButtonEvent, LV_EVENT_CLICKED, this);
    
    // PLUS button on RIGHT SIDE - FULL HEIGHT with BIG plus
    hvacElements.tempUpButton = createButton(screens[SCREEN_HVAC], "+++", 411, 70, 120, 165);
    lv_obj_set_style_bg_color(hvacElements.tempUpButton, lv_color_hex(0x404040), 0);
    lv_obj_set_style_text_color(lv_obj_get_child(hvacElements.tempUpButton, 0), lv_color_white(), 0);
    lv_obj_set_style_text_font(lv_obj_get_child(hvacElements.tempUpButton, 0), &lv_font_montserrat_22, 0);
    lv_obj_add_event_cb(hvacElements.tempUpButton, hvacTempUpButtonEvent, LV_EVENT_CLICKED, this);
    
    // Create HUGE "22" using ASCII art - First digit "2"
    createLabel(screens[SCREEN_HVAC], "222", 180, 100);
    createLabel(screens[SCREEN_HVAC], "  2", 180, 115);
    createLabel(screens[SCREEN_HVAC], "222", 180, 130);
    createLabel(screens[SCREEN_HVAC], "2", 180, 145);
    createLabel(screens[SCREEN_HVAC], "222", 180, 160);
    
    // Create HUGE "22" using ASCII art - Second digit "2"
    createLabel(screens[SCREEN_HVAC], "222", 250, 100);
    createLabel(screens[SCREEN_HVAC], "  2", 250, 115);
    createLabel(screens[SCREEN_HVAC], "222", 250, 130);
    createLabel(screens[SCREEN_HVAC], "2", 250, 145);
    createLabel(screens[SCREEN_HVAC], "222", 250, 160);
    
    // Use one of the labels as reference for updates
    hvacElements.targetTempValueLabel = createLabel(screens[SCREEN_HVAC], "", 0, 0);
    lv_obj_add_flag(hvacElements.targetTempValueLabel, LV_OBJ_FLAG_HIDDEN);
    
    Serial.println("HVAC screen created with HUGE ASCII art temperature");
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
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_22, 0);
    return label;
}

lv_obj_t* ScreenManager::createBar(lv_obj_t* parent, lv_coord_t x, lv_coord_t y, lv_coord_t w, int min_val, int max_val, int default_val) {
    lv_obj_t* bar = lv_bar_create(parent);
    lv_obj_set_pos(bar, x, y);
    lv_obj_set_width(bar, w);
    lv_obj_set_height(bar, 60);
    lv_bar_set_range(bar, min_val, max_val);
    lv_bar_set_value(bar, default_val, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0x333333), LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0xFF9500), LV_PART_INDICATOR);
    lv_obj_set_style_border_width(bar, 0, LV_PART_MAIN);
    
    lv_obj_add_flag(bar, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_GESTURE_BUBBLE);
    
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
    lv_obj_add_event_cb(screenContainer, gestureEventHandler, LV_EVENT_GESTURE, this);
    
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
    
    if (dir == LV_DIR_TOP) {
        Serial.println("Swipe UP detected - next screen");
        mgr->nextScreen();
    } else if (dir == LV_DIR_BOTTOM) {
        Serial.println("Swipe DOWN detected - previous screen");  
        mgr->previousScreen();
    }
}

void ScreenManager::updateLightStatus() {
    if (!lightElements.brightnessBar) return;
    
    if (lightState.available) {
        lv_bar_set_value(lightElements.brightnessBar, lightState.brightness, LV_ANIM_OFF);
        lv_bar_set_value(lightElements.colorTempBar, lightState.colorTemp, LV_ANIM_OFF);
    }
}

void ScreenManager::updateHVACStatus() {
    if (!hvacElements.offButton) return;
    
    if (hvacState.available) {
        if (hvacState.mode == "off" || hvacState.mode == "" || !hvacState.isOn) {
            lv_obj_set_style_bg_color(hvacElements.offButton, lv_color_hex(0x666666), 0);
            lv_obj_set_style_bg_color(hvacElements.coolButton, lv_color_hex(0x333333), 0);
        } else if (hvacState.mode == "cool") {
            lv_obj_set_style_bg_color(hvacElements.offButton, lv_color_hex(0x333333), 0);
            lv_obj_set_style_bg_color(hvacElements.coolButton, lv_color_hex(0x2196F3), 0);
        } else {
            lv_obj_set_style_bg_color(hvacElements.offButton, lv_color_hex(0x333333), 0);
            lv_obj_set_style_bg_color(hvacElements.coolButton, lv_color_hex(0x333333), 0);
        }
        
        // ASCII art temperature is static for now - would need complex redrawing for dynamic updates
        Serial.print("Temperature: ");
        Serial.println(hvacState.targetTemp);
    }
}

void ScreenManager::update() {
    // Handle any periodic updates here
}

void ScreenManager::lightPowerButtonEvent(lv_event_t* e) {
    // No power button in the new design
}

void ScreenManager::brightnessBarEvent(lv_event_t* e) {
    lv_obj_t* bar = lv_event_get_target(e);
    lv_point_t point;
    lv_indev_get_point(lv_indev_get_act(), &point);
    
    lv_coord_t bar_x = lv_obj_get_x(bar);
    lv_coord_t bar_width = lv_obj_get_width(bar);
    lv_coord_t touch_x = point.x - bar_x;
    
    if (touch_x >= 0 && touch_x <= bar_width) {
        int new_brightness = MIN_BRIGHTNESS + (touch_x * (MAX_BRIGHTNESS - MIN_BRIGHTNESS)) / bar_width;
        if (new_brightness < MIN_BRIGHTNESS) new_brightness = MIN_BRIGHTNESS;
        if (new_brightness > MAX_BRIGHTNESS) new_brightness = MAX_BRIGHTNESS;
        
        mqttHandler.setLightBrightness(new_brightness);
        lv_bar_set_value(bar, new_brightness, LV_ANIM_OFF);
    }
}

void ScreenManager::colorTempBarEvent(lv_event_t* e) {
    lv_obj_t* bar = lv_event_get_target(e);
    lv_point_t point;
    lv_indev_get_point(lv_indev_get_act(), &point);
    
    lv_coord_t bar_x = lv_obj_get_x(bar);
    lv_coord_t bar_width = lv_obj_get_width(bar);
    lv_coord_t touch_x = point.x - bar_x;
    
    if (touch_x >= 0 && touch_x <= bar_width) {
        int new_color_temp = MIN_COLOR_TEMP + (touch_x * (MAX_COLOR_TEMP - MIN_COLOR_TEMP)) / bar_width;
        if (new_color_temp < MIN_COLOR_TEMP) new_color_temp = MIN_COLOR_TEMP;
        if (new_color_temp > MAX_COLOR_TEMP) new_color_temp = MAX_COLOR_TEMP;
        
        mqttHandler.setLightColorTemp(new_color_temp);
        lv_bar_set_value(bar, new_color_temp, LV_ANIM_OFF);
    }
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
        
        // For now just print to console - will need to redraw ASCII art
        Serial.print("New temp: ");
        Serial.println((int)newTemp);
    }
}

void ScreenManager::hvacTempDownButtonEvent(lv_event_t* e) {
    float newTemp = hvacState.targetTemp - 1.0f;
    if (newTemp >= MIN_TEMPERATURE) {
        mqttHandler.setHVACTemperature(newTemp);
        
        // For now just print to console - will need to redraw ASCII art
        Serial.print("New temp: ");
        Serial.println((int)newTemp);
    }
}