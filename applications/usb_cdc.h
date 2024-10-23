#ifndef _USB_CDC_H
#define _USB_CDC_H

#include <rtthread.h>
#include <stdbool.h>

void cdc_init();

void cdc0_configured();
void cdc0_set_dtr(bool dtr);
void cdc0_read(uint8_t *buf, uint32_t nbytes);
void cdc0_next_read();
void cdc0_write(uint8_t *buf, uint32_t nbytes);

void cdc1_configured();
void cdc1_set_dtr(bool dtr);
void cdc1_read(uint8_t *buf, uint32_t nbytes);
void cdc1_next_read();
void cdc1_write(uint8_t *buf, uint32_t nbytes);

#endif
