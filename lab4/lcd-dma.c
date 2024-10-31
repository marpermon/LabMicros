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
#include <libopencm3/stm32/adc.h>

#include "clock.h"
#include "console.h"
#include "lcd-spi.h"
#include "sdram.h"

#include "gyro.h"
#include "pantalla.h"

static void button_setup(void);
static void boton(int *estado);
uint16_t read_adc(void);

uint16_t read_adc(void) {
    // Start ADC conversion
    adc_start_conversion_regular(ADC1);

    // Wait for conversion to complete
    while (!(ADC_SR(ADC1) & ADC_SR_EOC));

    // Read the ADC value
    return adc_read_regular(ADC1);
}

static void button_setup(void)
{
	/* Enable GPIOA clock. */
	rcc_periph_clock_enable(RCC_GPIOA);

	/* Set GPIO0 (in GPIO port A) to 'input open-drain'. */
	gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO0);
}

static void boton(int *estado)
{   
    bool A = false;
    if (gpio_get(GPIOA, GPIO0)) {
        for (int i = 0; i < 1500000; i++) {
            __asm__("nop");//debounce
            A = true;
        } if (gpio_get(GPIOA, GPIO0) && A) {
            *estado = !(*estado);
        }
    }
}

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


    // Verify gyroscope by reading WHO_AM_I register
    uint8_t who_am_i = read_register(GYR_WHO_AM_I);
    usart_print_int(who_am_i);
    usart_print("\r\n");

    // Configure gyroscope control registers
    write_register(GYR_CTRL_REG1, 0x0F);  // Normal mode, all axes enabled
    write_register(GYR_CTRL_REG4, 0x30);  // Full scale at ±2000 dps


    //* set up USART 1. */
    console_setup(115200);
    console_stdio_setup();

    /* set up SDRAM. */
    //El orden afecta el resultado
    sdram_init();
    lcd_dma_init();
    lcd_spi_init();


    // Clear the screen by setting all pixels to a background color
    uint32_t background_color = 0xFF000000; // Black background
    for (size_t i = 0; i < LCD_LAYER1_PIXELS; i++)
    {
        lcd_layer1_frame_buffer[i] = background_color;
    }

    // Set up HSI (internal 16MHz clock) as the system clock

    // Calibrate the gyroscope
    calibrate_gyroscope();

    // Draw your strings using the right-aligned string function
    
    uint32_t text_color = 0xFFFFFFFF; // White text

    // Set starting x position to the right edge minus margin
    int x_c = LCD_LAYER1_WIDTH - 10; // Right edge minus 10-pixel margin
    int y_c = 20; // Starting y position

    gpio_set(GPIOC, GPIO2);	   /* Turn off chip select */

  // Gyroscope CS pin on PC1
    gpio_set(GPIOC, GPIO1);
    // Continuously read and print X, Y, and Z axis data relative to baseline
	

    int16_t x = x_baseline;
    int16_t y = y_baseline;
    int16_t z = z_baseline;

    button_setup();

    int conectar = 1;

    gpio_mode_setup(GPIOB, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO1);
    rcc_periph_clock_enable(RCC_ADC1);

    // ADC configuration
    adc_power_on(ADC1);
    adc_set_sample_time(ADC1, ADC_CHANNEL1, ADC_SMPR_SMP_3CYC); // Set sample time for channel 14
    adc_set_regular_sequence(ADC1, 1, (uint8_t[]){1}); // Use channel 14 (PB14)
    int batt = read_adc();

    while (conectar) {
        
            draw_int(x_c-20, y_c, x, background_color); //borrar nùmero anterior
            draw_int(x_c-20, y_c+20, y, background_color);
            draw_int(x_c-20, y_c+40, z, background_color);
            draw_int(x_c-100, y_c+60, batt, background_color);
            
            boton(&conectar);

            //if (conectar == 1){
                x = read_axis(GYR_OUT_X_L, GYR_OUT_X_H) - x_baseline;
                y = read_axis(GYR_OUT_Y_L, GYR_OUT_Y_H) - y_baseline;
                z = read_axis(GYR_OUT_Z_L, GYR_OUT_Z_H) - z_baseline;           

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
                draw_string(x_c, y_c+40, "Z: ", text_color);
                draw_int(x_c-20, y_c+40, z, text_color);

                if (read_adc()<300) batt = 0;
                else batt = 1;
                usart_print("\tBattery: ");
                usart_print_int(batt);
                usart_print("\r\n");
                draw_string(x_c, y_c+60, "Battery: ", text_color);
                draw_int(x_c-100, y_c+60, batt, text_color);
            //}

            for (int i = 0; i < 3000000; i++) {//delay
                __asm__("NOP");
            }
        
    }

    return 0;
    

}


