#include <rtthread.h>
#include <rtdevice.h>
#define DBG_TAG "PINOUT"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>
#include "drv_common.h"
#include "drv_gpio.h"

#include "pinout.h"

static int app_gpio_init(void)
{
    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(LED0_PIN, PIN_HIGH);

    rt_pin_mode(LED1_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(LED1_PIN, PIN_HIGH);

    rt_pin_mode(USER_KEY_PIN, PIN_MODE_INPUT);

    rt_pin_mode(RTC_WKUP_PIN, PIN_MODE_INPUT);
    if (rt_pin_read(RTC_WKUP_PIN) == PIN_LOW)
        LOG_I("rtc wkup low");
    else
        LOG_I("rtc wkup high"); // XXX

    return RT_EOK;
}

INIT_COMPONENT_EXPORT(app_gpio_init);
