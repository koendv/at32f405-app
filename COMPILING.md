# compile at32f405-app

[![at32f405 tool](doc/at32f405-tool-small.jpeg)](doc/at32f405-tool.jpeg)

This is an arm  debugger. The project consists of hardware, bootloader and application:

- [hardware project](https://oshwlab.com/koendv/at32f405-tool).
- [bootloader](https://github.com/koendv/at32f405-uf2boot)
- [application](https://github.com/koendv/at32f405-app)

This document describes how to compile the app for at32f405.

## building

The RT-Thread operating system has an IDE, [RT-Thread Studio](https://www.rt-thread.io/studio.html),  and a command-line build system, [env](https://github.com/RT-Thread/env). I develop using the command line on linux.

### source tree

Set up the source tree.

```sh
git clone https://github.com/RT-Thread/rt-thread
cd rt-thread/
git checkout ebe2926cd610661e210b70be1a22bac13923f4fb
cd bsp/at32/
git clone --recursive https://github.com/koendv/at32f405-app/
cd at32f405-app
pkgs --update
cd packages/LVGL-v9.1.0/
patch -p0 < ../../patches/lvgl-9.1.0-event-double-clicked.patch
patch -p0 < ../../patches/lvgl-9.1.0-rtthread.patch
# back to rt-thread top
cd ../../../../..
patch -p1 < bsp/at32/at32f405-app/patches/usb_dc_dwc2.patch
patch -p1 < bsp/at32/at32f405-app/patches/drv_hard_i2c.patch
patch -p1 < bsp/at32/at32f405-app/patches/drv_spi.patch
patch -p1 < bsp/at32/at32f405-app/patches/at32f402_405_can.patch
```
### environment
Set up the [xpack](https://github.com/xpack-dev-tools/arm-none-eabi-gcc-xpack/releases/tag/v12.3.1-1.2) arm-none-eabi-gcc compiler.
Set up the  [env](https://github.com/RT-Thread/env) build environment. Install [bin2uf2](https://github.com/tinyvision-ai-inc/uf2-utils).

### black magic rtthread package

The "black magic" rt-thread package needs to be added to the rt-thread environment by hand. Follow the [instructions](https://github.com/koendv/blackmagic-rtthread/tree/main#installation).

### compiling

Compile first the .elf, and then the .uf2 binary.

```sh
cd rt-thread/bsp/at32/at32f405-app/
# compile rtthread.elf
scons
# extract binary from elf
arm-none-eabi-objcopy -O binary rtthread.elf rtthread.bin
# uf2 family id 0xf35c900d load address 0x90000000
bin2uf2 -f 0xf35c900d -o rtthread.uf2 0x90000000 rtthread.bin
```

