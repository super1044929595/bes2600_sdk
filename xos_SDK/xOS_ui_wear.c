#include "xOS_ui_wear.h"
#include "stdio.h"
#include "string.h"

#ifdef XOS_WQ_SDK_ENABLE
#include "dbglog.h"
#include "userapp_dbglog.h"
#include "app_lc_spp.h"
#endif

#define XOS_UI_DEBUG_ENABLE
#ifdef  XOS_UI_DEBUG_ENABLE 
#define xos_ui_debug(format,...)     TRACE(3,"%s (%d) " format "\n",__func__,__LINE__,##__VA_ARGS__);
#else
#define xos_ui_debug(format,...) 
#endif

//method
static void app_gesture_log_parse(uint8_t* pdata, uint16_t len);
static void app_gesture_xlogregister(void);
static xos_ui_module_register  _gesture_callback = NULL;


void app_gesture_log_parse(uint8_t* pdata, uint16_t len)
{
	char printfdata[126]={0};
	if(pdata==NULL || len<=0) return ;
	sprintf(printfdata,(char*)pdata,len);
	xos_ui_debug("%s",printfdata);
	xos_ui_debug("\r\n xlog--->app_gesture error_type:%d,len: %d",pdata[0],len);
}


void app_gesture_xlogregister(void)
{
	_gesture_callback = app_gesture_log_parse;
}

xos_ui_module_register app_gesture_getcallback(void)
{
	if (_gesture_callback == NULL) return NULL;
	return _gesture_callback;
}

void app_gesture_init(void)
{
	app_gesture_xlogregister();
}


bool xOS_UI_Wear_Init(void)
{

	return false;
}
