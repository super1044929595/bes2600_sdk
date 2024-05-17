#include "xos_bsp_wear.h"


#define XOS_BSP_WEAR_DEBUG_ENABLE
#ifdef  XOS_BSP_WEAR_DEBUG_ENABLE 
#define xos_bsp_wear_debug(format,...)     TRACE(5,"[xos bsp wear %d] %s "format "\n",__LINE__,__func__,##__VA_ARGS__);
#else
#define xos_bsp_wear_debug(format,...) 
#endif

#define XOS_BSP_USER_NUM    5
#define XOS_BSP_WEAR_ID     0x64

static XOS_BSP_Wear_Driver_s xos_bsp_touch_gh2203={
	.name		=  "xos bsp drive",
	.dev_name 	= "xos GH9023",
	.probe      = (void*)0
};
	
static XOS_BSP_Wear_Driver_s *xos_bsp_wear_drive_info=&xos_bsp_touch_gh2203;


int xos_bsp_wear_regester(uint32_t *pdata,uint16_t len)
{
	xos_bsp_wear_debug(" drive regester ");
	if(xos_bsp_wear_readchipid()==XOS_BSP_WEAR_ID){
		if(!xos_bsp_wear_drive_info->probe(pdata,len)){
			xos_bsp_wear_debug("regester error!");
		}
	}
	return 0;
}

int xos_bsp_wear_readchipid(void)
{
	int chip_id=0;
	
	return chip_id;
}

int xos_bsp_wear_power_on(uint32_t *pdata,uint16_t len)
{
	if((xos_bsp_wear_drive_info->poweron!=NULL)&&(!xos_bsp_wear_drive_info->poweron(pdata,len))){
		xos_bsp_wear_debug("power on error!");
	}
	return 0;
}

int xos_bsp_wear_power_down(uint32_t *pdata,uint16_t len)
{
	if((xos_bsp_wear_drive_info->shutdown!=NULL)&&(!xos_bsp_wear_drive_info->shutdown(pdata,len))){
		xos_bsp_wear_debug("power down error!");
	}
	return 0;
}

int xos_bsp_wear_unregester(uint32_t *pdata,uint16_t len)
{
	return 0;
}

int xos_bsp_wear_send(uint32_t *pdata,uint16_t len)
{
	return 0; 
}
int xos_bsp_wear_receive(uint32_t *pdata,uint16_t len)
{
	return 0;
}

int xos_bsp_wear_suspend(uint32_t *pdata,uint16_t len)
{
	if((xos_bsp_wear_drive_info->suspend!=NULL)&&(!xos_bsp_wear_drive_info->suspend(pdata,len))){
		xos_bsp_wear_debug(" suspend  error! ");
    }
	return 0;
}

int xos_bsp_wear_resume(uint32_t *pdata,uint16_t len)
{
	if((xos_bsp_wear_drive_info->resume!=NULL)&&(!xos_bsp_wear_drive_info->resume(pdata,len))){
		xos_bsp_wear_debug(" resume  error! ");
	}
	return 0;
}

int xos_bsp_wear_callback(uint32_t *pdata,uint16_t len)
{
	if((xos_bsp_wear_drive_info->callback !=NULL)&&(!xos_bsp_wear_drive_info->callback(pdata,len))){
		xos_bsp_wear_debug(" resume  error! ");
	}
	return 0;
}

