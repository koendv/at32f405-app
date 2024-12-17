#ifndef _USB_CDC_H
#define _USB_CDC_H

#include <rtthread.h>
#include <stdbool.h>

void cdc_init();

void cdc0_wait_for_dtr();
bool cdc0_connected();
bool cdc0_recv_empty();
char cdc0_getchar_timeout(uint32_t timeout_ticks);
void cdc0_write(uint8_t *buf, uint32_t nbytes);

void cdc1_wait_for_dtr();
bool cdc1_connected();
void cdc1_wait_for_char();
bool cdc1_recv_empty();
char cdc1_getchar();
void cdc1_write(uint8_t *buf, uint32_t nbytes);

#endif
