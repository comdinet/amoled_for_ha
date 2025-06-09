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

// Display Configuration - Updated for ESP32-S3-AMOLED-1.91
#define SCREEN_WIDTH 536
#define SCREEN_HEIGHT 240
#define SCREEN_ROTATION 1
#define EXAMPLE_USE_TOUCH 1

// Hardware Pins - Corrected for ESP32-S3-AMOLED-1.91
#define EXAMPLE_PIN_NUM_LCD_CS            6
#define EXAMPLE_PIN_NUM_LCD_PCLK          47
#define EXAMPLE_PIN_NUM_LCD_DATA0         18
#define EXAMPLE_PIN_NUM_LCD_DATA1         7
#define EXAMPLE_PIN_NUM_LCD_DATA2         48
#define EXAMPLE_PIN_NUM_LCD_DATA3         5
#define EXAMPLE_PIN_NUM_LCD_RST           17
#define EXAMPLE_PIN_NUM_BK_LIGHT          (-1)

#define EXAMPLE_PIN_NUM_TOUCH_SCL         39
#define EXAMPLE_PIN_NUM_TOUCH_SDA         40
#define EXAMPLE_PIN_NUM_TOUCH_RST         (-1)
#define EXAMPLE_PIN_NUM_TOUCH_INT         (-1)

#define STATUS_LED_PIN 2

// Screen Configuration
enum ScreenType {
    SCREEN_LIGHT = 0,
    SCREEN_HVAC = 1,
    SCREEN_COUNT
};

extern const char* SCREEN_NAMES[];

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
extern const char* HVAC_MODES[];
#define HVAC_MODE_COUNT 4

// Display - Using proper SH8601 configuration
#define LCD_HOST    SPI2_HOST
#define TOUCH_HOST  I2C_NUM_0
#define LCD_BIT_PER_PIXEL       16
#define EXAMPLE_LVGL_BUF_HEIGHT        (SCREEN_HEIGHT/4)
#define EXAMPLE_LVGL_TICK_PERIOD_MS    2
#define EXAMPLE_LVGL_TASK_MAX_DELAY_MS 500
#define EXAMPLE_LVGL_TASK_MIN_DELAY_MS 1
#define EXAMPLE_LVGL_TASK_STACK_SIZE   (4 * 1024)
#define EXAMPLE_LVGL_TASK_PRIORITY     2

#define I2C_ADDR_FT3168 0x38

// Colors - Fixed hex values
#define COLOR_PRIMARY   0x2196F3
#define COLOR_SECONDARY 0x03DAC6
#define COLOR_SUCCESS   0x4CAF50
#define COLOR_WARNING   0xFF9800
#define COLOR_ERROR     0xF44336
#define COLOR_BACKGROUND 0x000000  // Pure black instead of 0x121212
#define COLOR_SURFACE   0x1E1E1E
#define COLOR_ON_SURFACE 0xFFFFFF

#endif