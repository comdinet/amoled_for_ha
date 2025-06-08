#include "display_init.h"
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

static lv_display_t *display;
static lv_color_t *buf1;
static lv_color_t *buf2;

#if EXAMPLE_USE_TOUCH
static lv_indev_t *touch_indev;
#define TOUCH_I2C_ADDR 0x15
#endif

const uint8_t lcd_init_cmds[] = {
    0x11, 0,
    0xFF, 120,
    0x21, 0,
    0x3A, 1, 0x55,
    0x36, 1, 0x00,
    0x29, 0,
    0x00
};

void sendCommand(uint8_t cmd) {
    digitalWrite(TFT_DC, LOW);
    digitalWrite(TFT_CS, LOW);
    SPI.transfer(cmd);
    digitalWrite(TFT_CS, HIGH);
}

void sendData(uint8_t data) {
    digitalWrite(TFT_DC, HIGH);
    digitalWrite(TFT_CS, LOW);
    SPI.transfer(data);
    digitalWrite(TFT_CS, HIGH);
}

void sendData16(uint16_t data) {
    digitalWrite(TFT_DC, HIGH);
    digitalWrite(TFT_CS, LOW);
    SPI.transfer16(data);
    digitalWrite(TFT_CS, HIGH);
}

void setAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    sendCommand(0x2A);
    sendData16(x0);
    sendData16(x1);
    
    sendCommand(0x2B);
    sendData16(y0);
    sendData16(y1);
    
    sendCommand(0x2C);
}

bool initDisplay() {
    SPI.begin(TFT_CLK, -1, TFT_MOSI, TFT_CS);
    SPI.setFrequency(80000000);
    SPI.setDataMode(SPI_MODE0);
    
    pinMode(TFT_CS, OUTPUT);
    pinMode(TFT_DC, OUTPUT);
    pinMode(TFT_RST, OUTPUT);
    
    digitalWrite(TFT_CS, HIGH);
    digitalWrite(TFT_DC, HIGH);
    
    digitalWrite(TFT_RST, LOW);
    delay(100);
    digitalWrite(TFT_RST, HIGH);
    delay(100);
    
    const uint8_t *cmd = lcd_init_cmds;
    while (*cmd != 0x00) {
        uint8_t command = *cmd++;
        uint8_t numArgs = *cmd++;
        
        if (command == 0xFF) {
            delay(numArgs);
        } else {
            sendCommand(command);
            for (int i = 0; i < numArgs; i++) {
                sendData(*cmd++);
            }
        }
    }
    
    lv_init();
    
    buf1 = (lv_color_t*)heap_caps_malloc(SCREEN_BUFFER_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    buf2 = (lv_color_t*)heap_caps_malloc(SCREEN_BUFFER_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    
    if (!buf1 || !buf2) {
        Serial.println("Failed to allocate display buffers!");
        return false;
    }
    
    display = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!display) {
        Serial.println("Failed to create LVGL display!");
        return false;
    }
    
    lv_display_set_buffers(display, buf1, buf2, SCREEN_BUFFER_SIZE * sizeof(lv_color_t), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, displayFlush);

#if EXAMPLE_USE_TOUCH
    if (!initTouch()) {
        Serial.println("Touch init failed, continuing without touch");
    } else {
        touch_indev = lv_indev_create();
        lv_indev_set_type(touch_indev, LV_INDEV_TYPE_POINTER);
        lv_indev_set_read_cb(touch_indev, touchRead);
        Serial.println("Touch initialized");
    }
#endif
    
    Serial.println("Display initialization completed");
    return true;
}

void displayFlush(lv_display_t *disp, const lv_area_t *area, uint8_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    
    setAddressWindow(area->x1, area->y1, area->x2, area->y2);
    
    digitalWrite(TFT_DC, HIGH);
    digitalWrite(TFT_CS, LOW);
    
    uint32_t pixels = w * h;
    uint16_t *color16 = (uint16_t*)color_p;
    for (uint32_t i = 0; i < pixels; i++) {
        SPI.transfer16(color16[i]);
    }
    
    digitalWrite(TFT_CS, HIGH);
    
    lv_display_flush_ready(disp);
}

#if EXAMPLE_USE_TOUCH

bool initTouch() {
    Wire.begin(TOUCH_SDA, TOUCH_SCL);
    Wire.setClock(400000);
    
    if (TOUCH_RST >= 0) {
        pinMode(TOUCH_RST, OUTPUT);
        digitalWrite(TOUCH_RST, LOW);
        delay(10);
        digitalWrite(TOUCH_RST, HIGH);
        delay(50);
    }
    
    Wire.beginTransmission(TOUCH_I2C_ADDR);
    uint8_t error = Wire.endTransmission();
    
    if (error == 0) {
        Serial.println("Touch controller found");
        return true;
    } else {
        Serial.printf("Touch controller not found (error: %d)\n", error);
        return false;
    }
}

bool getTouchPoint(uint16_t *x, uint16_t *y) {
    Wire.beginTransmission(TOUCH_I2C_ADDR);
    Wire.write(0x01);
    if (Wire.endTransmission() != 0) {
        return false;
    }
    
    Wire.requestFrom(TOUCH_I2C_ADDR, 6);
    if (Wire.available() < 6) {
        return false;
    }
    
    uint8_t data[6];
    for (int i = 0; i < 6; i++) {
        data[i] = Wire.read();
    }
    
    if ((data[0] & 0x0F) == 0) {
        return false;
    }
    
    *x = ((data[1] & 0x0F) << 8) | data[2];
    *y = ((data[3] & 0x0F) << 8) | data[4];
    
    if (SCREEN_ROTATION == 1) {
        uint16_t temp = *x;
        *x = SCREEN_WIDTH - *y;
        *y = temp;
    }
    
    return true;
}

void touchRead(lv_indev_t *indev_drv, lv_indev_data_t *data) {
    uint16_t x, y;
    
    if (getTouchPoint(&x, &y)) {
        data->state = LV_INDEV_STATE_PRESSED;
        data->point.x = x;
        data->point.y = y;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

#else

void touchRead(lv_indev_t *indev_drv, lv_indev_data_t *data) {
    data->state = LV_INDEV_STATE_RELEASED;
}

#endif