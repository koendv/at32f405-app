#include <string.h>
#include <stdbool.h>
#include <rtthread.h>
#include <usbd_core.h>
#include <usbd_msc.h>
#include "uf2.h"
#include "usb_desc.h"

#define BLOCK_SIZE  512
#define BLOCK_COUNT 0x10109

static WriteState _wr_state = {0}; // keeps track of number of blocks written

void usbd_msc_get_cap(uint8_t busid, uint8_t lun, uint32_t *block_num, uint32_t *block_size)
{
    (void)busid, lun;
    *block_num  = BLOCK_COUNT; //Pretend having so many buffer,not has actually.
    *block_size = BLOCK_SIZE;
    uf2_init();
}

int usbd_msc_sector_read(uint8_t busid, uint8_t lun, uint32_t sector, uint8_t *buffer, uint32_t length)
{
    (void)busid, lun;
    if ((sector < BLOCK_COUNT) && (length == BLOCK_SIZE))
        uf2_read_block(sector, buffer);
    else
    {
        memset(buffer, 0, length);
        rt_kprintf("uf2 read error, block %d, length %d\r\n", sector, length);
    }
    return 0;
}

int usbd_msc_sector_write(uint8_t busid, uint8_t lun, uint32_t sector, uint8_t *buffer, uint32_t length)
{
    (void)busid, lun;
    if ((sector < BLOCK_COUNT) && (length == BLOCK_SIZE))
        uf2_write_block(0, buffer, &_wr_state);
    else
        rt_kprintf("uf2 write error, sector %d, length %d\r\n", sector, length);
    return 0;
}
