#ifndef _USB_CDC_H
#define _USB_CDC_H

#include <rtthread.h>
#include <rtdevice.h>
#include <stdbool.h>

extern struct rt_ringbuffer cdc1_read_rb;

void cdc0_write(uint8_t *buf, uint32_t nbytes);
void cdc1_write(uint8_t *buf, uint32_t nbytes);
bool get_cdc0_dtr();
bool get_cdc1_dtr();

#endif
