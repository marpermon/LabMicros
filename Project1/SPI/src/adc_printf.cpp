/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
 * Copyright (C) 2011 Stephen Caudle <scaudle@doceme.com>
 * Modified by Fernando Cortes <fermando.corcam@gmail.com>
 * Modified by Guillermo Rivera <memogrg@gmail.com>
 * Modified by Emmanuel Solano <emanuelsolanomonge6@gmail.com>
 * Modified by Gabriel Torres Garbanzo <Gabrielcr26g@gmail.com>
 * Modified by Kevin Chen Wu <hongwenchen.k.c.w.90@gmail.com>
 * Modified by Helber Meneses Navarro <helbermenesesn@gmail.com>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

// Include the necessary libraries and header files
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/gpio.h>
#include "adc.h"
#include "usart_utils.h"


void system_init(void);

// Function to set the ADC parameters
static void adc_setup_1(void)
{
	// Enable clock
	rcc_periph_clock_enable(RCC_ADC1);
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOC);
	
	// Set pins PA0,PA1,PA2,PA3,PC1,PC2,PC5 to be analog inputs
	gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO0 | GPIO1 | GPIO2 | GPIO3);
	gpio_mode_setup(GPIOC, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO1 | GPIO2 | GPIO5);
	
	
	adc_power_off(ADC1);
	adc_set_single_conversion_mode(ADC1);
	adc_set_clk_prescale(ADC_CCR_ADCPRE_BY8);
	adc_disable_external_trigger_regular(ADC1);
	adc_set_right_aligned(ADC1);
	
	// Set sample time
	adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_480CYC);
	adc_set_resolution(ADC1, ADC_CR1_RES_12BIT);
	adc_power_on(ADC1);
	
	// Wait for the ADC initialization
	int i;
	for (i = 0; i < 800000; i++)
		__asm__("nop");
}

void system_init(void)
{
	rcc_osc_on(RCC_HSI);
    rcc_wait_for_osc_ready(RCC_HSI);
    rcc_set_sysclk_source(RCC_CFGR_SW_HSI);
    rcc_apb1_frequency = 16000000;
    rcc_apb2_frequency = 16000000;
}

// Function to read the value from the given analog pin
static uint16_t read_adc_1(uint8_t channel)
{
	// Define the channel
	uint8_t channel_array[16];
	// Set the analog pin to read
	channel_array[0] = channel;
	adc_set_regular_sequence(ADC1, 1, channel_array);
	adc_start_conversion_regular(ADC1);
	// Read the value from the analog pin
	while (!adc_eoc(ADC1));
	uint16_t force = adc_read_regular(ADC1);
	return force;
}

static uint16_t avg_filter(uint16_t volt) {
	uint32_t avg_sample =0x00;
	const int N = 10;
	uint16_t adc_sample[N];
	uint8_t index = 0x00;
	uint16_t force_mv = 0;
		for (index=0; index<N; index++){
			adc_sample[index] = volt;
		}
		for (index=0; index<N; index++){
			avg_sample += adc_sample[index];
			if (index==N-1){
				force_mv = avg_sample/N;
    			avg_sample=0;
  			}
		}
		return force_mv;
}

int main(void)
{
	uint16_t fx_my_1 = 0, fx_my_2 = 0, fy_mx_1 = 0, fy_mx_2 = 0, mz = 0, fz_1 = 0, fz_2 = 0;
	
	adc_setup_1();
	system_init();

	
	while (1) {
		usart_setup();
    	spi_setup();
		// Computes the average value of N measurements for each
		// voltage measurement
		fx_my_2 = avg_filter(read_adc_1(ADC_VOLTAGE_FX_MY_2)); // PA0
		fy_mx_2 = avg_filter(read_adc_1(ADC_VOLTAGE_FY_MX_2)); // PA1
		fz_1    = avg_filter(read_adc_1(ADC_VOLTAGE_FZ_1));    // PA2
		fx_my_1 = avg_filter(read_adc_1(ADC_VOLTAGE_FX_MY_1)); // PA3
		mz      = avg_filter(read_adc_1(ADC_VOLTAGE_MZ));      // PC1
		fy_mx_1 = avg_filter(read_adc_1(ADC_VOLTAGE_FY_MX_1)); // PC2
		fz_2    = avg_filter(read_adc_1(ADC_VOLTAGE_FZ_2));    // PC5
		usart_print_int(fx_my_1);usart_print(",");
        usart_print_int(fx_my_2);usart_print(",");
        usart_print_int(fy_mx_1);usart_print(",");
        usart_print_int(fy_mx_2);usart_print(",");
        usart_print_int(mz);usart_print(",");
		usart_print_int(fz_1);usart_print(",");
		usart_print_int(fz_2);usart_print(";");
        usart_print("\r\n");
	}
	return 0;
}
