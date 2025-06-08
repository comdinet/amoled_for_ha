#ifndef CONFIG_H
#define CONFIG_H

// Device Identity
#define DEVICE_NAME "SimRoom_Control"
#define DEVICE_FRIENDLY_NAME "SimRoom Control Panel"

// WiFi Configuration
#define WIFI_SSID "Iris 2.4"
#define WIFI_PASSWORD "038770590"

// Home Assistant/MQTT Configuration
#define MQTT_SERVER "192.168.1.50"
#define MQTT_PORT 1883
#define MQTT_USER ""
#define MQTT_PASSWORD ""

// Home Assistant Entity IDs
#define LIGHT_ENTITY_ID "light.yeelight_ceiling_0x19fdaa55"
#define HVAC_ENTITY_ID "climate.khdr_shynh"

// MQTT Topics
#define LIGHT_STATE_TOPIC "homeassistant/light/" LIGHT_ENTITY_ID "/state"
#define LIGHT_COMMAND_TOPIC "homeassistant/light/" LIGHT_ENTITY_ID "/set"
#define HVAC_STATE_TOPIC "homeassistant/climate/" HVAC_ENTITY_ID "/state"
#define HVAC_COMMAND_TOPIC "homeassistant/climate/" HVAC_ENTITY_ID "/set"
#define DEVICE_STATUS_TOPIC "homeassistant/sensor/" DEVICE_NAME "/state"

// Display Configuration
#define SCREEN_WIDTH 536
#define SCREEN_HEIGHT 240
#define SCREEN_ROTATION 1
#define EXAMPLE_USE_TOUCH 1

// Hardware Pins
#define TFT_CS    10
#define TFT_DC    11
#define TFT_RST   12
#define TFT_MOSI  13
#define TFT_CLK   14

#define TOUCH_SDA 38
#define TOUCH_SCL 39
#define TOUCH_INT 40
#define TOUCH_RST 41

#define STATUS_LED_PIN 2

// Screen Configuration
enum ScreenType {
    SCREEN_LIGHT = 0,
    SCREEN_HVAC = 1,
    SCREEN_COUNT
};

const char* SCREEN_NAMES[] = {
    "Light Control",
    "HVAC Control"
};

// Timing
#define MQTT_RECONNECT_INTERVAL 5000
#define WIFI_RECONNECT_INTERVAL 10000
#define STATUS_UPDATE_INTERVAL 30000

// Control Limits
#define MIN_BRIGHTNESS 1
#define MAX_BRIGHTNESS 100
#define MIN_COLOR_TEMP 2700
#define MAX_COLOR_TEMP 6000
#define MIN_TEMPERATURE 16
#define MAX_TEMPERATURE 30

// HVAC Modes
const char* HVAC_MODES[] = {
    "off",
    "heat", 
    "cool",
    "auto"
};
#define HVAC_MODE_COUNT 4

// Display
#define SCREEN_BUFFER_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10)

// Colors
#define COLOR_PRIMARY   0x2196F3
#define COLOR_SECONDARY 0x03DAC6
#define COLOR_SUCCESS   0x4CAF50
#define COLOR_WARNING   0xFF9800
#define COLOR_ERROR     0xF44336
#define COLOR_BACKGROUND 0x121212
#define COLOR_SURFACE   0x1E1E1E
#define COLOR_ON_SURFACE 0xFFFFFF

#endif