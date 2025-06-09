#ifndef DISPLAY_INIT_H
#define DISPLAY_INIT_H

#include <lvgl.h>
#include "config.h"

bool initDisplay();
void lvgl_tick_task(void);  // Call this regularly from main loop
void displayFlushCb(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
void touchReadCb(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);

#if EXAMPLE_USE_TOUCH
bool initTouch();
bool getTouchPoint(uint16_t *x, uint16_t *y);
#endif

#endif