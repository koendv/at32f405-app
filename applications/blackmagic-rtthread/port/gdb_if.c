#include "general.h"
#include "platform.h"
#include "gdb_if.h"
#include "gdb_main.h"
#include "usb_cdc.h"

///////////////////////////////////////
/// black magic probe gdb interface
///////////////////////////////////////

#define GDB_BUFFER_LEN 511
static size_t gdb_buffer_used = 0U;
static char gdb_buffer[GDB_BUFFER_LEN];

void gdb_if_putchar(const char c, const int flush)
{
	gdb_buffer[gdb_buffer_used++] = c;
	if (flush || gdb_buffer_used == GDB_BUFFER_LEN) {
		cdc0_acm_data_send(gdb_buffer, gdb_buffer_used);
		gdb_buffer_used = 0;
	}
}

size_t debug_serial_debug_write(const char *buf, const size_t len)
{
	cdc1_acm_data_send((uint8_t *)buf, len);
	return len;
}
