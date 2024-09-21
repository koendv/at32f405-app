#ifndef LVGL_TOUCHPAD_H
#define LVGL_TOUCHPAD_H

/*
 * touchpad driver.
 *
 * click        sends LV_EVENT_CLICKED
 * double click sends LV_EVENT_DOUBLE_CLICKED
 * long press   sends LV_EVENT_LONG_PRESSED
 * swipe up/down/left/right sends LV_EVENT_GESTURE with LV_DIR_TOP, LV_DIR_BOTTOM, LV_DIR_LEFT or LV_DIR_RIGHT.
 */

/* touchpad_events: send queued touchpad events to lvgl */
void touchpad_events(void);

#endif
