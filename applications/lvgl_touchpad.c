/*
 * cst816t driver.
 * This driver uses the cst816t to decode gestures.
 * On-chip gesture decoding reduces the number of interrupts by half.
 */

#include <rtthread.h>
#include "drv_gpio.h"
#include "pinout.h"
#include "lv_nopoll.h"
#include "lvgl_touchpad.h"

#define I2C_ADDRESS 0x15

#ifdef BSP_USING_HARD_I2C1
#define I2C_BUS "hwi2c1"
#else
#define I2C_BUS "i2c1"
#endif

#define REG_GESTURE_ID      0x01
#define REG_FINGER_NUM      0x02
#define REG_XPOS_H          0x03
#define REG_XPOS_L          0x04
#define REG_YPOS_H          0x05
#define REG_YPOS_L          0x06
#define REG_CHIP_ID         0xA7
#define REG_PROJ_ID         0xA8
#define REG_FW_VERSION      0xA9
#define REG_FACTORY_ID      0xAA
#define REG_SLEEP_MODE      0xE5
#define REG_IRQ_CTL         0xFA
#define REG_LONG_PRESS_TICK 0xEB
#define REG_MOTION_MASK     0xEC
#define REG_DIS_AUTOSLEEP   0xFE

#define GESTURE_NONE         0x00
#define GESTURE_SWIPE_UP     0x01
#define GESTURE_SWIPE_DOWN   0x02
#define GESTURE_SWIPE_LEFT   0x03
#define GESTURE_SWIPE_RIGHT  0x04
#define GESTURE_SINGLE_CLICK 0x05
#define GESTURE_DOUBLE_CLICK 0x0B
#define GESTURE_LONG_PRESS   0x0C

/* MOTION_MASK_CONTINUOUS_UP_DOWN is useful for tracking a vertical scroll bar */
#define MOTION_MASK_CONTINUOUS_LEFT_RIGHT 0b100
#define MOTION_MASK_CONTINUOUS_UP_DOWN    0b010
#define MOTION_MASK_DOUBLE_CLICK          0b001

/* IRQ_EN_CHANGE interrupts when a finger is pressed or raised */
/* IRQ_EN_MOTION interrupts when a click or swipe is made */
#define IRQ_EN_TOUCH     0x40
#define IRQ_EN_CHANGE    0x20
#define IRQ_EN_MOTION    0x10
#define IRQ_EN_LONGPRESS 0x01

struct touchpad_data
{
    uint8_t  gesture;
    uint8_t  fingers;
    uint16_t x;
    uint16_t y;
};

static rt_sem_t                  cst816t_irq_sem;
static rt_mq_t                   touchpad_mq = RT_NULL;
static struct rt_i2c_bus_device *i2c_bus     = RT_NULL;

static void cst816t_thread();

/* i2c routines */

static rt_err_t cst816x_read(rt_uint8_t reg, rt_uint8_t *data_buf, size_t data_len)
{
    struct rt_i2c_msg msgs[2];
    rt_uint8_t        cmd_buf[1];

    cmd_buf[0] = reg;

    msgs[0].addr  = I2C_ADDRESS;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf   = cmd_buf;
    msgs[0].len   = sizeof(cmd_buf);

    msgs[1].addr  = I2C_ADDRESS;
    msgs[1].flags = RT_I2C_RD;
    msgs[1].buf   = data_buf;
    msgs[1].len   = data_len;

    if (rt_i2c_transfer(i2c_bus, msgs, 2) == 2)
        return RT_EOK;
    else
        return -RT_ERROR;
}

static rt_err_t cst816x_write(rt_uint8_t reg, rt_uint8_t dta)
{
    struct rt_i2c_msg msgs[1];
    rt_uint8_t        data_buf[2];

    data_buf[0] = reg;
    data_buf[1] = dta;

    msgs[0].addr  = I2C_ADDRESS;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf   = data_buf;
    msgs[0].len   = sizeof(data_buf);

    if (rt_i2c_transfer(i2c_bus, msgs, 1) == 1)
        return RT_EOK;
    else
        return -RT_ERROR;
}

/* called by lvgl to initialize the touchpad */
void lv_port_indev_init()
{
    rt_thread_t threadid = RT_NULL;
    cst816t_irq_sem      = rt_sem_create("touch", 0, RT_IPC_FLAG_FIFO);
    touchpad_mq          = rt_mq_create("touch", sizeof(struct touchpad_data), 8, RT_IPC_FLAG_FIFO);

    /* read from touchpad chip */
    threadid = rt_thread_create("cst816t", cst816t_thread, RT_NULL, 1024, 5, 10);
    if (threadid != RT_NULL)
        rt_thread_startup(threadid);

    /* register lvgl touchpad */
    lv_nopoll_create();
}

/* interrupt handler */
static void cst816t_handler(void *ptr)
{
    rt_sem_release(cst816t_irq_sem);
}

/* read from touchpad chip and queue clicks and swipes */
static void cst816t_thread()
{
    rt_uint8_t           rx_data[10] = {0};
    struct touchpad_data tp;

    /* reset cst816t */
    rt_pin_mode(TOUCH_INT_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(TOUCH_RST_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(TOUCH_RST_PIN, PIN_LOW);
    rt_thread_mdelay(10);
    rt_pin_write(TOUCH_RST_PIN, PIN_HIGH);
    rt_thread_mdelay(50);

    /* cst816t i2c bus */
    i2c_bus = rt_i2c_bus_device_find(I2C_BUS);
    RT_ASSERT(i2c_bus);
    if (rt_device_open(&i2c_bus->parent, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
    {
        rt_kprintf("cst816t: open i2c device failed.\n");
        return;
    }
    /* detect gestures, including long press and double click */
    if ((cst816x_write(REG_IRQ_CTL, IRQ_EN_MOTION | IRQ_EN_LONGPRESS) != RT_EOK) ||
        (cst816x_write(REG_MOTION_MASK, MOTION_MASK_DOUBLE_CLICK) != RT_EOK) ||
        (cst816x_write(REG_DIS_AUTOSLEEP, 0xFF) != RT_EOK))
    {
        rt_kprintf("cst816t: i2c write failed.\n");
        return;
    }

    /* watch cst816t interrupt pin */
    rt_pin_attach_irq(TOUCH_INT_PIN, PIN_IRQ_MODE_FALLING, cst816t_handler, RT_NULL);
    rt_pin_irq_enable(TOUCH_INT_PIN, PIN_IRQ_ENABLE);

    /* wait for interrupt pin to go down */
    while (1)
    {
        rt_sem_take(cst816t_irq_sem, RT_WAITING_FOREVER);
        if (cst816x_read(REG_GESTURE_ID, rx_data, 6) != RT_EOK) continue;
        tp.gesture = rx_data[0];
        tp.fingers = rx_data[1];
        tp.x       = (((uint32_t)rx_data[2] & 0x0F) << 8) | (uint32_t)rx_data[3];
        tp.y       = (((uint32_t)rx_data[4] & 0x0F) << 8) | (uint32_t)rx_data[5];
        rt_mq_send(touchpad_mq, &tp, sizeof(tp));
#if 0
        rt_kprintf("cst816t: gesture %d fingers %d x %d y %d\r\n", tp.gesture, tp.fingers, tp.x, tp.y);
#endif
    }
}

/* switch continuous tracking on/off for scrolling */

void touchpad_continuous(rt_bool_t left_right, rt_bool_t up_down)
{
    rt_uint8_t motion_mask = MOTION_MASK_DOUBLE_CLICK;
    if (left_right) motion_mask |= MOTION_MASK_CONTINUOUS_LEFT_RIGHT;
    if (up_down) motion_mask |= MOTION_MASK_CONTINUOUS_UP_DOWN;
    cst816x_write(REG_MOTION_MASK, motion_mask);
}

/* read clicks and swipes from queue and send lvgl events */
void touchpad_events()
{
    struct touchpad_data tp;
    while (rt_mq_recv(touchpad_mq, &tp, sizeof(tp), 0) == sizeof(tp))
    {
        switch (tp.gesture)
        {
        case GESTURE_SINGLE_CLICK:
            lv_nopoll_click(tp.x, tp.y);
            break;
        case GESTURE_SWIPE_UP:
            lv_nopoll_swipe_up(tp.x, tp.y);
            break;
        case GESTURE_SWIPE_DOWN:
            lv_nopoll_swipe_down(tp.x, tp.y);
            break;
        case GESTURE_SWIPE_LEFT:
            lv_nopoll_swipe_left(tp.x, tp.y);
            break;
        case GESTURE_SWIPE_RIGHT:
            lv_nopoll_swipe_right(tp.x, tp.y);
            break;
        case GESTURE_LONG_PRESS:
            lv_nopoll_long_press(tp.x, tp.y);
            break;
        case GESTURE_DOUBLE_CLICK:
            lv_nopoll_double_click(tp.x, tp.y);
            break;
        case GESTURE_NONE:
        default:
        }
    }
}
