#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include <lvgl.h>
#include "config.h"

class ScreenManager {
public:
    void init();
    
    void showScreen(ScreenType screen);
    ScreenType getCurrentScreen() const { return currentScreen; }
    void nextScreen();
    void previousScreen();
    
    void updateLightStatus();
    void updateHVACStatus();
    
    void update();
    
private:
    ScreenType currentScreen = SCREEN_LIGHT;
    lv_obj_t* screenContainer;
    lv_obj_t* screens[SCREEN_COUNT];
    bool screensCreated = false;
    
    void createAllScreens();
    void createLightScreen();
    void createHVACScreen();
    
    lv_obj_t* createButton(lv_obj_t* parent, const char* text, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h);
    lv_obj_t* createLabel(lv_obj_t* parent, const char* text, lv_coord_t x, lv_coord_t y);
    lv_obj_t* createSlider(lv_obj_t* parent, lv_coord_t x, lv_coord_t y, lv_coord_t w, int min_val, int max_val, int default_val);
    
    void setupGestureHandling();
    static void gestureEventHandler(lv_event_t* e);
    
    struct LightScreenElements {
        lv_obj_t* powerButton;
        lv_obj_t* statusLabel;
        lv_obj_t* brightnessSlider;
        lv_obj_t* brightnessLabel;
        lv_obj_t* colorTempSlider;
        lv_obj_t* colorTempLabel;
        lv_obj_t* brightnessValueLabel;
        lv_obj_t* colorTempValueLabel;
    } lightElements;
    
    struct HVACScreenElements {
        lv_obj_t* powerButton;
        lv_obj_t* statusLabel;
        lv_obj_t* currentTempLabel;
        lv_obj_t* targetTempLabel;
        lv_obj_t* tempUpButton;
        lv_obj_t* tempDownButton;
        lv_obj_t* modeButton;
        lv_obj_t* targetTempValueLabel;
    } hvacElements;
    
    static void lightPowerButtonEvent(lv_event_t* e);
    static void lightBrightnessSliderEvent(lv_event_t* e);
    static void lightColorTempSliderEvent(lv_event_t* e);
    
    static void hvacPowerButtonEvent(lv_event_t* e);
    static void hvacTempUpButtonEvent(lv_event_t* e);
    static void hvacTempDownButtonEvent(lv_event_t* e);
    static void hvacModeButtonEvent(lv_event_t* e);
    
    static ScreenManager* instance;
};

#endif