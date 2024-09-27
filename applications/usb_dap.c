#include "rtthread.h"
#include "usbd_core.h"
#include "usb_desc.h"
#include "dap_config.h"
#include "dap.h"

#define DAP_PACKET_COUNT DAP_CONFIG_PACKET_COUNT
#define DAP_PACKET_SIZE  DAP_CONFIG_PACKET_SIZE

static uint8_t USB_Request[DAP_PACKET_SIZE];  // Request  Buffer
static uint8_t USB_Response[DAP_PACKET_SIZE]; // Response Buffer

// Receive first DAP request.
void dap_configured(uint8_t busid)
{
    dap_init();
    usbd_ep_start_read(busid, DAP_OUT_EP, USB_Request, DAP_PACKET_SIZE);
}

// DAP request received. Send DAP response.
void dap_out_callback(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    uint32_t size = dap_process_request(USB_Request, DAP_PACKET_SIZE, USB_Response, DAP_PACKET_SIZE);
    usbd_ep_start_write(busid, DAP_IN_EP, USB_Response, size);
}

// DAP response sent. Receive next DAP request.
void dap_in_callback(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    usbd_ep_start_read(busid, DAP_OUT_EP, USB_Request, DAP_PACKET_SIZE);
}

