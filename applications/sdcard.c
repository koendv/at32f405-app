#include <stdbool.h>
#include <rtdevice.h>
#include <drv_spi.h>
#include <dev_spi_msd.h>
#include <dfs_fs.h>
#include "board.h"
#include "pinout.h"

bool sdcard_mounted = false;

static int sdcard_mount()
{
    rt_err_t err;
    bool     sd_detect = rt_pin_read(SD_DETECT_PIN) == PIN_LOW;

    if (sd_detect)
    {
        if (sdcard_mounted) return RT_EOK;

        rt_pin_write(SD_CS_PIN, PIN_HIGH); // sdcard chip select off
        rt_thread_mdelay(10);
        rt_pin_write(SD_ON_PIN, PIN_HIGH); // sdcard power on
        rt_thread_mdelay(10);
        rt_pin_write(SD_CS_PIN, PIN_LOW);  // sdcard chip select on
        rt_thread_mdelay(10);

        err = rt_hw_spi_device_attach("spi2", "spi20", GPIOF, GPIO_PINS_7);
        if (err != RT_EOK)
        {
            rt_kprintf("spi attach fail\r\n");
            return -RT_ERROR;
        }

        err = msd_init("sd0", "spi20");
        if (err != RT_EOK)
        {
            rt_kprintf("sd card msd init fail\r\n");
            return -RT_ERROR;
        }

        err = dfs_mount("sd0", "/", "elm", 0, 0);
        if (err != RT_EOK)
        {
            rt_kprintf("[E/SD] sd card mount fail\r\n");
            return -RT_ERROR;
        }

        sdcard_mounted = true;

        rt_kprintf("[I/SD] sd card mounted\r\n");
    }
    else
    {
        if (sdcard_mounted)
        {
            rt_pin_write(SD_CS_PIN, PIN_HIGH); // sdcard chip select off
            rt_thread_mdelay(10);
            rt_pin_write(SD_ON_PIN, PIN_LOW);  // sdcard power off

                                               // XXX unmount

            sdcard_mounted = false;
        }
    }

    return RT_EOK;
}

int app_sdcard_init(void)
{
    rt_pin_mode(SD_CS_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(SD_CS_PIN, PIN_HIGH);

    rt_pin_mode(SD_ON_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(SD_ON_PIN, PIN_LOW);

    rt_pin_mode(SD_DETECT_PIN, PIN_MODE_INPUT_PULLUP);

    sdcard_mount();

    return RT_EOK;
}

MSH_CMD_EXPORT(sdcard_mount, set up sd card);
//INIT_COMPONENT_EXPORT(app_sdcard_init); // XXX
