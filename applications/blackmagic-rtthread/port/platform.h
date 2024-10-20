#ifndef RT_BLACKMAGIC_PLATFORM_H
#define RT_BLACKMAGIC_PLATFORM_H

#include <sys/times.h>
#include <time.h>
#include <strings.h>
#include <stdio.h>

#include <rtthread.h>
#include <rtdevice.h>
#include <drv_gpio.h>
#include "pinout.h"

extern bool running_status;

#define PLATFORM_IDENT "rt-thread "

#define LED_IDLE_RUN LED0_PIN

/* target reset pin */
#define NRST_IN_PIN  TARGET_RST_IN_PIN
#define NRST_OUT_PIN TARGET_RST_PIN

/* target swd pins */
#define SWCLK_PIN TARGET_SWCLK_PIN
#define SWDIO_PIN TARGET_SWDIO_PIN
#define SWO_PIN   TARGET_SWO_PIN

/* direction for the level shifters */
#define SWCLK_DIR_PIN TARGET_SWCLK_DIR_PIN
#define SWDIO_DIR_PIN TARGET_SWDIO_DIR_PIN

/* jtag port */
#define TCK_PIN     TARGET_SWCLK_PIN
#define TMS_PIN     TARGET_SWDIO_PIN
#define TDI_PIN     TARGET_TDI_PIN
#define TDO_PIN     TARGET_SWO_PIN
#define TCK_DIR_PIN TARGET_SWCLK_DIR_PIN
#define TMS_DIR_PIN TARGET_SWDIO_DIR_PIN

/* dummy port definitions */
#define NRST_PORT  0
#define SWDIO_PORT 0
#define SWCLK_PORT 0
#define TCK_PORT   0
#define TMS_PORT   0
#define TDI_PORT   0
#define TDO_PORT   0

/* switch pins from read to write */
#define TMS_SET_MODE()                         \
	{                                          \
		rt_pin_mode(TMS_PIN, PIN_MODE_OUTPUT); \
		rt_pin_write(TMS_DIR_PIN, PIN_HIGH);   \
	}
#define SWDIO_MODE_FLOAT()                      \
	{                                           \
		rt_pin_mode(SWDIO_PIN, PIN_MODE_INPUT); \
		rt_pin_write(TMS_DIR_PIN, PIN_LOW);     \
	}
#define SWDIO_MODE_DRIVE()                       \
	{                                            \
		rt_pin_mode(SWDIO_PIN, PIN_MODE_OUTPUT); \
		rt_pin_write(TMS_DIR_PIN, PIN_HIGH);     \
	}

/* activity led */
#ifdef LED_IDLE_RUN
#define SET_RUN_STATE(state)                                    \
	{                                                           \
		rt_pin_write(LED_IDLE_RUN, state ? PIN_LOW : PIN_HIGH); \
		running_status = (state);                               \
	}
#define SET_IDLE_STATE(state)                                   \
	{                                                           \
		rt_pin_write(LED_IDLE_RUN, state ? PIN_HIGH : PIN_LOW); \
	}
#else
#define SET_RUN_STATE(state)      \
	{                             \
		running_status = (state); \
	}
#define SET_IDLE_STATE(state) \
	{                         \
	}
#endif
#define SET_ERROR_STATE(state)

/* libopencm3 to rt-thread */
#define gpio_clear(port, pin)          rt_pin_write(pin, PIN_LOW)
#define gpio_set(port, pin)            rt_pin_write(pin, PIN_HIGH);
#define gpio_set_val(port, pin, value) rt_pin_write(pin, value ? PIN_HIGH : PIN_LOW)
#define gpio_get(port, pin)            rt_pin_read(pin)

/* display */
extern void display_target_name(void);

#endif
