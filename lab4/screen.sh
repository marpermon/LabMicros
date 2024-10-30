#!/bin/bash



cd /home/yo/local/src/repos/github/libopencm3-examples/examples/stm32/f4/stm32f429i-discovery/lcd-dma
make
arm-none-eabi-objcopy -O binary lcd-dma.elf firmware.bin
st-flash --reset write firmware.bin 0x8000000
cd
cd ../..
minicom
