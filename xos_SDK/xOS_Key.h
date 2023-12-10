#ifndef __XOS_KEY_H
#define __XOS_KEY_H

#include "stdio.h"
#include "stdint.h"
#include "stddef.h"

#ifdef __cplusplus
    extern "C"{
#endif

#define XOS_KEY_SHORT_E  0
#define XOS_KEY_DOUBLE_E 1
#define XOS_KEY_THREE_E  2
#define XOS_KEY_LONG_E   3


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
}xOS_KEY_HandleStatus;


uint8_t xOS_Key_GroupHandle(uint8_t key_type, uint8_t value);

#ifdef __cplusplus
    }
#endif
#endif