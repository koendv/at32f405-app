/*
 * canbus.c canbus logger
 * receive can packetsi. write binary to logger. write slcan to usb cdc.
 */

/*
 * test:
 * - set RT_CAN_MODE_LOOPBACK
 * - shell command: canbus_send
 * - console response: "t10080123456789ABCDEF"
 * - shell command: canstat can1
 * - console response: 1 package sent, 1 package received, no errors.
 */

#include <rtthread.h>
#include <rtdevice.h>

#if defined(RT_USING_CAN) && defined(BSP_USING_CAN1)

#include <drv_can.h>

#define DBG_TAG "CAN"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define CAN_DEV   "can1"
#define SLCAN_MTU (sizeof("T1111222281122334455667788EA5F\r") + 1)

static uint8_t     char_tx_buffer[SLCAN_MTU];
static rt_device_t can_dev          = RT_NULL;
static rt_thread_t can_rx_thread_id = RT_NULL;
static rt_sem_t    can_rx_sem       = RT_NULL;

/* convert canbus message to slcan ascii text */

static int can2ascii(rt_can_msg_t msg, char *tx_buffer, rt_bool_t print_timestamp)
{
    int      pos  = 0;
    uint32_t temp = 0;

    rt_memset(tx_buffer, 0, SLCAN_MTU);

    /* RTR frame */
    if (msg->rtr == RT_CAN_RTR)
    {
        if (msg->ide == RT_CAN_EXTID)
        {
            tx_buffer[pos] = 'R';
        }
        else
        {
            tx_buffer[pos] = 'r';
        }
    }
    else /* data frame */
    {
        if (msg->ide == RT_CAN_EXTID)
        {
            tx_buffer[pos] = 'T';
        }
        else
        {
            tx_buffer[pos] = 't';
        }
    }
    pos++;
    /* id */
    temp = msg->id;
    if (msg->ide == RT_CAN_EXTID)
    {
        rt_snprintf(&(tx_buffer[pos]), 9, "%08X", temp);
        pos += 8;
    }
    else
    {
        rt_snprintf(&(tx_buffer[pos]), 4, "%03X", temp);
        pos += 3;
    }
    /* len */
    tx_buffer[pos] = '0' + (msg->len & 0x0f);
    pos++;
    /* data */
    if (msg->rtr != RT_CAN_RTR)
    {
        for (int i = 0; i < msg->len; i++)
        {
            rt_snprintf(&(tx_buffer[pos]), 3, "%02X", msg->data[i]);
            pos += 2;
        }
    }
    /* timestamp */
    if (print_timestamp)
    {
        uint32_t tick = rt_tick_get();
        rt_sprintf(&tx_buffer[pos], "%04lX", (tick & 0x0000FFFF));
        pos += 4;
    }
    /* end */
    tx_buffer[pos] = '\r';
    pos++;
    tx_buffer[pos] = 0;
    return pos;
}

static rt_err_t can_rx_handler(rt_device_t dev, rt_size_t size)
{
    if (can_rx_sem)
        rt_sem_release(can_rx_sem);
    return RT_EOK;
}

static void can_rx_thread(void *param)
{
    rt_err_t          res;
    struct rt_can_msg rx_msg = {0};
    while (1)
    {
        rx_msg.hdr_index = -1;
        rt_sem_take(can_rx_sem, RT_WAITING_FOREVER);
        rt_device_read(can_dev, 0, &rx_msg, sizeof(rx_msg));
        can2ascii(&rx_msg, char_tx_buffer, RT_FALSE);
        rt_kprintf("%s", char_tx_buffer);
    }
}

#if 1
/* send test packet */
static void canbus_send()
{
    struct rt_can_msg msg = {0};
    rt_size_t         size;

    msg.id      = 0x100;
    msg.ide     = RT_CAN_STDID;
    msg.rtr     = RT_CAN_DTR;
    msg.len     = 8;
    msg.data[0] = 0x01;
    msg.data[1] = 0x23;
    msg.data[2] = 0x45;
    msg.data[3] = 0x67;
    msg.data[4] = 0x89;
    msg.data[5] = 0xAB;
    msg.data[6] = 0xCD;
    msg.data[7] = 0xEF;

    size = rt_device_write(can_dev, 0, &msg, sizeof(msg));
    if (size == 0)
    {
        rt_kprintf("%s write failed!\n", CAN_DEV);
    }

    return;
}

MSH_CMD_EXPORT(canbus_send, send canbus test);
#endif

/* filter canbus messages in hardware */
static rt_err_t canbus_set_filter()
{
#ifdef RT_CAN_USING_HDR
    rt_err_t res;

    struct rt_can_filter_item items[5] =
        {
            RT_CAN_FILTER_ITEM_INIT(0x100, 0, 0, 0, 0x700, RT_NULL, RT_NULL),
            RT_CAN_FILTER_ITEM_INIT(0x300, 0, 0, 0, 0x700, RT_NULL, RT_NULL),
            RT_CAN_FILTER_ITEM_INIT(0x211, 0, 0, 0, 0x7ff, RT_NULL, RT_NULL),
            RT_CAN_FILTER_STD_INIT(0x486, RT_NULL, RT_NULL),
            {0x555, 0, 0, 0, 0x7ff, 7}
    };
    struct rt_can_filter_config cfg = {5, 1, items};

    res = rt_device_control(can_dev, RT_CAN_CMD_SET_FILTER, &cfg);
    if (res != RT_EOK)
    {
        LOG_E("canbus %s set filter failed!", CAN_DEV);
        return -RT_ERROR;
    }
#endif
}

int canbus_init(void)
{
    rt_err_t res;

    can_rx_sem = rt_sem_create("can1rx", 0, RT_IPC_FLAG_FIFO);

    can_dev = rt_device_find(CAN_DEV);
    if (!can_dev)
    {
        LOG_E("device %s not found!", CAN_DEV);
        return -RT_ERROR;
    }
    res = rt_device_open(can_dev, RT_DEVICE_FLAG_INT_TX | RT_DEVICE_FLAG_INT_RX);
    if (res != RT_EOK)
    {
        LOG_E("open device %s failed!", CAN_DEV);
        return -RT_ERROR;
    }
    can_rx_thread_id = rt_thread_create("can rx", can_rx_thread, RT_NULL, 1024, 25, 10);
    if (can_rx_thread_id != RT_NULL)
    {
        rt_thread_startup(can_rx_thread_id);
    }
    rt_device_control(can_dev, RT_CAN_CMD_SET_BAUD, (void *)CAN500kBaud);
#if 1
    rt_device_control(can_dev, RT_CAN_CMD_SET_MODE, (void *)RT_CAN_MODE_LISTEN);
#else
    rt_device_control(can_dev, RT_CAN_CMD_SET_MODE, (void *)RT_CAN_MODE_LOOPBACK);
#endif
    //canbus_set_filter();
    rt_device_set_rx_indicate(can_dev, can_rx_handler);
    LOG_I("%s canbus init ok", CAN_DEV);
    return RT_EOK;
}

INIT_APP_EXPORT(canbus_init);

#endif
