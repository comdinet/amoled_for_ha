#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include <lvgl.h>
#include "config.h"

// Forward declarations
class MQTTHandler;

// Screen element structures
struct LightElements {
    lv_obj_t* powerButton = nullptr;
    lv_obj_t* brightnessLabel = nullptr;
    lv_obj_t* brightnessBar = nullptr;
    lv_obj_t* colorTempLabel = nullptr;
    lv_obj_t* colorTempBar = nullptr;
};

struct HVACElements {
    lv_obj_t* offButton = nullptr;
    lv_obj_t* coolButton = nullptr;
    lv_obj_t* tempDownButton = nullptr;
    lv_obj_t* tempUpButton = nullptr;
    lv_obj_t* targetTempValueLabel = nullptr;
};

class ScreenManager {
public:
    void init();
    void showScreen(ScreenType screen);
    void nextScreen();
    void previousScreen();
    void updateLightStatus();
    void updateHVACStatus();
    void update();
    
    ScreenType getCurrentScreen() const { return currentScreen; }
    
    // Static callback functions
    static void lightPowerButtonEvent(lv_event_t* e);
    static void brightnessBarEvent(lv_event_t* e);
    static void colorTempBarEvent(lv_event_t* e);
    static void hvacOffButtonEvent(lv_event_t* e);
    static void hvacCoolButtonEvent(lv_event_t* e);
    static void hvacTempUpButtonEvent(lv_event_t* e);
    static void hvacTempDownButtonEvent(lv_event_t* e);
    static void gestureEventHandler(lv_event_t* e);
    
    static ScreenManager* instance;
    
    LightElements lightElements;
    HVACElements hvacElements;

private:
    lv_obj_t* screenContainer = nullptr;
    lv_obj_t* screens[SCREEN_COUNT];
    ScreenType currentScreen = SCREEN_LIGHT;
    bool screensCreated = false;
    
    void createAllScreens();
    void createLightScreen();
    void createHVACScreen();
    void setupGestureHandling();
    
    // Helper functions for creating UI elements
    lv_obj_t* createButton(lv_obj_t* parent, const char* text, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h);
    lv_obj_t* createLabel(lv_obj_t* parent, const char* text, lv_coord_t x, lv_coord_t y);
    lv_obj_t* createBar(lv_obj_t* parent, lv_coord_t x, lv_coord_t y, lv_coord_t w, int min_val, int max_val, int default_val);
};

#endif // SCREEN_MANAGER_H