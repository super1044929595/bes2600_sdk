#include "xos_ui_handle.h"


#define XOS_UI_DEBUG_ENABLE
#ifdef  XOS_UI_DEBUG_ENABLE 
#define xos_ui_debug(format,...)     TRACE(5,"[xos ui %d] %s "format "\n",__LINE__,__func__,##__VA_ARGS__);
#else
#define xos_ui_debug(format,...) 
#endif

//private members
//-------box state---------
#define XOS_BOX_STATE_IN     0
#define XOS_BOX_STATE_OUT    1
#define XOS_BOX_STATE_UNKNOW 2
static uint8_t xos_box_state=XOS_BOX_STATE_UNKNOW;
//-------cover state-------
#define XOS_COVER_STATE_IN      0
#define XOS_COVER_STATE_OUT     1
#define XOS_COVER_STATE_UNKNOW  2
static uint8_t xos_cover_state=XOS_COVER_STATE_UNKNOW;



bool xos_ui_poweroff(void)
{
	xos_ui_debug("xos_ui_poweroff");
	return false;
}

bool xos_ui_poweron(void)
{
	return false;
}



//---------------UI CONVER
uint8_t xos_ui_conver_state_get(void)
{
	return xos_cover_state;
}

bool xos_ui_cover_state_set(uint8_t state)
{
	xos_cover_state=state;
	return false;
}


//---------------UI BOX
bool xos_ui_box_state_set(uint8_t state)
{
	xos_box_state=state;
	return false;
}
// box state
uint8_t xos_ui_box_state_get(void)
{
	return xos_box_state;
}


//---------------UI WEAR
bool xos_UI_Wear_StateSet(void)
{
	return false;
}

bool xos_UI_Wear_StateGet(void)
{
	return false;
}
