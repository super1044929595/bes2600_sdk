#ifndef __XOS_SYSTEMINFO_H

#ifdef __cplusplus
	extern "C"{
#endif

#include "xos_typedef.h"
//#include "xos_sdk_configure.h"

#define XOS_SYS_CHANNEL_LEFT            1
#define XOS_SYS_CHANNEL_RIGHT           2
#define XOS_SYS_CHANNEL_UNKNOW          0
typedef struct{

	uint8_t pairtimeout_enable;
	uint8_t local_bat_level;
	uint8_t peer_bat_level;
	uint8_t local_role;
	uint8_t over_tws_disenable;
	uint8_t over_freeman_disdisenable;
}xos_SystemTimerInfo_s;

bool     xos_mobile_is_conencted(void);
bool     xos_SystemTimer_PairoutHandle(void);
bool     xos_ui_OverDisParameSet(uint8_t enable);

uint8_t  xos_SystemInfo_GetSide(void);
bool     xos_SystemInfo_SetSide(uint8_t leftorright);
bool     xos_SystemInfo_Init(void);
bool     xos_UI_SetPeerBat(uint8_t bat);
uint8_t  xos_UI_GetPeerBat(void);
bool     xos_SystemTimer_PiarCancel(void);
bool 	 xos_SystemMedia_IsOnGoing(void);
uint8_t  xos_SystemAcl_Isconnect(void);

//members 
extern xos_SystemTimerInfo_s xos_SystemTimerInfo;


#ifdef __cplusplus
	}
#endif

#endif
