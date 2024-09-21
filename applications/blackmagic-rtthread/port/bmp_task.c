#include <rtthread.h>

#include "general.h"
#include "platform.h"
#include "gdb_if.h"
#include "gdb_main.h"
#include "target.h"
#include "exception.h"
#include "gdb_packet.h"
#ifdef ENABLE_RTT
#include "rtt.h"
#endif

static rt_thread_t bmp_threadid = RT_NULL;

/* pbuf below is now only used by the remote protocol remote.c */

/* pbuf has to be aligned so the remote protocol can re-use it without causing Problems */
static char BMD_ALIGN_DEF(8) pbuf[GDB_PACKET_BUFFER_SIZE + 1U];

char *gdb_packet_buffer()
{
	return pbuf;
}

static void bmp_poll_loop(void)
{
	SET_IDLE_STATE(false);
	while (gdb_target_running && cur_target) {
		gdb_poll_target();

		// Check again, as `gdb_poll_target()` may
		// alter these variables.
		if (!gdb_target_running || !cur_target)
			break;
		char c = gdb_if_getchar_to(100); // changed 0 to 100 XXX
		if (c == '\x03' || c == '\x04')
			target_halt_request(cur_target);
		rt_thread_mdelay(10);
		rt_thread_yield(); // XXX platform_pace_poll();
#ifdef ENABLE_RTT
		if (rtt_enabled)
			poll_rtt(cur_target);
#endif
	}

	SET_IDLE_STATE(true);
	size_t size = gdb_getpacket(pbuf, GDB_PACKET_BUFFER_SIZE);
	// If port closed and target detached, stay idle
	if (pbuf[0] != '\x04' || cur_target)
		SET_IDLE_STATE(false);
	gdb_main(pbuf, GDB_PACKET_BUFFER_SIZE, size);
}

void bmp_func()
{
	platform_init();

	while (1) {
		TRY(EXCEPTION_ALL)
		{
			bmp_poll_loop();
		}
		CATCH()
		{
		default:
			gdb_putpacketz("EFF");
			target_list_free();
			gdb_outf("Uncaught exception: %s\n", exception_frame.msg);
		}
	}
}

int app_blackmagic_init(void)
{
	bmp_threadid = rt_thread_create("blackmagic", bmp_func, RT_NULL, 8192, 4, 10);
	if (bmp_threadid != RT_NULL)
		return rt_thread_startup(bmp_threadid);
	else
		return -RT_ERROR;
}
