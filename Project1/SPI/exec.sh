cd src
make -j
arm-none-eabi-objcopy -O binary adc_printf.elf firmware.bin
st-flash --reset write firmware.bin 0x8000000