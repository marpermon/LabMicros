## Descripción

La presente es una actualizaciòn y mejora de lo que correspondía al **lab5**.

## Instrucciones de uso:

1. Muévase a la carpeta `Intentolibopen`.

2. Ejecute el archivo `exec.sh` con el comando `./exec.sh`. Habilite permisos de ejecución.

3. Dicho archivo descomprimirá el zip con las librerías de **EdgeImpulse** y descargará **libopencm3** a su carpeta. También ejecutará el pryecto y lo flashea al **stm**.

4. Si desea borrar los archivos objeto, puede ejecutar el comando `make clean`.

5. Si quiere ejecutar los archivos que simplemente captan los ejes del giroscopio, ejecute el comando: `cp -r libopencm3 ../SPI` 

6. Vaya a la carpeta `SPI` y utilice el comando `make`.

