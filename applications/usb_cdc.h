#ifndef _USB_CDC_H
#define _USB_CDC_H

#include <rtthread.h>
#include <stdbool.h>

void cdc_init();

bool cdc0_connected();
void cdc0_wait_for_dtr();
char cdc0_getchar_timeout(uint32_t timeout_ticks);
void cdc0_write(uint8_t *buf, uint32_t nbytes);

void cdc1_wait_for_dtr();
void cdc1_wait_for_char();
bool cdc1_connected();
char cdc1_getchar();
bool cdc1_nodata();
void cdc1_write(uint8_t *buf, uint32_t nbytes);

#endif
