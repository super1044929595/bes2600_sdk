#ifndef __XOS_UI_WEAR_H
#define __XOS_UI_WEAR_H

#include "xOS_Parame_Configure.h"
#include "stdint.h"
#include "stddef.h"
#include "hal_trace.h"
#ifdef __cplusplus
extern "C" {
#endif

//typedef 
typedef void (*xos_ui_module_register)(uint8_t* pdata, uint16_t len);
//end

void app_gesture_init(void);
xos_ui_module_register app_gesture_getcallback(void);
bool xOS_UI_Wear_Init(void);

#ifdef __cplusplus
}
#endif

#endif
