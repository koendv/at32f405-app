#include "rtthread.h"
#include "rtdevice.h"
#include "usbd_core.h"
#include "usbd_cdc.h"
#include "usb_desc.h"
#include "usb_cdc.h"

/*
   implements two serial ports, cdc0 and cdc1.
   cdc0 is gdb server
   cdc1 is uart/rtt terminal
 */

// logging
#if 1
#undef USB_LOG_RAW
#define USB_LOG_RAW(...)
#endif

USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX static uint8_t cdc0_read_buffer[CDC_MAX_MPS];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX static uint8_t cdc0_write_buffer[CDC_MAX_MPS - 1];

USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX static uint8_t cdc1_read_buffer[CDC_MAX_MPS];

static uint8_t       cdc1_ring_buffer[128];
struct rt_ringbuffer cdc1_read_rb;

static uint32_t cdc0_read_size = 0;
static uint32_t cdc0_read_idx  = 0;
static uint32_t cdc1_read_idx  = 0;
static uint32_t cdc0_write_idx = 0;
static rt_sem_t cdc0_read_sem  = RT_NULL;
static rt_sem_t ep_write_sem   = RT_NULL;

volatile bool ep_tx_busy_flag = false;
bool          cdc0_dtr        = false;
bool          cdc1_dtr        = false;

void cdc_configured(uint8_t busid)
{
    rt_ringbuffer_init(&cdc1_read_rb, cdc1_ring_buffer, sizeof(cdc1_ring_buffer));
    cdc0_read_sem   = rt_sem_create("cdc read", 0, RT_IPC_FLAG_FIFO);
    ep_write_sem    = rt_sem_create("usb write", 0, RT_IPC_FLAG_FIFO);
    ep_tx_busy_flag = false;
    /* setup first out ep read transfer */
    //usbd_ep_start_read(busid, CDC0_OUT_EP, cdc0_read_buffer, sizeof(cdc0_read_buffer));
    usbd_ep_start_read(busid, CDC1_OUT_EP, cdc1_read_buffer, sizeof(cdc1_read_buffer));
}

void usbd_cdc0_acm_bulk_out(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    USB_LOG_RAW("cdc0 actual out len:%d\r\n", nbytes);
    cdc0_read_size = nbytes;
    cdc0_read_idx  = 0;
    rt_sem_release(cdc0_read_sem);
}

void usbd_cdc0_acm_bulk_in(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    USB_LOG_RAW("cdc0 actual in len:%d\r\n", nbytes);

    // XXX nbytes == cdc0_write_idx what if only part sent?
    cdc0_write_idx = 0;
    rt_sem_release(ep_write_sem);
    ep_tx_busy_flag = false;
}

void usbd_cdc1_acm_bulk_out(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    USB_LOG_RAW("cdc1 actual out len:%d\r\n", nbytes);
    rt_ringbuffer_put(&cdc1_read_rb, cdc1_read_buffer, nbytes);
    /* setup next out ep read transfer */
    usbd_ep_start_read(busid, CDC1_OUT_EP, cdc1_read_buffer, sizeof(cdc1_read_buffer));
}

void usbd_cdc1_acm_bulk_in(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    USB_LOG_RAW("cdc1 actual in len:%d\r\n", nbytes);
    ep_tx_busy_flag = false;
}

void usbd_cdc_acm_set_dtr(uint8_t busid, uint8_t intf, bool dtr)
{
    if (intf == CDC0_INTF)
    {
        USB_LOG_RAW("cdc0 intf %d dtr:%d\r\n", intf, dtr);
        cdc0_dtr = dtr;
    }
    else if (intf == CDC1_INTF)
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

void cdc0_write(uint8_t *buf, uint32_t nbytes)
{
    if (!cdc0_dtr) return;
    usbd_ep_start_write(BUSID0, CDC0_IN_EP, buf, nbytes);
    rt_sem_take(ep_write_sem, RT_WAITING_FOREVER);
}

void cdc1_write(uint8_t *buf, uint32_t nbytes)
{
    if (!cdc1_dtr) return;
    usbd_ep_start_write(BUSID0, CDC1_IN_EP, buf, nbytes);
    rt_sem_take(ep_write_sem, RT_WAITING_FOREVER);
}

/* cdc0: gdb server */

/* read one character from gdb server port. blocking */
char gdb_if_getchar(void)
{
    char ch;
    // return '\x04' if not connected
    if (!get_cdc0_dtr())
        return '\x04';
    while (cdc0_read_idx == cdc0_read_size)
    {
        usbd_ep_start_read(BUSID0, CDC0_OUT_EP, cdc0_read_buffer, sizeof(cdc0_read_buffer));
        rt_sem_take(cdc0_read_sem, RT_WAITING_FOREVER);
    }
    ch = cdc0_read_buffer[cdc0_read_idx++];
    return ch;
}

/* read one character from gdb server port. with timeout */
char gdb_if_getchar_to(const uint32_t timeout_ms)
{
    // return '\x04' if not connected
    if (!get_cdc0_dtr())
        return '\x04';
    // return immediately if timeout 0 and there are no characters */
    if (timeout_ms == 0 && cdc0_read_idx == cdc0_read_size) return -1;
    return gdb_if_getchar();
}

/* write one character to gdb server port. send usb packet if "flush" */
void gdb_if_putchar(const char c, const int flush)
{
    cdc0_write_buffer[cdc0_write_idx++] = c;
    if (flush || cdc0_write_idx == sizeof(cdc0_write_buffer))
    {
        usbd_ep_start_write(BUSID0, CDC0_IN_EP, cdc0_write_buffer, cdc0_write_idx);
        rt_sem_take(ep_write_sem, RT_WAITING_FOREVER);
    }
}

/* cdc1: uart/rtt terminal. see rtt_if.c */

