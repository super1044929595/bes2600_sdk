#ifndef __XOS_UI_SHELL_H
#define __XOS_UI_SHELL_H
#ifdef __plusplus
    extern "C"{
#endif
#include "xOS_typedef.h"
#include "cmsis_os.h"
#include "app_thread.h"
#include "app_utils.h"
#include "xOS_Timer.h"

typedef enum{
XOS_UI_WEAR_MOD,
XOS_UI_COVER_MOD,
XOS_UI_BOX_MOD,
XOS_UI_MAX_MOD,
}XOS_Shell_UI_Mod_E;

typedef bool (*xos_ui_shell_handle_typedef)(uint8_t *pdata,uint16_t len);

typedef struct{
	XOS_Shell_UI_Mod_E ui_mod;
	uint32_t           systime;
	uint8_t *pdata;
	uint16_t len;
}xOS_UI_shell_Info_t;
bool xos_ui_shell_init(void);
bool xos_ui_shell_send(XOS_Shell_UI_Mod_E mod,uint8_t *pdata,uint16_t len);

bool xos_ui_shell_register(XOS_Shell_UI_Mod_E xshell_mod,xos_ui_shell_handle_typedef xshell_app);

#ifdef __plusplus
   }
#endif
#endif
