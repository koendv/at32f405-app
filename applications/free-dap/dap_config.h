// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2017-2022, Alex Taradov <alex@taradov.com>. All rights reserved.

#ifndef _DAP_CONFIG_H_
#define _DAP_CONFIG_H_

#define DAP_CONFIG_ENABLE_JTAG 1

/*- Includes ----------------------------------------------------------------*/
#include "hal_config.h"
/* for CONFIG_USB_HS */
#include "usb_desc.h"
#ifdef PKG_USING_BLACKMAGIC
extern void platform_init(void);
#endif

/*- Definitions -------------------------------------------------------------*/
#define DAP_CONFIG_DEFAULT_PORT        DAP_PORT_SWD
#define DAP_CONFIG_DEFAULT_CLOCK       1000000 // Hz

#ifdef CONFIG_USB_HS
#define DAP_CONFIG_PACKET_SIZE         512
#else
#define DAP_CONFIG_PACKET_SIZE         64
#endif
#define DAP_CONFIG_PACKET_COUNT        2

#define DAP_CONFIG_JTAG_DEV_COUNT      8

// DAP_CONFIG_PRODUCT_STR must contain "CMSIS-DAP" to be compatible with the standard
#define DAP_CONFIG_VENDOR_STR          "Alex Taradov"
#define DAP_CONFIG_PRODUCT_STR         "Generic CMSIS-DAP Adapter"
#define DAP_CONFIG_SER_NUM_STR         usb_serial_number
#define DAP_CONFIG_CMSIS_DAP_VER_STR   "2.0.0"

//#define DAP_CONFIG_RESET_TARGET_FN     target_specific_reset_function
//#define DAP_CONFIG_VENDOR_FN           vendor_command_handler_function

// Attribute to use for performance-critical functions
#define DAP_CONFIG_PERFORMANCE_ATTR

// A value at which dap_clock_test() produces 1 kHz output on the SWCLK pin
#define DAP_CONFIG_DELAY_CONSTANT      14300

// A threshold for switching to fast clock (no added delays)
// This is the frequency produced by dap_clock_test(1) on the SWCLK pin
#define DAP_CONFIG_FAST_CLOCK          910000

/*- Prototypes --------------------------------------------------------------*/
extern uint8_t usb_serial_number[13];

/*- Implementations ---------------------------------------------------------*/

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWCLK_TCK_write(int value)
{
  HAL_GPIO_SWCLK_TCK_write(value);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWDIO_TMS_write(int value)
{
  HAL_GPIO_SWDIO_TMS_write(value);
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_TDI_write(int value)
{
#ifdef DAP_CONFIG_ENABLE_JTAG
  HAL_GPIO_TDI_write(value);
#else
  (void)value;
#endif
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_TDO_write(int value)
{
#ifdef DAP_CONFIG_ENABLE_JTAG
  HAL_GPIO_TDO_write(value);
#else
  (void)value;
#endif
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_nTRST_write(int value)
{
  (void)value;
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_nRESET_write(int value)
{
  HAL_GPIO_nRESET_write(value);
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_SWCLK_TCK_read(void)
{
  return HAL_GPIO_SWCLK_TCK_read();
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_SWDIO_TMS_read(void)
{
  return HAL_GPIO_SWDIO_TMS_read();
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_TDO_read(void)
{
#ifdef DAP_CONFIG_ENABLE_JTAG
  return HAL_GPIO_TDO_read();
#else
  return 0;
#endif
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_TDI_read(void)
{
#ifdef DAP_CONFIG_ENABLE_JTAG
  return HAL_GPIO_TDI_read();
#else
  return 0;
#endif
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_nTRST_read(void)
{
  return 0;
}

//-----------------------------------------------------------------------------
static inline int DAP_CONFIG_nRESET_read(void)
{
  return HAL_GPIO_nRESET_read();
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWCLK_TCK_set(void)
{
  HAL_GPIO_SWCLK_TCK_set();
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWCLK_TCK_clr(void)
{
  HAL_GPIO_SWCLK_TCK_clr();
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWDIO_TMS_in(void)
{
  HAL_GPIO_SWDIO_TMS_in();
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SWDIO_TMS_out(void)
{
  HAL_GPIO_SWDIO_TMS_out();
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_SETUP(void)
{
#ifdef PKG_USING_BLACKMAGIC
  /* set up jtag the same way as black magic probe */
  platform_init();
#else
  DAP_config();
  HAL_GPIO_SWCLK_TCK_in();
  HAL_GPIO_SWDIO_TMS_in();
  HAL_GPIO_nRESET_in();
#ifdef DAP_CONFIG_ENABLE_JTAG
  HAL_GPIO_TDO_in();
  HAL_GPIO_TDI_in();
#endif
#endif
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_DISCONNECT(void)
{
#ifdef PKG_USING_BLACKMAGIC
  /* leave jtag the same way as black magic probe */
  platform_init();
#else
  HAL_GPIO_SWCLK_TCK_in();
  HAL_GPIO_SWDIO_TMS_in();
  HAL_GPIO_nRESET_in();
#ifdef DAP_CONFIG_ENABLE_JTAG
  HAL_GPIO_TDO_in();
  HAL_GPIO_TDI_in();
#endif
#endif
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_CONNECT_SWD(void)
{
  HAL_GPIO_SWDIO_TMS_out();
  HAL_GPIO_SWDIO_TMS_set();

  HAL_GPIO_SWCLK_TCK_out();
  HAL_GPIO_SWCLK_TCK_set();

  HAL_GPIO_nRESET_out();
  HAL_GPIO_nRESET_set();

#ifdef DAP_CONFIG_ENABLE_JTAG
  HAL_GPIO_TDO_in();
  HAL_GPIO_TDI_in();
#endif
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_CONNECT_JTAG(void)
{
  HAL_GPIO_SWDIO_TMS_out();
  HAL_GPIO_SWDIO_TMS_set();

  HAL_GPIO_SWCLK_TCK_out();
  HAL_GPIO_SWCLK_TCK_set();

  HAL_GPIO_nRESET_out();
  HAL_GPIO_nRESET_set();

#ifdef DAP_CONFIG_ENABLE_JTAG
  HAL_GPIO_TDO_in();

  HAL_GPIO_TDI_out();
  HAL_GPIO_TDI_set();
#endif
}

//-----------------------------------------------------------------------------
static inline void DAP_CONFIG_LED(int index, int state)
{
  (void)index;
  (void)state;
#if 1
  if (index == 0)
    rt_pin_write(LED0_PIN, state ? PIN_LOW : PIN_HIGH);
#endif
}

//-----------------------------------------------------------------------------
__attribute__((always_inline)) static inline void DAP_CONFIG_DELAY(uint32_t cycles)
{
    for (volatile uint32_t counter = cycles; counter > 0; --counter)
        continue;
}

#endif // _DAP_CONFIG_H_
