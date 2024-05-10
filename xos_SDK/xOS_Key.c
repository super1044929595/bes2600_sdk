#include "xOS_key.h"
#include "xos_systeminfo.h"

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
    TRACE(0,"\r\n xOS_KeyHandle_Fucntion_Test--->");
    return false;
}

//---------------------------------------------------oneshot key 
xOS_KeyType_t xos_shortkeygroup_Left[] = {
    {
        xOS_KEY_Board_DOWN,
        xOS_KEY_FUNCTION_CALLUP,
        (xOS_KEY_HandleStatus)(2 << xOS_KEY_HANDLE_MEDIUM_ACTIVE),
        xOS_KeyHandle_Fucntion_Test,
    },
    {
        xOS_KEY_Board_DOWN,
        xOS_KEY_FUNCTION_ANC,
        (xOS_KEY_HandleStatus)(2 << xOS_KEY_HANDLE_CALL_INCOMING_ACTIVE),
        NULL
    },
    {
        xOS_KEY_Board_DOWN,
        xOS_KEY_FUNCTION_PLAYANDPAUSE,
        (xOS_KEY_HandleStatus)(2 << xOS_KEY_HANDLE_CALL_OUTING_ACTIVE),
        NULL
    },
};

xOS_KeyType_t xos_shortkeygroup_Right[] = {
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


//---------------------------------------------------double key 
xOS_KeyType_t xos_doublekeygroup_Left[] = {
    {
        xOS_KEY_Board_DOWN,
        xOS_KEY_FUNCTION_CALLUP,
        (xOS_KEY_HandleStatus)(2 << xOS_KEY_HANDLE_MEDIUM_ACTIVE),
        xOS_KeyHandle_Fucntion_Test,
    },
    {
        xOS_KEY_Board_DOWN,
        xOS_KEY_FUNCTION_PLAYANDPAUSE,
        (xOS_KEY_HandleStatus)(2 << xOS_KEY_HANDLE_CALL_INCOMING_ACTIVE),
        NULL
    },
    {
        xOS_KEY_Board_DOWN,
        xOS_KEY_FUNCTION_PLAYANDPAUSE,
        (xOS_KEY_HandleStatus)(2 << xOS_KEY_HANDLE_CALL_OUTING_ACTIVE),
        NULL
    },
};

 xOS_KeyType_t xos_doublekeygroup_Right[] = {
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




//--------------------------one shot key map
const xOS_KeyType_t * const xos_One_Shot_Key_Map[] = {
	xos_shortkeygroup_Left,
	xos_shortkeygroup_Right,
};

//--------------------------double key map
const xOS_KeyType_t * const xos_double_Key_Map[] = {
	xos_doublekeygroup_Left,
	xos_doublekeygroup_Right,
};

bool xos_KeyStatusSet(uint8_t tws_side,uint8_t key_type,uint8_t index,xOS_KEY_HandleStatus key_status)
{
	uint8_t _key_short_size=(uint8_t)sizeof(xos_shortkeygroup_Left)/sizeof(xos_shortkeygroup_Left[0]);

	if(key_type==XOS_KEY_SHORT_E){		
		
		if(index>=_key_short_size)	return true;

		if(tws_side==XOS_SYS_CHANNEL_LEFT){
			xos_shortkeygroup_Left[index].key_status=key_status;
		}else{
			xos_shortkeygroup_Right[index].key_status=key_status;
		}
		
	}else if(key_type==XOS_KEY_DOUBLE_E){

	}else if(key_type==XOS_KEY_THREE_E){

	}else if(key_type==XOS_KEY_LONG_E){

	}else{

	}
	return false;
}


// It will be call by the button when user press
uint8_t xOS_Key_GroupHandle(uint8_t key_type, uint8_t value)
{
	uint8_t _key_short_size=(uint8_t)sizeof(xos_shortkeygroup_Left)/sizeof(xos_shortkeygroup_Left[0]);

    TRACE(2,"xOS_Key_GroupHandle key_type:%d value:%d",key_type,value);
	
    switch (key_type) {

        case  XOS_KEY_SHORT_E:
            //short key handle			
			for(int i=0;i<_key_short_size;i++){
				if((xos_One_Shot_Key_Map[key_type][i].key_status)&&(xos_One_Shot_Key_Map[key_type][i].ke_handle)){
						xos_One_Shot_Key_Map[key_type][i].ke_handle(NULL,0);
				}
			}
            break;

            //double key
            case XOS_KEY_DOUBLE_E:
			if (xos_One_Shot_Key_Map[value]->ke_handle) {
			// get from sdk method
				if (xos_One_Shot_Key_Map[value]->key_status) { //*watch
					xos_One_Shot_Key_Map[value]->ke_handle(NULL, 0);
				}
			}
            break;

            //three key handle
            case XOS_KEY_THREE_E:

            break;

            case XOS_KEY_LONG_E:

            break;
        }

        return 0;
}
