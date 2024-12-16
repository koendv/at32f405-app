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

#define EVENT_CDC0_RX  (0x1 << 0)
#define EVENT_CDC1_RX  (0x1 << 1)
#define EVENT_CDC0_DTR (0x1 << 2)
#define EVENT_CDC1_DTR (0x1 << 3)

USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX static uint8_t cdc0_read_buffer[CDC_MAX_MPS];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX static uint8_t cdc1_read_buffer[CDC_MAX_MPS];

static bool                 cdc_is_configured = false;
static bool                 cdc0_dtr          = false;
static bool                 cdc1_dtr          = false;
static rt_sem_t             ep_write_sem      = RT_NULL;
static rt_sem_t             cdc_tx_busy_sem   = RT_NULL;
static rt_event_t           cdc_event         = RT_NULL;
static struct rt_ringbuffer cdc0_read_rb;
static uint8_t              cdc0_ring_buffer[2 * CDC_MAX_MPS];
static bool                 cdc0_read_busy = false;
static struct rt_ringbuffer cdc1_read_rb;
static uint8_t              cdc1_ring_buffer[2 * CDC_MAX_MPS];
static bool                 cdc1_read_busy = false;

static void cdc0_next_read();
static void cdc1_next_read();

void cdc_init()
{
    ep_write_sem    = rt_sem_create("usb write", 1, RT_IPC_FLAG_FIFO);
    cdc_tx_busy_sem = rt_sem_create("cdc_tx", 0, RT_IPC_FLAG_FIFO);
    cdc_event       = rt_event_create("cdc_rx", RT_IPC_FLAG_FIFO);
    rt_ringbuffer_init(&cdc0_read_rb, cdc0_ring_buffer, sizeof(cdc0_ring_buffer));
    rt_ringbuffer_init(&cdc1_read_rb, cdc1_ring_buffer, sizeof(cdc1_ring_buffer));
}

void cdc_configured(uint8_t busid)
{
    (void)busid;
    cdc_is_configured = true;
    /* setup first out ep read transfer */
    cdc0_next_read();
    cdc1_next_read();
}

void cdc0_wait_for_dtr()
{
    rt_event_recv(cdc_event, EVENT_CDC0_DTR, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, NULL);
}

void cdc1_wait_for_dtr()
{
    rt_event_recv(cdc_event, EVENT_CDC1_DTR, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, NULL);
}

void cdc1_wait_for_char()
{
    rt_event_recv(cdc_event, EVENT_CDC1_RX, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, NULL);
}

void usbd_cdc_acm_set_dtr(uint8_t busid, uint8_t intf, bool dtr)
{
    if (intf == CDC0_INTF)
    {
        USB_LOG_RAW("cdc0 intf %d dtr:%d\r\n", intf, dtr);
        cdc0_dtr = dtr;
        if (dtr && cdc_event)
            rt_event_send(cdc_event, EVENT_CDC0_DTR);
    }
    else if (intf == CDC1_INTF)
    {
        USB_LOG_RAW("cdc1 intf %d dtr:%d\r\n", intf, dtr);
        cdc1_dtr = dtr;
        if (dtr && cdc_event)
            rt_event_send(cdc_event, EVENT_CDC1_DTR);
    }
    else
    {
        USB_LOG_RAW("cdc? intf %d dtr:%d\r\n", intf, dtr);
    }
}

bool cdc0_connected()
{
    return cdc_is_configured && cdc0_dtr;
}

bool cdc1_connected()
{
    return cdc_is_configured && cdc1_dtr;
}

/* cdc0 writing to host */

void usbd_cdc0_acm_bulk_in(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    USB_LOG_RAW("cdc0 actual in len:%d\r\n", nbytes);
    rt_sem_release(cdc_tx_busy_sem);
}

void cdc0_write(uint8_t *buf, uint32_t nbytes)
{
    if (!(cdc_is_configured && cdc0_dtr)) return;
    // wait until usb transmit available
    rt_sem_take(ep_write_sem, RT_WAITING_FOREVER);
    usbd_ep_start_write(BUSID0, CDC0_IN_EP, buf, nbytes);
    // wait until usb write finished
    rt_sem_take(cdc_tx_busy_sem, RT_WAITING_FOREVER);
    rt_sem_release(ep_write_sem);
}

/* cdc1 writing to host */

void usbd_cdc1_acm_bulk_in(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    USB_LOG_RAW("cdc1 actual in len:%d\r\n", nbytes);
    rt_sem_release(cdc_tx_busy_sem);
}

void cdc1_write(uint8_t *buf, uint32_t nbytes)
{
    if (!(cdc_is_configured && cdc0_dtr)) return;
    // wait until usb transmit available
    rt_sem_take(ep_write_sem, RT_WAITING_FOREVER);
    usbd_ep_start_write(BUSID0, CDC1_IN_EP, buf, nbytes);
    // wait until usb write finished
    rt_sem_take(cdc_tx_busy_sem, RT_WAITING_FOREVER);
    rt_sem_release(ep_write_sem);
}

/* cdc0 reading from host */

static void cdc0_next_read()
{
    if (!cdc0_read_busy && rt_ringbuffer_space_len(&cdc0_read_rb) >= CDC_MAX_MPS)
    {
        usbd_ep_start_read(BUSID0, CDC0_OUT_EP, cdc0_read_buffer, sizeof(cdc0_read_buffer));
        cdc0_read_busy = true;
    }
}

void usbd_cdc0_acm_bulk_out(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    USB_LOG_RAW("cdc0 actual out len:%d\r\n", nbytes);
    rt_ringbuffer_put(&cdc0_read_rb, cdc0_read_buffer, nbytes);
    if (cdc_event != NULL && nbytes > 0)
        rt_event_send(cdc_event, EVENT_CDC0_RX);
    cdc0_read_busy = false;
    cdc0_next_read();
}


char cdc0_getchar()
{
    char      ch;
    rt_size_t len;
    len = rt_ringbuffer_getchar(&cdc0_read_rb, &ch);
    cdc0_next_read();
    if (len == 1)
        return ch;
    else
        return -1;
}

char cdc0_getchar_timeout(uint32_t timeout_ticks)
{
    char      ch  = -1;
    rt_size_t len = 0;

    /* take character from ringbuffer */
    len = rt_ringbuffer_getchar(&cdc0_read_rb, &ch);
    /* clear "character received" event */
    if (cdc_event != NULL)
        rt_event_recv(cdc_event, EVENT_CDC0_RX, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, 0, NULL);
    /* schedule next usb read */
    cdc0_next_read();
    /* use character from ringbuffer */
    if (len == 1)
        return ch;
    /* no characters in ringbuffer, wait until next character is read */
    if (cdc_event != NULL)
        rt_event_recv(cdc_event, EVENT_CDC0_RX, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, timeout_ticks, NULL);
    /* take character from ringbuffer */
    len = rt_ringbuffer_getchar(&cdc0_read_rb, &ch);
    /* use character from ringbuffer */
    if (len == 1)
        return ch;
    /* no characters in ringbuffer, timeout */
    return -1;
}

bool cdc0_nodata()
{
    return rt_ringbuffer_data_len(&cdc0_read_rb) == 0;
}

/* cdc1 reading from host */

static void cdc1_next_read()
{
    if (!cdc1_read_busy && rt_ringbuffer_space_len(&cdc1_read_rb) >= CDC_MAX_MPS)
    {
        usbd_ep_start_read(BUSID0, CDC1_OUT_EP, cdc1_read_buffer, sizeof(cdc1_read_buffer));
        cdc1_read_busy = true;
    }
}

void usbd_cdc1_acm_bulk_out(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    USB_LOG_RAW("cdc1 actual out len:%d\r\n", nbytes);
    rt_ringbuffer_put(&cdc1_read_rb, cdc1_read_buffer, nbytes);
    if (cdc_event != NULL && nbytes > 0)
        rt_event_send(cdc_event, EVENT_CDC1_RX);
    cdc1_read_busy = false;
    cdc1_next_read();
}

char cdc1_getchar()
{
    char      ch;
    rt_size_t len;
    len = rt_ringbuffer_getchar(&cdc1_read_rb, &ch);
    cdc1_next_read();
    if (len == 1)
        return ch;
    else
        return -1;
}

bool cdc1_nodata()
{
    return rt_ringbuffer_data_len(&cdc1_read_rb) == 0;
}
