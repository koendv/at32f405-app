#ifndef DS3231_UTIL_H
#define DS3231_UTIL_H

/*
 * ds3231 is a real-time clock.
 * the ds3231 runs on an external battery, and keeps time even when the system is switched off.
 * the ds3231 is very accurate, because it compensates for temperature changes.
 */

/*
 * sets the ds3231 time. 
 * the 'ds3231_date' command is similar to the 'date' command.
 * arguments: ds3231_date year month day hour minute seconds
 * example: ds3231_date 2024 6 8 8 16 0
 * 'ds3231_date' without arguments prints the current ds3231 time.
 */
void ds3231_date(int argc, char **argv);

/* 
 * sets the rtc time to the ds3231 time
 */

void ds3231_sync();

/*
 * ds3231 temperature, in units of 0.25 degree C
 */

rt_int16_t ds3231_temp_x4(void);

/*
 * prints the temperature of the ds3231 chip.
 */
void ds3231_temp(void);

#endif
