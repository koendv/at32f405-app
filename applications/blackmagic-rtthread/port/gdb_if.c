#include "general.h"
#include "platform.h"
#include "usb_desc.h"
#include "usb_cdc.h"
#include "gdb_packet.h"
#include "gdb_main.h"
#include "gdb_if.h"
#include "gdb_task.h"
#include <rtthread.h>

bool cdc0_dtr = false;
static bool cdc0_read_busy = false;
static bool configured = false;
static struct rt_completion cdc0_read_complete;
static struct rt_ringbuffer cdc0_read_rb;
static uint8_t cdc0_ring_buffer[2 * CDC_MAX_MPS];

void cdc0_configured()
{
	configured = true;
	rt_completion_init(&cdc0_read_complete);
	rt_ringbuffer_init(&cdc0_read_rb, cdc0_ring_buffer, sizeof(cdc0_ring_buffer));
	cdc0_read_busy = true;
}

void cdc0_set_dtr(bool dtr)
{
	cdc0_dtr = dtr;
	if (dtr) // start gdb process
		release_target();
}

void cdc0_read(uint8_t *buf, uint32_t nbytes)
{
	rt_ringbuffer_put(&cdc0_read_rb, buf, nbytes);
	rt_completion_done(&cdc0_read_complete);
	if (rt_ringbuffer_space_len(&cdc0_read_rb) >= CDC_MAX_MPS)
		cdc0_next_read();
	else
		cdc0_read_busy = false;
}

/* read one character from gdb port */
char gdb_getchar(rt_int32_t timeout_ticks)
{
	char ch;
	if (!(configured && cdc0_dtr)) {
		rt_thread_mdelay(100);
		return '\x04';
	}
	if (rt_ringbuffer_getchar(&cdc0_read_rb, &ch))
		return ch;
	if (!cdc0_read_busy && (rt_ringbuffer_space_len(&cdc0_read_rb) >= CDC_MAX_MPS)) {
		cdc0_next_read();
		cdc0_read_busy = true;
	}
	// wait until next usb packet arrives
	rt_completion_wait(&cdc0_read_complete, timeout_ticks);
	if (rt_ringbuffer_getchar(&cdc0_read_rb, &ch))
		return ch;
	return -1;
}

/* read one character from gdb port, no time-out */

char gdb_if_getchar()
{
	return gdb_getchar(RT_WAITING_FOREVER);
}

/* read one character from gdb port with time-out */

char gdb_if_getchar_to(uint32_t timeout_ms)
{
	return gdb_getchar(timeout_ms * 1000 / RT_TICK_PER_SECOND);
}

/* write one character to gdb server port. send usb packet if "flush" */
static uint8_t cdc0_write_buffer[CDC_MAX_MPS - 1];
static uint32_t cdc0_write_idx = 0;

void gdb_if_putchar(const char c, const int flush)
{
	cdc0_write_buffer[cdc0_write_idx++] = c;
	if (flush || cdc0_write_idx == sizeof(cdc0_write_buffer)) {
		cdc0_write(cdc0_write_buffer, cdc0_write_idx);
		cdc0_write_idx = 0;
	}
}
