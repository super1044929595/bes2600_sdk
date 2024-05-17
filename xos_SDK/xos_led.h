#ifndef __XOS_LED_H
#define __XOS_LED_H

#ifdef __cplusplus
    extern "C"{
#endif

#include "xos_typedef.h"

typedef enum {

	XOS_LED_POWER_OFF=0,
	XOS_LED_POWER_ON,
	XOS_LED_CONNECTABLE,
	xOS_LED_DISCOVERABLE,
	xOS_LED_MAX,

}xOS_LED_Type_E;

bool xos_led_status_set(xOS_LED_Type_E led_type);

#ifdef __cplusplus
    	}
#endif
#endif


