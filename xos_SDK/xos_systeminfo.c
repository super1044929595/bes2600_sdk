#include "xos_systeminfo.h"
#include "xos_sdk_configure.h"

#include "hal_trace.h"
#include "apps.h"
#include "adapter_service.h"
#include "app_bt.h"
#include "xos_timer.h"
#include "app_battery.h"
#include "app_a2dp.h"
#include "app_ui_api.h"
#include "app_hfp.h"
#include "app_tws_ibrt_conn_api.h"
#include "earbud_ux_api.h"
#include "app_a2dp.h"


#define XOS_SYSTEMINFO_DEBUG_ENABLE
#ifdef  XOS_SYSTEMINFO_DEBUG_ENABLE 
#define xos_sys_debug(format,...)     TRACE(5,"[xos sysinfo %d] %s "format "\n",__LINE__,__func__,##__VA_ARGS__);
#else
#define xos_sys_debug(format,...) 
#endif

//members
xos_SystemTimerInfo_s xos_SystemTimerInfo;

bool xos_SystemInfo_Init(void)
{
	xos_SystemTimerInfo.local_bat_level=(uint8_t)app_battery_current_level();
	xos_SystemTimerInfo.local_role=XOS_SYS_CHANNEL_UNKNOW;
	xos_SystemTimerInfo.peer_bat_level=0;
	xos_SystemTimerInfo.local_bat_level=0;
	xos_SystemTimerInfo.over_tws_disenable=0;
	xos_SystemTimerInfo.over_freeman_disdisenable=0;
	return false;
}

static uint32_t xos_ui_overdis_timeouthandle(uint32_t argc ,uint32_t *argv)
{
	if(xos_SystemAcl_Isconnect()>=1) return 0;

	if(app_ibrt_if_is_in_freeman_mode()){
		if(app_ui_get_local_box_state()>IBRT_IN_BOX_CLOSED){	
			if(app_bt_ibrt_has_mobile_link_connected()){
			}else{
				
				TRACE(0,"overdis_monitor freeman mode timeout!");
				app_shutdown(); 
			}
		}
	}else{
		if(!app_ibrt_conn_is_tws_connected()){
			if(app_ui_get_local_box_state()>IBRT_IN_BOX_CLOSED){				
				if(btapp_hfp_is_sco_active()||a2dp_is_music_ongoing()){
				}else{
					
					TRACE(0,"overdis_monitor tws mode timeout!");
					app_shutdown(); 				
				}			
			}else{
				TRACE(0,"[xos overdis] shunt down timeout error !");
			}
		}
	}
	
	return 0;
}


bool xos_ui_OverDisParameSet(uint8_t enable)
{
	#define XOS_UI_OVERDIS_TIMEOUT  (600000U)

	if(app_tws_is_connected()){//tws mode
		xos_SystemTimerInfo.over_tws_disenable=enable;
		xos_sys_debug("overdis_monitor tws [%d]",enable);
	}else{//freeman mode
		xos_SystemTimerInfo.over_freeman_disdisenable=enable;		
		xos_sys_debug("overdis_monitor freeman [%d]",enable);
	}

	if(enable==1){
		Software_TimerStart(xOS_Timer_Module_OVERDIS_ID,JW_SOFTWARE_PERIOD_ONECE,XOS_UI_OVERDIS_TIMEOUT,xos_ui_overdis_timeouthandle,0,NULL);
	}else{
		Software_TimerCancel(xOS_Timer_Module_OVERDIS_ID);
	}
	return false;
}


bool xos_UI_SetPeerBat(uint8_t bat)
{
	xos_SystemTimerInfo.peer_bat_level=bat;	
	xos_sys_debug("xos bat %d",bat);
	return false;
}

uint8_t xos_UI_GetPeerBat(void)
{
	xos_sys_debug("xos bat %d",xos_SystemTimerInfo.peer_bat_level);
	return xos_SystemTimerInfo.peer_bat_level;
}

uint32_t xos_SystemTimer_callback(uint32_t argc ,uint32_t *argv)
{

	xos_sys_debug("xos_SystemTimer_callback %d",xos_SystemTimerInfo.pairtimeout_enable);
	xos_sys_debug("xos_SystemTimer_PairoutHandle conn[%d]",xos_mobile_is_conencted());


	if(xos_SystemTimerInfo.pairtimeout_enable && !xos_mobile_is_conencted()){
		xos_SystemTimerInfo.pairtimeout_enable=0;

		//add the pairouttime handle by user
		app_user_timer_stop(7);
#ifndef BLE_ONLY_ENABLED
		PairingTransferToConnectable();
#endif
	
		pairtimeout_shutdown(); //jan add pairtimeout shutdown
	}else{
		xos_sys_debug("xos_SystemTimer_PairoutHandle is ok ");
	}

	return 0;
}

bool xos_mobile_is_conencted(void)
{

	ibrt_mobile_info_t *p_mobile_info = app_ibrt_conn_get_mobile_info_ext();

	if(app_bt_ibrt_has_mobile_link_connected()||app_tws_ibrt_slave_ibrt_link_connected(&p_mobile_info->mobile_addr))
		return true;
	else
		return false;
	
}

bool xos_SystemTimer_PairoutHandle(void)
{
	xos_SystemTimerInfo.pairtimeout_enable=1;
#ifdef XOS_TIMER_FUNCTION_ENABLE
	Software_TimerStart(xOS_Timer_Module_POWERCONSUMPTION_ID,JW_SOFTWARE_PERIOD_ONECE,(3*60*1000+500),xos_SystemTimer_callback,0,NULL);
#endif
	xos_sys_debug("pairtimeout start [%d]",xos_SystemTimerInfo.pairtimeout_enable);

	return false;
}

bool xos_SystemTimer_PiarCancel(void)
{
	return false;
}

uint8_t  xos_SystemInfo_GetSide(void)
{
	return xos_SystemTimerInfo.local_role;
}

bool  xos_SystemInfo_SetSide(uint8_t leftorright)
{
	xos_SystemTimerInfo.local_role=leftorright;
	return false;
}

bool xos_SystemMedia_IsOnGoing(void)
{
	xos_sys_debug("media play status [%d]",a2dp_is_music_ongoing());
	return a2dp_is_music_ongoing();
}

uint8_t xos_SystemAcl_Isconnect(void)
{
	uint8_t xos_cnt=0;
	struct BT_DEVICE_T *device = NULL;

	for (int i = 0; i < BT_DEVICE_NUM; ++i)
	{
		device = app_bt_get_device(i);
		if (device->acl_is_connected)
		{
			xos_cnt++;
		}
	}
	xos_sys_debug("current device num:%d",xos_cnt);
	return xos_cnt;
}
