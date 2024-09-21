# patches

- drv_hard_i2c.patch

    [PR](https://github.com/RT-Thread/rt-thread/pull/9335) duplicate dma_isr() in hard_i2c and spi

- drv_spi.patch

    remove busy waiting from spi driver so other threads get cpu.
    used to update the display and write to sdcard in the background.

- lvgl-9.1.0-event-double-clicked.patch

    The CST816T touch panel is able to detect double clicks. Add LV_EVENT_DOUBLE_CLICKED to list of events.  This event may become part of lvgl, see [PR](https://githubissues.com/lvgl/lvgl/5351).
    
- lvgl-9.1.0-rtthread.patch

    [PR](https://github.com/lvgl/lvgl/pull/6667) fix st7789 display driver hang
    Also, put [mutex](https://docs.lvgl.io/master/porting/os.html) lv_lock()/lv_unlock() mutex around lvgl task. 
    
- spi3.patch

    add SPI3 device to at32f405-start bsp

- usb_dc_dwc2.patch

    CherryUSB uses SystemCoreClock. at32 uses system_core_clock
