#ifndef __XOS_TIMER_H
#define __XOS_TIMER_H

#ifdef __cplusplus
	extern "C"{
#endif

//#include "xOS_Parame_Configure.h"
#define XOS_RTX_OSSDK_ENABLE

#ifdef XOS_RTX_OSSDK_ENABLE
#include "hal_trace.h"
#include "app_utils.h"
#include "app_thread.h"
#include "hal_timer.h"
#include "cmsis_os.h"
#include "app_thread.h"
#include "app_utils.h"
#endif


typedef enum{
    xOS_Timer_Module_INIT_ID=0,
	xOS_Timer_Module_WEAR_ID,
	xOS_Timer_Module_INBOX_ID,
	xOS_Timer_Module_COVER_ID,
    xOS_Timer_Module_POWERCONSUMPTION_ID,
	//add cutomer timer id
	//end
	xOS_Timer_Module_MAX,
}xOS_Timer_Module_ID;

typedef enum{
    JW_SOFTWARE_PERIOD_ONECE,
    JW_SOFTWARE_PERIOD_PERIOD
}JW_SOFTWARE_PERIOD_MODE;

bool xOS_SDKCreate_Timer(void);
void Software_TimerStart(uint32_t timer_id,uint8_t time_mode,uint32_t delay,  uint32_t (*timer_cb)\
    (uint32_t argc ,uint32_t *argv),uint32_t argc,uint32_t *argv);
void Software_TimerCancel(uint32_t timer_id);
#ifdef __cplusplus
}
#endif

#endif

