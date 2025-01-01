/*
   aux port.
   used to connect target console to the host.
 */

/* contains code from comment in drivers/dev_serial_v2.h */

#include <rtthread.h>
#include <platform.h>
#include <usb_cdc.h>
#include <aux_uart.h>

#define DBG_TAG "UART"
#define DBG_LVL DBG_ERR
#include <rtdbg.h>

#ifdef AUX_UART

#define MSG_POOL_SIZE 256

struct rx_msg
{
    rt_device_t dev;
    rt_size_t   size;
};
static rt_device_t            serial;
static struct rt_messagequeue rx_mq;

static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    struct rx_msg msg;
    rt_err_t      result;

    msg.dev  = dev;
    msg.size = size;

    result = rt_mq_send(&rx_mq, &msg, sizeof(msg));
    if (result == -RT_EFULL)
    {
        LOG_E("message queue full!");
    }
    return result;
}

static void aux_uart_thread(void *parameter)
{
    struct rx_msg msg;
    rt_err_t      result;

    rt_uint32_t rx_length;
    static char rx_buffer[AUX_RX_BUFSIZE + 1];

    while (1)
    {
        rt_memset(&msg, 0, sizeof(msg));
        result = rt_mq_recv(&rx_mq, &msg, sizeof(msg), RT_WAITING_FOREVER);
        if (result > 0)
        {
            rx_length = rt_device_read(msg.dev, 0, rx_buffer, msg.size);
            if (rx_length > 0)
                cdc1_write(rx_buffer, rx_length);
        }
    }
}

/* serial port send */
int32_t aux_uart_send(uint8_t *buf, uint32_t len)
{
    int32_t ret = -RT_ERROR;
    if (serial)
        ret = rt_device_write(serial, 0, buf, len);
    return ret;
}

/* set serial port speed */
void cdc1_set_speed(uint32_t speed)
{
    rt_err_t                err;
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;

    LOG_D(AUX_UART " speed %d", speed);
    if (serial)
    {
        rt_device_close(serial);
    }
    else
    {
        serial = rt_device_find(AUX_UART);
        if (!serial)
        {
            LOG_E(AUX_UART " find failed!");
            return;
        }
    }
    config.baud_rate = speed;
    config.data_bits = DATA_BITS_8;
    config.stop_bits = STOP_BITS_1;
    config.parity    = PARITY_NONE;
    err              = rt_device_control(serial, RT_DEVICE_CTRL_CONFIG, &config);
    if (err != RT_EOK)
    {
        LOG_E(AUX_UART " set speed failed!");
        return;
    }
    err = rt_device_open(serial, RT_DEVICE_FLAG_RX_NON_BLOCKING | RT_DEVICE_FLAG_TX_BLOCKING);
    if (err != RT_EOK)
    {
        LOG_E(AUX_UART " open failed!");
        return;
    }
    err = rt_device_set_rx_indicate(serial, uart_input);
    if (err != RT_EOK)
    {
        LOG_E(AUX_UART " receive failed!");
        return;
    }

    return;
}

static int aux_uart_init()
{
    rt_err_t    ret = RT_EOK;
    static char msg_pool[MSG_POOL_SIZE];

    serial = rt_device_find(AUX_UART);
    if (!serial)
    {
        LOG_E(AUX_UART " find failed!");
        return RT_ERROR;
    }

    rt_mq_init(&rx_mq, "rx_mq",
               msg_pool,
               sizeof(struct rx_msg),
               sizeof(msg_pool),
               RT_IPC_FLAG_FIFO);

    rt_device_open(serial, RT_DEVICE_FLAG_RX_NON_BLOCKING | RT_DEVICE_FLAG_TX_BLOCKING);
    rt_device_set_rx_indicate(serial, uart_input);

    rt_thread_t thread = rt_thread_create("aux uart", aux_uart_thread, RT_NULL, 1024, 25, 10);
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        LOG_E(AUX_UART " thread failed!");
        ret = RT_ERROR;
    }
    return ret;
}

INIT_APP_EXPORT(aux_uart_init);

#endif
