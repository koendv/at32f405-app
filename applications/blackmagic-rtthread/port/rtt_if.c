/*
 * This file is part of the Black Magic Debug project.
 *
 * MIT License
 *
 * Copyright (c) 2021 Koen De Vleeschauwer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "general.h"
#include "platform.h"
#include "rtt.h"
#include "rtt_if.h"

#include "usb_cdc.h"

/*********************************************************************
*
*       rtt terminal i/o
*
**********************************************************************
*/

/* rtt host to target: read one character */
int32_t rtt_getchar(const uint32_t channel)
{
	char ch;
	rt_ringbuffer_getchar(&cdc1_read_rb, &ch);
	return ch;
}

/* rtt host to target: true if no characters available for reading */
bool rtt_nodata(const uint32_t channel)
{
	return rt_ringbuffer_data_len(&cdc1_read_rb) != 0;
}

/* rtt target to host: write string */
uint32_t rtt_write(const uint32_t channel, const char *buf, uint32_t len)
{
	cdc1_write((uint8_t *)buf, len);
	return len;
}
