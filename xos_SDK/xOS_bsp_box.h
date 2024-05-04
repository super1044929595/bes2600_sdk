#ifndef __XOS_BSP_BOX_H
#define __XOS_BSP_BOX_H
#ifdef __cplusplus
	extern "C"{
#endif

#include "xOS_bsp_box.h"
#include "xOS_typedef.h"

typedef struct{
	uint8_t (*init)(uint8_t* pdata,uint16_t len);
	uint8_t (*deinit)(uint8_t* pdata,uint16_t len);
	
	uint8_t (*device_send)(uint8_t* pdata,uint16_t len);	
	uint8_t (*device_recv)(uint8_t* pdata,uint16_t len);
}xos_box_driver;


bool xos_bsp_box_init(void);

#ifdef __cplusplus
	}
#endif
#endif
