/*
 * gdb threads.
 * There are two gdb threads:
 * - gdb_task: execute gdb commands
 *       gdb_packet receives gdb remote protocol on usb
 *       gdb_main executes gdb remote protocol
 * - gdb_poll: poll running target
 *       gdb_poll_target checks if target is running or halted
 *       poll_rtt checks if there are rtt console messages
 *       poll_memwatch checks if watchpoint variables have changed
 *
 * A semaphore is used to make sure only one thread accesses the target at a time:
 * - take_target()
 * - release_target()
 * The first release of the semaphore is done in cdc0_dtr() when dtr goes high.
 */

#include <rtthread.h>

#include "general.h"
#include "platform.h"
#include "gdb_if.h"
#include "gdb_main.h"
#include "gdb_task.h"
#include "target.h"
#include "exception.h"
#include "gdb_packet.h"
#include "memwatch.h"
#include "usb_cdc.h"
#ifdef ENABLE_RTT
#include "rtt.h"
#endif

// don't kill target by polling too much
#define POLLS_PER_SECOND 50
#define POLL_TICKS       (RT_TICK_PER_SECOND / POLLS_PER_SECOND)

static rt_thread_t gdb_server_thread = RT_NULL;

/* pbuf below is now only used by the remote protocol remote.c */

/* pbuf has to be aligned so the remote protocol can re-use it without causing Problems */
static char BMD_ALIGN_DEF(8) pbuf[GDB_PACKET_BUFFER_SIZE + 1U];

char *gdb_packet_buffer()
{
	return pbuf;
}

static void bmp_poll_loop(void)
{
	rt_tick_t tick_start, tick_stop;
	SET_IDLE_STATE(false);
	while (gdb_target_running && cur_target) {
		tick_start = rt_tick_get();
		gdb_poll_target();
		// Check again, as `gdb_poll_target()` may
		// alter these variables.
		if (!gdb_target_running || !cur_target)
			break;
		char c = gdb_if_getchar_to(0);
		if (c == '\x03' || c == '\x04')
			target_halt_request(cur_target);
		// pacing
		tick_stop = rt_tick_get();
		if (tick_stop - tick_start > POLL_TICKS)
			rt_thread_delay(POLL_TICKS + tick_start - tick_stop);
#ifdef ENABLE_RTT
		if (rtt_enabled)
			poll_rtt(cur_target);
#endif
		if (memwatch_cnt != 0)
			poll_memwatch(cur_target);
	}

	SET_IDLE_STATE(true);
	size_t size = gdb_getpacket(pbuf, GDB_PACKET_BUFFER_SIZE);
	// If port closed and target detached, stay idle
	if (pbuf[0] != '\x04' || cur_target)
		SET_IDLE_STATE(false);
	gdb_main(pbuf, GDB_PACKET_BUFFER_SIZE, size);
}

void gdb_server_task()
{
	platform_init();
	while (1) {
		cdc0_wait_for_dtr();
		TRY(EXCEPTION_ALL)
		{
			while (cdc0_connected())
				bmp_poll_loop();
		}
		CATCH()
		{
		default:
			gdb_putpacketz("EFF");
			target_list_free();
		}
	}
}

int app_blackmagic_init(void)
{
	gdb_server_thread = rt_thread_create("gdb server", gdb_server_task, RT_NULL, 8192, 4, 10);
	if (gdb_server_thread != RT_NULL)
		rt_thread_startup(gdb_server_thread);
}
