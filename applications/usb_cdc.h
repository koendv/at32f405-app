#ifndef _USB_CDC_H
#define _USB_CDC_H

#include <rtthread.h>
#include <stdbool.h>
#include "chry_ringbuffer.h"

void cdc0_acm_data_send(uint8_t *buf, uint32_t nbytes);
void cdc1_acm_data_send(uint8_t *buf, uint32_t nbytes);
bool get_cdc0_dtr();
bool get_cdc1_dtr();

#endif
