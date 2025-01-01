#include <rtthread.h>
#include <rtdevice.h>

#include "drv_common.h"
#include "at32_msp.h"
#include "usb_desc.h"
#include "usb_cdc.h"

/*
 for at32:
 patch rt-thread/components/drivers/usb/cherryusb/port/dwc2/usb_dc_dwc2.c:

 #ifdef SOC_FAMILY_AT32
 #define SystemCoreClock system_core_clock
 extern unsigned int system_core_clock;
 #else
 extern uint32_t SystemCoreClock;
 #endif

 */

void usb_dc_low_level_init(void)
{
    at32_msp_usb_init(NULL);
    crm_periph_clock_enable(CRM_OTGHS_PERIPH_CLOCK, TRUE);
    nvic_irq_enable(OTGHS_IRQn, 0, 0);
}

void OTGHS_IRQHandler(void)
{
    extern void USBD_IRQHandler(uint8_t busid);
    USBD_IRQHandler(0);
}

int usbd_app_init(void)
{
    cdc_init();
    cdc_acm_init(0, OTGHS_BASE);
    return 0;
}

INIT_COMPONENT_EXPORT(usbd_app_init);
