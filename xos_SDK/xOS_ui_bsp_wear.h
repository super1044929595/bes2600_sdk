#ifndef __XOS_UI_BSP_WEAR_H
#define __XOS_UI_BSP_WEAR_H

#include "xOS_Parame_Configure.h"
#include "stdint.h"
#include "stddef.h"
#include "hal_trace.h"
#ifdef __cplusplus
extern "C" {
#endif

//#include "xlog.h"

typedef void (*xos_ui_module_register)(uint8_t* pdata, uint16_t len);

void app_gesture_init(void);
xos_ui_module_register app_gesture_getcallback(void);

#ifdef __cplusplus
}
#endif

#endif
