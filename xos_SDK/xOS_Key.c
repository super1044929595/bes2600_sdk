#include "xOS_Key.h"
#include "stdint.h"


////////////////////////////////////////////////////////////////////////////////////////
//XOS Key_Configure type
typedef struct{
    xOS_KEY_BoardId            key_value;
    xOS_KEY_FunctionId         key_function;
    xOS_KEY_HandleStatus       key_status;
}xOS_KeyType_t;




xOS_KeyType_t xos_shortkeygroup[]={
    {
        .key_value=xOS_KEY_Board_POWER,
        .key_function=xOS_KEY_FUNCTION_PLAYANDPAUSE,
        .key_status=1<<xOS_KEY_HANDLE_MEDIUM_ACTIVE,
    },
    {
        .key_value=xOS_KEY_Board_POWER,
        .key_function=xOS_KEY_FUNCTION_CALLUP,
        .key_status=1<<xOS_KEY_HANDLE_CALL_INCOMING_ACTIVE,
    },
    {
        .key_value=xOS_KEY_Board_POWER,
        .key_function=xOS_KEY_FUNCTION_CALLUP,
        .key_status=1<<xOS_KEY_HANDLE_CALL_OUTING_ACTIVE,
    },
};

xOS_KeyType_t xos_doublekeygroup[]={
    {
        .key_value=xOS_KEY_Board_POWER,
        .key_function=xOS_KEY_FUNCTION_PLAYANDPAUSE,
        .key_status=1<<xOS_KEY_HANDLE_MEDIUM_ACTIVE,
    },
    {
        .key_value=xOS_KEY_Board_POWER,
        .key_function=xOS_KEY_FUNCTION_CALLUP,
        .key_status=1<<xOS_KEY_HANDLE_CALL_INCOMING_ACTIVE,
    },
    {
        .key_value=xOS_KEY_Board_POWER,
        .key_function=xOS_KEY_FUNCTION_CALLUP,
        .key_status=1<<xOS_KEY_HANDLE_CALL_OUTING_ACTIVE,
    },
};

xOS_KeyType_t xos_threekeygroup[]={
    {
        .key_value=xOS_KEY_Board_POWER,
        .key_function=xOS_KEY_FUNCTION_PLAYANDPAUSE,
        .key_status=1<<xOS_KEY_HANDLE_MEDIUM_ACTIVE,
    },
    {
        .key_value=xOS_KEY_Board_POWER,
        .key_function=xOS_KEY_FUNCTION_CALLUP,
        .key_status=1<<xOS_KEY_HANDLE_CALL_INCOMING_ACTIVE,
    },
    {
        .key_value=xOS_KEY_Board_POWER,
        .key_function=xOS_KEY_FUNCTION_CALLUP,
        .key_status=1<<xOS_KEY_HANDLE_CALL_OUTING_ACTIVE,
    },
};

xOS_KeyType_t xos_longkeygroup[]={
    {
        .key_value=xOS_KEY_Board_POWER,
        .key_function=xOS_KEY_FUNCTION_PLAYANDPAUSE,
        .key_status=1<<xOS_KEY_HANDLE_MEDIUM_ACTIVE,
    },
    {
        .key_value=xOS_KEY_Board_POWER,
        .key_function=xOS_KEY_FUNCTION_CALLUP,
        .key_status=1<<xOS_KEY_HANDLE_CALL_INCOMING_ACTIVE,
    },
    {
        .key_value=xOS_KEY_Board_POWER,
        .key_function=xOS_KEY_FUNCTION_CALLUP,
        .key_status=1<<xOS_KEY_HANDLE_CALL_OUTING_ACTIVE,
    },
};


uint8_t xOS_Key_GroupHandle(uint8_t key_type, uint8_t value)
{

    switch(key_type){

        case  XOS_KEY_SHORT_E:
        //short key handle
        for( uint8_t i=0;i<sizeof(xos_shortkeygroup);i++){
            if( xos_shortkeygroup[i].key_status ) {


            }
        }
        break;

        //double key
        case XOS_KEY_DOUBLE_E:

        break;

        //three key handle
        case XOS_KEY_THREE_E:

        break;

        case XOS_KEY_LONG_E:

        break;
    }

    return 0;
}
