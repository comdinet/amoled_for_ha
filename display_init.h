#ifndef DISPLAY_INIT_H
#define DISPLAY_INIT_H

#include <lvgl.h>
#include "config.h"

bool initDisplay();
void displayFlush(lv_display_t *display, const lv_area_t *area, uint8_t *color_p);
void touchRead(lv_indev_t *indev_drv, lv_indev_data_t *data);

#if EXAMPLE_USE_TOUCH
bool initTouch();
bool getTouchPoint(uint16_t *x, uint16_t *y);
#endif

#endif