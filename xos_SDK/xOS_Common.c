#include "xOS_Common.h"
//#include "app_tws_ibrt_cmd_handler.h"
#include "app_ibrt_customif_cmd.h"
#include "app_tws_ibrt.h"
#include "earbud_ux_api.h"
#include "app_ui_api.h"


#ifdef XOS_COMMON_ENABLE


#define XOS_UI_DEBUG_ENABLE
#ifdef  XOS_UI_DEBUG_ENABLE 
#define xos_ui_debug(format,...)     TRACE(3,"[xui]--> %s (%d) " format "\n",__func__,__LINE__,##__VA_ARGS__);
#else
#define xos_ui_debug(format,...) 
#endif


static bt_avrcp_play_status_t  xos_current_play_status=BT_AVRCP_PLAY_STATUS_STOPPED;
static XOS_TWS_LocalState_t    xos_current_local_status={0};
static XOS_TWS_LocalState_t    xos_current_peer_status={0};


bt_avrcp_play_status_t xOS_UI_GetMediumState(void)
{
  return xos_current_play_status;
}

bool xOS_UI_SetMediumState(bt_avrcp_play_status_t state)
{
    xos_ui_debug("%s ,avrcp_status :%d",__FUNCTION__,state);
    xos_current_local_status.medium_state=state;
    return false;
}

XOS_TWS_LocalState_t xOS_UI_GetLocalInfo(void)
{
    return xos_current_local_status;
}

XOS_TWS_LocalState_t xos_UI_GetPeerInfo(void)
{
    return xos_current_peer_status;
}

bool xOS_UI_PeerInfo_Handle(uint8_t *pbuff,uint16_t len)
{
    if ( ( pbuff!=NULL ) && ( len>0 ) ){
        xos_ui_debug("%s",__FUNCTION__);
        memcpy((char*)&xos_current_peer_status,(char*)pbuff,len);
    }
    return false;
}

bool xOS_UI_LocalInfoSendToPeer(void)
{
    if( app_tws_ibrt_tws_link_connected()  && ( app_ibrt_if_get_ui_role()==TWS_SLAVE ) ){		
       //if( ( !app_ibrt_customif_ui_is_tws_switching() )  ){
       //}
	   app_ibrt_custom_sync_moto_wear_status_send((uint8_t *)&xos_current_local_status,sizeof(XOS_TWS_LocalState_t));	
       xos_ui_debug("%s",__FUNCTION__);					
    }
    return false;
}



#endif