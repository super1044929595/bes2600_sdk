#include "xOS_ui_box.h"


#define XOS_UI_BOX_DEBUG_ENABLE
#ifdef  XOS_UI_BOX_DEBUG_ENABLE 
#define xos_ui_box_debug(format,...)     TRACE(3,"[xos box]%s (%d) " format "\n",__func__,__LINE__,##__VA_ARGS__);
#else
#define xos_ui_box_debug(format,...) 
#endif

static os_ui_handle_callback xos_ui_box_handle_cb=(os_ui_handle_callback)NULL;

bool xos_ui_box_gester(os_ui_handle_callback _cb)
{

	return false;
}

os_ui_handle_callback xos_ui_box_gethandleId(void)
{
	return xos_ui_box_handle_cb;
}

bool xos_ui_box_init(void)
{
	//xos_bsp_box_init();

	
	return false;
}