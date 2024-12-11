#include <stdbool.h>
#include <rtdevice.h>
#include <drv_spi.h>
#include <dev_spi_msd.h>
#include <dfs_fs.h>
#include "board.h"
#include "pinout.h"

#define SD_DEVICE         "sd0"
#define SD_DIR            "/"
#define SD_SPI_BUS        "spi2"
#define SD_SPI_DEV        "spi20"
#define SD0_CS_GPIO       GPIOF
#define SD0_CS_GPIO_PIN   GPIO_PINS_7
#define SD_DEBOUNCE_TICKS (RT_TICK_PER_SECOND / 10)
#define SD_BIG_STACK      (5 * 1024)

bool              sdcard_mounted    = false;
static rt_timer_t sdcard_change_tim = RT_NULL;

static void sdcard_mount()
{
    rt_err_t err;
    bool     sd_detect = FALSE;

    sd_detect = rt_pin_read(SD_DETECT_PIN) == PIN_LOW;
    if (sd_detect)
    {
        if (sdcard_mounted)
            return;

        rt_kprintf("sdcard mount\r\n");

        rt_pin_write(SD_CS_PIN, PIN_HIGH); // sdcard chip select off
        rt_thread_mdelay(10);
        rt_pin_write(SD_ON_PIN, PIN_HIGH); // sdcard power on
        rt_thread_mdelay(10);
        rt_pin_write(SD_CS_PIN, PIN_LOW);  // sdcard chip select on
        rt_thread_mdelay(10);

        err = rt_hw_spi_device_attach(SD_SPI_BUS, SD_SPI_DEV, SD0_CS_GPIO, SD0_CS_GPIO_PIN);
        if (err != RT_EOK)
        {
            rt_kprintf("spi attach fail\r\n");
            return;
        }

        err = msd_init(SD_DEVICE, SD_SPI_DEV);
        if (err != RT_EOK)
        {
            rt_kprintf("sd card msd init fail\r\n");
            return;
        }

        err = dfs_mount(SD_DEVICE, SD_DIR, "elm", 0, 0);
        if (err != RT_EOK)
        {
            rt_kprintf("[E/SD] sd card mount fail\r\n");
            return;
        }

        sdcard_mounted = true;

        rt_kprintf("[I/SD] sd card mounted\r\n");
    }
    else
    {
        if (sdcard_mounted)
        {
            rt_kprintf("sdcard unmount\r\n");

            dfs_unmount(SD_DIR);               // unmount
            rt_pin_write(SD_CS_PIN, PIN_HIGH); // sdcard chip select off
            rt_thread_mdelay(10);
            rt_pin_write(SD_ON_PIN, PIN_LOW);  // sdcard power off
            sdcard_mounted = false;
        }
    }

    return;
}

/* thread called when debounce timer expires */
static void sdcard_change_thread(void *parameter)
{
    rt_thread_t threadid = RT_NULL;

    // mounting the sdcard requires a lot of stack,
    // so we start up a separate thread with a big stack.
    // increase stack if mounting sdcard crashes.

    threadid = rt_thread_create("sd mount", sdcard_mount, RT_NULL, SD_BIG_STACK, 5, 10);
    if (threadid != RT_NULL)
        rt_thread_startup(threadid);

    rt_timer_stop(sdcard_change_tim);
}

/* interrupt handler */
static void sdcard_change_handler(void *ptr)
{
    if (sdcard_change_tim != RT_NULL)
        rt_timer_start(sdcard_change_tim);
}

int app_sdcard_init(void)
{
    rt_pin_mode(SD_CS_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(SD_CS_PIN, PIN_HIGH);

    rt_pin_mode(SD_ON_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(SD_ON_PIN, PIN_LOW);

    rt_pin_mode(SD_DETECT_PIN, PIN_MODE_INPUT_PULLUP);

    sdcard_change_tim = rt_timer_create("sdcard",
                                        sdcard_change_thread,
                                        RT_NULL,
                                        SD_DEBOUNCE_TICKS,
                                        RT_TIMER_FLAG_ONE_SHOT | RT_TIMER_FLAG_SOFT_TIMER);

    rt_pin_attach_irq(SD_DETECT_PIN, PIN_IRQ_MODE_RISING_FALLING, sdcard_change_handler, RT_NULL);
    rt_pin_irq_enable(SD_DETECT_PIN, PIN_IRQ_ENABLE);

    sdcard_mount();

    return RT_EOK;
}

INIT_APP_EXPORT(app_sdcard_init);
