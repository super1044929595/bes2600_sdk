#include "omni_pt.h"
#include "omni_led.h"
#include "rc_gap.h"
#include "omni_platform_common.h"

//#include "omni_pt_atts_gatt.h"
//#include "omni_pt_param_gap.h"



#define XOS_PT_DEBUG_ENABLE
#ifdef XOS_PT_DEBUG_ENABLE 
#define xos_pt_debug(format,...)    printf("OMNI PT----------- [ %s:%s (%d) ]:" format "\n" ,__FILE__,__FUNCTION__,__LINE__,##__VA_ARGS__);
#else
#define xos_pt_debug(format,...) 
#endif




//private method 
static void    customer_pt_init(uint8_t *pdata,uint16_t in_len,uint8_t *pout,uint16_t len);
static void    customer_pt_enter_dut(uint8_t *pdata,uint16_t in_len,uint8_t *pout,uint16_t len);



//private member
static bool  xCustomer_PT_Mode=false;


/********************************** PT register ********************************************/
const OMNI_PT_CMD pt_command[]={
	{OMNI_PT_INIT_NONE, "pt_init"             ,	8,	8,	customer_pt_init },
	{OMNI_PT_CHECKSELF, "pt_entern_dut_mode " ,	8,	8,	customer_pt_enter_dut},			
};
	
static void    customer_pt_init(uint8_t *pdata,uint16_t in_len,uint8_t *pout,uint16_t len)
{
	xCustomer_PT_Mode=true;
}

static void    customer_pt_enter_dut(uint8_t *pdata,uint16_t in_len,uint8_t *pout,uint16_t len)
{

}








