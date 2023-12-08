#include "xOS_App.h"
#include "xOS_State.h"
#include "stdio.h"
#include "hal_trace.h"
#include "app_utils.h"
#include "app_thread.h"
#include "cmsis_os.h"
#include "app_thread.h"
#include "app_utils.h"
#include "xOS_Timer.h"

#define XOS_APP_DEBUG_ENABLE
#ifdef  XOS_APP_DEBUG_ENABLE 
#define xos_app_debug(format,...)     TRACE(3,"%s (%d) " format "\n",__func__,__LINE__,##__VA_ARGS__);
//printf("app[%s:%s (%d)]:" format "\n" ,__FILE__,__FUNCTION__,__LINE__,##__VA_ARGS__);
#else
#define xos_app_debug(format,...) 
#endif


//---------------------------------------------------------------------------------------------------
#define osSTATE_DEFAULT_INEDEX                                      0U
#define osAPP_MONITOR_INTERVER_TIME                                 1000u

static  xOS_App_Op_E     xOS_APP_Current_Op=XOS_APP_OP_INITING_E;
static  xOS_App_State_E  xos_APP_current_State=XOS_APP_STATE_INITING_E;
static  uint8_t xApp_Sleep_Mode = 0;


static uint8_t User_App_Power_SetSleep(uint8_t sleepenable);
#if 1
//add timer configure
static void xos_power_state_timerhandler(void const *param);
osTimerDef(xOS_State_timerID,xos_power_state_timerhandler);
static osTimerId xOS_StateTimerhandlerId = NULL;
#endif

// UI handle
static void
osState_op_tws_InitDone(xos_handle_state pre,xos_handle_operate operate,xos_handle_state next);
static void
osState_OP_tws_BoxIN_Hanlde(xos_handle_state pre,xos_handle_operate operate,xos_handle_state next);
//static void
//osState_OP_tws_BoxOut_Hanlde(xos_handle_state pre,xos_handle_operate operate,xos_handle_state next);
static void 
osState_OP_tws_CoverIn_Hanlde(xos_handle_state pre, xos_handle_operate operate, xos_handle_state next);
//static void
//osState_OP_tws_CoverOut_Hanlde(xos_handle_state pre, xos_handle_operate operate, xos_handle_state next);
//static void
//osState_OP_tws_WearOff_Hanlde(xos_handle_state pre, xos_handle_operate operate, xos_handle_state next);
//static void
//osState_OP_tws_WearOn_Hanlde(xos_handle_state pre, xos_handle_operate operate, xos_handle_state next);
static void 
osState_App_handle_public(xos_handle_state pre,xos_handle_operate operate,xos_handle_state next);

/*+---------------------------------------------------------------------------
	              App User Table Init
  +--------------------------------------------------------------------------*/
  
const xOS_StateInfo appUserTabel[]={
    {XOS_STATE(XOS_APP_STATE_INITING_E  , XOS_APP_STATE_INITDONE_E )      ,XOS_APP_OP_INITING_E      ,osState_op_tws_InitDone},
    {XOS_STATE(XOS_APP_STATE_INITDONE_E , XOS_APP_STATE_BOX_IN_E   )      ,XOS_APP_OP_BOX_IN_E       ,osState_OP_tws_BoxIN_Hanlde},   
    {XOS_STATE(XOS_APP_STATE_BOX_IN_E  ,  XOS_APP_STATE_COVER_IN_E )      ,XOS_APP_OP_BOX_OUT_E      ,osState_OP_tws_CoverIn_Hanlde},
};



static void osState_op_tws_InitDone(xos_handle_state pre,xos_handle_operate operate,xos_handle_state next)
{
	xos_app_debug("osState_op_tws_InitDone");
};
	
static void osState_OP_tws_BoxIN_Hanlde(xos_handle_state pre,xos_handle_operate operate,xos_handle_state next)
{
	xos_app_debug("osState_OP_tws_BoxIN_Hanlde");
	xos_APP_current_State = XOS_APP_STATE_BOX_IN_E ;
	//add  power cost 

}

static void osState_OP_tws_CoverIn_Hanlde(xos_handle_state pre, xos_handle_operate operate, xos_handle_state next)
{
	xos_app_debug("osState_OP_tws_CoverIn_Hanlde");

}

/*
static void osState_OP_Initing(xos_handle_state pre,xos_handle_operate operate,xos_handle_state next)
{
	xos_app_debug("osState_OP_Initing");
	//----------------------------------------------------------

	//----------------------------------------------------------

}
static void osState_OP_tws_BoxOut_Hanlde(xos_handle_state pre,xos_handle_operate operate,xos_handle_state next)
{
	xos_app_debug("osState_OP_tws_BoxOut_Hanlde");

}


static void osState_OP_tws_CoverOut_Hanlde(xos_handle_state pre, xos_handle_operate operate, xos_handle_state next)
{
	xos_app_debug("osState_OP_tws_CoverOut_Hanlde");

}

static void osState_OP_tws_WearOff_Hanlde(xos_handle_state pre, xos_handle_operate operate, xos_handle_state next)
{
	xos_app_debug("osState_OP_tws_WearOff_Hanlde");
}

static void osState_OP_tws_WearOn_Hanlde(xos_handle_state pre, xos_handle_operate operate, xos_handle_state next)
{
	xos_app_debug("osState_OP_tws_WearOn_Hanlde");

}
*/

static void osState_App_handle_public(xos_handle_state pre, xos_handle_operate operate, xos_handle_state next)
{
	xos_app_debug("osState_App_handle_public");
}

/*+---------------------------------------------------------------------------
			      App User Table End
	+--------------------------------------------------------------------------*/

static uint8_t xOS_APP_TimerActiveCnt=0;
#if 1
static void xos_power_state_timerhandler(void const *param)
{
	if( xApp_Sleep_Mode ==1 ){
			if(++xOS_APP_TimerActiveCnt>=3){
				app_sysfreq_req(APP_SYSFREQ_USER_APP_INIT, APP_SYSFREQ_32K );
				xOS_APP_TimerActiveCnt=0;
				xos_app_debug("osState_OP_tws_BoxIN_Hanlde APP_SYSFREQ_32K ");
			}
	}
}
#endif


uint8_t User_APP_Init(void)
{
	//led_demo_init();
    os_HanldeTableInit(0,appUserTabel,sizeof(appUserTabel)/sizeof(appUserTabel[0]));

    os_handleTable_PublicHandle_Register(0, osState_App_handle_public);

	xos_app_debug("ADV timeout duration :[%d]", osSTATE_DEFAULT_INEDEX);

	os_Handle_StateSwitch(osSTATE_DEFAULT_INEDEX, XOS_APP_OP_INITING_E);

	//create timer 
	if(xOS_StateTimerhandlerId==NULL){
		xOS_StateTimerhandlerId=osTimerCreate (osTimer(xOS_State_timerID), osTimerPeriodic, NULL);
        //osTimerStart(xOS_StateTimerhandlerId,500);
  	}

	xOS_SDKCreate_Timer();
	return osState_True;
}




uint8_t User_APP_PowerState_Set(xOS_App_Op_E op)
{
 	switch(op){

		case XOS_APP_OP_BOX_IN_E:
		xos_app_debug("jw XOS_APP_OP_BOX_IN_E");
		os_Handle_CurrentState_JumpeSet(osSTATE_DEFAULT_INEDEX,XOS_APP_STATE_INITDONE_E,XOS_APP_OP_BOX_IN_E );
		os_Handle_StateSwitch(osSTATE_DEFAULT_INEDEX, XOS_APP_OP_BOX_IN_E);
		User_App_Power_SetSleep(1);
		osTimerStart(xOS_StateTimerhandlerId,osAPP_MONITOR_INTERVER_TIME);
		break;

		case XOS_APP_OP_BOX_OUT_E:
		xos_app_debug("jw XOS_APP_OP_BOX_OUT_E");
		User_App_Power_SetSleep(0);
		if(osTimerIsRunning(xOS_StateTimerhandlerId)){
			osTimerStop(xOS_StateTimerhandlerId);
		}
		xOS_APP_TimerActiveCnt=0;
		break;

		case XOS_APP_OP_COVER_IN_E:
		xos_app_debug("jw XOS_APP_OP_COVER_IN_E");
		User_App_Power_SetSleep(1);
		osTimerStart(xOS_StateTimerhandlerId,osAPP_MONITOR_INTERVER_TIME);
		break;

		case XOS_APP_OP_COVER_OUT_E:
		xos_app_debug("jw XOS_APP_OP_COVER_OUT_E");
		User_App_Power_SetSleep(0);
		if(osTimerIsRunning(xOS_StateTimerhandlerId)){
			osTimerStop(xOS_StateTimerhandlerId);
		}
		xOS_APP_TimerActiveCnt=0;
		break;

		default:
		break;

	}
	os_Handle_StateSwitch(osSTATE_DEFAULT_INEDEX,op);
	xOS_APP_Current_Op=op;
	//app_sysfreq_req(APP_SYSFREQ_USER_APP_4,APP_SYSFREQ_26M);
    return 0;
}


static uint8_t User_App_Power_SetSleep(uint8_t sleepenable)
{
    xApp_Sleep_Mode = sleepenable;
	return 0;
}

uint8_t User_App_Power_GetSleep(void)
{
	return xApp_Sleep_Mode;
}

