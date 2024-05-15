#ifndef __XOS_BSP_WEAR_H
#define __XOS_BSP_WEAR_H

#ifdef __cplusplus
	extern "C"{
#endif

#include "xos_typedef.h"

typedef struct{

	const char *name;
	const char *dev_name;

	int (*match)(void);
	int (*uevent)(void);
	int (*probe)(uint32_t *pdata,uint16_t len);
	int (*remove)(uint32_t *pdata,uint16_t len);

	int (*readchipid)(void);
	int (*poweron)(uint32_t *pdata,uint16_t len);
	int (*shutdown)(uint32_t *pdata,uint16_t len);

	int (*online)(uint32_t *pdata,uint16_t len);
	int (*offline)(uint32_t *pdata,uint16_t len);

	int (*suspend)(uint32_t *pdata,uint16_t len);
	int (*resume)(uint32_t *pdata,uint16_t len);

	int (*callback)(uint32_t *pdata,uint16_t len);

}XOS_BSP_Wear_Driver_s;

int xos_bsp_wear_regester(uint32_t *pdata,uint16_t len);
int xos_bsp_wear_readchipid(void);
int xos_bsp_wear_power_on(uint32_t *pdata,uint16_t len);
int xos_bsp_wear_power_down(uint32_t *pdata,uint16_t len);
int xos_bsp_wear_unregester(uint32_t *pdata,uint16_t len);
int xos_bsp_wear_send(uint32_t *pdata,uint16_t len);
int xos_bsp_wear_receive(uint32_t *pdata,uint16_t len);
int xos_bsp_wear_suspend(uint32_t *pdata,uint16_t len);
int xos_bsp_wear_resume(uint32_t *pdata,uint16_t len);
int xos_bsp_wear_callback(uint32_t *pdata,uint16_t len);


#ifdef __cplusplus
	}
#endif

#endif
