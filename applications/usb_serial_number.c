#include <rtthread.h>
#include <stdio.h>

#define MCU_ID1 (0x1FFFF7E8)

uint8_t usb_serial_number[13] = {0};

static int get_usb_serial_number()
{
    /* unique device ID (96 bits) stored at 0x1FFFF7E8 */
    uint16_t *uid = (uint16_t *)MCU_ID1;
    snprintf(usb_serial_number, sizeof(usb_serial_number), "%04X%04X%04X", uid[1] + uid[5], uid[0] + uid[4], uid[3]);
    usb_serial_number[sizeof(usb_serial_number) - 1] = '\0';
    return RT_EOK;
}

INIT_DEVICE_EXPORT(get_usb_serial_number);
