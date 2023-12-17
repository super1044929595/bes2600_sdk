#include "xOS_Key.h"
#include "stdint.h"

typedef  bool (*xOS_KeyHandle_Fucntion)(uint8_t * *pdata,uint16_t len);

//XOS Key_Configure type
typedef struct {
    xOS_KEY_BoardId            key_value;
    xOS_KEY_FunctionId         key_function;
    xOS_KEY_HandleStatus       key_status;
    xOS_KeyHandle_Fucntion     ke_handle;
}xOS_KeyType_t;

bool xOS_KeyHandle_Fucntion_Test(uint8_t** pdata, uint16_t len)
{
    printf("\r\n xOS_KeyHandle_Fucntion_Test--->");
    return false;
}


xOS_KeyType_t xos_shortkeygroup1[] = {
    {
        xOS_KEY_Board_DOWN,
        xOS_KEY_FUNCTION_CALLUP,
        (xOS_KEY_HandleStatus)(2 << xOS_KEY_HANDLE_MEDIUM_ACTIVE),
        xOS_KeyHandle_Fucntion_Test,
    },
    {
        xOS_KEY_Board_DOWN,
        xOS_KEY_FUNCTION_CALLUP,
        (xOS_KEY_HandleStatus)(2 << xOS_KEY_HANDLE_CALL_INCOMING_ACTIVE),
        NULL
    },
    {
        xOS_KEY_Board_DOWN,
        xOS_KEY_FUNCTION_CALLUP,
        (xOS_KEY_HandleStatus)(2 << xOS_KEY_HANDLE_CALL_OUTING_ACTIVE),
        NULL
    },
};

xOS_KeyType_t xos_shortkeygroup2[] = {
    {
        xOS_KEY_Board_DOWN,
        xOS_KEY_FUNCTION_CALLUP,
        (xOS_KEY_HandleStatus)(2 << xOS_KEY_HANDLE_MEDIUM_ACTIVE),
        xOS_KeyHandle_Fucntion_Test,
    },
    {
        xOS_KEY_Board_DOWN,
        xOS_KEY_FUNCTION_CALLUP,
        (xOS_KEY_HandleStatus)(2 << xOS_KEY_HANDLE_CALL_INCOMING_ACTIVE),
        NULL
    },
    {
        xOS_KEY_Board_DOWN,
        xOS_KEY_FUNCTION_CALLUP,
        (xOS_KEY_HandleStatus)(2 << xOS_KEY_HANDLE_CALL_OUTING_ACTIVE),
        NULL
    },
};

xOS_KeyType_t xos_shortkeygroup3[] = {
    {
        xOS_KEY_Board_DOWN,
        xOS_KEY_FUNCTION_CALLUP,
        (xOS_KEY_HandleStatus)(2 << xOS_KEY_HANDLE_MEDIUM_ACTIVE),
        xOS_KeyHandle_Fucntion_Test,
    },
    {
        xOS_KEY_Board_DOWN,
        xOS_KEY_FUNCTION_CALLUP,
        (xOS_KEY_HandleStatus)(2 << xOS_KEY_HANDLE_CALL_INCOMING_ACTIVE),
        NULL
    },
    {
        xOS_KEY_Board_DOWN,
        xOS_KEY_FUNCTION_CALLUP,
        (xOS_KEY_HandleStatus)(2 << xOS_KEY_HANDLE_CALL_OUTING_ACTIVE),
        NULL
    },
};

 const xOS_KeyType_t * const xosOne_ShortKey_Map[] = {
    xos_shortkeygroup1,
    xos_shortkeygroup2,
    xos_shortkeygroup3,
};



uint8_t xOS_Key_GroupHandle(uint8_t key_type, uint8_t value)
{

    switch (key_type) {

        case  XOS_KEY_SHORT_E:
            //short key handle
            if (xosOne_ShortKey_Map[value]->ke_handle) {
                // get from sdk method
                if (xosOne_ShortKey_Map[value]->key_status) { //*watch
                    xosOne_ShortKey_Map[value]->ke_handle(NULL, 0);
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

