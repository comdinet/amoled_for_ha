#include "display_init.h"
#include <Arduino.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/spi_master.h"
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_lcd_sh8601.h"

static const char *TAG = "display_init";
static SemaphoreHandle_t lvgl_mux = NULL;
static esp_lcd_panel_handle_t panel_handle = NULL;

#if EXAMPLE_USE_TOUCH
static lv_indev_t *touch_indev;
#endif

// LCD initialization commands for SH8601
static const sh8601_lcd_init_cmd_t lcd_init_cmds[] = {
    {0x11, (uint8_t []){0x00}, 0, 120},   
    {0x36, (uint8_t []){0xF0}, 1, 0},   
    {0x3A, (uint8_t []){0x55}, 1, 0},  //16bits-RGB565
    {0x2A, (uint8_t []){0x00,0x00,0x02,0x17}, 4, 0}, 
    {0x2B, (uint8_t []){0x00,0x00,0x00,0xEF}, 4, 0},
    {0x51, (uint8_t []){0x00}, 1, 10},
    {0x29, (uint8_t []){0x00}, 0, 10},
    {0x51, (uint8_t []){0xFF}, 1, 0},
};

static bool example_notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_disp_drv_t *disp_driver = (lv_disp_drv_t *)user_ctx;
    lv_disp_flush_ready(disp_driver);
    return false;
}

void displayFlushCb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) drv->user_data;
    const int offsetx1 = area->x1;
    const int offsetx2 = area->x2;
    const int offsety1 = area->y1;
    const int offsety2 = area->y2;

    // copy a buffer's content to a specific area of the display
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);
}

static void displayUpdateCallback(lv_disp_drv_t *drv)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) drv->user_data;

    switch (drv->rotated)
    {
        case LV_DISP_ROT_NONE:
            esp_lcd_panel_swap_xy(panel_handle, false);
            esp_lcd_panel_mirror(panel_handle, true, false);
            break;
        case LV_DISP_ROT_90:
            esp_lcd_panel_swap_xy(panel_handle, true);
            esp_lcd_panel_mirror(panel_handle, true, true);
            break;
        case LV_DISP_ROT_180:
            esp_lcd_panel_swap_xy(panel_handle, false);
            esp_lcd_panel_mirror(panel_handle, false, true);
            break;
        case LV_DISP_ROT_270:
            esp_lcd_panel_swap_xy(panel_handle, true);
            esp_lcd_panel_mirror(panel_handle, false, false);
            break;
    }
}

void displayRounderCallback(struct _lv_disp_drv_t *disp_drv, lv_area_t *area)
{
    uint16_t x1 = area->x1;
    uint16_t x2 = area->x2;
    uint16_t y1 = area->y1;
    uint16_t y2 = area->y2;

    // round the start of coordinate down to the nearest 2M number
    area->x1 = (x1 >> 1) << 1;
    area->y1 = (y1 >> 1) << 1;
    // round the end of coordinate up to the nearest 2N+1 number
    area->x2 = ((x2 >> 1) << 1) + 1;
    area->y2 = ((y2 >> 1) << 1) + 1;
}

#if EXAMPLE_USE_TOUCH
bool initTouch() {
    i2c_config_t i2c_conf;
    i2c_conf.mode = I2C_MODE_MASTER;
    i2c_conf.sda_io_num = EXAMPLE_PIN_NUM_TOUCH_SDA;
    i2c_conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_conf.scl_io_num = EXAMPLE_PIN_NUM_TOUCH_SCL;
    i2c_conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_conf.master.clk_speed = 300 * 1000;
    
    ESP_ERROR_CHECK(i2c_param_config(TOUCH_HOST, &i2c_conf));
    ESP_ERROR_CHECK(i2c_driver_install(TOUCH_HOST, i2c_conf.mode, 0, 0, 0));

    uint8_t data = 0x00;
    esp_err_t ret = i2c_master_write_to_device(TOUCH_HOST, I2C_ADDR_FT3168, &data, 1, 1000);
    
    if (ret == ESP_OK) {
        Serial.println("Touch controller initialized");
        return true;
    } else {
        Serial.printf("Touch controller init failed: %d\n", ret);
        return false;
    }
}

bool getTouchPoint(uint16_t *x, uint16_t *y) {
    uint8_t data;
    uint8_t buf[4];
    esp_err_t ret;
    
    uint8_t reg = 0x02;
    ret = i2c_master_write_read_device(TOUCH_HOST, I2C_ADDR_FT3168, &reg, 1, &data, 1, 1000);
    if (ret != ESP_OK || !data) {
        return false;
    }
    
    reg = 0x03;
    ret = i2c_master_write_read_device(TOUCH_HOST, I2C_ADDR_FT3168, &reg, 1, buf, 4, 1000);
    if (ret != ESP_OK) {
        return false;
    }
    
    *y = (((uint16_t)buf[0] & 0x0f)<<8) | (uint16_t)buf[1];
    *x = (((uint16_t)buf[2] & 0x0f)<<8) | (uint16_t)buf[3];
    
    if(*x > SCREEN_WIDTH) *x = SCREEN_WIDTH;
    if(*y > SCREEN_HEIGHT) *y = SCREEN_HEIGHT;
    *y = SCREEN_HEIGHT - *y;
    
    return true;
}

void touchReadCb(lv_indev_drv_t *indev_drv, lv_indev_data_t *data) {
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
void touchReadCb(lv_indev_drv_t *indev_drv, lv_indev_data_t *data) {
    data->state = LV_INDEV_STATE_RELEASED;
}
#endif

// LVGL timing is now handled automatically via lv_conf.h
void lvgl_tick_task(void) {
    // With LV_TICK_CUSTOM set to use millis(), no manual tick increment needed
    // This function can be empty or removed
}

static bool lvglLock(int timeout_ms) {
    assert(lvgl_mux && "initDisplay must be called first");
    const TickType_t timeout_ticks = (timeout_ms == -1) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return xSemaphoreTake(lvgl_mux, timeout_ticks) == pdTRUE;
}

static void lvglUnlock(void) {
    assert(lvgl_mux && "initDisplay must be called first");
    xSemaphoreGive(lvgl_mux);
}

static void lvglPortTask(void *arg) {
    ESP_LOGI(TAG, "Starting LVGL task");
    uint32_t task_delay_ms = EXAMPLE_LVGL_TASK_MAX_DELAY_MS;
    while (1) {
        if (lvglLock(-1)) {
            task_delay_ms = lv_timer_handler();
            lvglUnlock();
        }
        if (task_delay_ms > EXAMPLE_LVGL_TASK_MAX_DELAY_MS) {
            task_delay_ms = EXAMPLE_LVGL_TASK_MAX_DELAY_MS;
        } else if (task_delay_ms < EXAMPLE_LVGL_TASK_MIN_DELAY_MS) {
            task_delay_ms = EXAMPLE_LVGL_TASK_MIN_DELAY_MS;
        }
        vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
    }
}

bool initDisplay() {
    static lv_disp_draw_buf_t disp_buf;
    static lv_disp_drv_t disp_drv;
    
    ESP_LOGI(TAG, "Initialize SPI bus");
    const spi_bus_config_t buscfg = SH8601_PANEL_BUS_QSPI_CONFIG(EXAMPLE_PIN_NUM_LCD_PCLK,
                                                                 EXAMPLE_PIN_NUM_LCD_DATA0,
                                                                 EXAMPLE_PIN_NUM_LCD_DATA1,
                                                                 EXAMPLE_PIN_NUM_LCD_DATA2,
                                                                 EXAMPLE_PIN_NUM_LCD_DATA3,
                                                                 SCREEN_WIDTH * SCREEN_HEIGHT * LCD_BIT_PER_PIXEL / 8);
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    ESP_LOGI(TAG, "Install panel IO");
    esp_lcd_panel_io_handle_t io_handle = NULL;
    const esp_lcd_panel_io_spi_config_t io_config = SH8601_PANEL_IO_QSPI_CONFIG(EXAMPLE_PIN_NUM_LCD_CS,
                                                                                  example_notify_lvgl_flush_ready,
                                                                                  &disp_drv);
    sh8601_vendor_config_t vendor_config = {
        .init_cmds = lcd_init_cmds,
        .init_cmds_size = sizeof(lcd_init_cmds) / sizeof(lcd_init_cmds[0]),
        .flags = {
            .use_qspi_interface = 1,
        },
    };
    
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = LCD_BIT_PER_PIXEL,
        .vendor_config = &vendor_config,
    };
    
    ESP_LOGI(TAG, "Install SH8601 panel driver");
    ESP_ERROR_CHECK(esp_lcd_new_panel_sh8601(io_handle, &panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

#if EXAMPLE_USE_TOUCH
    if (!initTouch()) {
        Serial.println("Touch init failed, continuing without touch");
    }
#endif

    ESP_LOGI(TAG, "Initialize LVGL library");
    lv_init();
    
    // Allocate draw buffers
    lv_color_t *buf1 = (lv_color_t*)heap_caps_malloc(SCREEN_WIDTH * EXAMPLE_LVGL_BUF_HEIGHT * sizeof(lv_color_t), MALLOC_CAP_DMA);
    if (!buf1) {
        Serial.println("Failed to allocate display buffer 1!");
        return false;
    }
    
    lv_color_t *buf2 = (lv_color_t*)heap_caps_malloc(SCREEN_WIDTH * EXAMPLE_LVGL_BUF_HEIGHT * sizeof(lv_color_t), MALLOC_CAP_DMA);
    if (!buf2) {
        Serial.println("Failed to allocate display buffer 2!");
        return false;
    }
    
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, SCREEN_WIDTH * EXAMPLE_LVGL_BUF_HEIGHT);

    ESP_LOGI(TAG, "Register display driver to LVGL");
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = SCREEN_WIDTH;
    disp_drv.ver_res = SCREEN_HEIGHT;
    disp_drv.flush_cb = displayFlushCb;
    disp_drv.rounder_cb = displayRounderCallback;
    disp_drv.drv_update_cb = displayUpdateCallback;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.user_data = panel_handle;
    lv_disp_t *disp = lv_disp_drv_register(&disp_drv);

    ESP_LOGI(TAG, "LVGL initialization completed - call lvgl_tick_task() from main loop");
    
    // Note: Call lvgl_tick_task() regularly from main loop for timing

#if EXAMPLE_USE_TOUCH
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.disp = disp;
    indev_drv.read_cb = touchReadCb;
    touch_indev = lv_indev_drv_register(&indev_drv);
    Serial.println("Touch input device registered");
#endif

    lvgl_mux = xSemaphoreCreateMutex();
    if (!lvgl_mux) {
        Serial.println("Failed to create LVGL mutex!");
        return false;
    }
    
    xTaskCreate(lvglPortTask, "LVGL", EXAMPLE_LVGL_TASK_STACK_SIZE, NULL, EXAMPLE_LVGL_TASK_PRIORITY, NULL);

    Serial.println("Display initialization completed");
    return true;
}