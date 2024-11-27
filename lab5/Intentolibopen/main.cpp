#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/spi.h>
#include <edge-impulse-sdk/classifier/ei_run_classifier.h>
#include <edge-impulse-sdk/dsp/numpy.hpp>
#include <string.h>

// Gyroscope CS pin on PC1
#define GYR_CS GPIOC, GPIO1

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

#define BUFFER_SIZE 384
#define RAW_SAMPLE_COUNT EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME
#define SAMPLE_STRIDE 3                             // 3 values per sample (X, Y, Z)

// Function prototypes
static void usart_setup(void);
static void spi_setup(void);
static void usart_print(const char *str);
static void usart_print_int(int32_t value);
void write_register(uint8_t reg, uint8_t value);
uint8_t read_register(uint8_t reg);
int16_t read_axis(uint8_t reg_low, uint8_t reg_high);
void calibrate_gyroscope(void);

int16_t x_baseline = 0;
int16_t y_baseline = 0;
int16_t z_baseline = 0;

// Buffer for raw sensor data (1D array for X, Y, Z values)
float raw_buffer[BUFFER_SIZE];
size_t buffer_offset = 0; // Tracks the current buffer position




static void usart_setup(void)
{
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

static void spi_setup(void)
{
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

static void usart_print(const char *str)
{
    while (*str) {
        usart_send_blocking(USART1, *str++);
    }
}

static void usart_print_int(int32_t value)
{
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

void write_register(uint8_t reg, uint8_t value)
{
    gpio_clear(GPIOC, GPIO1);
    spi_send(SPI5, reg);
    spi_read(SPI5);  // Clear the read buffer
    spi_send(SPI5, value);
    spi_read(SPI5);
    gpio_set(GPIOC, GPIO1);
}

uint8_t read_register(uint8_t reg)
{
    uint8_t value;
    gpio_clear(GPIOC, GPIO1);
    spi_send(SPI5, reg | 0x80);  // Set MSB high for read
    spi_read(SPI5);               // Clear the buffer
    spi_send(SPI5, 0x00);
    value = spi_read(SPI5);
    gpio_set(GPIOC, GPIO1);
    return value;
}

int16_t read_axis(uint8_t reg_low, uint8_t reg_high)
{
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


void collect_gyroscope_data() {
    // Read gyroscope axes
    int16_t x = read_axis(GYR_OUT_X_L, GYR_OUT_X_H) - x_baseline;
    int16_t y = read_axis(GYR_OUT_Y_L, GYR_OUT_Y_H) - y_baseline;
    int16_t z = read_axis(GYR_OUT_Z_L, GYR_OUT_Z_H) - z_baseline;

    // Normalize and add to the sliding buffer
    raw_buffer[buffer_offset++] = (float)x / 2000.0f; // Normalize
    raw_buffer[buffer_offset++] = (float)y / 2000.0f;
    raw_buffer[buffer_offset++] = (float)z / 2000.0f;
}


int main(void)
{
    // Set up HSI (internal 16MHz clock) as the system clock
    rcc_osc_on(RCC_HSI);
    rcc_wait_for_osc_ready(RCC_HSI);
    rcc_set_sysclk_source(RCC_CFGR_SW_HSI);
    rcc_apb1_frequency = 16000000;
    rcc_apb2_frequency = 16000000;
    usart_setup();
    spi_setup();
    signal_t signal;
    numpy::signal_from_buffer(raw_buffer, BUFFER_SIZE, &signal);

    // Configure gyroscope control registers
    write_register(GYR_CTRL_REG1, 0x0F);  // Normal mode, all axes enabled
    write_register(GYR_CTRL_REG4, 0x30);  // Full scale at ±2000 dps

    // Calibrate the gyroscope
    calibrate_gyroscope();
    ei_impulse_result_t result;

    // Continuously read and print X, Y, and Z axis data relative to baseline
    while (1) {
        collect_gyroscope_data();
        /*usart_print_int((int32_t)buffer_offset);
        usart_print("\r\n");*/

        if (buffer_offset >= BUFFER_SIZE) {
            // Wrap the raw buffer in a signal object
            // Perform continuous classification
            EI_IMPULSE_ERROR err = run_classifier(&signal, &result, false);
            if (err != EI_IMPULSE_OK) {
                usart_print("Failed to run classifier ");
                usart_print_int(err);
                usart_print("\r\n");
                continue;
            }

            // Print classification results
            usart_print("Predictions:\n");
            for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {

                usart_print(result.classification[ix].label);
                usart_print(" ");
                usart_print_int(result.classification[ix].value * 100);
                usart_print("\r\n");
                //printf("    %s: %.2f%%\n", result.classification[ix].label,
                       //result.classification[ix].value * 100);
            }

            // Shift the buffer for sliding window (preserve overlap)
            size_t overlap = EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME * SAMPLE_STRIDE; // Samples to keep for overlap
            memmove(raw_buffer, raw_buffer + overlap, (BUFFER_SIZE - overlap) * sizeof(float));
            buffer_offset = BUFFER_SIZE - overlap;
        }



    }

    return 0;
}

        /*usart_print_int(x);
        usart_print(" ");
        usart_print_int(y);
        usart_print(" ");
        usart_print_int(z);
        usart_print("\r\n");*/