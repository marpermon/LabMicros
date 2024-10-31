# Instrucciones de uso: 

1. Mueva los archivos de esta carpeta a la carpeta del ejemplo lcd-dma del directorio ~/local/src/repos/github/libopencm3-examples/examples/stm32/f4/stm32f429i-discovery/lcd-dma, o donde usted la haya guardado, de forma que el archivo lcd-dma.c reemplace al que está en la carpeta
2. Utilice los siguientes comandos dentro de esta:

make
  
arm-none-eabi-objcopy -O binary lcd-dma.elf firmware.bin

st-flash --reset write firmware.bin 0x8000000

4. Abra otra terminal y corra el archivo sismo.py de la siguiente forma: python3 sismo.py
