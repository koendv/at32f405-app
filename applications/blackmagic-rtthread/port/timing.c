/*
 * This file is part of the Black Magic Debug project.
 *
 * Copyright (C) 2015 Gareth McMullin <gareth@blacksphere.co.nz>
 * Copyright (C) 2023 1BitSquared <info@1bitsquared.com>
 * Modified by Rachel Mant <git@dragonmux.network>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "general.h"
#include "platform.h"

bool running_status = false;
uint32_t target_clk_divider = 1;

/* delay ms milliseconds */
void platform_delay(uint32_t ms)
{
	rt_thread_mdelay(ms);
}

/* milliseconds since boot */
uint32_t platform_time_ms(void)
{
	return rt_tick_get_millisecond();
}

/* set bit-banging delay */
uint32_t platform_max_frequency_get(void)
{
	uint32_t f;
	if (target_clk_divider == UINT32_MAX)
		f = BLACKMAGIC_FASTCLOCK;
	else
		f = BLACKMAGIC_DELAY_CONSTANT * 1000 / (target_clk_divider + 1);
	return f;
}

void platform_max_frequency_set(const uint32_t frequency)
{
	if (frequency >= BLACKMAGIC_FASTCLOCK)
		target_clk_divider = UINT32_MAX;
	else
		target_clk_divider = BLACKMAGIC_DELAY_CONSTANT * 1000 / (frequency + 1);
}

/* FINSH clock_test command 
   simulate toggling SWCLK and SWDIO to calibrate delay loop
   and determine BLACKMAGIC_DELAY_CONSTANT, BLACKMAGIC_FASTCLOCK  */

static void clock_test(int argc, char **argv)
{
	uint32_t delay = 0;
	if (argc >= 1)
		delay = atoi(argv[0]);

	rt_pin_mode(SWCLK_PIN, PIN_MODE_OUTPUT);
	rt_pin_mode(SWDIO_PIN, PIN_MODE_OUTPUT);

	if (delay) {
		while (1) {
			rt_pin_write(SWCLK_PIN, PIN_LOW);
			rt_pin_write(SWDIO_PIN, PIN_HIGH);
			for (volatile uint32_t counter = delay; counter > 0; --counter)
				continue;
			rt_pin_write(SWCLK_PIN, PIN_HIGH);
			rt_pin_write(SWDIO_PIN, PIN_LOW);
			for (volatile uint32_t counter = delay; counter > 0; --counter)
				continue;
		}
	} else {
		while (1) {
			rt_pin_write(SWCLK_PIN, PIN_LOW);
			rt_pin_write(SWDIO_PIN, PIN_HIGH);
			rt_pin_write(SWCLK_PIN, PIN_HIGH);
			rt_pin_write(SWDIO_PIN, PIN_LOW);
		}
	}
}

MSH_CMD_EXPORT(clock_test, clock_test i : calibrate bit - banging delay loop);
