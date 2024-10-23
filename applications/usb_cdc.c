#include "rtthread.h"
#include "rtdevice.h"
#include "usbd_core.h"
#include "usbd_cdc.h"
#include "usb_desc.h"
#include "usb_cdc.h"

/*
   implements two serial ports, cdc0 and cdc1.
   cdc0 is gdb server. see gdb_if.c
   cdc1 is uart/rtt terminal. see rtt_if.c
 */

// logging
#if 1
#undef USB_LOG_RAW
#define USB_LOG_RAW(...)
#endif

USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX static uint8_t cdc0_read_buffer[CDC_MAX_MPS];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX static uint8_t cdc1_read_buffer[CDC_MAX_MPS];

static rt_sem_t ep_write_sem = RT_NULL;
static rt_sem_t cdc_tx_busy_sem = RT_NULL;

void cdc_init()
{
	ep_write_sem = rt_sem_create("usb write", 1, RT_IPC_FLAG_FIFO);
	cdc_tx_busy_sem = rt_sem_create("cdc_tx", 0, RT_IPC_FLAG_FIFO);
}

void cdc_configured(uint8_t busid)
{
	cdc0_configured();
	cdc1_configured();
	/* setup first out ep read transfer */
	cdc0_next_read();
	cdc1_next_read();
}

void cdc0_next_read()
{
	usbd_ep_start_read(BUSID0, CDC0_OUT_EP, cdc0_read_buffer, sizeof(cdc0_read_buffer));
}

void cdc1_next_read()
{
	usbd_ep_start_read(BUSID0, CDC1_OUT_EP, cdc1_read_buffer, sizeof(cdc1_read_buffer));
}

void usbd_cdc0_acm_bulk_out(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
	USB_LOG_RAW("cdc0 actual out len:%d\r\n", nbytes);
	cdc0_read(cdc0_read_buffer, nbytes);
}

void usbd_cdc0_acm_bulk_in(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
	USB_LOG_RAW("cdc0 actual in len:%d\r\n", nbytes);
	// XXX assumes nbytes == cdc0_write_idx what if only part sent?
	rt_sem_release(cdc_tx_busy_sem);
}

void usbd_cdc1_acm_bulk_out(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
	USB_LOG_RAW("cdc1 actual out len:%d\r\n", nbytes);
	cdc1_read(cdc1_read_buffer, nbytes);
}

void usbd_cdc1_acm_bulk_in(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
	USB_LOG_RAW("cdc1 actual in len:%d\r\n", nbytes);
	rt_sem_release(cdc_tx_busy_sem);
}

void usbd_cdc_acm_set_dtr(uint8_t busid, uint8_t intf, bool dtr)
{
	if (intf == CDC0_INTF) {
		USB_LOG_RAW("cdc0 intf %d dtr:%d\r\n", intf, dtr);
		cdc0_set_dtr(dtr);
	} else if (intf == CDC1_INTF) {
		USB_LOG_RAW("cdc1 intf %d dtr:%d\r\n", intf, dtr);
		cdc1_set_dtr(dtr);
	} else {
		USB_LOG_RAW("cdc? intf %d dtr:%d\r\n", intf, dtr);
	}
}

void cdc0_write(uint8_t *buf, uint32_t nbytes)
{
	// wait until usb transmit available
	rt_sem_take(ep_write_sem, RT_WAITING_FOREVER);
	usbd_ep_start_write(BUSID0, CDC0_IN_EP, buf, nbytes);
	// wait until usb write finished
	rt_sem_take(cdc_tx_busy_sem, RT_WAITING_FOREVER);
	rt_sem_release(ep_write_sem);
}

void cdc1_write(uint8_t *buf, uint32_t nbytes)
{
	// wait until usb transmit available
	rt_sem_take(ep_write_sem, RT_WAITING_FOREVER);
	usbd_ep_start_write(BUSID0, CDC1_IN_EP, buf, nbytes);
	// wait until usb write finished
	rt_sem_take(cdc_tx_busy_sem, RT_WAITING_FOREVER);
	rt_sem_release(ep_write_sem);
}
