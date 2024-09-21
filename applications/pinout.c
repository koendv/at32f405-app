#include <rtthread.h>
#include <rtdevice.h>
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
        rt_kprintf("rtc wkup low\r\n");
    else
        rt_kprintf("rtc wkup high\r\n"); // XXX

    return RT_EOK;
}

INIT_COMPONENT_EXPORT(app_gpio_init);
