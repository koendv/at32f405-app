#ifndef _USB_CDC_H
#define _USB_CDC_H

#include <rtthread.h>
#include <stdbool.h>

void cdc_init();

bool     cdc0_connected();
void     cdc0_set_speed(uint32_t speed);
void     cdc0_set_dtr(bool dtr);
bool     cdc0_recv_empty();
void     cdc0_wait_for_char();
uint32_t cdc0_get(uint8_t *buf, uint16_t length);
char     cdc0_getchar_timeout(uint32_t timeout_ticks);
void     cdc0_write(uint8_t *buf, uint32_t nbytes);

void     cdc1_wait_for_dtr();
bool     cdc1_connected();
void     cdc1_set_speed(uint32_t speed);
void     cdc1_set_dtr(bool dtr);
void     cdc1_wait_for_char();
bool     cdc1_recv_empty();
uint32_t cdc1_get(uint8_t *buf, uint16_t length);
char     cdc1_getchar();
void     cdc1_write(uint8_t *buf, uint32_t nbytes);

#endif
