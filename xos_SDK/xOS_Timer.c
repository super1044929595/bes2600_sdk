//include 
#include "xOS_Timer.h"
#include "stdint.h"
#include "stddef.h"

//debug -------------------------------------------------
#define XOS_TIMER_DEBUG_ENABLE
#ifdef  XOS_TIMER_DEBUG_ENABLE 
#define xos_timer_debug(format,...)     TRACE(3,"[xtimer]%s (%d) " format "\n",__func__,__LINE__,##__VA_ARGS__);
#else
#define xos_timer_debug(format,...) 
#endif
//end debug 

//typedef 
#define JW_SOFTWARE_TIMERNUMS 100U
typedef uint32_t (*xos_timer_callback)(uint32_t argc ,uint32_t *argv);


typedef enum{
  SOFTWARE_STATUS_INIT=0,
  SOFTWARE_STATUS_TIMEOUT,
  SOFTWARE_STATUS_STOP
}JW_SOFTWARE_STATUS_E;

typedef struct{
    JW_SOFTWARE_STATUS_E  status;
    uint8_t  mode;
    uint32_t match;
    uint32_t period;
    uint32_t (*timer_cb)(uint32_t argc ,uint32_t *argv);
	//xos_timer_callback timer_cb;
    uint32_t argc;
    uint32_t *argv;
}jw_Software_TimerInfo;
jw_Software_TimerInfo jw_gSoftwareTimerinfo[JW_SOFTWARE_TIMERNUMS];


//--------------------------------------------------------------------
#ifdef XOS_RTX_OSSDK_ENABLE
#define XOS_COMMON_BASETIME    100U
static void xos_common_timeout_timer_cb(void const *n);
osTimerDef (xOS_COMMON_TIMER_ID, xos_common_timeout_timer_cb);
static osTimerId xos_common_timer_id = NULL;
#endif
static void Software_TimerUpdate(void);
//--------------------------------------------------------------------
osMutexId xos_common_timer_mutex_id = NULL;
osMutexDef(xos_common_timer_mutex);



uint32_t Software_TimerGetSystick(void)
{
	uint32_t end_time = TICKS_TO_MS(hal_sys_timer_get());
    //uint32_t deltaMs = TICKS_TO_US(end_time - start_time);
	//xos_timer_debug( "\r\n sw jw Software_TimerGetSystick %d" ,end_time );
    return end_time;
}

void Software_TimerCreate(void)
{
    int i=0;
    do{
        jw_gSoftwareTimerinfo[i].status=(JW_SOFTWARE_STATUS_E)SOFTWARE_STATUS_INIT;
        jw_gSoftwareTimerinfo[i].timer_cb=(void*)NULL;
        jw_gSoftwareTimerinfo[i].match=Software_TimerGetSystick();
    }while(i++<JW_SOFTWARE_TIMERNUMS);
}
void Software_TimerStart(uint32_t timer_id,uint8_t time_mode,uint32_t delay,  uint32_t (*timer_cb)\
    (uint32_t argc ,uint32_t *argv),uint32_t argc,uint32_t *argv)
{
	if( (timer_id>=xOS_Timer_Module_MAX) || (delay<=0) ) return ;

	if( timer_cb == NULL) return ;
	
	delay=delay;//gain 

    jw_gSoftwareTimerinfo[timer_id].period   =Software_TimerGetSystick();
    jw_gSoftwareTimerinfo[timer_id].match    =Software_TimerGetSystick()+delay;
    jw_gSoftwareTimerinfo[timer_id].timer_cb =timer_cb;
    jw_gSoftwareTimerinfo[timer_id].argv     =argv;
    jw_gSoftwareTimerinfo[timer_id].argc     =argc;
    jw_gSoftwareTimerinfo[timer_id].status   =SOFTWARE_STATUS_INIT;
	jw_gSoftwareTimerinfo[timer_id].mode     =time_mode;

	xos_timer_debug("jw Software_TimerStart ");
	
#ifdef XOS_RTX_OSSDK_ENABLE
	if(xos_common_timer_id!=NULL){
		
		if(osTimerIsRunning(xos_common_timer_id)){
			osTimerStop(xos_common_timer_id);
		}	

		if(osOK!=osTimerStart(xos_common_timer_id,XOS_COMMON_BASETIME)){
			xos_timer_debug("jw xtimer start error ");
		}

	}
#endif
}

void Software_TimerCancel(uint32_t timer_id)
{
	if( timer_id <0 || timer_id>=xOS_Timer_Module_MAX ) return;
	
}

void Software_TimerStop(uint32_t timer_id)
{
    jw_gSoftwareTimerinfo[timer_id].status=SOFTWARE_STATUS_STOP;
}

static void Software_TimerUpdate(void)
{
	//xos_timer_debug("jw Software_TimerUpdate ");
	for(uint32_t i=0;i<JW_SOFTWARE_TIMERNUMS;i++){
		switch(jw_gSoftwareTimerinfo[i].status){

		case SOFTWARE_STATUS_INIT:
		if (jw_gSoftwareTimerinfo[i].match<Software_TimerGetSystick()) {
			jw_gSoftwareTimerinfo[i].status=SOFTWARE_STATUS_TIMEOUT;
		}
		break;
		
		case SOFTWARE_STATUS_TIMEOUT:

			jw_gSoftwareTimerinfo[i].match=Software_TimerGetSystick()+jw_gSoftwareTimerinfo[i].period;

			if (jw_gSoftwareTimerinfo[i].mode==JW_SOFTWARE_PERIOD_ONECE) {
				jw_gSoftwareTimerinfo[i].status=SOFTWARE_STATUS_STOP;
			}else {
				jw_gSoftwareTimerinfo[i].status=SOFTWARE_STATUS_INIT;
				jw_gSoftwareTimerinfo[i].match=Software_TimerGetSystick();
			}
			
			if( jw_gSoftwareTimerinfo[i].timer_cb !=NULL ){
				jw_gSoftwareTimerinfo[i].timer_cb(jw_gSoftwareTimerinfo[i].argc,jw_gSoftwareTimerinfo[i].argv);
			}
			
		break;
		
		case SOFTWARE_STATUS_STOP:
		break;

		}
	}
}


#ifdef XOS_RTX_OSSDK_ENABLE
static void xos_common_timeout_timer_cb(void const *n)
{
	//osMutexWait(xos_common_timer_id, osWaitForever);
	Software_TimerUpdate();//insert 
	//osMutexRelease(xos_common_timer_id);		
}
#endif

uint32_t xos_timer_periodcallback(uint32_t argc ,uint32_t *argv)
{
	argc=argc;
	argv=argv;
	xos_timer_debug("\r\n xos_timer_periodcallback !");	
	return 0;
}

uint32_t xos_timer_onececallback(uint32_t argc ,uint32_t *argv)
{
	argc=argc;
	argv=argv;
	xos_timer_debug("\r\n xos_timer_onececallback!");	
	return 0;
}



bool xOS_SDKCreate_Timer(void)
{
	
	Software_TimerCreate();

#ifdef XOS_RTX_OSSDK_ENABLE
    if(xos_common_timer_mutex_id == NULL)
    {
        xos_common_timer_mutex_id = osMutexCreate((osMutex(xos_common_timer_mutex)));
    }
	osMutexRelease(xos_common_timer_id);	
	
	if( xos_common_timer_id == NULL ){
		xos_common_timer_id = osTimerCreate(osTimer(xOS_COMMON_TIMER_ID), osTimerPeriodic, NULL);
	}else{
		xos_timer_debug("\r\n xOS_SDKCreate_Timer create error !");	
		return true;
	}

	//osTimerStart(xos_common_timer_id,JW_SOFTWARE_TIMERNUMS);
#endif
//eg
//	Software_TimerStart(0,JW_SOFTWARE_PERIOD_ONECE,500,xos_timer_onececallback,0,NULL);
//	Software_TimerStart(1,JW_SOFTWARE_PERIOD_PERIOD,500,xos_timer_periodcallback,0,NULL);
	xos_timer_debug("\r\n  jw xOS_SDKCreate_Timer");	
	return false;
}


 
