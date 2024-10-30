#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/ltdc.h>

#include "clock.h"
#include "console.h"
#include "lcd-spi.h"
#include "sdram.h"

#define LCD_WIDTH  240
#define LCD_HEIGHT 320
#define REFRESH_RATE 70 /* Hz */

#define HSYNC       10
#define HBP         20
#define HFP         10

#define VSYNC        2
#define VBP          2
#define VFP          4

/* Layer 1 (bottom layer) is ARGB8888 format, full screen. */

typedef uint32_t layer1_pixel;
#define LCD_LAYER1_PIXFORMAT LTDC_LxPFCR_ARGB8888

layer1_pixel *const lcd_layer1_frame_buffer = (void *)SDRAM_BASE_ADDRESS;
#define LCD_LAYER1_PIXEL_SIZE (sizeof(layer1_pixel))
#define LCD_LAYER1_WIDTH  LCD_WIDTH
#define LCD_LAYER1_HEIGHT LCD_HEIGHT
#define LCD_LAYER1_PIXELS (LCD_LAYER1_WIDTH * LCD_LAYER1_HEIGHT)
#define LCD_LAYER1_BYTES  (LCD_LAYER1_PIXELS * LCD_LAYER1_PIXEL_SIZE)

/*
 * Pin assignments
 *     R2      = PC10, AF14
 *     R3      = PB0,  AF09
 *     R4      = PA11, AF14
 *     R5      = PA12, AF14
 *     R6      = PB1,  AF09
 *     R7      = PG6,  AF14
 *
 *     G2      = PA6,  AF14
 *     G3      = PG10, AF09
 *     G4      = PB10, AF14
 *     G5      = PB11, AF14
 *     G6      = PC7,  AF14
 *     G7      = PD3,  AF14
 *
 *     B2      = PD6,  AF14
 *     B3      = PG11, AF11
 *     B4      = PG12, AF09
 *     B5      = PA3,  AF14
 *     B6      = PB8,  AF14
 *     B7      = PB9,  AF14
 *
 * More pins...
 *     ENABLE  = PF10, AF14
 *     DOTCLK  = PG7,  AF14
 *     HSYNC   = PC6,  AF14
 *     VSYNC   = PA4,  AF14
 *     CSX     = PC2         used in lcd-spi
 *     RDX     = PD12        not used: read SPI
 *     TE      = PD11        not used: tearing effect interrupt
 *     WRX_DCX = PD13        used in lcd-spi
 *     DCX_SCL = PF7         used in lcd-spi
 *     SDA     = PF9         used in lcd-spi
 *     NRST    = NRST
 */
uint8_t reverse_bits(uint8_t b);
static void lcd_dma_init(void);
void draw_char(int x, int y, char c, uint32_t color);
void draw_string(int x, int y, char *str, uint32_t color);
void draw_int(int x, int y, int num, uint32_t color);


static void lcd_dma_init(void) {
     /* init GPIO clocks */
    rcc_periph_clock_enable(RCC_GPIOA | RCC_GPIOB | RCC_GPIOC |
                RCC_GPIOD | RCC_GPIOF | RCC_GPIOG);

    /* set GPIO pin modes */
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE,
            GPIO3 | GPIO4 | GPIO6 | GPIO11 | GPIO12);
    gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                GPIO3 | GPIO4 | GPIO6 | GPIO11 | GPIO12);
    gpio_set_af(GPIOA, GPIO_AF14, GPIO3 | GPIO4 | GPIO6 | GPIO11 | GPIO12);

    gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE,
            GPIO0 | GPIO1 | GPIO8 | GPIO9 | GPIO10 | GPIO11);
    gpio_set_output_options(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                GPIO0 | GPIO1 | GPIO8 |
                GPIO9 | GPIO10 | GPIO11);
    gpio_set_af(GPIOB, GPIO_AF9, GPIO0 | GPIO1);
    gpio_set_af(GPIOB, GPIO_AF14, GPIO8 | GPIO9 | GPIO10 | GPIO11);

    gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE,
            GPIO6 | GPIO7 | GPIO10);
    gpio_set_output_options(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                GPIO6 | GPIO7 | GPIO10);
    gpio_set_af(GPIOC, GPIO_AF14, GPIO6 | GPIO7 | GPIO10);

    gpio_mode_setup(GPIOD, GPIO_MODE_AF, GPIO_PUPD_NONE,
            GPIO3 | GPIO6);
    gpio_set_output_options(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                GPIO3 | GPIO6);
    gpio_set_af(GPIOD, GPIO_AF14, GPIO3 | GPIO6);

    gpio_mode_setup(GPIOF, GPIO_MODE_AF, GPIO_PUPD_NONE,
            GPIO10);
    gpio_set_output_options(GPIOF, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                GPIO10);
    gpio_set_af(GPIOF, GPIO_AF14, GPIO10);

    gpio_mode_setup(GPIOG, GPIO_MODE_AF, GPIO_PUPD_NONE,
            GPIO6 | GPIO7 | GPIO10 | GPIO11 | GPIO12);
    gpio_set_output_options(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                GPIO6 | GPIO7 | GPIO10 | GPIO11 | GPIO12);
    gpio_set_af(GPIOG, GPIO_AF9, GPIO10 | GPIO12);
    gpio_set_af(GPIOG, GPIO_AF14, GPIO6 | GPIO7 | GPIO11);

    

    uint32_t sain = 192;
    uint32_t saiq = (RCC_PLLSAICFGR >> RCC_PLLSAICFGR_PLLSAIQ_SHIFT) &
            RCC_PLLSAICFGR_PLLSAIQ_MASK;
    uint32_t sair = 4;
    RCC_PLLSAICFGR = (sain << RCC_PLLSAICFGR_PLLSAIN_SHIFT |
              saiq << RCC_PLLSAICFGR_PLLSAIQ_SHIFT |
              sair << RCC_PLLSAICFGR_PLLSAIR_SHIFT);
    RCC_DCKCFGR |= RCC_DCKCFGR_PLLSAIDIVR_DIVR_8 << RCC_DCKCFGR_PLLSAIDIVR_SHIFT;
    RCC_CR |= RCC_CR_PLLSAION;
    while ((RCC_CR & RCC_CR_PLLSAIRDY) == 0) {
        continue;
    }
    RCC_APB2ENR |= RCC_APB2ENR_LTDCEN;

    
    /*
     * Configure the Synchronous timings: VSYNC, HSNC,
     * Vertical and Horizontal back porch, active data area, and
     * the front porch timings.
     */
    LTDC_SSCR = (HSYNC - 1) << LTDC_SSCR_HSW_SHIFT |
                (VSYNC - 1) << LTDC_SSCR_VSH_SHIFT;
    LTDC_BPCR = (HSYNC + HBP - 1) << LTDC_BPCR_AHBP_SHIFT |
                (VSYNC + VBP - 1) << LTDC_BPCR_AVBP_SHIFT;
    LTDC_AWCR = (HSYNC + HBP + LCD_WIDTH - 1) << LTDC_AWCR_AAW_SHIFT |
                (VSYNC + VBP + LCD_HEIGHT - 1) << LTDC_AWCR_AAH_SHIFT;
    LTDC_TWCR =
        (HSYNC + HBP + LCD_WIDTH + HFP - 1) << LTDC_TWCR_TOTALW_SHIFT |
        (VSYNC + VBP + LCD_HEIGHT + VFP - 1) << LTDC_TWCR_TOTALH_SHIFT;

    /* Configure the synchronous signals and clock polarity. */
    LTDC_GCR |= LTDC_GCR_PCPOL_ACTIVE_HIGH;

    /* If needed, configure the background color. */
    LTDC_BCCR = 0x00000000;

    /* Configure the needed interrupts. */
    LTDC_IER = LTDC_IER_RRIE;
    nvic_enable_irq(NVIC_LCD_TFT_IRQ);

    /* Configure the Layer 1 parameters.
     * (Layer 1 is the bottom layer.)    */
    {
        /* The Layer window horizontal and vertical position */
        uint32_t h_start = HSYNC + HBP + 0;
        uint32_t h_stop = HSYNC + HBP + LCD_LAYER1_WIDTH - 1;
        LTDC_L1WHPCR = h_stop << LTDC_LxWHPCR_WHSPPOS_SHIFT |
                       h_start << LTDC_LxWHPCR_WHSTPOS_SHIFT;
        uint32_t v_start = VSYNC + VBP + 0;
        uint32_t v_stop = VSYNC + VBP + LCD_LAYER1_HEIGHT - 1;
        LTDC_L1WVPCR = v_stop << LTDC_LxWVPCR_WVSPPOS_SHIFT |
                       v_start << LTDC_LxWVPCR_WVSTPOS_SHIFT;

        /* The pixel input format */
        LTDC_L1PFCR = LCD_LAYER1_PIXFORMAT;

        /* The color frame buffer start address */
        LTDC_L1CFBAR = (uint32_t)lcd_layer1_frame_buffer;

        /* The line length and pitch of the color frame buffer */
        uint32_t pitch = LCD_LAYER1_WIDTH * LCD_LAYER1_PIXEL_SIZE;
        uint32_t length = LCD_LAYER1_WIDTH * LCD_LAYER1_PIXEL_SIZE + 3;
        LTDC_L1CFBLR = pitch << LTDC_LxCFBLR_CFBP_SHIFT |
                       length << LTDC_LxCFBLR_CFBLL_SHIFT;

        /* The number of lines of the color frame buffer */
        LTDC_L1CFBLNR = LCD_LAYER1_HEIGHT;

        /* If needed, load the CLUT */
        /* (not using CLUT) */

        /* If needed, configure the default color and blending
         * factors
         */
        LTDC_L1CACR = 0x000000FF;
        LTDC_L1BFCR = LTDC_LxBFCR_BF1_PIXEL_ALPHA_x_CONST_ALPHA |
                      LTDC_LxBFCR_BF2_PIXEL_ALPHA_x_CONST_ALPHA;
    }

    /* Enable Layer1 */
    LTDC_L1CR |= LTDC_LxCR_LAYER_ENABLE;

    /* Reload the shadow registers to active registers. */
    LTDC_SRCR |= LTDC_SRCR_VBR;

    /* Enable the LCD-TFT controller. */
    LTDC_GCR |= LTDC_GCR_LTDC_ENABLE;
}

void lcd_tft_isr(void)
{
    LTDC_ICR |= LTDC_ICR_CRRIF;

    /* Reload the shadow registers to active registers */
    LTDC_SRCR |= LTDC_SRCR_VBR;
}

/* Define a simple 8x8 font for ASCII characters from ' ' (32) to '~' (126) */
const uint8_t font8x8_basic[95][8] = {
    /* Space ' ' (ASCII 32) */
    {0x00, 0x00, 0x00, 0x00, 0x00, /* ... */ 0x00},
    /* '!' */
    {0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x18, 0x00},
    /* '"' */
    {0x36, 0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* '#' */
    {0x36, 0x36, 0x7F, 0x36, 0x7F, 0x36, 0x36, 0x00},
    /* '$' */
    {0x0C, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x0C, 0x00},
    /* '%' */
    {0x00, 0x63, 0x33, 0x18, 0x0C, 0x66, 0x63, 0x00},
    /* '&' */
    {0x1C, 0x36, 0x1C, 0x6E, 0x3B, 0x33, 0x6E, 0x00},
    /* ''' */
    {0x06, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* '(' */
    {0x18, 0x0C, 0x06, 0x06, 0x06, 0x0C, 0x18, 0x00},
    /* ')' */
    {0x06, 0x0C, 0x18, 0x18, 0x18, 0x0C, 0x06, 0x00},
    /* '*' */
    {0x00, 0x36, 0x1C, 0x7F, 0x1C, 0x36, 0x00, 0x00},
    /* '+' */
    {0x00, 0x0C, 0x0C, 0x7F, 0x0C, 0x0C, 0x00, 0x00},
    /* ',' */
    {0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x06, 0x00},
    /* '-' */
    {0x00, 0x00, 0x00, 0x7F, 0x00, 0x00, 0x00, 0x00},
    /* '.' */
    {0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00, 0x00},
    /* '/' */
    {0x60, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x01, 0x00},
    /* '0' */
    {0x3E, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x3E, 0x00},
    /* '1' */
    {0x0C, 0x0E, 0x0F, 0x0C, 0x0C, 0x0C, 0x3F, 0x00},
    /* '2' */
    {0x1E, 0x33, 0x30, 0x1C, 0x06, 0x33, 0x3F, 0x00},
    /* '3' */
    {0x1E, 0x33, 0x30, 0x1C, 0x30, 0x33, 0x1E, 0x00},
    /* '4' */
    {0x38, 0x3C, 0x36, 0x33, 0x7F, 0x30, 0x78, 0x00},
    /* '5' */
    {0x3F, 0x03, 0x1F, 0x30, 0x30, 0x33, 0x1E, 0x00},
    /* '6' */
    {0x1C, 0x06, 0x03, 0x1F, 0x33, 0x33, 0x1E, 0x00},
    /* '7' */
    {0x3F, 0x33, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x00},
    /* '8' */
    {0x1E, 0x33, 0x33, 0x1E, 0x33, 0x33, 0x1E, 0x00},
    /* '9' */
    {0x1E, 0x33, 0x33, 0x3E, 0x30, 0x18, 0x0E, 0x00},
    /* ':' */
    {0x00, 0x0C, 0x0C, 0x00, 0x0C, 0x0C, 0x00, 0x00},
    /* ';' */
    {0x00, 0x0C, 0x0C, 0x00, 0x0C, 0x0C, 0x06, 0x00},
    /* '<' */
    {0x18, 0x0C, 0x06, 0x03, 0x06, 0x0C, 0x18, 0x00},
    /* '=' */
    {0x00, 0x00, 0x7F, 0x00, 0x7F, 0x00, 0x00, 0x00},
    /* '>' */
    {0x06, 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x06, 0x00},
    /* '?' */
    {0x1E, 0x33, 0x30, 0x18, 0x0C, 0x00, 0x0C, 0x00},
    /* '@' */
    {0x3E, 0x63, 0x7B, 0x7B, 0x7B, 0x03, 0x1E, 0x00},
    /* 'A' */
    {0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x00},
    /* 'B' */
    {0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3F, 0x00},
    /* 'C' */
    {0x3C, 0x66, 0x03, 0x03, 0x03, 0x66, 0x3C, 0x00},
    /* 'D' */
    {0x1F, 0x36, 0x66, 0x66, 0x66, 0x36, 0x1F, 0x00},
    /* 'E' */
    {0x7F, 0x46, 0x16, 0x1E, 0x16, 0x46, 0x7F, 0x00},
    /* 'F' */
    {0x7F, 0x46, 0x16, 0x1E, 0x16, 0x06, 0x0F, 0x00},
    /* 'G' */
    {0x3C, 0x66, 0x03, 0x03, 0x73, 0x66, 0x7C, 0x00},
    /* 'H' */
    {0x33, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x33, 0x00},
    /* 'I' */
    {0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},
    /* 'J' */
    {0x78, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E, 0x00},
    /* 'K' */
    {0x67, 0x66, 0x36, 0x1E, 0x36, 0x66, 0x67, 0x00},
    /* 'L' */
    {0x0F, 0x06, 0x06, 0x06, 0x46, 0x66, 0x7F, 0x00},
    /* 'M' */
    {0x63, 0x77, 0x7F, 0x7F, 0x6B, 0x63, 0x63, 0x00},
    /* 'N' */
    {0x63, 0x67, 0x6F, 0x7B, 0x73, 0x63, 0x63, 0x00},
    /* 'O' */
    {0x1C, 0x36, 0x63, 0x63, 0x63, 0x36, 0x1C, 0x00},
    /* 'P' */
    {0x3F, 0x66, 0x66, 0x3F, 0x06, 0x06, 0x0F, 0x00},
    /* 'Q' */
    {0x1E, 0x33, 0x33, 0x33, 0x3B, 0x1E, 0x38, 0x00},
    /* 'R' */
    {0x3F, 0x66, 0x66, 0x3F, 0x36, 0x66, 0x67, 0x00},
    /* 'S' */
    {0x1E, 0x33, 0x07, 0x1E, 0x38, 0x33, 0x1E, 0x00},
    /* 'T' */
    {0x3F, 0x2D, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},
    /* 'U' */
    {0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3F, 0x00},
    /* 'V' */
    {0x33, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x00},
    /* 'W' */
    {0x63, 0x63, 0x6B, 0x7F, 0x7F, 0x77, 0x63, 0x00},
    /* 'X' */
    {0x63, 0x36, 0x1C, 0x08, 0x1C, 0x36, 0x63, 0x00},
    /* 'Y' */
    {0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x1E, 0x00},
    /* 'Z' */
    {0x7F, 0x73, 0x19, 0x0C, 0x46, 0x63, 0x7F, 0x00},
    /* '[' */
    {0x1E, 0x06, 0x06, 0x06, 0x06, 0x06, 0x1E, 0x00},
    /* '\' */
    {0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x40, 0x00},
    /* ']' */
    {0x1E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1E, 0x00},
    /* '^' */
    {0x08, 0x1C, 0x36, 0x63, 0x00, 0x00, 0x00, 0x00},
    /* '_' */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00},
    /* '`' */
    {0x0C, 0x0C, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00},
    /* 'a' */
    {0x00, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x6E, 0x00},
    /* 'b' */
    {0x07, 0x06, 0x06, 0x1E, 0x36, 0x36, 0x1D, 0x00},
    /* 'c' */
    {0x00, 0x00, 0x1E, 0x33, 0x03, 0x33, 0x1E, 0x00},
    /* 'd' */
    {0x38, 0x30, 0x30, 0x3E, 0x33, 0x33, 0x6E, 0x00},
    /* 'e' */
    {0x00, 0x00, 0x1E, 0x33, 0x3F, 0x03, 0x1E, 0x00},
    /* 'f' */
    {0x1C, 0x36, 0x06, 0x0F, 0x06, 0x06, 0x0F, 0x00},
    /* 'g' */
    {0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x1F},
    /* 'h' */
    {0x07, 0x06, 0x36, 0x6E, 0x66, 0x66, 0x67, 0x00},
    /* 'i' */
    {0x0C, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},
    /* 'j' */
    {0x30, 0x00, 0x38, 0x30, 0x30, 0x33, 0x33, 0x1E},
    /* 'k' */
    {0x07, 0x06, 0x66, 0x36, 0x1E, 0x36, 0x67, 0x00},
    /* 'l' */
    {0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},
    /* 'm' */
    {0x00, 0x00, 0x33, 0x7F, 0x7F, 0x6B, 0x63, 0x00},
    /* 'n' */
    {0x00, 0x00, 0x1F, 0x33, 0x33, 0x33, 0x33, 0x00},
    /* 'o' */
    {0x00, 0x00, 0x1E, 0x33, 0x33, 0x33, 0x1E, 0x00},
    /* 'p' */
    {0x00, 0x00, 0x1D, 0x36, 0x36, 0x1E, 0x06, 0x0F},
    /* 'q' */
    {0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x78},
    /* 'r' */
    {0x00, 0x00, 0x1B, 0x3E, 0x36, 0x06, 0x0F, 0x00},
    /* 's' */
    {0x00, 0x00, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x00},
    /* 't' */
    {0x08, 0x0C, 0x3E, 0x0C, 0x0C, 0x2C, 0x18, 0x00},
    /* 'u' */
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x6E, 0x00},
    /* 'v' */
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00},
    /* 'w' */
    {0x00, 0x00, 0x63, 0x6B, 0x7F, 0x7F, 0x36, 0x00},
    /* 'x' */
    {0x00, 0x00, 0x63, 0x36, 0x1C, 0x36, 0x63, 0x00},
    /* 'y' */
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x3E, 0x30, 0x1F},
    /* 'z' */
    {0x00, 0x00, 0x3F, 0x19, 0x0C, 0x26, 0x3F, 0x00},
};



/* Function to draw a single character */
void draw_char(int x, int y, char c, uint32_t color)
{
    if (c < 32 || c > 126) return; // Only supports ASCII 32 to 126

    const uint8_t *bitmap = font8x8_basic[c - 32];
    for (int row = 0; row < 8; row++)
    {       
        uint8_t bits = bitmap[row];

        for (int col = 0; col<8; col++)
        {
            if (bits & (1 << col))
            {
                int px = x - col;
                int py = y + row;

                if (px >= 0 && px < LCD_LAYER1_WIDTH && py >= 0 && py < LCD_LAYER1_HEIGHT)
                {
                    size_t i = py * LCD_LAYER1_WIDTH + px;
                    lcd_layer1_frame_buffer[i] = color;
                }
            }
        }
    }
}


/* Function to draw a string */
void draw_string(int x, int y, char *str, uint32_t color)
{   gpio_clear(GPIOC, GPIO2); /* Select the LCD talvexz no es necesario*/
    while (*str)
    {
        // Find the end of the current line
        char *line_end = str;
        while (*line_end && *line_end != '\n')
            line_end++;

        // Calculate the length of the line
        int line_length = line_end - str;

        // Calculate total line width in pixels (8 pixels per character)
        int line_width = line_length * 8;

        // Calculate starting x position for right alignment
        int current_x = x - line_width; // x is the right edge minus margin

        // Draw the line in normal order
        for (int i = line_length; i >= 0 ; i--)
        {
            char c = str[i];
            draw_char(current_x, y, c, color);
            current_x += 8; // Move right for the next character
        }

        // Move to next line
        if (*line_end == '\n')
        {
            y += 8; // Move down to next line
            str = line_end + 1; // Move past the newline character
        }
        else
        {
            // End of string
            break;
        }
    }
    gpio_set(GPIOC, GPIO2);	   /* Turn off chip select */
}


void draw_int(int x, int y, int num, uint32_t color) {
    char buffer[12];  // Allocate memory for the string
    snprintf(buffer, sizeof(buffer), "%d", num);  // Convert int to string
    draw_string(x, y, buffer, color);           // Print the string representation
}






