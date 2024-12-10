#include <rtthread.h>
#include "drv_gpio.h"
#include "drv_spi.h"
#include "pinout.h"
#include "at32_msp.h"
#include "lvgl.h"
#include "lv_st7789.h"

/*
   Writing to the display is done in the background.
   This way, the cpu can calculate and send pixels at the same time.

   see patches/drv_spi.patch
*/

/* https://github.com/lvgl/lvgl/blob/master/docs/integration/driver/display/gen_mipi.rst */

#define LCD_H_RES        240
#define LCD_V_RES        280
#define LCD_BUF_LINES    12
#define SPI_BUS          "spi3"
#define SPI_DEV          "spi30"
#define DISP_CS_GPIO     GPIOD
#define DISP_CS_GPIO_PIN GPIO_PINS_2

static lv_display_t *lcd_disp       = RT_NULL;
static uint8_t      *buf1           = RT_NULL;
static uint8_t      *buf2           = RT_NULL;
bool                 lvgl_init_done = FALSE;

static int32_t lcd_io_init(void);
static void    lcd_send_cmd(lv_display_t *disp, const uint8_t *cmd, size_t cmd_size, const uint8_t *param, size_t param_size);
static void    lcd_send_color(lv_display_t *disp, const uint8_t *cmd, size_t cmd_size, uint8_t *param, size_t param_size);

static void disp_init()
{
    if (lcd_io_init() != RT_EOK) return;

    rt_pin_mode(DISP_DC_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(DISP_DC_PIN, PIN_HIGH);

    rt_pin_mode(DISP_CS_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(DISP_CS_PIN, PIN_HIGH);

    /* reset st7789 */
    rt_pin_mode(DISP_RST_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(DISP_RST_PIN, PIN_HIGH);
    rt_thread_mdelay(10);
    rt_pin_write(DISP_RST_PIN, PIN_LOW);
    rt_thread_mdelay(10);
    rt_pin_write(DISP_RST_PIN, PIN_HIGH);
    rt_thread_mdelay(10);
}

void lv_port_disp_init()
{
    disp_init();

    /* Create the LVGL display object and the LCD display driver */
    lcd_disp = lv_st7789_create(LCD_H_RES, LCD_V_RES, LV_LCD_FLAG_NONE, lcd_send_cmd, lcd_send_color);

    /* set color to inverted */
    lv_st7789_set_invert(lcd_disp, true);

    /* the 240x280 pixel display is centered in the 240x320 pixel st7789 memory */
    lv_st7789_set_gap(lcd_disp, 0, 20);

    /* set up draw buffers */
    uint32_t buf_size = LCD_H_RES * LCD_BUF_LINES * lv_color_format_get_size(lv_display_get_color_format(lcd_disp));
    // rt_kprintf("lvgl buf size %d bytes\r\n", buf_size);

    buf1 = lv_malloc(buf_size);
    LV_ASSERT_MALLOC(buf1);

#if 1
    // faster, but uses more memory
    buf2 = lv_malloc(buf_size);
    LV_ASSERT_MALLOC(buf2);
#endif

    lv_display_set_buffers(lcd_disp, buf1, buf2, buf_size, LV_DISPLAY_RENDER_MODE_PARTIAL);

    lvgl_init_done = TRUE;
}

void lv_user_gui_init()
{
    extern void ui_init(void);
    ui_init();
}

/*************************************************************************/

static struct rt_spi_device       *spi_dev = RT_NULL;
static struct rt_spi_configuration spi_cfg_8bit;
static struct rt_spi_configuration spi_cfg_16bit;
static rt_thread_t                 pixel_threadid = NULL;
struct pixel_msg
{
    uint8_t *dta;
    uint32_t dta_length;
};
static rt_mq_t pixel_mq = RT_NULL;

static void lcd_send_color_thread(void *);

static int32_t lcd_io_init(void)
{
    /* st7789 spi init */
    at32_msp_spi_init(SPI3);

    // XXX hardcoded: pin D2 is DISP_CS_PIN
    rt_hw_spi_device_attach(SPI_BUS, SPI_DEV, DISP_CS_GPIO, DISP_CS_GPIO_PIN);

    spi_cfg_8bit.data_width = 8;
    spi_cfg_8bit.mode       = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
    spi_cfg_8bit.max_hz     = 36000000; /* 36MHz */

    spi_cfg_16bit.data_width = 16;
    spi_cfg_16bit.mode       = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
    spi_cfg_16bit.max_hz     = 36000000; /* 36MHz */

    spi_dev = (struct rt_spi_device *)rt_device_find(SPI_DEV);
    if (spi_dev == RT_NULL)
    {
        rt_kprintf("spi abort\r\n");
        return -RT_ERROR;
    }
    rt_spi_configure(spi_dev, &spi_cfg_8bit);

    /* background pixel queue */
    pixel_mq = rt_mq_create("pixel", sizeof(struct pixel_msg), 2, RT_IPC_FLAG_FIFO);

    pixel_threadid = rt_thread_create("pixelpump", lcd_send_color_thread, RT_NULL, 768, 5, 10);
    if (pixel_threadid != RT_NULL)
        rt_thread_startup(pixel_threadid);

    return RT_EOK;
}

static void lcd_send_cmd(lv_display_t *disp, const uint8_t *cmd, size_t cmd_size, const uint8_t *param, size_t param_size)
{
    if (spi_dev == RT_NULL) return;
    if (disp == RT_NULL) return;
    rt_pin_write(DISP_DC_PIN, PIN_LOW);
    rt_spi_send(spi_dev, cmd, cmd_size);
    rt_pin_write(DISP_DC_PIN, PIN_HIGH);
    rt_spi_send(spi_dev, param, param_size);
}

static void lcd_send_color(lv_display_t *disp, const uint8_t *cmd, size_t cmd_size, uint8_t *param, size_t param_size)
{
    struct pixel_msg pixels;
    if (spi_dev == RT_NULL) return;
    if (disp == RT_NULL) return;
    rt_pin_write(DISP_DC_PIN, PIN_LOW);
    rt_spi_send(spi_dev, cmd, cmd_size);
    rt_pin_write(DISP_DC_PIN, PIN_HIGH);
    /* schedule pixel write in the background */
    pixels.dta        = param;
    pixels.dta_length = param_size;
    rt_mq_send(pixel_mq, &pixels, sizeof(pixels));
    /* return immediately */
    return;
}

static void lcd_send_color_thread(void *p)
{
    struct pixel_msg pixels;
    while (1)
    {
        rt_mq_recv(pixel_mq, &pixels, sizeof(pixels), RT_WAITING_FOREVER);
        /* sending the pixels in 16bit fixes endianness and is faster */
        rt_spi_configure(spi_dev, &spi_cfg_16bit);
        rt_spi_send(spi_dev, pixels.dta, pixels.dta_length / 2);
        rt_spi_configure(spi_dev, &spi_cfg_8bit);
        /* inform lvgl the data has been sent */
        lv_display_flush_ready(lcd_disp);
    }
}

/*************************************************************************/
