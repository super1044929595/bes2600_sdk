#include "xlog.h"

#ifdef XOS_LOG_ENABLE

#include "xlog_flash.h"
#include "app_lc_spp.h"
#include "os_mem.h"
#include "os_task.h"
#include "os_queue.h"
#include "os_utils.h"
#include "os_timer.h"
#include "types.h"
#include "userapp_dbglog.h"
#include "os_lock.h"
#include "testcallback.h"

typedef struct{
	uint8_t time_month;
	uint8_t time_date;
	uint8_t time_hour;
	uint8_t time_min;
	uint8_t time_sec;
}xlog_timerinfo;


//DEBUD_macro
#define XLOG_DEBUG(fmt, arg...)          (DBGLOG_INFO(IOT_SENSOR_HUB_MANAGER_MID, fmt, ##arg))
#define XLOG_DEBUG_ERROR(fmt, arg...)    (DBGLOG_ERROR(IOT_SENSOR_HUB_MANAGER_MID, fmt, ##arg))

#if __riscv_xlen == 64
	#define portMAX_DELAY 			( uint32_t ) 0xffffffffffffffffUL
	#define portPOINTER_SIZE_TYPE 	uint64_t
#elif __riscv_xlen == 32
	#define portMAX_DELAY ( uint32_t ) 0xffffffffUL
#else
	#error Assembler did not define __riscv_xlen
#endif


static xLog_Record_Info     xlog_record_info;
static xLog_Moudule_Info    *xlog_head = NULL;
#if WQ_SDK_OS_ENABLE
static os_queue_h           xlog_queue = 0;
static os_mutex_h           xlog_mutex = 0;
static timer_id_t           xlog_debug_timer = 0;
#endif
static uint32_t xlog_buffer_offerset = 0;
static uint8_t xLog_Buffer[2][XLOG_BUFFER_LEN] = { 0 };
static uint16_t	xlog_flash_readlen = 0;
static void offline_handle(void* arg);
static char* offline_log_cloer(char* log_data, xLog_LEVEL_Enum log_level);
static void xlog_printForward(void);
static void xlog_printBackward(void);
static void xlog_timert_init(void);
static void xlog_readflash(uint8_t index);
static void log_module_parsehandle(LOG_MODULE_ENUM log_module, uint8_t* pdata, uint16_t len);
static void xlog_write_flash(uint8_t index);
static bool log_get_systimer(xlog_timerinfo *time);
static bool log_set_systimer(xlog_timerinfo *time);
static xlog_module_register  _xlog_module_callback = NULL;
static xlog_timerinfo        glog_timerinfo={0};

#if WQ_SDK_OS_ENABLE
static void offline_handle(void* arg)
{
	UNUSED(arg);
	Log_Common_Message msg;
	while (1)
	{
		if (os_queue_receive_timeout(xlog_queue, &msg, portMAX_DELAY)) {
			
			if(msg.error_type>0 && msg.pdata==NULL){
				offline_xlog_add(msg.module_id, (uint8_t*)&msg.error_type, msg.len, xLOG_LEVEL_INFO);
				//XLOG_DEBUG("xLOG---->jw offline_handle [id:%d],[len:%d] [error :%d]",msg.module_id,msg.len,msg.error_type);
			}
			if(msg.error_type==0 && msg.pdata!=NULL){
               offline_xlog_add(msg.module_id, (uint8_t*)msg.pdata, msg.len, xLOG_LEVEL_INFO);
			   os_mem_free(msg.pdata);
			   //XLOG_DEBUG("xLOG---->jw offline_handle [id:%d],[len:%d] [pdata0 :%d]",msg.module_id,msg.len,msg.pdata[0]);
			}
			//offline_log_cloer("huajsa log ====>",xLOG_LEVEL_DEBUG);	
		}
	}
}
#endif

void offline_log_init(void)
{
	//parames 
#if WQ_SDK_OS_ENABLE
	XLOG_DEBUG("xlog init start");
#else
	printf("xlog init start");
#endif
	if (xlog_record_info.log_init_enable == 1) return;
	xlog_record_info.log_init_enable = (uint8_t)0;
	xlog_record_info.log_write_len = xlog_record_info.log_read_len = 0;
	xlog_record_info.log_current_index = 0;
	xlog_buffer_offerset = 0;
#if WQ_SDK_OS_ENABLE
	//timer init
	os_create_task_ext(offline_handle, NULL, 5, 1024, "offline_log_taskhandle");
	//queue 
	xlog_queue = os_queue_create(IOT_VENDOR_MESSAGE_MID,64,sizeof(xLog_Moudule_Info));
	assert(xlog_queue);
	xlog_mutex = os_create_mutex(IOT_APP_DEMO_MID);
	assert(xlog_mutex);
	if (xlog_mutex == NULL) {
		//error
	}
	if (1) {
		xlog_timert_init();
	}
	XLOG_DEBUG("xlog init ok");
#else
	printf("xlog init ok");
#endif

}



#if 1
static char* offline_log_cloer(char* log_data, xLog_LEVEL_Enum log_level)
{
	static char log_buf_with_coler[126];
	switch (log_level)
	{
	case xLOG_LEVEL_DEBUG:
		sprintf(log_buf_with_coler, "\r\n xLOG_LEVEL_DEBUG-->:\033[31m%s\033[0m", log_data);
		break;
	case xLOG_LEVEL_WARING:
		sprintf(log_buf_with_coler, "\r\n xLOG_LEVEL_WARING-->:\033[32m%s\033[0m", log_data);
		break;
	case xLOG_LEVEL_INFO:
		sprintf(log_buf_with_coler, "\r\n xLOG_LEVEL_INFO-->:\033[33m%s\033[0m", log_data);
		break;
	case xLOG_LEVEL_ERROR:
		sprintf(log_buf_with_coler, "\r\n xLOG_LEVEL_ERROR-->:\033[34m%s\033[0m", log_data);
		break;
	case xLOG_LEVEL_DEBUG_HIGH:
		sprintf(log_buf_with_coler, "\r\n xLOG_LEVEL_DEBUG_HIGH-->:\033[35m%s\033[0m\r\n", log_data);
		break;
	case xLOG_LEVEL_DEBUG_LOW:
		sprintf(log_buf_with_coler, "\r\n xLOG_LEVEL_DEBUG_LOW-->:\033[36m%s\033[0m\r\n", log_data);
		break;
	default:
		sprintf(log_buf_with_coler, "%s", log_data);
		break;
	}
	//printf(log_buf_with_coler);
#if WQ_SDK_OS_ENABLE
	XLOG_DEBUG("xlog_parse: --->%s", log_data);
#else
	printf("\r\n %s", log_data);
#endif
	return log_buf_with_coler;
}
#endif

static uint8_t xlog_sec = 0;

static uint32_t mallocsize=0;
static uint32_t freesize=0;
void offline_xlog_add(LOG_MODULE_ENUM log_id, uint8_t* pdata, uint16_t len, xLog_LEVEL_Enum xlog_level)
{
	uint16_t left_len = 0;
	uint8_t* src_data = NULL;
	xlog_timerinfo timeinfo={0};
	os_acquire_mutex(xlog_mutex);
	if ((xlog_record_info.log_write_len + len) > XLOG_BUFFER_LEN) return;
	if (xlog_record_info.log_init_enable == 0) {
		xlog_record_info.log_init_enable = 1;
		xlog_sec = 0;
		offline_log_init();
	}
	xlog_sec++;
#if WQ_SDK_OS_ENABLE
	xLog_Moudule_Info* newlog = (xLog_Moudule_Info*)os_mem_malloc(IOT_APP_MID, sizeof(xLog_Moudule_Info));
#elif(COMMON_SDK_OS_ENABLE)
	xLog_Moudule_Info* newlog = (xLog_Moudule_Info*)malloc(sizeof(xLog_Moudule_Info));

#endif
	memset((char*)newlog, 0, sizeof(xLog_Moudule_Info));
#if WQ_SDK_OS_ENABLE
	src_data = (uint8_t*)os_mem_malloc(IOT_APP_MID, len);
#elif (COMMON_SDK_OS_ENABLE)
	src_data = (uint8_t*)malloc(len);
#endif
	mallocsize+= sizeof(xLog_Moudule_Info)+len;
	memcpy((char*)src_data, (char*)pdata, len);
	newlog->time_sec = xlog_sec;
	newlog->log_module = (LOG_MODULE_ENUM)log_id;
	newlog->next = (xLog_Moudule_Info*)NULL;
	newlog->log_level = (xLog_LEVEL_Enum)xlog_level;
	newlog->len = (uint16_t)len;
	newlog->pdata = src_data;
	newlog->next = NULL;
	log_get_systimer(&timeinfo);
	//get systime time

	if (xlog_head == (xLog_Moudule_Info*)NULL) {
		newlog->prev = (xLog_Moudule_Info*)NULL;
		xlog_head = newlog;
		xlog_record_info.log_write_len += (uint16_t)newlog->len + sizeof(xLog_Moudule_Info);
		left_len = (uint16_t)(XLOG_BUFFER_LEN - xlog_record_info.log_write_len);
#if WQ_SDK_OS_ENABLE

#else
		//XLOG_DEBUG("\r\n xlog [page: %4d ,index: %4d]----len:%4d, left_len :%4d", xlog_record_info.log_current_index, newlog->time_sec,newlog->len,left_len);
#endif
	    os_release_mutex(xlog_mutex);
		return;
	}
	xLog_Moudule_Info* last = (xLog_Moudule_Info*)xlog_head;
	while (last->next != (xLog_Moudule_Info*)NULL ) {
		last = last->next;
	}
	last->next = newlog;
	xlog_record_info.log_write_len += (uint16_t)newlog->len + sizeof(xLog_Moudule_Info);
	left_len = (uint16_t)(XLOG_BUFFER_LEN - xlog_record_info.log_write_len);
	//XLOG_DEBUG("\r\n xlog [page: %4d ,index: %4d]----len:%4d, left_len :%4d-----jwpdata:%d", xlog_record_info.log_current_index, newlog->time_sec, newlog->len, left_len,newlog->pdata[0]);
	if (left_len <= (sizeof(xLog_Moudule_Info) + newlog->len)) {
		xlog_record_info.log_write_len = (uint16_t)0;
		left_len = XLOG_BUFFER_LEN;
		xlog_printForward();
		xlog_head = NULL;
		xlog_record_info.log_current_index = xlog_record_info.log_current_index == 0 ? 1 : 0;
	}
	//XLOG_DEBUG("\r\n xlog-->page:%d,total:%d",page_index,total_len);
	os_release_mutex(xlog_mutex);

}

static void xlog_printForward(void)
{
	char printf_data[1072] = { 0 };
	uint16_t left_len = 0;
	uint16_t offset_set = 0;
	XLOG_DEBUG("\r\n|----------------------------xlog_kv_start----------------------------|");
	xLog_Moudule_Info* log_info = (xLog_Moudule_Info*)xlog_head;
	while ( log_info != (xLog_Moudule_Info*)NULL){
		left_len += (uint16_t)(log_info->len + sizeof(xLog_Moudule_Info));
		sprintf(printf_data, "\r\n time:%4d:%d:%4d--xlog module:[%4d],len:[%4d],left_len[:%4d]", log_info->time_hour, log_info->time_min, log_info->time_sec, log_info->log_module, \
			log_info->len, \
			(XLOG_BUFFER_LEN - left_len));
		offline_log_cloer((char*)printf_data, (xLog_LEVEL_Enum)log_info->log_level);
		memcpy((uint8_t*)&xLog_Buffer[xlog_record_info.log_current_index][offset_set], (uint8_t*)log_info, sizeof(xLog_Moudule_Info));
		memcpy((uint8_t*)&xLog_Buffer[xlog_record_info.log_current_index][offset_set + sizeof(xLog_Moudule_Info)], (uint8_t*)log_info->pdata, log_info->len);
		//XLOG_DEBUG("\r\n |-------xlog_printForward pdata :%d,%d,%d",log_info->pdata[0],log_info->pdata[1],log_info->pdata[2]);
		offset_set += (uint16_t)(log_info->len + sizeof(xLog_Moudule_Info));
#if OS_DYNAMEIC_MEM_ENABLE
#if WQ_SDK_OS_ENABLE
		XLOG_DEBUG("\r\n |-------");
		XLOG_DEBUG(printf_data);
#else
		printf("\r\n|------");
		printf(printf_data);
#endif
#endif
		log_info = (xLog_Moudule_Info*)log_info->next;
	}
	xlog_printBackward();
	XLOG_DEBUG("\r\n|----------------------------xlog_kv_end----------------------------|\r\n\n\n\n\n");
	//add flash 
	//xlog_readflash(xlog_record_info.log_current_index);
	xlog_write_flash(xlog_record_info.log_current_index);
}

static void xlog_printBackward(void)
{
	xLog_Moudule_Info* node = xlog_head;
	while(node!=NULL){
		freesize +=sizeof(xLog_Moudule_Info)+node->len;
		os_mem_free((void*)node->pdata);
		os_mem_free((void*)node);
		node=node->next;
	}
	if(freesize==mallocsize){
		XLOG_DEBUG("\r\n ---xlog-- parse size ok ");
	}else{
		XLOG_DEBUG("\r\n ---xlog-- parse size error ");
	}
}


//log handle by user 
static void log_module_parsehandle(LOG_MODULE_ENUM log_module, uint8_t* pdata, uint16_t len)
{
	
	if(pdata==NULL || len<=0) return ;

	switch (log_module) {
	case LOG_MODULE_BT:
		//handle bt log  callback 

		break;
	case LOG_MODULE_PRESS:
		_xlog_module_callback = app_gesture_getcallback();
		if(_xlog_module_callback!=NULL){
			_xlog_module_callback(pdata,len);
		}
		break;
	default:
		break;
	}

}


void xlog_write_flash(uint8_t index)
{
	uint8_t *psrc=&xLog_Buffer[index][0];	
	//if(app_charger_is_in_box()){
		xlog__custom_data_erase();
		xlog__custom_data_write(0,&xLog_Buffer[index][0],XLOG_BUFFER_LEN);
		xlog_custom_data_get(0,&psrc);
		memset(&xLog_Buffer[index][0],0,XLOG_BUFFER_LEN);
		memcpy((uint8_t*)&xLog_Buffer[index][0],psrc,XLOG_BUFFER_LEN);
		//handle
		xlog_readflash(index);
	//}
}



void xlog_readflash(uint8_t index)
{
	char printf_data[1072] = { 0 };
	uint16_t left_len = 0;
	uint16_t data_offerset = 0;
	uint16_t flash_offerset = 0;
	xLog_Moudule_Info* log_info = (xLog_Moudule_Info*)&xLog_Buffer[index][0];

	XLOG_DEBUG("\r\n\n\n\n\n|++++++++++++++++++++++++++++++++++++++++++++FLASH START++++++++++++++++++++++++++++++++++++++++++++|");

	while (log_info != (xLog_Moudule_Info*)NULL && ((log_info->log_module < LOG_MOUDULE_END)&&(log_info->log_module > 0))) { //||
		left_len += (uint16_t)(log_info->len + sizeof(xLog_Moudule_Info));
		if (left_len >= XLOG_BUFFER_LEN) break;
		sprintf(printf_data, "\r\n [xlog readflash--index:%d->:]time:%4d:%d:%4d--xlog module:[%4d],len:[%4d],[offerset:%4d],left_len[:%4d]", index, log_info->time_hour, log_info->time_min, log_info->time_sec, log_info->log_module, \
			log_info->len, left_len, (XLOG_BUFFER_LEN - left_len));
		XLOG_DEBUG(printf_data);
		#if 0//  find the context of each module when usd the mobile 
		//lc_send_data((uint8_t*)printf_data,strlen(printf_data)); //add spp protocol 
		#endif
		if (&xLog_Buffer[index][left_len] != NULL) {
			log_info = (xLog_Moudule_Info*)&xLog_Buffer[index][left_len];
			log_module_parsehandle(log_info->log_module,&xLog_Buffer[index][left_len+sizeof(xLog_Moudule_Info)],log_info->len);
		}
		xlog_flash_readlen += log_info->len;
	}
	memset((uint8_t*)&xLog_Buffer[index][0], 0, XLOG_BUFFER_LEN);

	XLOG_DEBUG("\r\n|++++++++++++++++++++++++++++++++++++++++++++ FLASH END ++++++++++++++++++++++++++++++++++++++++++++|\r\n\n\n\n");

	XLOG_DEBUG("\r\n xlog_read_flash %d ,data_offerset :%d ,xlog_flash_readlen", xlog_flash_readlen,data_offerset,flash_offerset);

}



#if WQ_SDK_OS_ENABLE
static void xlog_debug_timeout_handler(timer_id_t timer_id, void* param)
{
	UNUSED(timer_id);
	UNUSED(param);	
	uint8_t report=MODULE_AUDIO_SCO_STOP_ERROR;
	log_module_sendDataMsg((LOG_MODULE_ENUM)LOG_MODULE_PRESS,&report,1);
}

static void xlog_timert_init(void)
{
	xlog_timerinfo time;
	xlog_debug_timer = os_create_timer(LIB_KEYMGMT_MID, true, xlog_debug_timeout_handler, NULL);
	os_start_timer(xlog_debug_timer, 1000);
	log_set_systimer(&time); //get from mobile phone 
}
#endif



bool log_module_sendMsg(LOG_MODULE_ENUM log_module, MODULE_LOG_REPORT error_type, uint16_t len)
{
	//only test 
#if WQ_SDK_OS_ENABLE
	Log_Common_Message msg;
	if (xlog_queue != NULL) {
		msg.module_id = log_module;
		msg.len = len;
		msg.error_type = error_type;
		msg.pdata =NULL;
		os_queue_send(xlog_queue, &msg);
		return false;
	}
#endif
	return true;
}

bool log_module_sendDataMsg(LOG_MODULE_ENUM log_module, uint8_t *pdata, uint16_t len)
{
	//only test 
	uint8_t *pdst=(uint8_t*)os_mem_malloc(IOT_APP_MID,len);
	memset(pdst,0,len);
	memcpy((char*)pdst,pdata,len);

#if WQ_SDK_OS_ENABLE
	Log_Common_Message msg;
	if (xlog_queue != NULL) {
		msg.module_id = log_module;
		msg.len = len;
		msg.pdata =pdst;
		msg.error_type=0;
		os_queue_send(xlog_queue, &msg);		
		return false;
	}
#endif
	return true;
}





 
static bool log_get_systimer(xlog_timerinfo *time)
{
	UNUSED(time);
	return false;
}

static bool log_set_systimer(xlog_timerinfo *time)
{
	UNUSED(time);
	return false;
}

// call by used  
bool log_systimer_tickinc(void)
{
	if(++glog_timerinfo.time_sec>=60){
		glog_timerinfo.time_sec=0;
		if(++glog_timerinfo.time_min>=60){
			glog_timerinfo.time_min=0;
			if(++glog_timerinfo.time_hour>=24){
				glog_timerinfo.time_hour=0;
			}
		}
	}
	return false;
}
#endif