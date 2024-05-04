#ifndef __XOS_UI_BOX_H
#define __XOS_UI_BOX_H

#include "xOS_Parame_Configure.h"
#include "stdint.h"
#include "stddef.h"
#include "hal_trace.h"
#include "xOS_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef bool (*os_ui_handle_callback)(uint8_t *pdata,uint16_t len);

//os_ui_box handle 
os_ui_handle_callback xos_ui_box_gethandleId(void);
bool xos_ui_box_init(void);


#ifdef __cplusplus
}
#endif

#endif
