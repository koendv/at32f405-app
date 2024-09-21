
/*
 * Copyright (c) 2020 panrui <https://github.com/Prry/rtt-ds3231>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-03-01     panrui      the first version
 */

#include <rtdevice.h>
#include <sys/time.h>
#include <stdlib.h>

#define DS3231_ADDR    0x68   /* i2c address */

#ifdef BSP_USING_HARD_I2C1
#define DS3231_I2C_BUS "hwi2c1" /* i2c bus */
#else
#define DS3231_I2C_BUS "i2c1" /* i2c bus */
#endif

#define DS3231_REG_TIME    0x00
#define DS3231_REG_ALARM1  0x07
#define DS3231_REG_ALARM2  0x0B
#define DS3231_REG_CONTROL 0x0E
#define DS3231_REG_STATUS  0x0F
#define DS3231_REG_TEMP    0x11

static struct rt_i2c_bus_device *i2c_bus = RT_NULL;

static uint8_t bcd_to_bin(uint8_t val)
{
    return val - 6 * (val >> 4);
}
static uint8_t bin_to_bcd(uint8_t val)
{
    return val + 6 * (val / 10);
}

/*
 * day of the week. 
 * input year y, month m 1..12, day d 1..32. 
 * returns day of the week. Sunday is 0, Saturday is 6.
 * e.g. Jan 1, 2000 is a Saturday, i.e. day_of_week(2000, 1, 1) returns 6
 */

const static rt_uint8_t days_in_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30};

static uint8_t day_of_week(uint16_t y, uint8_t m, uint8_t d)
{
    uint16_t days = d;
    uint8_t  dow;

    if (y >= 2000U) y -= 2000U;
    for (uint8_t i = 1; i < m; ++i)
        days += days_in_month[i - 1];
    if (m > 2 && y % 4 == 0) ++days;
    days += 365 * y + (y + 3) / 4 - 1;
    dow   = (days + 6) % 7;
    return dow;
}

static rt_err_t ds3231_write_reg(rt_uint8_t reg, rt_uint8_t *data, rt_uint8_t data_size)
{
    struct rt_i2c_msg msg[2];

    if (i2c_bus == RT_NULL)
    {
        i2c_bus = rt_i2c_bus_device_find(DS3231_I2C_BUS);
        if (i2c_bus == RT_NULL) return -RT_ERROR;
    }

    msg[0].addr  = DS3231_ADDR;
    msg[0].flags = RT_I2C_WR;
    msg[0].len   = 1;
    msg[0].buf   = &reg;
    msg[1].addr  = DS3231_ADDR;
    msg[1].flags = RT_I2C_WR | RT_I2C_NO_START;
    msg[1].len   = data_size;
    msg[1].buf   = data;

    if (rt_i2c_transfer(i2c_bus, msg, 2) == 2)
    {
        return RT_EOK;
    }
    else
    {
        rt_kprintf("i2c write error\r\n");
        return -RT_ERROR;
    }
}

static rt_err_t ds3231_read_reg(rt_uint8_t reg, rt_uint8_t *data, rt_uint8_t data_size)
{
    struct rt_i2c_msg msg[2];

    if (i2c_bus == RT_NULL)
    {
        i2c_bus = rt_i2c_bus_device_find(DS3231_I2C_BUS);
        if (i2c_bus == RT_NULL) return -RT_ERROR;
    }

    msg[0].addr  = DS3231_ADDR;
    msg[0].flags = RT_I2C_WR;
    msg[0].len   = 1;
    msg[0].buf   = &reg;
    msg[1].addr  = DS3231_ADDR;
    msg[1].flags = RT_I2C_RD;
    msg[1].len   = data_size;
    msg[1].buf   = data;

    if (rt_i2c_transfer(i2c_bus, msg, 2) == 2)
    {
        return RT_EOK;
    }
    else
    {
        rt_kprintf("i2c read error\r\n");
        return -RT_ERROR;
    }
}

/* set processor rtc time to ds3231 time */

void ds3231_sync()
{
    rt_err_t    ret = RT_EOK;
    rt_uint16_t year, mon, mday, hour, min, sec, wday;
    rt_uint8_t  buff[7];
    rt_uint8_t  control_reg;
    rt_uint8_t  status_reg;

    /* get time */
    /* XXX enable ertc clock, else setting rtc may fail */
    time_t now = time(RT_NULL);

    /* read control and status register */
    ret = ds3231_read_reg(DS3231_REG_CONTROL, buff, 2);
    if (ret != RT_EOK)
    {
        rt_kprintf("ds3231: i2c error\r\n");
        return;
    }
    control_reg = buff[0];
    status_reg  = buff[1];

    /* disable 32.768kHz output */
    if (status_reg & 0x08) // EN32KHZ
    {
        status_reg &= ~0x08;
        ret         = ds3231_write_reg(DS3231_REG_STATUS, &status_reg, 1);
    }
    if (status_reg & 0x80)
    { // ds3231 time invalid?
        rt_kprintf("ds3231: control %02x status %02x\r\n", control_reg, status_reg);
        rt_kprintf("check clock battery\r\n");
        return;
    }

    /* read time */
    ret = ds3231_read_reg(DS3231_REG_TIME, buff, 7);
    if (ret != RT_EOK) return;

    year = bcd_to_bin(buff[6]) + 2000;
    mon  = bcd_to_bin(buff[5] & 0x7f);
    mday = bcd_to_bin(buff[4]);
    hour = bcd_to_bin(buff[2]);
    min  = bcd_to_bin(buff[1]);
    sec  = bcd_to_bin(buff[0]);

#if 0
    rt_kprintf("year: %d\r\n", year);
    rt_kprintf("month: %d\r\n", mon);
    rt_kprintf("day: %d\r\n", mday);
    rt_kprintf("hour: %d\r\n", hour);
    rt_kprintf("min: %d\r\n", min);
    rt_kprintf("sec: %d\r\n", sec);
#endif

    if ((year <= 0) || (mon > 11) || (mday == 0) || (mday > 31) || (hour > 23) || (min > 59) || (sec > 60))
    {
        rt_kprintf("ds3231 date error\n");
        return;
    }

    ret = set_date(year, mon, mday);
    if (ret == RT_EOK)
    {
        ret = set_time(hour, min, sec);
    }

#if 0
    if (ret != RT_EOK)
    {
        rt_kprintf("rtc already set\r\n");
    }
#endif
    return;
}

void ds3231_date(int argc, char **argv)
{
    rt_err_t    ret = RT_EOK;
    rt_uint16_t year, mon, mday, hour, min, sec, wday;
    rt_uint8_t  buff[7];
    rt_uint8_t  status_reg;

    if (argc < 7)
    {
        /* check ds3231 time initialized */
        ret = ds3231_read_reg(DS3231_REG_STATUS, &status_reg, 1);
        if (ret != RT_EOK) return;
        if (status_reg >> 7)
        { // ds3231 clock invalid?
            rt_kprintf("date not set\r\n");
            return;
        }
        /* read time from ds3231 */
        ret = ds3231_read_reg(DS3231_REG_TIME, buff, 7);
        if (ret != RT_EOK) return;

        year = bcd_to_bin(buff[6]) + 2000;
        mon  = bcd_to_bin(buff[5] & 0x7f);
        mday = bcd_to_bin(buff[4]);
        hour = bcd_to_bin(buff[2]);
        min  = bcd_to_bin(buff[1]);
        sec  = bcd_to_bin(buff[0]);

        rt_kprintf("%d %d %d %d %d %d\r\n", year, mon, mday, hour, min, sec);
        return;
    }

    /* set time in ds3231 */
    year = atoi(argv[1]);
    mon  = atoi(argv[2]);
    mday = atoi(argv[3]);
    hour = atoi(argv[4]);
    min  = atoi(argv[5]);
    sec  = atoi(argv[6]);
    wday = day_of_week(year, mon, mday);

    buff[6] = bin_to_bcd(year % 100);
    buff[5] = bin_to_bcd(mon);
    buff[4] = bin_to_bcd(mday);
    buff[3] = bin_to_bcd(wday + 1);
    buff[2] = bin_to_bcd(hour);
    buff[1] = bin_to_bcd(min);
    buff[0] = bin_to_bcd(sec);

    ret = ds3231_write_reg(DS3231_REG_TIME, buff, 7);

    /* clear ds3231 clock stopped bit */
    ret = ds3231_read_reg(DS3231_REG_STATUS, &status_reg, 1);
    if (ret != RT_EOK) return;
    if (status_reg & 0x80)
    {
        status_reg &= ~0x80;
        ret         = ds3231_write_reg(DS3231_REG_STATUS, &status_reg, 1);
    }

    return;
}

/* returns ds3231 temperature, in units of 0.25 degree C */
rt_int16_t ds3231_temp_x4(void)
{
    rt_err_t   ret = RT_EOK;
    rt_uint8_t buf[2];
    rt_int16_t temp_x4;

    ret = ds3231_read_reg(DS3231_REG_TEMP, buf, 2);
    if (ret != RT_EOK) return -9999; // returns -9999 if i2c error

    temp_x4 = (buf[0] & 0x7f) << 2 | (buf[1] >> 6);
    if (buf[0] >> 7) temp_x4 = -temp_x4;

    return temp_x4;
}

/* prints ds3231 temperature, in units of 0.25 degree C */
void ds3231_temp(void)
{
    rt_err_t   ret = RT_EOK;
    rt_uint8_t buf[2];
    rt_uint8_t i, j, s;

    ret = ds3231_read_reg(DS3231_REG_TEMP, buf, 2);
    if (ret != RT_EOK) return;

    s = buf[0] >> 7 ? '-' : ' ';
    i = buf[0] & 0x7f;
    j = buf[1] >> 6;
    rt_kprintf("%c%d.%02dC\r\n", s, i, 25 * j);
}

#ifdef RT_USING_FINSH
#include <finsh.h>

MSH_CMD_EXPORT(ds3231_temp, print ds3231 temperature);
MSH_CMD_EXPORT(ds3231_sync, set rtc time to ds3231 time);
MSH_CMD_EXPORT(ds3231_date, set ds3231 time);
#endif /* RT_USING_FINSH & FINSH_USING_MSH */

