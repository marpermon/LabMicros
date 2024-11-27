unzip AI.zip
git clone https://github.com/libopencm3/libopencm3.git
make -j
arm-none-eabi-objcopy -O binary build/main.elf firmware.bin
st-flash --reset write firmware.bin 0x8000000