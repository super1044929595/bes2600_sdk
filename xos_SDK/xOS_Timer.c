//include 
#include "xOS_Timer.h"

#ifdef XOS_TIMER_FUNCTION_ENABLE

//debug -------------------------------------------------
#define XOS_TIMER_DEBUG_ENABLE
#ifdef  XOS_TIMER_DEBUG_ENABLE 
#define xos_timer_debug(format,...)     TRACE(5,"[ xos timer %d ] %s "format "\n",__LINE__,__func__,##__VA_ARGS__);
#else
#define xos_timer_debug(format,...) 
#endif

//typedef 
#define JW_SOFTWARE_TIMERNUMS     xOS_Timer_Module_MAX 
#define XOS_TIMER_SELFT_DEBUGx 
//end debug 
const char * xos_timer_string[2]={

	"[PER_ONECE]",
	"[PER_PERIOD]",

};

const char* xos_timer_id_string[JW_SOFTWARE_TIMERNUMS]={

	"[Timer_INIT]",
	"[Timer_User]",
	"[Timer_WEAR]",
	"[Timer_INBOX]",
	"[Timer_COVER]",
	"[Timer_POWERCONSUMPTION]",
	"[Timer_TWSVOLTABLE]",
	"[Timer_TWS_BAT]",
	"[Timer_OVERDIS]"
	
};


typedef uint32_t (*xos_timer_callback)(uint32_t argc ,uint32_t *argv);
//static bool xos_TimerSafeCheck(uint32_t curtimer);
static void Software_TimerStop(uint32_t timer_id);
#ifdef XOS_TIMER_SELFT_DEBUG
static bool xos_TimerPrintfBarprocess(uint8_t xos_index);
static bool xos_TimerRecordPrint(void);
#endif


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
#define XOS_COMMON_BASETIME    200U
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

#ifdef XOS_TIMER_SELFT_DEBUG
bool xos_TimerPrintfBarprocess(uint8_t xos_index)
{
	int i;

	if(xos_index>xOS_Timer_Module_MAX) return true;

	xos_index=(1+xos_index)%xOS_Timer_Module_MAX;
	
	const char* arr = "-\\|/";
	char buf[120];
	TRACE(0,"\n");
	if (xos_index > 100) return true;
	memset(buf, 0x00, sizeof(buf));
	buf[0] = '-';
	buf[1] = '>';
	for (i = 1; i <= xos_index; i++) {
		xos_timer_debug("\rprocess:[%-100s]%d%% %c", buf, i, arr[i % 4]);
	    //fflush(stdout);
		buf[i] = '-';
		buf[i + 1] = '>';
		//fflush(stdout);
		//sleep()
		//osdelay(1);
	}
	return false;
}

static bool xos_TimerRecordPrint(void)
{
	for(int i=0;i<JW_SOFTWARE_TIMERNUMS;i++){
		xos_timer_debug("in:%d m:%s",i,xos_timer_string[jw_gSoftwareTimerinfo[i].mode])
	}
	return false;
}

#endif


void Software_TimerStart(uint32_t timer_id,uint8_t time_mode,uint32_t delay,  uint32_t (*timer_cb)\
    (uint32_t argc ,uint32_t *argv),uint32_t argc,uint32_t *argv)
{

	if( xos_common_timer_id==NULL ) return;

	if( (timer_id>=xOS_Timer_Module_MAX) || (delay<=0) ) return ;

	if( timer_cb == NULL) return ;


#ifdef XOS_TIMER_SELFT_DEBUG
	xos_TimerPrintfBarprocess((uint8_t)timer_id);
#endif

	delay=(uint32_t)(delay-XOS_COMMON_BASETIME);//gain 

	jw_gSoftwareTimerinfo[timer_id].period   =delay;
	jw_gSoftwareTimerinfo[timer_id].match    =Software_TimerGetSystick()+delay;
	jw_gSoftwareTimerinfo[timer_id].timer_cb =timer_cb;
	jw_gSoftwareTimerinfo[timer_id].argv     =argv;
	jw_gSoftwareTimerinfo[timer_id].argc     =argc;
	jw_gSoftwareTimerinfo[timer_id].status   =SOFTWARE_STATUS_INIT;
	jw_gSoftwareTimerinfo[timer_id].mode     =time_mode;

	xos_timer_debug("timer mode %s id:%s",xos_timer_string[time_mode],xos_timer_id_string[timer_id]);
	
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

bool Software_TimerCancel(uint32_t timer_id)
{

	if( timer_id <0 || timer_id>=xOS_Timer_Module_MAX ) return true;

	Software_TimerStop(timer_id);

	return false;
	
}

void Software_TimerStop(uint32_t timer_id)
{
    jw_gSoftwareTimerinfo[timer_id].status=SOFTWARE_STATUS_STOP;
}

static void Software_TimerUpdate(void)
{
	//safe check 
	//xos_TimerSafeCheck(Software_TimerGetSystick());
	
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
	xos_timer_debug("\r\n [xos timer]xos_timer_periodcallback !");	
	return 0;
}

uint32_t xos_timer_onececallback(uint32_t argc ,uint32_t *argv)
{
	argc=argc;
	argv=argv;
	xos_timer_debug("\r\n [xos timer]xos_timer_onececallback !");	
	return 0;
}


/*
static bool xos_TimerSafeCheck(uint32_t curtimer)
{
	static uint32_t _pretime=0;
	if(	(_pretime!=0) && ((curtimer-_pretime)> (XOS_COMMON_BASETIME*2))){
			xos_timer_debug("loss time ---->[ waring ]");
			//assert handle 
			return true;
	}else{
			//xos_timer_debug("loss time ---->[ ok ]");
	}
	_pretime=curtimer;
	return false;
}*/

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
	
	TRACE(1,"xos_common_timer_id %d",(int)xos_common_timer_id);

#endif
//eg
//	Software_TimerStart(0,JW_SOFTWARE_PERIOD_ONECE,500,xos_timer_onececallback,0,NULL);
//  Software_TimerStart(1,JW_SOFTWARE_PERIOD_PERIOD,1000,xos_timer_periodcallback,0,NULL);
	xos_timer_debug("xOS_SDKCreate_Timer active max num:%d",JW_SOFTWARE_TIMERNUMS);	
	return false;
}


#endif
