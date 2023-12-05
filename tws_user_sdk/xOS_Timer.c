//include 
#include "xOS_Timer.h"
#include "xOS_Parame_Configure.h"
#ifdef XOS_RTX_OSSDK_ENABLE
#include "rtx_lib.h"
#include "rtx_timer.h"
#endif


#define JW_SOFTWARE_TIMERNUMS 40U


typedef enum{
    xOS_Timer_Module_INIT_ID=0,
	xOS_Timer_Module_WEAR_ID,
	xOS_Timer_Module_INBOX_ID,
	xOS_Timer_Module_COVER_ID,
	//add cutomer timer id

	//end
	xOS_Timer_Module_MAX,
}xOS_Timer_Module_ID;


typedef enum{
  SOFTWARE_STATUS_INIT=0,
  SOFTWARE_STATUS_TIMEOUT,
  SOFTWARE_STATUS_STOP
}JW_SOFTWARE_STATUS_E;
typedef enum{
    JW_SOFTWARE_PERIOD_ONECE,
    JW_SOFTWARE_PERIOD_PERIOD
}JW_SOFTWARE_PERIOD_MODE;
typedef struct{
    JW_SOFTWARE_STATUS_E  status;
    uint8_t  mode;
    uint32_t match;
    uint32_t period;
    uint32_t (*timer_cb)(uint32_t argc ,uint32_t *argv);
    uint32_t argc;
    uint32_t *argv;
}jw_Software_TimerInfo;
jw_Software_TimerInfo jw_gSoftwareTimerinfo[JW_SOFTWARE_TIMERNUMS];

#ifdef XOS_RTX_OSSDK_ENABLE
static bool xOS_SDKCreate_Timer(void);
#define XOS_COMMON_BASETIME    100U
static void xos_common_timeout_timer_cb(void const *n);
osTimerDef (xOS_COMMON_TIMER_ID, xos_common_timeout_timer_cb);
static osTimerId xos_common_timer_id = NULL;
#endif
static void Software_TimerUpdate(void);



uint32_t Software_TimerGetSystick(void)
{
    return 0;
}
void Software_TimerCreate(void)
{
    int i=0;
    do{
        jw_gSoftwareTimerinfo[i].status=(JW_SOFTWARE_STATUS_E)SOFTWARE_STATUS_INIT;
        jw_gSoftwareTimerinfo[i].timer_cb=NULL;
        jw_gSoftwareTimerinfo[i].match=Software_TimerGetSystick();
    }while(i++<JW_SOFTWARE_TIMERNUMS);
}
void Software_TimerStart(uint32_t timer_id,uint8_t time_mode,uint32_t delay,  uint32_t (*timer_cb)\
    (uint32_t argc ,uint32_t *argv),uint32_t argc,uint32_t *argv)
{

	if( (timer_id>=xOS_Timer_Module_MAX) || (delay<=0) ) return ;

	if( timer_cb == NULL) return ;
	
    jw_gSoftwareTimerinfo[timer_id].period   =Software_TimerGetSystick();
    jw_gSoftwareTimerinfo[timer_id].match    =Software_TimerGetSystick()+delay;
    jw_gSoftwareTimerinfo[timer_id].timer_cb =timer_cb;
    jw_gSoftwareTimerinfo[timer_id].argv     =argv;
    jw_gSoftwareTimerinfo[timer_id].argc     =argc;
    jw_gSoftwareTimerinfo[timer_id].status   =SOFTWARE_STATUS_INIT;
	jw_gSoftwareTimerinfo[timer_id].mode     =time_mode;

	
#ifdef XOS_RTX_OSSDK_ENABLE
	if(osOK!=osTimerStart(xos_common_timer_id,XOS_COMMON_BASETIME)){

	}
#endif
}
void Software_TimerStop(uint32_t timer_id)
{
    jw_gSoftwareTimerinfo[timer_id].status=SOFTWARE_STATUS_STOP;
}
static void Software_TimerUpdate(void)
{
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
			
			if(jw_gSoftwareTimerinfo[i].timer_cb !=NULL){
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
	Software_TimerUpdate();//insert 
}

static bool xOS_SDKCreate_Timer(void)
{
	if( xos_common_timer_id == NULL ){
		xos_common_timer_id = osTimerCreate(osTimer(xOS_COMMON_TIMER_ID), osTimerOnce, NULL);
	}else{
			
	}
	return false;
}

#endif


