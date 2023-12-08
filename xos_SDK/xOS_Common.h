#ifndef __XOS_COMMON_H
#define __XOS_COMMON_H
#include "adapter_service.h"
#include "xOS_Parame_Configure.h"
#ifdef __cplusplus
	extern "C"{
#endif

#ifdef XOS_COMMON_ENABLE

typedef struct{
    //tws base
    uint8_t wear_state; // 0: wear off 1: wear on
    uint8_t inbox_state;// 0: inbox_in 1:inbox_out
    uint8_t cover_state;// 0: cover_in 1:cover_out

    // current state 
    bt_avrcp_play_status_t medium_state;//0: audio invaild    1: audio active
    uint8_t hfp_state   ;//0: call  active     0: call invaild
}XOS_TWS_LocalState_t;

bt_avrcp_play_status_t xOS_UI_GetMediumState(void);
bool xOS_UI_SetMediumState(bt_avrcp_play_status_t state);
XOS_TWS_LocalState_t xOS_UI_GetLocalInfo(void);
//add  tws sysnc
bool xOS_UI_LocalInfoSendToPeer(void);
bool xOS_UI_PeerInfo_Handle(uint8_t *pbuff,uint16_t len);
#endif

#ifdef __cplusplus
    }
#endif

#endif
