/*
 * delay.c
 *
 * Created: 12/7/2011 5:53:54 PM
 *  Author: lwlab
 * Design for 1MHz oscillator
 */ 
void mdelay(unsigned char x)
{
volatile unsigned char w,y,g;

for (w=0; w<x ; w++) {
	for (y=0; y<2; y++) {
		for (g=0; g<41; g++) {
		asm("nop");
		}
	}
}
}