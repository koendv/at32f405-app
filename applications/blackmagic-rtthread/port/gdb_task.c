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
#ifdef ENABLE_RTT
#include "rtt.h"
#endif

static rt_thread_t gdb_server_thread = RT_NULL;
static rt_thread_t gdb_poll_thread = RT_NULL;
static rt_sem_t target_access_sem = RT_NULL;
static rt_sem_t target_poll_sem = RT_NULL;

/* semaphore to guard target */

void take_target()
{
	rt_sem_take(target_access_sem, RT_WAITING_FOREVER);
}

void release_target()
{
	rt_sem_release(target_access_sem);
}

/* pbuf below is now only used by the remote protocol remote.c */

/* pbuf has to be aligned so the remote protocol can re-use it without causing Problems */
static char BMD_ALIGN_DEF(8) pbuf[GDB_PACKET_BUFFER_SIZE + 1U];

char *gdb_packet_buffer()
{
	return pbuf;
}

void gdb_server()
{
	while (1) {
		size_t size = gdb_getpacket(pbuf, GDB_PACKET_BUFFER_SIZE);
		take_target();
		SET_IDLE_STATE(0);
		TRY(EXCEPTION_ALL)
		{
			gdb_main(pbuf, GDB_PACKET_BUFFER_SIZE, size);
		}
		CATCH()
		{
		default:
			gdb_putpacketz("EFF");
			target_list_free();
		}
		SET_IDLE_STATE(1);
		if (gdb_target_running && cur_target)
			rt_sem_release(target_poll_sem); // start polling
		if (cdc0_dtr)                        // if !cdc0_dtr wait for cdc0_dtr(true)
			release_target();
	}
}

void gdb_poll()
{
	while (1) {
		// wait for gdb "run" or "continue" command
		rt_sem_take(target_poll_sem, RT_WAITING_FOREVER);
		while (gdb_target_running && cur_target) {
			take_target();
			SET_IDLE_STATE(0);
			TRY(EXCEPTION_ALL)
			{
				gdb_poll_target();
				// Check again, as `gdb_poll_target()` may
				// alter these variables.
				if (!gdb_target_running || !cur_target)
					break;
#ifdef ENABLE_RTT
				if (rtt_enabled)
					poll_rtt(cur_target);
#endif
				if (memwatch_cnt != 0)
					poll_memwatch(cur_target);
			}
			CATCH()
			{
			default:
				gdb_putpacketz("EFF");
				target_list_free();
			}
			SET_IDLE_STATE(1);
			release_target();
			rt_thread_mdelay(8);
		}
	}
}

int app_blackmagic_init(void)
{
	target_access_sem = rt_sem_create("gdb target", 0, RT_IPC_FLAG_FIFO);
	rt_sem_control(target_access_sem, RT_IPC_CMD_SET_VLIMIT, (void *)1);

	target_poll_sem = rt_sem_create("gdb poll", 0, RT_IPC_FLAG_FIFO);
	rt_sem_control(target_poll_sem, RT_IPC_CMD_SET_VLIMIT, (void *)1);

	gdb_server_thread = rt_thread_create("gdb server", gdb_server, RT_NULL, 8192, 4, 10);
	if (gdb_server_thread != RT_NULL)
		rt_thread_startup(gdb_server_thread);

	gdb_poll_thread = rt_thread_create("gdb poll", gdb_poll, RT_NULL, 4096, 4, 10);
	if (gdb_poll_thread != RT_NULL)
		rt_thread_startup(gdb_poll_thread);
}
