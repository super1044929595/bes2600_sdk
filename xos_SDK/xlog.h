
#ifndef  __XLOG_TYPE_H__
#define  __XLOG_TYPE_H__

#ifdef __cplusplus
	extern "C" {
#endif

#include "xOS_Parame_Configure.h"

#ifdef XOS_LOG_ENABLE
#include "string.h"
#include "stdint.h"
#include "stdio.h"
#include "types.h"
#include "app_charger.h"
//add the module error  enum 
typedef enum {
	//peripheral
	MODE_PERIPHERAL_START=0x00,
	MODE_PERIPHERAL_CONFIGURE,
	MODE_PERIPHERAL_FIRWARE_ERROR,
	MODE_PERIPHERAL_DEVICE_ERROR,
	MODE_PERIPHERAL_SENSOR_INITR_ERROR,
	MODE_PERIPHERAL_SENSOR_OPENERROR,
	MODE_PERIPHERAL_END,
	//ui module
	MODE_UI_START=0x20,
	MODE_UI_ANC_OPEN_ERROR,
	MODE_UI_ANC_CLOSE_ERROR,
	MODE_UI_SCO_ANSWER_ERROR,
	MODE_UI_SCO_REJECT_ERROR,
	MODE_UI_GOOGLE_VOICE_ERROR,
	MODE_UI_MUSIC_START_ERROR,
	MODE_UI_MUSIC_STOP_ERROR,
	MODE_UI_END,
	// os
	MODULE_OS_START=0x30,
	MODULE_OS_FLASH_INIT_ERROR,
	MODULE_OS_FLASH_WRITE_ERROR,
	MODULE_OS_FLASH_READ_ERROR,
	MODUEL_OS_END,
	//power 
	MODULE_POWER_START=0x40,
	MODULE_POWER_INIT_ERROR,
	MODULE_POWER_OVER_CURRENT_ERROR,
	MODULE_POWER_OVER_TEMPER_ERROR,
	MODULE_POWER_SEND_ERROR,
	MODULE_POWER_END,
	//bt
	MODULE_BT_START=0x50,
	MODULE_BT_END,
	//audio
	MODULE_AUDIO_START=0x60,
	MODULE_AUDIO_SET_PARAME_INIT_ERROR,
	MODULE_AUDIO_A2DP_START_ERROR,
	MODULE_AUDIO_A2DP_STOP_ERROR,
	MODULE_AUDIO_SCO_START_ERROR,
	MODULE_AUDIO_SCO_STOP_ERROR,
	MODULE_AUDIO_ARC_START_ERROR,
	MODULE_AUDIO_ARC_STOP_ERROR,
	MODULE_AUDIO_A2DP_SET_VOL_ERROR,
	MODULE_AUDIO_SCO_SET_VOL_ERROR,
	MODULE_AUDIO_LOCAL_TONE_SET_ERROR,
	MODULE_AUDIO_MUTE_SET_ERROR,
	MODULE_AUDIO_END,
}MODULE_LOG_REPORT;
//end 


//macor sdk
#define OS_DYNAMEIC_MEM_ENABLE 0
#define WQ_SDK_OS_ENABLE       1
#define COMMON_SDK_OS_ENABLE   0


//typedef 
#if COMMON_SDK_OS_ENABLE
typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;
#endif


#define XLOG_BUFFER_CHANNEL_NUMBER  2u
#define XLOG_BUFFER_LEN             1072u //1072u
#define XLOG_SET_TIMEOUT            5000u

//private
typedef enum {
	xLOG_LEVEL_DEBUG = 0,
	xLOG_LEVEL_WARING,
	xLOG_LEVEL_INFO,
	xLOG_LEVEL_ERROR,
	xLOG_LEVEL_DEBUG_LOW,
	xLOG_LEVEL_DEBUG_HIGH
}xLog_LEVEL_Enum;

typedef struct {
	uint8_t 	log_init_enable;
	uint8_t 	log_current_index;
	uint16_t	log_write_len;
	uint16_t	log_read_len;
}__attribute__((packed)) xLog_Record_Info;

//---------------message info--------------------
typedef enum {
	LOG_MODULE_START = 0,
	LOG_MODULE_BT,
	LOG_MODULE_PRESS,
	LOG_MOUDULE_WEAR,
	LOG_MOUDULE_END
}LOG_MODULE_ENUM;

typedef struct _xLog_Moudule_Info {
	LOG_MODULE_ENUM log_module;
	//date info
	uint8_t time_month;
	uint8_t time_date;
	uint8_t time_hour;
	uint8_t time_min;
	uint8_t time_sec;
	//data
	uint8_t* pdata;
	uint16_t len;
	xLog_LEVEL_Enum log_level;
	struct _xLog_Moudule_Info* prev;
	struct _xLog_Moudule_Info* next;
}__attribute__((packed)) xLog_Moudule_Info;


//public method 
typedef struct {
	LOG_MODULE_ENUM    module_id;
	uint16_t           len;
	MODULE_LOG_REPORT  error_type;
	uint8_t* pdata;
}__attribute__((packed)) Log_Common_Message;

typedef void (*xlog_module_register)(uint8_t* pdata, uint16_t len);

void offline_xlog_add(LOG_MODULE_ENUM log_id, uint8_t* pdata, uint16_t len, xLog_LEVEL_Enum xlog_level);

bool log_systimer_tickinc(void);

void offline_log_init(void);

bool log_module_sendMsg(LOG_MODULE_ENUM log_module, MODULE_LOG_REPORT error_type, uint16_t len);

bool log_module_sendDataMsg(LOG_MODULE_ENUM log_module, uint8_t *pdata, uint16_t len);

#endif
#ifdef __cplusplus
	}
#endif

#endif