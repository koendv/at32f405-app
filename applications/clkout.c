#include <rtthread.h>
#include <rtdevice.h>
#include "drv_common.h"
#include "drv_gpio.h"

/*
 * clkout() outputs the 32.768kHz LSE crystal on pin PB13.
 * Use to accurately measure LSE oscillator frequency.
 *
 * Note: Do not measure LSE oscillator touching the crystal with 
 * an oscilloscope probe; the capacitance of the probe will 
 * change the oscillator frequency slightly.
 */

/*
 * code from AT32 Workbench.
 * In PinOut Configuration: CRM->Clock Output
 * In Clock Configuration: Choose lext or hext.
 * When outputting hext, set division by 12: clockoutdiv1 /3,  clockoutdiv2 /4
 */

/**
  * @brief  init clkout_32k function
  * @param  none
  * @retval none
  */
void clkout_32k(void)
{
    gpio_init_type gpio_init_struct;

    /* enable periph clock */
    crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);

    /* set default parameter */
    gpio_default_para_init(&gpio_init_struct);
    /* config gpio */
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode           = GPIO_MODE_MUX;
    gpio_init_struct.gpio_pins           = GPIO_PINS_13;
    gpio_init_struct.gpio_pull           = GPIO_PULL_NONE;
    gpio_init(GPIOB, &gpio_init_struct);
    /* config gpio mux function */
    gpio_pin_mux_config(GPIOB, GPIO_PINS_SOURCE13, GPIO_MUX_0);

    /* config clkout output clock source */
    crm_clock_out_set(CRM_CLKOUT_LEXT);
    /* config clkout div */
    crm_clkout_div_set(CRM_CLKOUT_DIV1_1, CRM_CLKOUT_DIV2_1);
}

/*
 * clkout() outputs the 12MHz HSE crystal on pin PB13.
 * The frequency is divided by 12, to give 1MHz exactly.
 * Use to accurately measure HSE oscillator frequency.
 *
 */

/**
  * @brief  init clkout_12m function
  * @param  none
  * @retval none
  */
void clkout_12m(void)
{
    gpio_init_type gpio_init_struct;

    /* enable periph clock */
    crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);

    /* set default parameter */
    gpio_default_para_init(&gpio_init_struct);
    /* config gpio */
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode           = GPIO_MODE_MUX;
    gpio_init_struct.gpio_pins           = GPIO_PINS_13;
    gpio_init_struct.gpio_pull           = GPIO_PULL_NONE;
    gpio_init(GPIOB, &gpio_init_struct);
    /* config gpio mux function */
    gpio_pin_mux_config(GPIOB, GPIO_PINS_SOURCE13, GPIO_MUX_0);

    /* config clkout output clock source */
    crm_clock_out_set(CRM_CLKOUT_HEXT);
    /* config clkout div */
    crm_clkout_div_set(CRM_CLKOUT_DIV1_3, CRM_CLKOUT_DIV2_4);
}

MSH_CMD_EXPORT(clkout_32k, mirror 32.768kHz LSE clock on pin PB13);
MSH_CMD_EXPORT(clkout_12m, mirror 12Mhz HSE clock on pin PB13);
