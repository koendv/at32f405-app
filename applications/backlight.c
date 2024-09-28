#include <rtthread.h>
#include <rtdevice.h>
#include "drv_common.h"
#include "drv_gpio.h"
#include "lvgl.h"

#include "pinout.h"

#define ADC_DEV_NAME        "adc1" /* ADC device name */
#define ADC_LIGHT_CHANNEL   10     /* ADC light sensor channel */
#define ADC_VIO_CHANNEL     13     /* ADC target volyage channel */
#define ADC_REF_VOLTAGE     3300   /* ADC reference voltage 3.3V in millivolts */
#define ADC_CONVERT_BITS    12     /* ADC value 0 .. 4096 */
#define PWM_DEV_NAME        "pwm1" /* PWM device name */
#define PWM_DISPLAY_CHANNEL 1      /* PWM channel of display led */
#define PWM_PERIOD_BITS     23     /* PWM period is 1 << 23 nanoseconds or 0.008 seconds */
#define UPDATE_DELAY        497    /* delay in ms between brightness updates */
#define LOWPASS_BETA        3      /* brightness lowpass filter, larger is slower */
#define SLEEP_TIME          300    /* seconds of inactivity before screen blanking */

/*
 * CONFIG_RT_USING_ADC=y
 * CONFIG_BSP_USING_ADC=y
 * CONFIG_BSP_USING_ADC1=y
 * CONFIG_RT_USING_PWM=y
 * CONFIG_BSP_USING_PWM=y
 * CONFIG_BSP_USING_PWM1=y
 * CONFIG_BSP_USING_PWM1_CH1=y
 */

/*
 * updates display backlight every two seconds. 
 * sets display led intensity proportional with ambient light sensor.
 * reads target vio voltage.
 */

static rt_thread_t adc_threadid = RT_NULL;
static rt_uint32_t brightness;

void adc_read_func()
{
    rt_adc_device_t       adc_dev;
    struct rt_device_pwm *pwm_dev; /* PWM device handle */
    rt_uint32_t           pwm_period = 1;
    rt_uint32_t           pwm_duty_cycle;
    rt_uint32_t           light_sensor;

    /* ambient light sensor adc */
    adc_dev = (rt_adc_device_t)rt_device_find(ADC_DEV_NAME);
    if (adc_dev == RT_NULL)
    {
        rt_kprintf("adc error\n");
        return;
    }
    rt_adc_enable(adc_dev, ADC_LIGHT_CHANNEL);
    rt_adc_enable(adc_dev, ADC_VIO_CHANNEL);
    rt_pin_mode(DISP_LED_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(DISP_LED_PIN, PIN_HIGH);

    /* initialize lowpass filter */
    brightness = rt_adc_read(adc_dev, ADC_LIGHT_CHANNEL);

    /* display led pwm */
    pwm_dev = (struct rt_device_pwm *)rt_device_find(PWM_DEV_NAME);
    if (pwm_dev == RT_NULL)
    {
        rt_kprintf("pwm error\n");
        return;
    }
    rt_pwm_set(pwm_dev, PWM_DISPLAY_CHANNEL, pwm_period, 0);
    rt_pwm_enable(pwm_dev, PWM_DISPLAY_CHANNEL);

    pwm_period = tmr_period_value_get(TMR1);
    // to avoid flickering, update pwm duty cycle when new duty cycle begins
    tmr_output_channel_buffer_enable(TMR1, TMR_SELECT_CHANNEL_1, TRUE);

    while (1)
    {
        /* ambient light sensor */
        light_sensor = rt_adc_read(adc_dev, ADC_LIGHT_CHANNEL);

        /* lowpass filter */
        brightness   = (brightness << LOWPASS_BETA) - brightness;
        brightness  += light_sensor;
        brightness >>= LOWPASS_BETA;

        /* scale brightness to pwm range */
        pwm_duty_cycle = brightness * pwm_period / 4095;
        if (pwm_duty_cycle > pwm_period) pwm_duty_cycle = pwm_period;

        /* sleeping */
        if (lv_disp_get_inactive_time(NULL) > SLEEP_TIME * RT_TICK_PER_SECOND) pwm_duty_cycle = 0;

        /* set pwm duty cycle */
        tmr_channel_value_set(TMR1, TMR_SELECT_CHANNEL_1, pwm_duty_cycle);
        rt_pwm_set(pwm_dev, PWM_DISPLAY_CHANNEL, pwm_period, pwm_duty_cycle);
        rt_pwm_enable(pwm_dev, PWM_DISPLAY_CHANNEL); // XXX ought not to be necessary
        rt_thread_mdelay(UPDATE_DELAY);              // update twice per second
    }
}

int app_adc_init(void)
{
    adc_threadid = rt_thread_create("adc", adc_read_func, RT_NULL, 1024, 5, 10);
    if (adc_threadid != RT_NULL)
        return rt_thread_startup(adc_threadid);
    else
        return -RT_ERROR;
}

INIT_APP_EXPORT(app_adc_init);