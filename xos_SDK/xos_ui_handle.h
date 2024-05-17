#ifndef __XOS_UI_HANDLE_H
#define __XOS_UI_HANDLE_H

#ifdef __cplusplus
	extern "C"{
#endif
#include "xos_typedef.h"

bool    xos_ui_poweroff(void);
bool    xos_ui_poweron(void);

bool    xos_ui_box_state_set(uint8_t state);
uint8_t xos_ui_box_state_get(void);

bool    xos_ui_cover_state_set(uint8_t state);
uint8_t xos_ui_conver_state_get(void);

bool xos_UI_Wear_StateSet(void);
bool xos_UI_Wear_StateGet(void);


#ifdef __cplusplus
}
#endif
#endif

