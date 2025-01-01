#include <rtthread.h>
#include <rtdevice.h>
#include "usb_desc.h"
#include "usb_cdc.h"
#include "platform.h"
#include "rtt.h"
#include "aux_uart.h"

#define DBG_TAG "AUX"
#define DBG_LVL DBG_ERR
#include <rtdbg.h>

#ifdef AUX_UART

static uint8_t rx_buffer[RT_SERIAL_RX_MINBUFSZ + 1];

static void aux_cdc_thread()
{
    int32_t rx_length;

    while (1)
    {
        cdc1_wait_for_char();
        /* if rtt is running input goes to the rtt console */
        if (!rtt_enabled)
            while (!cdc1_recv_empty())
            {
                rx_length = cdc1_get(rx_buffer, sizeof(rx_buffer));
                if (rx_length > 0)
                    aux_uart_send(rx_buffer, rx_length);
            }
    }
}

static int aux_cdc_init()
{
    rt_thread_t thread = rt_thread_create("aux cdc", aux_cdc_thread, RT_NULL, 1024, 25, 10);
    rt_err_t    ret    = RT_EOK;

    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        LOG_E("aux thread failed!");
        ret = RT_ERROR;
    }
    return ret;
}

INIT_APP_EXPORT(aux_cdc_init);

#endif
