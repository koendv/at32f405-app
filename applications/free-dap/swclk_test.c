#include <rtthread.h>
#include <stdlib.h>
#include <stdint.h>
#include "dap.h"

/* FINSH swclk_test command 
   determine DAP_CONFIG_DELAY_CONSTANT, DAP_CONFIG_FAST_CLOCK  */

static void swclk_test(int argc, char **argv)
{
    uint32_t delay = 0;
    if (argc >= 1)
        delay = atoi(argv[0]);
    dap_clock_test(delay);
}

MSH_CMD_EXPORT(swclk_test, calibrate bit - banging delay loop);
