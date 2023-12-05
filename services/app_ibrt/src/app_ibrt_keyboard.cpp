/***************************************************************************
 *
 * Copyright 2015-2019 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#include "cmsis_os.h"
#include <string.h>
#include "app_ibrt_keyboard.h"
#include "app_tws_ibrt_trace.h"
#include "app_ibrt_if.h"
#include "app_ibrt_ui_test.h"
#include "app_tws_ibrt_cmd_handler.h"
#include "app_tws_ctrl_thread.h"
#include "btapp.h"
#include "apps.h"

#if defined(IBRT)

#ifdef TILE_DATAPATH
extern "C" void app_tile_key_handler(APP_KEY_STATUS *status, void *param);
#endif

#ifdef SUPPORT_SIRI
//extern int app_hfp_siri_report();
extern int app_hfp_siri_voice(bool en);
extern int open_siri_flag;
#endif

#ifdef IBRT_SEARCH_UI
extern struct BT_DEVICE_T  app_bt_device;
extern void app_otaMode_enter(APP_KEY_STATUS *status, void *param);

void app_ibrt_search_ui_handle_key(APP_KEY_STATUS *status, void *param)
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    TRACE(3,"%s %d,%d",__func__, status->code, status->event);
    switch(status->event)
    {
        case APP_KEY_EVENT_CLICK:
#if 0
            app_ibrt_ui_tws_switch();
#else
            bt_key_handle_func_click();
#endif
            break;

        case APP_KEY_EVENT_DOUBLECLICK:
            TRACE(0,"double kill");
            if(IBRT_UNKNOW==p_ibrt_ctrl->current_role)
                app_start_tws_serching_direactly();
            else
               bt_key_handle_func_doubleclick();
            break;

        case APP_KEY_EVENT_LONGPRESS:
// dont use this key for customer release due to
// it is auto triggered by circuit of 3s high-level voltage.
#if 0
            app_ibrt_ui_judge_scan_type(IBRT_FREEMAN_PAIR_TRIGGER,NO_LINK_TYPE, IBRT_UI_NO_ERROR);
            app_ibrt_ui_set_freeman_enable();
#endif
            break;

        case APP_KEY_EVENT_TRIPLECLICK:
#ifdef TILE_DATAPATH
                app_tile_key_handler(status,NULL);
#else
            app_otaMode_enter(NULL,NULL);
#endif

            break;
        case HAL_KEY_EVENT_LONGLONGPRESS:
            TRACE(0,"long long press");
            app_shutdown();
            break;

        case APP_KEY_EVENT_ULTRACLICK:
            TRACE(0,"ultra kill");
            break;

        case APP_KEY_EVENT_RAMPAGECLICK:
            TRACE(0,"rampage kill!you are crazy!");
            break;

        case APP_KEY_EVENT_UP:
            break;
    }

#ifdef TILE_DATAPATH
    if(APP_KEY_CODE_TILE == status->code)
        app_tile_key_handler(status,NULL);
#endif

}
#endif

void app_ibrt_normal_ui_handle_key(APP_KEY_STATUS *status, void *param)
{
    switch(status->event)
    {
        case APP_KEY_EVENT_CLICK:
#if 0
            app_ibrt_ui_tws_switch();
#else
            bt_key_handle_func_click();
#endif
            break;

        case APP_KEY_EVENT_DOUBLECLICK:
            TRACE(0,"double kill");
            app_ibrt_if_enter_freeman_pairing();
            break;

        case APP_KEY_EVENT_LONGPRESS:
            app_ibrt_if_enter_pairing_after_tws_connected();
            break;

        case APP_KEY_EVENT_TRIPLECLICK:
              TRACE(0,"triple press");
#ifdef TILE_DATAPATH
            app_tile_key_handler(status,NULL);
#endif
            break;

        case HAL_KEY_EVENT_LONGLONGPRESS:
            TRACE(0,"long long press");
            app_shutdown();
            break;

        case APP_KEY_EVENT_ULTRACLICK:
            TRACE(0,"ultra kill");
            break;

        case APP_KEY_EVENT_RAMPAGECLICK:
            TRACE(0,"rampage kill!you are crazy!");
            break;

        case APP_KEY_EVENT_UP:
            break;
    }
#ifdef TILE_DATAPATH
    if(APP_KEY_CODE_TILE == status->code)
        app_tile_key_handler(status,NULL);
#endif
}

int app_ibrt_if_keyboard_notify(APP_KEY_STATUS *status, void *param)
{
    if (app_tws_ibrt_slave_ibrt_link_connected())
    {
        tws_ctrl_send_cmd(APP_TWS_CMD_KEYBOARD_REQUEST, (uint8_t *)status, sizeof(APP_KEY_STATUS));
    }
    return 0;
}

void app_ibrt_send_keyboard_request(uint8_t *p_buff, uint16_t length)
{
    if (app_tws_ibrt_slave_ibrt_link_connected())
    {
        app_ibrt_send_cmd_without_rsp(APP_TWS_CMD_KEYBOARD_REQUEST, p_buff, length);
    }
}

void app_ibrt_keyboard_request_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    if (app_tws_ibrt_mobile_link_connected())
    {
#ifdef IBRT_SEARCH_UI
        app_ibrt_search_ui_handle_key((APP_KEY_STATUS *)p_buff, NULL);
#else
        app_ibrt_normal_ui_handle_key((APP_KEY_STATUS *)p_buff, NULL);
#endif
#ifdef __AI_VOICE__
        app_ibrt_ui_test_voice_assistant_key((APP_KEY_STATUS *)p_buff, NULL);
#endif
    }
}

void app_ibrt_if_start_user_action(uint8_t *p_buff, uint16_t length)
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    TRACE(3,"%s role %d code %d",__func__, p_ibrt_ctrl->current_role, p_buff[0]);

    if (IBRT_SLAVE == p_ibrt_ctrl->current_role)
    {
        app_ibrt_ui_send_user_action(p_buff, length);
    }
    else
    {
        app_ibrt_ui_perform_user_action(p_buff, length);
    }
}


void app_ibrt_ui_perform_user_action(uint8_t *p_buff, uint16_t length)
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    if (IBRT_MASTER != p_ibrt_ctrl->current_role)
    {
        TRACE(2,"%s not ibrt master, skip cmd %02x\n", __func__, p_buff[0]);
        return;
    }

    switch (p_buff[0])
    {
        case IBRT_ACTION_PLAY:
            a2dp_handleKey(AVRCP_KEY_PLAY_INTERNAL);
            break;
        case IBRT_ACTION_PAUSE:
            a2dp_handleKey(AVRCP_KEY_PAUSE_INTERNAL);
            break;
        case IBRT_ACTION_SIRI:
        	if(open_siri_flag == 1)
            {
                TRACE(0,"open siri");
                app_hfp_siri_voice(true);
                open_siri_flag = 0;
            }
            else
            {
                TRACE(0,"evnet none close siri");
                app_hfp_siri_voice(false);
            }
            break;
        case IBRT_ACTION_FORWARD:
            a2dp_handleKey(AVRCP_KEY_FORWARD);
            break;
        case IBRT_ACTION_BACKWARD:
            a2dp_handleKey(AVRCP_KEY_BACKWARD);
            break;
        case IBRT_ACTION_REDIAL:
            hfp_handle_key(HFP_KEY_REDIAL_LAST_CALL);
            break;
        default:
            TRACE(2,"%s unknown user action %d\n", __func__, p_buff[0]);
            break;
    }
}

#endif

