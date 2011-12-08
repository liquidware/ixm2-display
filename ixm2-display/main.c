/*
 * HDMI.c
 *
 * Created: 12/7/2011 12:22:36 PM
 *  Author: lwlab
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#include "bitops.h"
#include "display.h"

#define CTRL_DDR DDRF
#define CTRL_PIN PINF
#define CTRL_PORT PORTF
#define DISPLAY_PD_PIN	6
#define DVI_PD_PIN		5
#define DSS_IO_OE_PIN	4

#define D5_0V_DVID_PORT	PORTD
#define D5_0V_DVID_PIN	PIND
#define D5_0V_DVID_DDR  DDRD
#define D5_0V_DVID_PNUM	2		

extern struct omap_dss_device oled43_driver;

volatile uint8_t isDVICableAttached = 0;
volatile uint8_t isDisplayEnabled = 0;

SIGNAL (SIG_INTERRUPT2) {
	;
}	

void dss_init(void) {
	SETBIT(CTRL_DDR, DISPLAY_PD_PIN); 
	SETBIT(CTRL_DDR, DVI_PD_PIN);
	SETBIT(CTRL_DDR, DSS_IO_OE_PIN);
	CLRBIT(D5_0V_DVID_DDR, D5_0V_DVID_PNUM); //input
	SETBIT(D5_0V_DVID_PORT, D5_0V_DVID_PNUM); //enable pull-up
	CLRBIT(CTRL_DDR, 0);  //Rev A boards have PF0 jumped to INT2, remove on future revisions
	CLRBIT(CTRL_PORT, 0); //Rev A boards have PF0 jumped to INT2, remove on future revisions
	
	/* Enable interrupt on rising edge, enable INT2 */
	SETBIT(EICRA, ISC20);
	SETBIT(EICRA, ISC21);
	SETBIT(EIMSK, INT2);
}

void dss_enable(void) {
	CLRBIT(CTRL_PORT, DISPLAY_PD_PIN); //enable system 5V
	SETBIT(CTRL_PORT, DVI_PD_PIN); //enable DVI IO
	CLRBIT(CTRL_PORT, DSS_IO_OE_PIN); //enable DSS level shifter	
}

void dss_disable(void) {
	SETBIT(CTRL_PORT, DISPLAY_PD_PIN); //disable system 5V
	CLRBIT(CTRL_PORT, DVI_PD_PIN); //disable DVI IO
	SETBIT(CTRL_PORT, DSS_IO_OE_PIN); //disable DSS level shifter	
}

int main(void)
{		
	dss_init();
	
	SETBIT(SMCR, SE);	//Sleep enable
	SETBIT(SMCR, SM1); // Power down mode enable
	sei(); //enable interrupts
	
	oled43_driver.init();
	
    while(1) {
		isDVICableAttached = (CHECKBIT(D5_0V_DVID_PIN, D5_0V_DVID_PNUM) &&
						CHECKBIT(D5_0V_DVID_PIN, D5_0V_DVID_PNUM) && 
						CHECKBIT(D5_0V_DVID_PIN, D5_0V_DVID_PNUM) && 
						CHECKBIT(D5_0V_DVID_PIN, D5_0V_DVID_PNUM) &&
						CHECKBIT(D5_0V_DVID_PIN, D5_0V_DVID_PNUM) &&
						CHECKBIT(D5_0V_DVID_PIN, D5_0V_DVID_PNUM));
						
		if (isDVICableAttached && !isDisplayEnabled) {
			dss_enable();
			oled43_driver.enable();
			isDisplayEnabled = 1;
		} else if (!isDVICableAttached) {
			dss_disable();
			oled43_driver.disable();
			isDisplayEnabled = 0;
			asm("sleep");
		}
    }
	
	
	return 0;
}