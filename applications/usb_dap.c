#include "rtthread.h"
#include "usbd_core.h"
#include "usb_desc.h"
#include "DAP_config.h"
#include "DAP.h"

static rt_thread_t dap_threadid = RT_NULL;

static volatile uint16_t USB_RequestIndexI;  // Request  Index In
static volatile uint16_t USB_RequestIndexO;  // Request  Index Out
static volatile uint16_t USB_RequestCountI;  // Request  Count In
static volatile uint16_t USB_RequestCountO;  // Request  Count Out
static volatile uint8_t  USB_RequestIdle;    // Request  Idle  Flag

static volatile uint16_t USB_ResponseIndexI; // Response Index In
static volatile uint16_t USB_ResponseIndexO; // Response Index Out
static volatile uint16_t USB_ResponseCountI; // Response Count In
static volatile uint16_t USB_ResponseCountO; // Response Count Out
static volatile uint8_t  USB_ResponseIdle;   // Response Idle  Flag


static USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t USB_Request[DAP_PACKET_COUNT][DAP_PACKET_SIZE];  // Request  Buffer
static USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t USB_Response[DAP_PACKET_COUNT][DAP_PACKET_SIZE]; // Response Buffer
static uint16_t                                       USB_RespSize[DAP_PACKET_COUNT];                  // Response Size

void dap_configured(uint8_t busid)
{
    USB_RequestIdle = 0U;
    usbd_ep_start_read(busid, DAP_OUT_EP, USB_Request[0], DAP_PACKET_SIZE);
}

void dap_out_callback(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    if (USB_Request[USB_RequestIndexI][0] == ID_DAP_TransferAbort)
    {
        DAP_TransferAbort = 1U;
    }
    else
    {
        USB_RequestIndexI++;
        if (USB_RequestIndexI == DAP_PACKET_COUNT)
        {
            USB_RequestIndexI = 0U;
        }
        USB_RequestCountI++;
    }

    // Start reception of next request packet
    if ((uint16_t)(USB_RequestCountI - USB_RequestCountO) != DAP_PACKET_COUNT)
    {
        usbd_ep_start_read(busid, DAP_OUT_EP, USB_Request[USB_RequestIndexI], DAP_PACKET_SIZE);
    }
    else
    {
        USB_RequestIdle = 1U;
    }
}

void dap_in_callback(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    if (USB_ResponseCountI != USB_ResponseCountO)
    {
        // Load data from response buffer to be sent back
        usbd_ep_start_write(busid, DAP_IN_EP, USB_Response[USB_ResponseIndexO], USB_RespSize[USB_ResponseIndexO]);
        USB_ResponseIndexO++;
        if (USB_ResponseIndexO == DAP_PACKET_COUNT)
        {
            USB_ResponseIndexO = 0U;
        }
        USB_ResponseCountO++;
    }
    else
    {
        USB_ResponseIdle = 1U;
    }
}

static void chry_dap_state_init(void)
{
    // Initialize variables
    USB_RequestIndexI  = 0U;
    USB_RequestIndexO  = 0U;
    USB_RequestCountI  = 0U;
    USB_RequestCountO  = 0U;
    USB_RequestIdle    = 1U;
    USB_ResponseIndexI = 0U;
    USB_ResponseIndexO = 0U;
    USB_ResponseCountI = 0U;
    USB_ResponseCountO = 0U;
    USB_ResponseIdle   = 1U;
}

void chry_dap_handle()
{
    uint32_t n;

    // Process pending requests
    while (USB_RequestCountI != USB_RequestCountO)
    {
        // Handle Queue Commands
        n = USB_RequestIndexO;
        while (USB_Request[n][0] == ID_DAP_QueueCommands)
        {
            USB_Request[n][0] = ID_DAP_ExecuteCommands;
            n++;
            if (n == DAP_PACKET_COUNT)
            {
                n = 0U;
            }
            if (n == USB_RequestIndexI)
            {
                // flags = osThreadFlagsWait(0x81U, osFlagsWaitAny, osWaitForever);
                // if (flags & 0x80U) {
                //     break;
                // }
            }
        }

        // Execute DAP Command (process request and prepare response)
        rt_pin_write(LED0_PIN, PIN_LOW);  // activity led on
        USB_RespSize[USB_ResponseIndexI] = (uint16_t)DAP_ExecuteCommand(USB_Request[USB_RequestIndexO], USB_Response[USB_ResponseIndexI]);
        rt_pin_write(LED0_PIN, PIN_HIGH); // activity led off

        // Update Request Index and Count
        USB_RequestIndexO++;
        if (USB_RequestIndexO == DAP_PACKET_COUNT)
        {
            USB_RequestIndexO = 0U;
        }
        USB_RequestCountO++;

        if (USB_RequestIdle)
        {
            if ((uint16_t)(USB_RequestCountI - USB_RequestCountO) != DAP_PACKET_COUNT)
            {
                USB_RequestIdle = 0U;
                usbd_ep_start_read(BUSID0, DAP_OUT_EP, USB_Request[USB_RequestIndexI], DAP_PACKET_SIZE);
            }
        }

        // Update Response Index and Count
        USB_ResponseIndexI++;
        if (USB_ResponseIndexI == DAP_PACKET_COUNT)
        {
            USB_ResponseIndexI = 0U;
        }
        USB_ResponseCountI++;

        if (USB_ResponseIdle)
        {
            if (USB_ResponseCountI != USB_ResponseCountO)
            {
                // Load data from response buffer to be sent back
                n = USB_ResponseIndexO++;
                if (USB_ResponseIndexO == DAP_PACKET_COUNT)
                {
                    USB_ResponseIndexO = 0U;
                }
                USB_ResponseCountO++;
                USB_ResponseIdle = 0U;
                usbd_ep_start_write(BUSID0, DAP_IN_EP, USB_Response[n], USB_RespSize[n]);
            }
        }
    }
}

void dap_task()
{
    DAP_Setup();
    chry_dap_state_init();
    while (1)
    {
        chry_dap_handle();
        rt_thread_delay(10); // XXX bad
    }
}

int app_dap_init(void)
{
    dap_threadid = rt_thread_create("dap", dap_task, RT_NULL, 1024, 5, 10);
    if (dap_threadid != RT_NULL)
        return rt_thread_startup(dap_threadid);
    else
        return -RT_ERROR;
}

//XXX
#if 0
INIT_APP_EXPORT(app_dap_init);
#endif
