#ifndef __XOS_KEY_H
#define __XOS_KEY_H

#include "xos_typedef.h"

#ifdef __cplusplus
    extern "C"{
#endif

#define XOS_KEY_SHORT_E    0
#define XOS_KEY_DOUBLE_E   1
#define XOS_KEY_THREE_E    2 
#define XOS_KEY_LONG_E     3


//XOS  Board  Key
typedef enum{
    xOS_KEY_Board_POWER,
    xOS_KEY_Board_UP,
    xOS_KEY_Board_DOWN,
}xOS_KEY_BoardId;

//XOS  Key Function
typedef enum{
    xOS_KEY_FUNCTION_PLAYANDPAUSE,
    xOS_KEY_FUNCTION_MUSIC_BACK,
    xOS_KEY_FUNCTION_MUSIC_FORWARD,
    xOS_KEY_FUNCTION_CALLUP,
    xOS_KEY_FUNCTION_ANC,
}xOS_KEY_FunctionId;

//XOS Key Handle Status
typedef enum{
    xOS_KEY_HANDLE_MEDIUM_ACTIVE,
    xOS_KEY_HANDLE_CALL_INCOMING_ACTIVE,
    xOS_KEY_HANDLE_CALL_OUTING_ACTIVE,
    xOS_KEY_HANDLE_OTHER_ACTIVE,   
    //-------------------------------------------add by jw 
	xOS_KEY_STATE_DUTMODE,
	xOS_KEY_STATE_CHARGERING,
	xOS_KEY_STATE_TWS_DISCONNECTED,
	xOS_KEY_STATE_TWS_CONNECTED,
	xOS_KEY_STATE_TWS_PAIRING,
	xOS_KEY_STATE_MEDIA_STREAMING,
	xOS_KEY_STATE_SECOND_CALL_ON_HOLD,
	xOS_KEY_STATE_SECOND_CALL_INCOMMING,
	xOS_KEY_STATE_OUTING_CALL,
	xOS_KEY_STATE_INCOMING_CALL,
	xOS_KEY_STATE_CALL_ACTIVE_ON_PHONE,
	xOS_KEY_STATE_CALL_ACTIVE,
	xOS_KEY_STATE_CONNECTED,
	xOS_KEY_STATE_DISCOVERABLE,
	xOS_KEY_STATE_CONNETABLE,
	xOS_KEY_STATE_POWEROFF,
}xOS_KEY_HandleStatus;


typedef enum{
	xOS_KEY_EVENT_PLAY_PAUSE=0,
	xOS_KEY_EVENT_ACCEPT_CALL,
	xOS_KEY_EVENT_END_CALL,
	xOS_KEY_EVENT_BACKWARD,
	xOS_KEY_EVENT_VOICE_ASSISTANT,
	xOS_KEY_EVENT_FACTROY_RESET,
	xOS_KEY_EVENT_POWER_ON,
	xOS_KEY_EVENT_REJECT_CALL,
	xOS_KEY_EVENT_POWER_OFF,
	xOS_KEY_EVENT_VOL_DOWN,
	xOS_KEY_EVENT_VOL_UP,
}xOS_KEY_EVENT_ID;

bool xos_KeyStatusSet(uint8_t tws_side,uint8_t key_type,uint8_t index,xOS_KEY_HandleStatus key_status);
uint8_t xOS_Key_GroupHandle(uint8_t key_type, uint8_t value);

#ifdef __cplusplus
    }
#endif
#endif

