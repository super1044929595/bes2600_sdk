#ifndef __XOS_INTTERRUPT_HANDLER_H
#define __XOS_INTTERRUPT_HANDLER_H

#ifdef __cplusplus
	extern "C"{
#endif
#include "xos_typedef.h"

typedef struct{
struct taskintterrupt_t *next;
uint8_t  xos_intterrupt_state;
uint32_t xos_intterrupt_cnt;
void (*func)(uint8_t *pdate,uint16_t len);
uint8_t func_parame;
}taskintterrupt_t;

 typedef struct{
	taskintterrupt_t *list;
 }xos_taskTableHead_t;



#ifdef __cplusplus
	}
#endif

#endif
