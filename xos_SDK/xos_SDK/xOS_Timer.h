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



bool xOS_SDKCreate_Timer(void);


#ifdef __cplusplus
}
#endif

#endif

