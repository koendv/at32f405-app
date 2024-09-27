#ifndef FREEDAP_HAL_CONFIG_H
#define FREEDAP_HAL_CONFIG_H

#include <rtthread.h>
#include <rtconfig.h>
#include <drv_gpio.h>
#include <pinout.h>

#define SWCLK_PIN    TARGET_SWCLK_PIN
#define SWDIO_PIN    TARGET_SWDIO_PIN
#define TDI_PIN      TARGET_TDI_PIN
#define TDO_PIN      TARGET_SWO_PIN
#define TCK_DIR_PIN  TARGET_SWCLK_DIR_PIN
#define SWCLK_DIR_PIN  TARGET_SWCLK_DIR_PIN
#define TMS_DIR_PIN  TARGET_SWDIO_DIR_PIN
#define NRST_IN_PIN  TARGET_RST_IN_PIN
#define NRST_OUT_PIN TARGET_RST_PIN

static inline void HAL_GPIO_SWCLK_TCK_write(int value)
{
    rt_pin_write(SWCLK_PIN, value ? PIN_HIGH : PIN_LOW);
}

static inline void HAL_GPIO_SWCLK_TCK_set()
{
    rt_pin_write(SWCLK_PIN, PIN_HIGH);
}

static inline void HAL_GPIO_SWCLK_TCK_clr()
{
    rt_pin_write(SWCLK_PIN, PIN_LOW);
}

static inline int HAL_GPIO_SWCLK_TCK_read()
{
    return rt_pin_read(SWCLK_PIN);
}

static inline void HAL_GPIO_SWCLK_TCK_out()
{
    rt_pin_write(SWCLK_DIR_PIN, PIN_HIGH);
    rt_pin_mode(SWCLK_PIN, PIN_MODE_OUTPUT);
}

static inline void HAL_GPIO_SWCLK_TCK_in()
{
    rt_pin_mode(SWCLK_PIN, PIN_MODE_INPUT);
    rt_pin_write(SWCLK_DIR_PIN, PIN_LOW);
}

static inline void HAL_GPIO_SWDIO_TMS_write(int value)
{
    rt_pin_write(SWDIO_PIN, value ? PIN_HIGH : PIN_LOW);
}

static inline void HAL_GPIO_SWDIO_TMS_set()
{
    rt_pin_write(SWDIO_PIN, PIN_HIGH);
}

static inline int HAL_GPIO_SWDIO_TMS_read()
{
    return rt_pin_read(SWDIO_PIN);
}

static inline void HAL_GPIO_SWDIO_TMS_out()
{
    rt_pin_mode(SWDIO_PIN, PIN_MODE_OUTPUT);
}

static inline void HAL_GPIO_SWDIO_TMS_in()
{
    rt_pin_mode(SWDIO_PIN, PIN_MODE_INPUT);
}

static inline void HAL_GPIO_nRESET_write(int value)
{
    rt_pin_write(NRST_OUT_PIN, value ? PIN_HIGH : PIN_LOW);
}

static inline void HAL_GPIO_nRESET_set()
{
    rt_pin_write(NRST_OUT_PIN, PIN_HIGH);
}

static inline int HAL_GPIO_nRESET_read()
{
    return rt_pin_read(NRST_IN_PIN);
}

static inline void HAL_GPIO_nRESET_out()
{
    rt_pin_mode(NRST_OUT_PIN, PIN_MODE_OUTPUT);
}

static inline void HAL_GPIO_nRESET_in()
{
    return rt_pin_mode(NRST_OUT_PIN, PIN_MODE_INPUT);
}

#ifdef DAP_CONFIG_ENABLE_JTAG
static inline void HAL_GPIO_TDI_write(int value)
{
    rt_pin_write(TDI_PIN, value ? PIN_HIGH : PIN_LOW);
}

static inline void HAL_GPIO_TDI_set()
{
    rt_pin_write(TDI_PIN, PIN_HIGH);
}

static inline int HAL_GPIO_TDI_read()
{
    return rt_pin_read(TDI_PIN);
}

static inline void HAL_GPIO_TDI_in()
{
    rt_pin_mode(TDI_PIN, PIN_MODE_INPUT);
}

static inline void HAL_GPIO_TDI_out()
{
    rt_pin_mode(TDI_PIN, PIN_MODE_OUTPUT);
}

static inline void HAL_GPIO_TDO_write(int value)
{
    rt_pin_write(TDO_PIN, value ? PIN_HIGH : PIN_LOW);
}

static inline int HAL_GPIO_TDO_read()
{
    return rt_pin_read(TDO_PIN);
}

static inline void HAL_GPIO_TDO_in()
{
    rt_pin_mode(TDO_PIN, PIN_MODE_INPUT);
}

static inline void HAL_GPIO_TDO_out()
{
    rt_pin_mode(TDO_PIN, PIN_MODE_OUTPUT);
}
#endif

static inline void DAP_config(void)
{
#ifdef TARGET_OE_PIN
    rt_pin_write(TARGET_OE_PIN, PIN_LOW); // enable SWD/JTAG output
    rt_pin_mode(TARGET_OE_PIN, PIN_MODE_OUTPUT);
#endif

#ifdef SWCLK_DIR_PIN
    rt_pin_write(SWCLK_DIR_PIN, PIN_LOW);
    rt_pin_mode(SWCLK_DIR_PIN, PIN_MODE_OUTPUT);
#endif

#ifdef SWDIO_DIR_PIN
    rt_pin_write(SWDIO_DIR_PIN, PIN_LOW);
    rt_pin_mode(SWDIO_DIR_PIN, PIN_MODE_OUTPUT);
#endif

#ifdef NRST_OUT_PIN
    rt_pin_mode(NRST_OUT_PIN, PIN_MODE_OUTPUT);
#endif

#ifdef NRST_IN_PIN
    rt_pin_mode(NRST_IN_PIN, PIN_MODE_INPUT);
#endif

#ifdef TARGET_IO0_PIN
    rt_pin_mode(TARGET_IO0_PIN, PIN_MODE_INPUT);
#endif
}

#endif
