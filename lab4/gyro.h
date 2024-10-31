#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/spi.h>
#include "console.h"


// Gyroscope CS pin on PC1

// Gyroscope register addresses
#define GYR_WHO_AM_I 0x0F
#define GYR_CTRL_REG1 0x20
#define GYR_CTRL_REG4 0x23
#define GYR_OUT_X_L 0x28
#define GYR_OUT_X_H 0x29
#define GYR_OUT_Y_L 0x2A
#define GYR_OUT_Y_H 0x2B
#define GYR_OUT_Z_L 0x2C
#define GYR_OUT_Z_H 0x2D

// Function prototypes
static void usart_setup(void);
static void spi_setup(void);
static void usart_print(const char *str);
static void usart_print_int(int32_t value);
void write_register(uint8_t reg, uint8_t value);
uint8_t read_register(uint8_t reg);
int16_t read_axis(uint8_t reg_low, uint8_t reg_high);
void calibrate_gyroscope(void);
uint8_t check_gyroscope_connection(void);

// Baseline values for gyroscope calibration
int16_t x_baseline = 0;
int16_t y_baseline = 0;
int16_t z_baseline = 0;

// USART setup for console communication
static void usart_setup(void) {
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_USART1);

    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9);
    gpio_set_af(GPIOA, GPIO_AF7, GPIO9);

    usart_set_baudrate(USART1, 115200);
    usart_set_databits(USART1, 8);
    usart_set_stopbits(USART1, USART_STOPBITS_1);
    usart_set_mode(USART1, USART_MODE_TX);
    usart_set_parity(USART1, USART_PARITY_NONE);
    usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);

    usart_enable(USART1);
}

// SPI setup for gyroscope communication
static void spi_setup(void) {
    rcc_periph_clock_enable(RCC_SPI5);
    rcc_periph_clock_enable(RCC_GPIOF);
    rcc_periph_clock_enable(RCC_GPIOC);

    gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO1);
    gpio_set(GPIOC, GPIO1);  // Set CS high (inactive)

    gpio_mode_setup(GPIOF, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO7 | GPIO8 | GPIO9);
    gpio_set_af(GPIOF, GPIO_AF5, GPIO7 | GPIO8 | GPIO9);

    spi_set_master_mode(SPI5);
    spi_set_baudrate_prescaler(SPI5, SPI_CR1_BR_FPCLK_DIV_64);
    spi_set_clock_polarity_0(SPI5);
    spi_set_clock_phase_0(SPI5);
    spi_set_dff_8bit(SPI5);  // 8-bit data frame
    spi_enable_software_slave_management(SPI5);
    spi_set_nss_high(SPI5);
    SPI_I2SCFGR(SPI5) &= ~SPI_I2SCFGR_I2SMOD;  // Disable I2S mode
    spi_enable(SPI5);
}




// Read a register from the gyroscope over SPI
uint8_t read_register(uint8_t reg) {
    uint8_t value;
    gpio_clear(GPIOC, GPIO1);  // Assert CS low, 
    spi_send(SPI5, reg | 0x80);  // Set MSB high for read
    spi_read(SPI5);               // Clear the buffer
    spi_send(SPI5, 0x00);
    value = spi_read(SPI5);
    gpio_set(GPIOC, GPIO1);  // Deassert CS high , importante para la comunicaciòn 
    //SPI
    //usart_print("A\n");

    return value;
}

// Write a value to a gyroscope register over SPI
void write_register(uint8_t reg, uint8_t value) {
    gpio_clear(GPIOC, GPIO1);  // Assert CS low
    spi_send(SPI5, reg);
    spi_read(SPI5);  // Clear the read buffer
    spi_send(SPI5, value);
    spi_read(SPI5);
    gpio_set(GPIOC, GPIO1);  // Deassert CS high
}

// Read and combine low and high register bytes for axis data
int16_t read_axis(uint8_t reg_low, uint8_t reg_high) {
    int16_t axis_data = read_register(reg_low);
    axis_data |= read_register(reg_high) << 8;
    return axis_data;   
}

// Calibrate gyroscope by setting the baseline for each axis
void calibrate_gyroscope(void) {
    gpio_clear(GPIOC, GPIO1);  // Assert CS low

    x_baseline = read_axis(GYR_OUT_X_L, GYR_OUT_X_H);
    y_baseline = read_axis(GYR_OUT_Y_L, GYR_OUT_Y_H);
    z_baseline = read_axis(GYR_OUT_Z_L, GYR_OUT_Z_H);

    usart_print_int(x_baseline);
    usart_print_int(y_baseline);
    usart_print_int(z_baseline);
        gpio_set(GPIOC, GPIO1);  // Deassert CS high
    }

// Print string over USART for debugging
static void usart_print(const char *str) {
    while (*str) {
        usart_send_blocking(USART1, *str++);
    }
}

// Print integer over USART for debugging
static void usart_print_int(int32_t value) {
    char buffer[12];
    int pos = 10;
    buffer[11] = '\0';

    if (value < 0) {
        usart_send_blocking(USART1, '-');
        value = -value;
    }

    do {
        buffer[pos--] = '0' + (value % 10);
        value /= 10;
    } while (value > 0);

    usart_print(&buffer[pos + 1]);
}
