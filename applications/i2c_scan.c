#include <rtthread.h>
#include <rtdevice.h>
#include "drv_common.h"
#include "drv_gpio.h"

struct rt_i2c_bus_device *i2c_bus;


int i2c_probe(char addr)
{
    unsigned char cmd[1];
    cmd[0] = 0;

    struct rt_i2c_msg msgs;
    msgs.addr  = addr;
    msgs.flags = RT_I2C_WR;
    msgs.buf   = cmd;
    msgs.len   = 0;

    return rt_i2c_transfer(i2c_bus, &msgs, 1);
}


void i2c_scan()
{
    i2c_bus               = rt_i2c_bus_device_find("i2c1");
    rt_uint8_t start_addr = 0x08;
    rt_uint8_t stop_addr  = 0x7f;
    rt_kputs("    00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");
    rt_uint8_t pos = start_addr <= 0xF ? 0x00 : start_addr & 0xF;
    for (; pos < stop_addr; pos++)
    {
        if ((pos & 0xF) == 0)
        {
            rt_kprintf("\n%02X: ", pos);
        }
        if (pos < start_addr)
        {
            rt_kputs("   ");
            continue;
        }
        if (i2c_probe(pos) == 1)
        {
            rt_kprintf("%02X", pos);
        }
        else
        {
            rt_kputs("--");
        }
        rt_kputs(" ");
    }
    rt_kputs("\n");
}

MSH_CMD_EXPORT(i2c_scan, i2c tools);

