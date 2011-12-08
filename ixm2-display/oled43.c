/*
 * oled43.c
 *
 * Created: 12/7/2011 4:56:07 PM
 *  Author: lwlab
 */
#include <avr/io.h>
#include "bitops.h"
#include "display.h"
#include "delay.h"

/* Liquidware BeagleTouch */
//    OMAP3_MUX(SDMMC2_DAT7, OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),       //GPIO 139, CS
//    OMAP3_MUX(UART2_TX,    OMAP_MUX_MODE4 | OMAP_PIN_INPUT_PULLUP), //GPIO 146
//    OMAP3_MUX(UART2_CTS,   OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),       //GPIO 144, MOSI
//    OMAP3_MUX(SDMMC2_DAT6, OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),       //GPIO 138, CLK
//    OMAP3_MUX(SDMMC2_DAT5, OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),       //GPIO 137, RESET
//    OMAP3_MUX(SDMMC1_DAT7, OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),       //GPIO 129
//    OMAP3_MUX(MCBSP3_FSX,  OMAP_MUX_MODE4 | OMAP_PIN_OUTPUT),       //GPIO 143, PWR

#define CS_PIN          PORTD, 4
#define MOSI_PIN        PORTB, 7
#define CLK_PIN         PORTD, 6
#define RESET_PIN       PORTD, 7
#define PANEL_PWR_PIN   PORTC, 7

/* Note: VALUE direction (SET/CLR) is inverted because 
	we're driving N-Chan open collector level shifters */
#define digitalWrite(PORT_PIN, VALUE) { \
										if (VALUE == 1) { \
											CLRBIT(PORT_PIN); \
										} else { \
											SETBIT(PORT_PIN); \
										}					  \
									}

#define CS_LOW digitalWrite(CS_PIN, 0)
#define CS_HIGH digitalWrite(CS_PIN, 1)
#define MOSI_LOW digitalWrite(MOSI_PIN, 0)
#define MOSI_HIGH digitalWrite(MOSI_PIN, 1)
#define CLK_LOW digitalWrite(CLK_PIN, 0)
#define CLK_HIGH digitalWrite(CLK_PIN, 1)
#define RESET_LOW digitalWrite(RESET_PIN, 0)
#define RESET_HIGH digitalWrite(RESET_PIN, 1)
#define PANEL_PWR_LOW digitalWrite(PANEL_PWR_PIN, 0)
#define PANEL_PWR_HIGH digitalWrite(PANEL_PWR_PIN, 1)

/* Initialize the software SPI interface */
static void oled43_spi_init(void) {
    CS_HIGH;
    MOSI_HIGH;
    CLK_HIGH;
}

/* Write an 8-bit byte onto the SPI bus */
static void oled43_spi_write8(uint8_t data) {
uint8_t ii;
uint8_t bit;

    for (ii=0; ii < 8; ii++) {
        bit = (data >> (7-ii)) & 1;
        if (bit) {
            MOSI_HIGH;
        } else {
            MOSI_LOW;
        }

        CLK_LOW;		//clock the data in
        CLK_HIGH;
    }
}

/* Write a value to the OLED panel */
static void oled43_writeReg(uint8_t index, uint8_t val) {
    CS_LOW;
    oled43_spi_write8((index << 1));
    oled43_spi_write8(val);
    CS_HIGH;
}

static int oled43_hardware_init(void) {
uint8_t brightness;
double percent;

	/* Panel init sequence from the panel datasheet */
	PANEL_PWR_LOW;						// just to be sure, hold the oled power supply off
	MOSI_LOW;
	RESET_LOW;							// panel in reset
	oled43_spi_init();					// init spi interface
	RESET_HIGH;							// panel out of reset

    oled43_writeReg(0x04, 0x23); //DISPLAY_MODE2
    oled43_writeReg(0x05, 0x82); //DISPLAY_MODE3

    oled43_writeReg(0x07, 0x0F); //DRIVER_CAPABILITY
    oled43_writeReg(0x34, 0x18);
    oled43_writeReg(0x35, 0x28);
    oled43_writeReg(0x36, 0x16);
    oled43_writeReg(0x37, 0x01);

    oled43_writeReg(0x03, 35);   //VGAM1_LEVEL

    /* Set the brightness
     *  0x20 - 200 nits
     *  0x1E - 175 nits
     *  0x1C - 150 nits
     *  0x17 - 100 nits
     *  0x14 -  70 nits
     *  0x11 -  50 nits */
    brightness = 0x14;
    percent = ((double)brightness)/
               (0x20) * 100.0;

    //printk(KERN_INFO "cmel_oled43_panel: Setting brightness to %d percent\n", (int)percent);

    oled43_writeReg(0x3A, brightness);
    
    /* Display ON */
    oled43_writeReg(0x06, 0x03); //POWER_CTRL1

	return 0;
}

static void oled43_panel_power_off(void)
{
    /* Turn off the power supply */
	PANEL_PWR_LOW;
}

static void oled43_panel_power_on(void)
{
	mdelay(50);
    oled43_hardware_init();
	PANEL_PWR_HIGH;
}

static void oled43_panel_suspend(void)
{
	oled43_panel_power_off();
}

static void oled43_panel_resume(void)
{
	oled43_panel_power_on();
}

static void oled43_panel_enable(void)
{
	oled43_panel_power_on();
}

static void oled43_panel_disable(void)
{
	oled43_panel_power_off();
}

static void oled43_panel_drv_init(void)
{
	/* Set the data direction registers */
	SETBIT(DDRD, 4);
	SETBIT(DDRB, 7);
	SETBIT(DDRD, 6);
	SETBIT(DDRD, 7);
	SETBIT(DDRC, 7);
	
	/* Hold the power supply off until enabled */
	oled43_panel_power_off();
}

struct omap_dss_device oled43_driver = {
	.init       = oled43_panel_drv_init,
	.enable		= oled43_panel_enable,
	.disable	= oled43_panel_disable,
	.suspend	= oled43_panel_suspend,
	.resume		= oled43_panel_resume,
};