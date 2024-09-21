#include "general.h"
#include "platform.h"
#include "gdb_if.h"
#include "gdb_main.h"
#include "usb_cdc.h"

///////////////////////////////////////
/// black magic probe gdb interface
///////////////////////////////////////

#define GDB_BUFFER_LEN 512
static size_t gdb_buffer_used = 0U;
static char gdb_buffer[GDB_BUFFER_LEN];

/* read one character from gdb port */
char gdb_if_getchar(void)
{
	// XXX fixme
	return -1;
}

/* read one character from gdb port, with a timeout if no character read with x ms */
char gdb_if_getchar_to(const uint32_t timeout_ms)
{
	// XXX fixme
	return -1;
}

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
