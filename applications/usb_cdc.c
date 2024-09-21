#include "rtthread.h"
#include "usbd_core.h"
#include "usbd_cdc.h"
#include "usb_desc.h"
#include "usb_cdc.h"

USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t cdc0_read_buffer[CDC_MAX_MPS];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t cdc1_read_buffer[CDC_MAX_MPS];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t write_buffer[2048]; // XXX

volatile bool ep_tx_busy_flag = false;
bool          cdc0_dtr        = false;
bool          cdc1_dtr        = false;

void cdc_configured(uint8_t busid)
{
    ep_tx_busy_flag = false;
    /* setup first out ep read transfer */
    usbd_ep_start_read(busid, CDC0_OUT_EP, cdc0_read_buffer, sizeof(cdc0_read_buffer));
    usbd_ep_start_read(busid, CDC1_OUT_EP, cdc1_read_buffer, sizeof(cdc1_read_buffer));
}

void usbd_cdc0_acm_bulk_out(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    USB_LOG_RAW("cdc0 actual out len:%d\r\n", nbytes);
    /* setup next out ep read transfer */
    usbd_ep_start_read(busid, CDC0_OUT_EP, cdc0_read_buffer, sizeof(cdc0_read_buffer));
}

void usbd_cdc0_acm_bulk_in(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    USB_LOG_RAW("cdc0 actual in len:%d\r\n", nbytes);

    if ((nbytes % usbd_get_ep_mps(busid, ep)) == 0 && nbytes)
    {
        /* send zlp */
        usbd_ep_start_write(busid, CDC0_IN_EP, NULL, 0);
    }
    else
    {
        ep_tx_busy_flag = false;
    }
}

void usbd_cdc1_acm_bulk_out(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    USB_LOG_RAW("cdc1 actual out len:%d\r\n", nbytes);
    /* setup next out ep read transfer */
    usbd_ep_start_read(busid, CDC1_OUT_EP, cdc1_read_buffer, sizeof(cdc1_read_buffer));
}

void usbd_cdc1_acm_bulk_in(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    USB_LOG_RAW("cdc1 actual in len:%d\r\n", nbytes);

    if ((nbytes % usbd_get_ep_mps(busid, ep)) == 0 && nbytes)
    {
        /* send zlp */
        usbd_ep_start_write(busid, CDC1_IN_EP, NULL, 0);
    }
    else
    {
        ep_tx_busy_flag = false;
    }
}

void usbd_cdc_acm_set_dtr(uint8_t busid, uint8_t intf, bool dtr)
{
    if (intf == 0)
    {
        USB_LOG_RAW("cdc0 intf %d dtr:%d\r\n", intf, dtr);
        cdc0_dtr = dtr;
    }
    else if (intf == 2)
    {
        USB_LOG_RAW("cdc1 intf %d dtr:%d\r\n", intf, dtr);
        cdc1_dtr = dtr;
    }
    else
    {
        USB_LOG_RAW("cdc? intf %d dtr:%d\r\n", intf, dtr);
    }
}

bool get_cdc0_dtr()
{
    return cdc0_dtr;
}

bool get_cdc1_dtr()
{
    return cdc1_dtr;
}

void cdc0_acm_data_send(uint8_t *buf, uint32_t nbytes)
{
    if (!cdc0_dtr) return;
    ep_tx_busy_flag = true;
    usbd_ep_start_write(BUSID, CDC0_IN_EP, buf, nbytes);
    // XXX fixme - replace busy waiting with semaphore
    while (ep_tx_busy_flag)
    {
    }
}

void cdc1_acm_data_send(uint8_t *buf, uint32_t nbytes)
{
    if (!cdc1_dtr) return;
    ep_tx_busy_flag = true;
    usbd_ep_start_write(BUSID, CDC1_IN_EP, buf, nbytes);
    while (ep_tx_busy_flag)
    {
    // XXX fixme - replace busy waiting with semaphore
    }
}

