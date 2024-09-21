#!/bin/bash -x
# build
CURRENT_UF2=/media/$USER/CherryUF2/CURRENT.UF2
#scons -c
scons
# extract binary from elf
arm-none-eabi-objcopy -O binary rtthread.elf rtthread.bin
# uf2 family id 0xf35c900d load address 0x90000000
bin2uf2 -f 0xf35c900d -o rtthread.uf2 0x90000000 rtthread.bin
# flash
if [ -e rtthread.uf2 -a -e $CURRENT_UF2 ]
then
  cp rtthread.uf2 $CURRENT_UF2
fi
