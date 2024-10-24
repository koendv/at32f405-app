/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-10-18     shelton      first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include "drv_common.h"
#include "drv_gpio.h"
#include "usbd_core.h"

#include "pinout.h"
#include "app.h"
#include "ds3231_util.h"

int main(void)
{
    rt_kprintf("boot\r\n");
    //ds3231_sync();
    app_sdcard_init();
    app_blackmagic_init();

    while (1)
    {
        rt_pin_write(LED1_PIN, PIN_LOW);
        rt_thread_mdelay(500);
        rt_pin_write(LED1_PIN, PIN_HIGH);
        rt_thread_mdelay(4500);
    }
}
