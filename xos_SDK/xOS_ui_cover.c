#include "xOS_ui_cover.h"


#include "stdio.h"
#include "string.h"

#ifdef XOS_WQ_SDK_ENABLE
#include "dbglog.h"
#include "userapp_dbglog.h"
#include "app_lc_spp.h"
#endif

#define XOS_UI_COVER_DEBUG_ENABLE
#ifdef  XOS_UI_COVER_DEBUG_ENABLE 
#define xos_ui_cover_debug(format,...)     TRACE(3,"[xos cover]%s (%d) " format "\n",__func__,__LINE__,##__VA_ARGS__);
#else
#define xos_ui_cover_debug(format,...) 
#endif