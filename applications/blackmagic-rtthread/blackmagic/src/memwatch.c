#include "general.h"
#include "gdb_packet.h"
#include "memwatch.h"
#include "rtt_if.h"
#if PC_HOSTED == 1
#include <unistd.h>
#endif

memwatch_s memwatch_table[MEMWATCH_NUM];
uint32_t memwatch_cnt = 0;
bool memwatch_timestamp = false;

void poll_memwatch(target_s *cur_target)
{
	union val32_u {
		uint32_t i;
		volatile float f;
	} val;

	char buf[64];
	char timestamp[64];
	uint32_t len;
	if (!cur_target || (memwatch_cnt == 0))
		return;

	for (uint32_t i = 0; i < memwatch_cnt; i++) {
		if (!target_mem32_read(cur_target, &val.i, memwatch_table[i].addr, sizeof(val.i)) &&
			(val.i != memwatch_table[i].value)) {
			if (memwatch_timestamp)
				snprintf(timestamp, sizeof(timestamp), "%" PRIu32 " ", platform_time_ms());
			else
				timestamp[0] = '\0';
			switch (memwatch_table[i].format) {
			case MEMWATCH_FMT_SIGNED:
				len = snprintf(buf, sizeof(buf), "%s%s %" PRId32 "\r\n", timestamp, memwatch_table[i].name, val.i);
				break;
			case MEMWATCH_FMT_UNSIGNED:
				len = snprintf(buf, sizeof(buf), "%s%s %" PRIu32 "\r\n", timestamp, memwatch_table[i].name, val.i);
				break;
			case MEMWATCH_FMT_FLOAT:
				len = snprintf(buf, sizeof(buf), "%s%s %.*g\r\n", timestamp, memwatch_table[i].name,
					memwatch_table[i].precision, val.f);
				break;
			case MEMWATCH_FMT_HEX:
			default:
				len = snprintf(buf, sizeof(buf), "%s%s 0x%" PRIx32 "\r\n", timestamp, memwatch_table[i].name, val.i);
				break;
			}
			buf[sizeof(buf) - 1] = '\0';
			rtt_write(buf, len);
			memwatch_table[i].value = val.i;
		}
	}
	return;
}
