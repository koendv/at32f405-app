#include <stdint.h>
#include <rtthread.h>
#include "at32f402_405.h"
#include "at32_msp.h"
#include "usb_desc.h"
#include "usb_cdc.h"

/*
 for at32 high-speed usb
 in rt-thread/components/drivers/usb/cherryusb/port/dwc2/usb_dc_dwc2.c add:
 #define SystemCoreClock system_core_clock
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

int app_usbd_init(void)
{
	cdc_init();
	cdc_acm_msc_init(0, OTGHS_BASE);
	return 0;
}

INIT_APP_EXPORT(app_usbd_init);
