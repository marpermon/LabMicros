#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/ltdc.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/spi.h>

#include "clock.h"
#include "console.h"
#include "lcd-spi.h"
#include "sdram.h"

#include "gyro.h"
#include "pantalla.h"

// USART setup for console

int main(void) {
    rcc_osc_on(RCC_HSI);
    rcc_wait_for_osc_ready(RCC_HSI);
    rcc_set_sysclk_source(RCC_CFGR_SW_HSI);
    rcc_apb1_frequency = 16000000;
    rcc_apb2_frequency = 16000000;
    usart_setup();
    spi_setup();
    clock_setup();

    usart_print("Starting gyroscope setup...\r\n");

    // Verify gyroscope by reading WHO_AM_I register
    uint8_t who_am_i = read_register(GYR_WHO_AM_I);
    usart_print("WHO_AM_I: ");
    usart_print_int(who_am_i);
    usart_print("\r\n");

    // Configure gyroscope control registers
    write_register(GYR_CTRL_REG1, 0x0F);  // Normal mode, all axes enabled
    usart_print("reg1\r\n");
    write_register(GYR_CTRL_REG4, 0x30);  // Full scale at ±2000 dps

    usart_print("Gyroscope initialized.\r\n");

    //* set up USART 1. */
    console_setup(115200);
    console_stdio_setup();

    /* set up SDRAM. */
    //El orden afecta el resultado
    sdram_init();
    lcd_dma_init();
    lcd_spi_init();
    usart_print("LCD\r\n");
    


    // Clear the screen by setting all pixels to a background color
    uint32_t background_color = 0xFF000000; // Black background
    for (size_t i = 0; i < LCD_LAYER1_PIXELS; i++)
    {
        lcd_layer1_frame_buffer[i] = background_color;

    }

    // Set up HSI (internal 16MHz clock) as the system clock
    usart_print("Screen\r\n");
    

    
    // Calibrate the gyroscope
    calibrate_gyroscope();

    // Draw your strings using the right-aligned string function
    
    uint32_t text_color = 0xFFFFFFFF; // White text

    // Set starting x position to the right edge minus margin
    int x_c = LCD_LAYER1_WIDTH - 10; // Right edge minus 10-pixel margin
    int y_c = 20; // Starting y position

    gpio_set(GPIOC, GPIO2);	   /* Turn off chip select */
    //gpio_set(GYR_CS);  // Deassert CS high
    gpio_set(GPIOC, GPIO1);
    // Continuously read and print X, Y, and Z axis data relative to baseline
    while (1) {
        int16_t x = read_axis(GYR_OUT_X_L, GYR_OUT_X_H) - x_baseline;
        int16_t y = read_axis(GYR_OUT_Y_L, GYR_OUT_Y_H) - y_baseline;
        int16_t z = read_axis(GYR_OUT_Z_L, GYR_OUT_Z_H) - z_baseline;

        usart_print("X: ");
        usart_print_int(x);
        draw_string(x_c, y_c, "X: ", text_color);
        draw_int(x_c-20, y_c, x, text_color);
        usart_print("\tY: ");
        usart_print_int(y);
        draw_string(x_c, y_c+20, "Y: ", text_color);
        draw_int(x_c-20, y_c+20, y, text_color);
        usart_print("\tZ: ");
        usart_print_int(z);
        usart_print("\r\n");
        draw_string(x_c, y_c+40, "Z: ", text_color);
        draw_int(x_c-20, y_c+40, z, text_color);

        for (int i = 0; i < 3000000; i++) {
            __asm__("NOP");
        }
    }

    return 0;
    

}


