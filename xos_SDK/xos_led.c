#include "xos_led.h"

#define XOS_LED_DEBUG_ENABLE
#ifdef  XOS_LED_DEBUG_ENABLE 
#define xos_led_debug(format,...)     TRACE(5,"[xos led %d] %s "format "\n",__LINE__,__func__,##__VA_ARGS__);
#else
#define xos_led_debug(format,...) 
#endif

static const char * const xos_led_string_type[] ={

	"[XOS_LED_POWER_OFF]",
	"[XOS_LED_POWER_ON]",
	"[XOS_LED_CONNECTABLE]",
	"[xOS_LED_DISCOVERABLE]",

};


bool xos_led_status_set(xOS_LED_Type_E led_type)
{
	xos_led_debug("entern xos_led_status_set type:%s",xos_led_string_type[led_type]);
	
	if(led_type>=xOS_LED_MAX)	return true;

	
	switch(led_type){

		case XOS_LED_POWER_OFF:

		break;

		default:

		break;
	}
	return false;
}

