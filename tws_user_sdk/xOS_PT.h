#ifndef _OMNI_PT_H
#define _OMNI_PT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xOS_PT.h"
#include "stdint.h"


//pt sw version
#define SW_PT_VERSION_ID_HIGH  24
#define SW_PT_VERSION_IF_LOW   29


/*---------- BoardKey ID -----------*/
#define VK_TV_POWER  28
#define VK_STB_POWER 0  
#define VK_HOME      14
#define VK_PROFILE   5
#define VK_EPG       35
#define VK_UP        21
#define VK_LIVE_TV   7
#define VK_LEFT      36
#define VK_OK        22
#define VK_RIGHT     8
#define VK_BACK      29
#define VK_DOWN      15
#define VK_CONTEXT   1
#define VK_VOLUP     30
#define VK_VOICE     16
#define VK_CHUP      2
#define VK_VOLDOWN    37
#define VK_RECORD     23
#define VK_CHDOWN     9
#define VK_REWIND     38
#define VK_PLAYPAUSE  24
#define VK_FASTFORWARD  10
#define VK_KEY_1    31
#define VK_KEY_2    17
#define VK_KEY_3    3
#define VK_KEY_4    32 //please check it
#define VK_KEY_5    18
#define VK_KEY_6    4
#define VK_KEY_7    39
#define VK_KEY_8    25
#define VK_KEY_9    11
#define VK_MUTE     40
#define VK_KEY_0    26
#define VK_INPUT    12
#define VK_RED      34
#define VK_GREEN    41
#define VK_YELLOW   27
#define VK_BLUE     13

/*---------- BT Key ID -----------*/

#define CSM_PT_USAGE 0x000C
#define KBD_PT_USAGE 0x0007

#define CSM_PT(x) ((CSM_PT_USAGE << 16) | x)
#define KBD_PT(x) ((KBD_PT_USAGE << 16) | x)

#define VK_BT_POWER      CSM_PT(VK_TV_POWER) //
#define VK_BT_STB_POWER  CSM_PT(VK_STB_POWER)
#define VK_BT_HOME       CSM_PT(VK_HOME)
#define VK_BT_PROFILE    CSM_PT(VK_PROFILE)
#define VK_BT_EPG        CSM_PT(VK_EPG)
#define VK_BT_UP         CSM_PT(VK_UP)
#define VK_BT_LIVE_TV    CSM_PT(VK_LIVE_TV)
#define VK_BT_LEFT       CSM_PT(VK_LEFT)
#define VK_BT_OK         CSM_PT(VK_OK)
#define VK_BT_RIGHT      CSM_PT(VK_RIGHT)
#define VK_BT_BACK       CSM_PT(VK_BACK)
#define VK_BT_DOWN       CSM_PT(VK_DOWN)
#define VK_BT_CONTEXT    CSM_PT(VK_CONTEXT)
#define VK_BT_VOLUP 	 CSM_PT(VK_VOLUP)
#define VK_BT_VOICE      CSM_PT(VK_VOICE)

#define VK_BT_CHUP       CSM_PT(VK_CHUP)
#define VK_BT_VOLDOWN    CSM_PT(VK_VOLDOWN)
#define VK_BT_RECORD     CSM_PT(VK_RECORD)
#define VK_BT_CHDOWN     CSM_PT(VK_CHDOWN)
#define VK_BT_REWIND     CSM_PT(VK_REWIND)
#define VK_BT_PLAY_PAUSE    CSM_PT(VK_PLAYPAUSE)
#define VK_BT_FASTFORWARD   CSM_PT(VK_FASTFORWARD)
#define VK_BT_KEY_1      CSM_PT(VK_KEY_1)
#define VK_BT_KEY_2      CSM_PT(VK_KEY_2)
#define VK_BT_KEY_3      CSM_PT(VK_KEY_3)
#define VK_BT_KEY_4      CSM_PT(VK_KEY_4)
#define VK_BT_KEY_5      CSM_PT(VK_KEY_5)
#define VK_BT_KEY_6      CSM_PT(VK_KEY_6)
#define VK_BT_KEY_7      CSM_PT(VK_KEY_7)
#define VK_BT_KEY_8      CSM_PT(VK_KEY_8)
#define VK_BT_KEY_9      CSM_PT(VK_KEY_9)
#define VK_BT_MUTE       CSM_PT(VK_MUTE)
#define VK_BT_KEY_0      CSM_PT(VK_KEY_0)
#define VK_BT_INPUT      CSM_PT(VK_INPUT)
#define VK_BT_RED        CSM_PT(VK_RED)
#define VK_BT_GREEN      CSM_PT(VK_GREEN)
#define VK_BT_YELLOW     CSM_PT(VK_YELLOW)
#define VK_BT_BLUE       CSM_PT(VK_BLUE)



//---------------------------------------------------
typedef  void (*omni_pt_callback)(uint8_t *pdata,uint16_t in_len,uint8_t *pout,uint16_t len);
typedef  void (*omni_pt_externcallback)(void);


#define CFG_ENABLE_SVC_CHG  1
#define OMNI_FACTORY_KEY_CODE    18u
typedef enum{
	OMNI_FACTORY_NORMAL,
	OMNI_FACTORY_FTM1_ENTERN,
	OMNI_FACTORY_FTM1_EXIT,		
	OMNI_FACTORY_FTM2_ENTERN,
	OMNI_FACTORY_FTM2_EXIT,
	OMNI_FACTORY_FTM3_ENTERN,
	OMNI_FACTORY_FTM3_EXIT,
	OMNI_FACTORY_FTM4_ENTERN,
	OMNI_FACTORY_FTM4_EXIT,
	OMNI_FACTORY_FTM5_ENTERN,
	OMNI_FACTORY_FTM5_EXIT,
}OMNI_FACTORY_MODE_ENUM;

typedef struct{
	uint8_t key_value[3];
}OMNI_PT_CombkeyId;

//pt macro  
typedef enum{
	OMNI_PT_INIT_NONE,
	OMNI_PT_CHECKSELF,
	OMNI_PT_DUT,
}OMNI_PT_CMD_ENUM;
typedef struct{
	uint8_t cmdtype;
	const char * cmddes;
	uint8_t input_len;
	uint8_t output_len;
	omni_pt_callback callfunction;
}OMNI_PT_CMD;

typedef struct{
	uint8_t key_boardid;
	uint8_t key_irid;
}PT_IR_Keymap;


void omni_pt_init(void);
bool omni_pt_iswork(void);


#ifdef __cplusplus
}
#endif

#endif                                                                                                                                                                                                    
