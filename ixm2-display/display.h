/*
 * display.h
 *
 * Created: 12/7/2011 5:05:49 PM
 *  Author: lwlab
 */ 


#ifndef DISPLAY_H_
#define DISPLAY_H_

struct omap_dss_device {
	void (*init)(void);
	void (*enable)(void);
	void (*disable)(void);

	void (*suspend)(void);
	void (*resume)(void);	
};

#endif /* DISPLAY_H_ */