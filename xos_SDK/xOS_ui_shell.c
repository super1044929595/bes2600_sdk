#include "xOS_ui_shell.h"

#define XOS_SHELL_DEBUG_ENABLE
#ifdef  XOS_SHELL_DEBUG_ENABLE 
#define xos_shell_debug(format,...)    TRACE(5,"[xshell:]%s (%d)"format"",__func__,__LINE__,##__VA_ARGS__);
#else
#define xos_shell_debug(format,...) 
#endif
// shel loop task 
#ifdef XOS_BES_SDK_ENABLE
static void xos_ui_shell_thread(void const *argument);
osThreadDef(xos_shell_thread, osPriorityHigh,1,1024, "xshell_thread");
static osThreadId xos_ui_shell_threadid;

osMailQDef (xos_ui_shell_mailbox,200U,xOS_UI_shell_Info_t);
static osMailQId xos_ui_shell_mailbox = NULL;
static uint8_t xos_ui_shell_mailbox_cnt = 0;

osPoolDef (xos_shell_mempoolid, 1000U, uint8_t);
osPoolId   xos_shell_mempoolid = NULL;
#endif

//method
static bool xOS_ui_shell_handle(XOS_Shell_UI_Mod_E mod,uint8_t *pdata,uint16_t len);
//member
static xos_ui_shell_handle_typedef xos_shell_module_handle[XOS_UI_MAX_MOD];



static void xos_ui_shell_thread(void const *argument)
{
    osEvent              _xshell_event;
    xOS_UI_shell_Info_t *_xshell_info=NULL;
    for( ; ; )
    {
        //wait os singal
        #ifdef XOS_BES_SDK_ENABLE
		_xshell_event=osMailGet(xos_ui_shell_mailbox,100000);
		if(_xshell_event.status=osEventMail){
			_xshell_info=(xOS_UI_shell_Info_t*)_xshell_event.value;
			if(_xshell_info){
				xOS_ui_shell_handle(_xshell_info->ui_mod,_xshell_info->pdata,_xshell_info->len);
				osPoolFree(_xshell_info->pdata);
			}			
			osMailFree(xos_ui_shell_mailbox,&_xshell_event);
		}
		#endif
    }
    return false;
}


static bool xOS_ui_shell_handle(XOS_Shell_UI_Mod_E mod,uint8_t *pdata,uint16_t len)
{

	switch(mod){

	case XOS_UI_WEAR_MOD:
		if(xos_shell_module_handle[XOS_UI_WEAR_MOD]){
			xos_shell_debug("xOS_ui_shell_handle XOS_UI_WEAR_MOD");
			xos_shell_module_handle[XOS_UI_WEAR_MOD](pdata,len);
		}
	break;
	
	default:
	break;
	}
}


bool xos_ui_shell_init(void)
{

#ifdef XOS_BES_SDK_ENABLE
//mail create
	xos_ui_shell_mailbox = osMailCreate(osMailQ(xos_ui_shell_mailbox), NULL);
	if (xos_ui_shell_mailbox == NULL)  {	
		xos_shell_debug("Failed to Create xshell mail");
		return true;
	}
	xos_ui_shell_mailbox_cnt = 0;
	
//thread create
	xos_ui_shell_threadid=osThreadCreate(osThread(xos_shell_thread), NULL)
    if (xos_ui_shell_threadid == NULL)  {
        xos_shell_debug("Failed to Create xshell thread");
        return true
    }

//xopool
	if (xos_shell_mempoolid == NULL){
		xos_shell_mempoolid = osPoolCreate(osPool(xos_shell_mempoolid));
		ASSERT(xos_shell_mempoolid, "[%s] ERROR: xos_shell_mempool != NULL", __func__);
	}
 #endif
    return false;
}




bool xos_ui_shell_send(XOS_Shell_UI_Mod_E mod,uint8_t *pdata,uint16_t len)
{
    if( pdata==NULL || len<=0) return true;
    //
    xOS_UI_shell_Info_t *_xshell_info=NULL;
    _xshell_info=(xOS_UI_shell_Info_t*)osMailAlloc(xos_ui_shell_mailbox, 1000);
	_xshell_info->len=len;
	_xshell_info->pdata=(uint8_t*)osPoolCAlloc (app_audio_status_mempool);
	ASSERT(_xshell_info->pdata,"[%s] ERROR: xos_ui_shell_send ptr != NULL", __func__);
	memcpy((uint8_t*)_xshell_info->pdata,(uint8_t*)pdata,len);
    if(osOK!=osMailPut(xos_ui_shell_mailbox, const void * mail)){
		xos_shell_debug("\r\n xos_ui_shell_send error!");
    }else{
    	xos_shell_debug("\r\n xos_ui_shell_send");
    }
    //end
    return false;
}


bool xos_ui_shell_register(XOS_Shell_UI_Mod_E xshell_mod,xos_ui_shell_handle_typedef xshell_app)
{
	ASSERT(xshell_app, "[%s] ERROR: xos_ui_shell_register != NULL", __func__);
	xos_shell_module_handle[xshell_mod]=xshell_app;
	return false;
}

