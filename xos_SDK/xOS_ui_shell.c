#include "xos_shell.h"

#define XOS_SHELL_DEBUG_ENABLE
#ifdef  XOS_SHELL_DEBUG_ENABLE 
#define xos_shell_debug(format,...)    TRACE(5,"[xshell:]%s (%d)"format"",__func__,__LINE__,##__VA_ARGS__);
#else
#define xos_shell_debug(format,...) 
#endif

//private members
static uint8_t xos_module_cnt=0;

//--------------------XOS SHELL LIST -----------
typedef struct {

	uint32_t 	xos_shell_active_num;
	osMailQId   xos_shell_mailboxid;
	osThreadId  xos_shell_threadid;
	osPoolId    xos_shell_mempoolid;

}xos_Shell_List;
static xos_Shell_List xos_ShellListInfo;
//--------------------XOS SHELL END -----------

// shel loop task 
#ifdef XOS_BES_SDK_ENABLE
static void xos_ui_shell_thread(void const *argument);
osThreadDef(xos_ui_shell_thread, osPriorityHigh,1,1024, "xshell_thread");
osMailQDef(xos_shell_mailboxid,200U,xOS_UI_shell_Info_t);
osPoolDef(xos_shell_mempoolid, 1000U, uint8_t);
static uint8_t xos_ui_shell_mailbox_cnt = 0;
#endif
  

//member
#define XOS_SHELL_MOD_ACTIVE_MAX_NUM    	32
static xos_ui_shell_handle_typedef xos_shell_module_handle[XOS_SHELL_MOD_ACTIVE_MAX_NUM];
#ifdef XOS_BES_SDK_ENABLE
//method
static bool xOS_ui_shell_handle(XOS_Shell_UI_Mod_E mod,uint8_t *pdata,uint16_t len);

static void xos_ui_shell_thread(void const *argument)
{
	argument=argument;
	#ifdef XOS_BES_SDK_ENABLE
    osEvent              _xshell_event;
    xOS_UI_shell_Info_t *_xshell_info=NULL;
    for( ; ; )
    {
        //wait os singal
		_xshell_event=osMailGet(xos_ShellListInfo.xos_shell_mailboxid,100000);
		if(_xshell_event.status==osOK){
			_xshell_info=(xOS_UI_shell_Info_t*)&_xshell_event.value;
			if(_xshell_info){
				xOS_ui_shell_handle(_xshell_info->ui_mod,_xshell_info->pdata,_xshell_info->len);
				osPoolFree(_xshell_info->pdata,_xshell_info);// watch on 
			}			
			osMailFree(xos_ShellListInfo.xos_shell_mailboxid,&_xshell_event);
		}
		
    }
	#endif
}


static bool xOS_ui_shell_handle(XOS_Shell_UI_Mod_E mod,uint8_t *pdata,uint16_t len)
{

	switch(mod){

	case XOS_UI_WEAR_MOD:
		if(xos_shell_module_handle[XOS_UI_WEAR_MOD]){
			xos_shell_debug("xOS_ui_shell_handle XOS_UI_WEAR_MOD!");
			xos_shell_module_handle[XOS_UI_WEAR_MOD](pdata,len);
		}
	break;

	case XOS_UI_COVER_MOD:
		if(xos_shell_module_handle[XOS_UI_COVER_MOD]){
			xos_shell_debug("xOS_ui_shell_handle XOS_UI_COVER_MOD!");
			xos_shell_module_handle[XOS_UI_COVER_MOD](pdata,len);
		}
	break;

	case XOS_UI_BOX_MOD:
		if(xos_shell_module_handle[XOS_UI_BOX_MOD]){
			xos_shell_debug("xOS_ui_shell_handle XOS_UI_BOX_MOD!");
			xos_shell_module_handle[XOS_UI_BOX_MOD](pdata,len);
		}
	break;
	
	default:
	break;
	}
	
	return false;

}
#endif

bool xos_ui_shell_init(void)
{

#ifdef XOS_BES_SDK_ENABLE
//mail create
	xos_ShellListInfo.xos_shell_mailboxid = osMailCreate(osMailQ(xos_shell_mailboxid), NULL);
	if (xos_ShellListInfo.xos_shell_mailboxid == NULL)  {	
		xos_shell_debug("Failed to Create xshell mail");
		return true;
	}
	xos_ui_shell_mailbox_cnt = 0;
	
//thread create
	xos_ShellListInfo.xos_shell_threadid=osThreadCreate(osThread(xos_ui_shell_thread), NULL);
    if (xos_ShellListInfo.xos_shell_threadid == NULL)  {
        xos_shell_debug("Failed to Create xshell thread");
        return true;
    }

//xopool
	if (xos_ShellListInfo.xos_shell_mempoolid==NULL){
		xos_ShellListInfo.xos_shell_mempoolid = osPoolCreate(osPool(xos_shell_mempoolid));
		ASSERT(xos_ShellListInfo.xos_shell_mempoolid, "[%s] ERROR: xos_shell_mempool != NULL", __func__);
	}
 #endif
 
    return false;
}

bool xos_ui_shell_send(XOS_Shell_UI_Mod_E mod,uint8_t *pdata,uint16_t len)
{
    if( pdata==NULL || len<=0) return true;
	#ifdef XOS_BES_SDK_ENABLE
    //
    xOS_UI_shell_Info_t *_xshell_info=NULL;
    _xshell_info=(xOS_UI_shell_Info_t*)osMailAlloc(xos_ShellListInfo.xos_shell_mailboxid, 1000);
	_xshell_info->len=len;
	_xshell_info->pdata=(uint8_t*)osPoolCAlloc (xos_ShellListInfo.xos_shell_mempoolid);
	ASSERT(_xshell_info->pdata,"[%s] ERROR: xos_ui_shell_send ptr != NULL", __func__);
	memcpy((uint8_t*)_xshell_info->pdata,(uint8_t*)pdata,len);
    if(osOK!=osMailPut(xos_ShellListInfo.xos_shell_mailboxid, (const void*)_xshell_info)){
		xos_shell_debug("\r\n xos_ui_shell_send error!");
    }else{
    	xos_shell_debug("\r\n xos_ui_shell_send");
    }
    //end
	#endif
    return false;
}


bool xos_ui_shell_register(XOS_Shell_UI_Mod_E xshell_mod,xos_ui_shell_handle_typedef xshell_app)
{
	ASSERT(xshell_app, "[%s] ERROR: xos_ui_shell_register != NULL", __func__);
	xos_shell_module_handle[xshell_mod]=xshell_app;
	if(++xos_module_cnt>XOS_SHELL_MOD_ACTIVE_MAX_NUM){
		xos_shell_debug(" xos_ui_shell_register register module is over! ");
		return true;
	}
	return false;
}
