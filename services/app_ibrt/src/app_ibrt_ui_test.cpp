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
#include <string.h>
#include "app_tws_ibrt_trace.h"
#include "factory_section.h"
#include "apps.h"
#include "app_battery.h"
#include "app_anc.h"
#include "app_key.h"
#include "app_ibrt_if.h"
#include "app_ibrt_ui_test.h"
#include "app_ibrt_ui_test_cmd_if.h"
#include "app_ibrt_peripheral_manager.h"
#include "a2dp_decoder.h"
#include "app_ibrt_keyboard.h"
#include "nvrecord_env.h"
#include "nvrecord_ble.h"
#include "app_tws_if.h"
#include "besbt.h"
#include "app_bt.h"
#include "nvrecord_extension.h"
#include "app_key.h"
#include "app_ble_mode_switch.h"
#include "app.h"
#include "norflash_api.h"

#ifdef __AI_VOICE__
#include "ai_thread.h"
#include "ai_control.h"
#endif
#if defined(VOICE_DATAPATH)
#include "app_voicepath.h"
#endif

#ifdef BES_OTA_BASIC
#include "ota_control.h"
#endif

#include "app_anc.h"
#include "pmu.h"
#include "hal_bootmode.h"

#if defined(ANC_APP)
extern "C" int32_t app_anc_tws_sync_change(uint8_t *buf, uint32_t len);
extern void app_anc_key(APP_KEY_STATUS *status, void *param);
#endif

#if defined(ANC_ASSIST_ENABLED)
extern "C" int32_t anc_assist_tws_sync_status_change(uint8_t *buf, uint32_t len);
#endif
#if defined(IBRT)
#include "btapp.h"
extern struct BT_DEVICE_T  app_bt_device;

bt_bdaddr_t master_ble_addr = {0x76, 0x33, 0x33, 0x22, 0x11, 0x11};
bt_bdaddr_t slave_ble_addr  = {0x77, 0x33, 0x33, 0x22, 0x11, 0x11};
bt_bdaddr_t box_ble_addr    = {0x78, 0x33, 0x33, 0x22, 0x11, 0x11};

#ifdef IBRT_SEARCH_UI
void app_ibrt_battery_callback(APP_BATTERY_MV_T currvolt, uint8_t currlevel,enum APP_BATTERY_STATUS_T curstatus,uint32_t status, union APP_BATTERY_MSG_PRAMS prams);
void app_ibrt_simulate_charger_plug_in_test(void)
{
    union APP_BATTERY_MSG_PRAMS msg_prams;
    msg_prams.charger = APP_BATTERY_CHARGER_PLUGIN;
    app_ibrt_battery_callback(0, 0, APP_BATTERY_STATUS_CHARGING, 1, msg_prams);
}
void app_ibrt_simulate_charger_plug_out_test(void)
{
    union APP_BATTERY_MSG_PRAMS msg_prams;
    msg_prams.charger = APP_BATTERY_CHARGER_PLUGOUT;
    app_ibrt_battery_callback(0, 0, APP_BATTERY_STATUS_CHARGING, 1, msg_prams);
}
void app_ibrt_simulate_charger_plug_box_test(void)
{
    static int count = 0;
    if (count++ % 2 == 0)
    {
        app_ibrt_simulate_charger_plug_in_test();
    }
    else
    {
        app_ibrt_simulate_charger_plug_out_test();
    }
}
void app_ibrt_simulate_charger_plugin_key(APP_KEY_STATUS *status, void *param)
{
    app_ibrt_simulate_charger_plug_in_test();
}
void app_ibrt_simulate_charger_plugout_key(APP_KEY_STATUS *status, void *param)
{
    app_ibrt_simulate_charger_plug_out_test();
}
void app_ibrt_simulate_tws_role_switch(APP_KEY_STATUS *status, void *param)
{
    app_ibrt_ui_tws_switch();
}
#endif
void app_ibrt_ui_audio_play(void)
{
    uint8_t action[] = {IBRT_ACTION_PLAY};
    app_ibrt_if_start_user_action(action, sizeof(action));
}
void app_ibrt_ui_audio_pause(void)
{
    uint8_t action[] = {IBRT_ACTION_PAUSE};
    app_ibrt_if_start_user_action(action, sizeof(action));
}

void app_role_switch(void)
{
    app_ibrt_ui_tws_switch();
}
void app_poweroff_test(void)
{
    hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT);
    pmu_shutdown();
}
/*****************************************************************************
 Prototype    : app_ibrt_ui_shut_down_test
 Description  : shut down test
 Input        : void
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2019/4/10
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/
extern "C" int app_shutdown(void);
void app_ibrt_ui_shut_down_test(void)
{
    app_shutdown();
}

static bt_bdaddr_t g_app_bt_test1_addr = {0x14, 0x71, 0xda, 0x7d, 0x1a, 0x00};
static bt_bdaddr_t g_app_bt_test2_addr = {0x14, 0x71, 0xda, 0x7d, 0x1a, 0x00};
bt_bdaddr_t *app_bt_get_test1_address(void)
{
    return &g_app_bt_test1_addr;
}

bt_bdaddr_t *app_bt_get_test2_address(void)
{
    return &g_app_bt_test2_addr;
}

void app_ibrt_ui_test_disconnect_mobile_01(void)
{
    app_ibrt_if_disconnet_moblie_device(app_bt_get_test1_address());
    app_ibrt_if_reconnect_moblie_device(app_bt_get_test1_address());
}

void app_ibrt_ui_test_reconnect_mobile_01(void)
{
    app_ibrt_if_reconnect_moblie_device(app_bt_get_test1_address());
}

void app_ibrt_ui_test_disconnect_mobile_02(void)
{
    app_ibrt_if_disconnet_moblie_device(app_bt_get_test2_address());
    app_ibrt_if_reconnect_moblie_device(app_bt_get_test2_address());
}

void app_ibrt_ui_test_reconnect_mobile_02(void)
{
    app_ibrt_if_reconnect_moblie_device(app_bt_get_test2_address());
}

const app_uart_handle_t app_ibrt_uart_test_handle[]=
{
    {"open_box_event_test",app_ibrt_ui_open_box_event_test},
    {"fetch_out_box_event_test",app_ibrt_ui_fetch_out_box_event_test},
    {"put_in_box_event_test",app_ibrt_ui_put_in_box_event_test},
    {"close_box_event_test",app_ibrt_ui_close_box_event_test},
    {"reconnect_event_test",app_ibrt_ui_reconnect_event_test},
    {"wear_up_event_test",app_ibrt_ui_ware_up_event_test},
    {"wear_down_event_test",app_ibrt_ui_ware_down_event_test},
    {"shut_down_test",app_ibrt_ui_shut_down_test},
    {"phone_connect_event_test",app_ibrt_ui_phone_connect_event_test},
    {"switch_ibrt_test",app_ibrt_ui_tws_swtich_test},
    {"suspend_ibrt_test",app_ibrt_ui_suspend_ibrt_test},
    {"resume_ibrt_test",app_ibrt_ui_resume_ibrt_test},
    {"conn_second_mobile_test",app_ibrt_ui_choice_connect_second_mobile},
    {"pairing_mode_test",app_ibrt_ui_pairing_mode_test},
    {"single_earbuds_pairing",app_ibrt_ui_single_earbud_pairing_mode_test},
    {"freeman_mode_test",app_ibrt_ui_freeman_pairing_mode_test},
    {"avrcp_play_test",app_ibrt_ui_audio_play_test},
    {"avrcp_pause_test",app_ibrt_ui_audio_pause_test},
    {"avrcp_next_track_test",app_ibrt_ui_audio_forward_test},
    {"avrcp_prev_track_test",app_ibrt_ui_audio_backward_test},
    {"avrcp_vol_up_test",app_ibrt_ui_avrcp_volume_up_test},
    {"avrcp_vol_down_test",app_ibrt_ui_avrcp_volume_down_test},
    {"hfsco_create",app_ibrt_ui_hfsco_create_test},
    {"hfsco_disc",app_ibrt_ui_hfsco_disc_test},
    {"call_redial",app_ibrt_ui_call_redial_test},
    {"hfp_answer_test",app_ibrt_ui_call_answer_test},
    {"hfp_hangup_test",app_ibrt_ui_call_hangup_test},
    {"volume_up",app_ibrt_ui_local_volume_up_test},
    {"volume_down",app_ibrt_ui_local_volume_down_test},
    {"get_a2dp_state_test",app_ibrt_ui_get_a2dp_state_test},
    {"get_avrcp_state_test",app_ibrt_ui_get_avrcp_state_test},
    {"get_hfp_state_test",app_ibrt_ui_get_hfp_state_test},
    {"get_call_status",app_ibrt_ui_get_call_status_test},
    {"tws_get_role_test",app_ibrt_ui_get_ibrt_role_test},
    {"get_tws_state",app_ibrt_ui_get_tws_conn_state_test},
    {"iic_switch", app_ibrt_ui_iic_uart_switch_test},
    {"soft_reset", app_ibrt_ui_soft_reset_test},
    {"rx_close",app_trigger_rx_close_test},
    {"role_switch",app_role_switch},
    {"power_off",app_poweroff_test},
    {"disconnect_mobile1_test",app_ibrt_ui_test_disconnect_mobile_01},
    {"reconnect_mobile1_test",app_ibrt_ui_test_reconnect_mobile_01},
    {"disconnect_mobile2_test",app_ibrt_ui_test_disconnect_mobile_02},
    {"reconnect_mobile2_test",app_ibrt_ui_test_reconnect_mobile_02},
#ifdef IBRT_SEARCH_UI
    {"plug_in_test",app_ibrt_simulate_charger_plug_in_test},
    {"plug_out_test",app_ibrt_simulate_charger_plug_out_test},
    {"plug_box_test",app_ibrt_simulate_charger_plug_box_test},
#endif
#ifdef IBRT_ENHANCED_STACK_PTS
    {"hf_create_service_link",btif_pts_hf_create_link_with_pts},
    {"hf_disc_service_link",btif_pts_hf_disc_service_link},
    {"hf_create_audio_link",btif_pts_hf_create_audio_link},
    {"hf_disc_audio_link",btif_pts_hf_disc_audio_link},
    {"hf_answer_call",btif_pts_hf_answer_call},
    {"hf_hangup_call",btif_pts_hf_hangup_call},
    {"rfc_register",btif_pts_rfc_register_channel},
    {"rfc_close",btif_pts_rfc_close},
    {"av_create_channel",btif_pts_av_create_channel_with_pts},
    {"av_disc_channel",btif_pts_av_disc_channel},
    {"ar_connect",btif_pts_ar_connect_with_pts},
    {"ar_disconnect",btif_pts_ar_disconnect},
    {"ar_panel_play",btif_pts_ar_panel_play},
    {"ar_panel_pause",btif_pts_ar_panel_pause},
    {"ar_panel_stop",btif_pts_ar_panel_stop},
    {"ar_panel_forward",btif_pts_ar_panel_forward},
    {"ar_panel_backward",btif_pts_ar_panel_backward},
    {"ar_volume_up",btif_pts_ar_volume_up},
    {"ar_volume_down",btif_pts_ar_volume_down},
    {"ar_volume_notify",btif_pts_ar_volume_notify},
    {"ar_volume_change",btif_pts_ar_volume_change},
    {"ar_set_absolute_volume",btif_pts_ar_set_absolute_volume},
#endif

    //A2DP/SNK/AVP/BI-01-C
    {"AVDTP_reject_INVALID_OBJECT_TYPE", btif_pts_reject_INVALID_OBJECT_TYPE},
    //A2DP/SNK/AVP/BI-02-C
    {"AVDTP_reject_INVALID_CHANNELS", btif_pts_reject_INVALID_CHANNELS},
    //A2DP/SNK/AVP/BI-03-C
    {"AVDTP_reject_INVALID_SAMPLING_FREQUENCY", btif_pts_reject_INVALID_SAMPLING_FREQUENCY},
     //A2DP/SNK/AVP/BI-04-C
    {"AVDTP_reject_INVALID_DRC", btif_pts_reject_INVALID_DRC},
    //A2DP/SNK/AVP/BI-06-C
    {"AVDTP_reject_NOT_SUPPORTED_OBJECT_TYPE", btif_pts_reject_NOT_SUPPORTED_OBJECT_TYPE},
    //A2DP/SNK/AVP/BI-07-C
    {"AVDTP_reject_NOT_SUPPORTED_CHANNELS", btif_pts_reject_NOT_SUPPORTED_CHANNELS},
    //A2DP/SNK/AVP/BI-08-C
    {"AVDTP_reject_NOT_SUPPORTED_SAMPLING_FREQUENCY", btif_pts_reject_NOT_SUPPORTED_SAMPLING_FREQUENCY},
    //A2DP/SNK/AVP/BI-09-C
    {"AVDTP_reject_NOT_SUPPORTED_DRC", btif_pts_reject_NOT_SUPPORTED_DRC},
    //A2DP/SNK/AVP/BI-10-C
    {"AVDTP_reject_INVALID_CODEC_TYPE", btif_pts_reject_INVALID_CODEC_TYPE},
    //A2DP/SNK/AVP/BI-11-C
    {"AVDTP_reject_INVALID_CHANNEL_MODE", btif_pts_reject_INVALID_CHANNEL_MODE},
    //A2DP/SNK/AVP/BI-12-C
    {"AVDTP_reject_INVALID_SUBBANDS", btif_pts_reject_INVALID_SUBBANDS},
    //A2DP/SNK/AVP/BI-13-C
    {"AVDTP_reject_INVALID_ALLOCATION_METHOD", btif_pts_reject_INVALID_ALLOCATION_METHOD},
    //A2DP/SNK/AVP/BI-14-C
    {"AVDTP_reject_INVALID_MINIMUM_BITPOOL_VALUE", btif_pts_reject_INVALID_MINIMUM_BITPOOL_VALUE},
    //A2DP/SNK/AVP/BI-15-C
    {"AVDTP_reject_INVALID_MAXIMUM_BITPOOL_VALUE", btif_pts_reject_INVALID_MAXIMUM_BITPOOL_VALUE},
    //A2DP/SNK/AVP/BI-16-C
    {"AVDTP_reject_INVALID_BLOCK_LENGTH", btif_pts_reject_INVALID_BLOCK_LENGTH},
    //A2DP/SNK/AVP/BI-17-C
    {"AVDTP_reject_INVALID_CP_TYPE", btif_pts_reject_INVALID_CP_TYPE},
    //A2DP/SNK/AVP/BI-18-C
    {"AVDTP_reject_INVALID_CP_FORMAT", btif_pts_reject_INVALID_CP_FORMAT},
    //A2DP/SNK/AVP/BI-20-C
    {"AVDTP_reject_NOT_SUPPORTED_CODEC_TYPE", btif_pts_reject_NOT_SUPPORTED_CODEC_TYPE},

};
/*****************************************************************************
 Prototype    : app_ibrt_ui_find_uart_handle
 Description  : find the test cmd handle
 Input        : uint8_t* buf
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2019/3/30
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/
app_uart_test_function_handle app_ibrt_ui_find_uart_handle(unsigned char* buf)
{
    app_uart_test_function_handle p = NULL;
    for(uint8_t i = 0; i<sizeof(app_ibrt_uart_test_handle)/sizeof(app_uart_handle_t); i++)
    {
        if(strncmp((char*)buf, app_ibrt_uart_test_handle[i].string, strlen(app_ibrt_uart_test_handle[i].string))==0 ||
           strstr(app_ibrt_uart_test_handle[i].string, (char*)buf))
        {
            p = app_ibrt_uart_test_handle[i].function;
            break;
        }
    }
    return p;
}
/*****************************************************************************
 Prototype    : app_ibrt_ui_test_cmd_handler
 Description  : ibrt ui test cmd handler
 Input        : uint8_t *buf
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2019/3/30
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/
extern "C" int app_ibrt_ui_test_cmd_handler(unsigned char *buf, unsigned int length)
{
    int ret = 0;

    if (buf[length-2] == 0x0d ||
        buf[length-2] == 0x0a)
    {
        buf[length-2] = 0;
    }

    app_uart_test_function_handle handl_function = app_ibrt_ui_find_uart_handle(buf);
    if(handl_function)
    {
        handl_function();
    }
    else
    {
        ret = -1;
        TRACE(0,"can not find handle function");
    }
    return ret;
}
#ifdef BES_AUDIO_DEV_Main_Board_9v0
#ifdef USE_PERIPHERAL_THREAD
void app_ibrt_key1(APP_KEY_STATUS *status, void *param)
{
    TRACE(3,"%s %d,%d",__func__, status->code, status->event);
    TWS_PD_MSG_BLOCK msg;
    msg.msg_body.message_id = 0;
    msg.msg_body.message_ptr = (uint32_t)NULL;
    app_ibrt_peripheral_mailbox_put(&msg);
}

void app_ibrt_key2(APP_KEY_STATUS *status, void *param)
{
    TRACE(3,"%s %d,%d",__func__, status->code, status->event);
    TWS_PD_MSG_BLOCK msg;
    msg.msg_body.message_id = 1;
    msg.msg_body.message_ptr = (uint32_t)NULL;
    app_ibrt_peripheral_mailbox_put(&msg);
}

void app_ibrt_key3(APP_KEY_STATUS *status, void *param)
{
    TRACE(3,"%s %d,%d",__func__, status->code, status->event);
    TWS_PD_MSG_BLOCK msg;
    msg.msg_body.message_id = 2;
    msg.msg_body.message_ptr = (uint32_t)NULL;
    app_ibrt_peripheral_mailbox_put(&msg);
}

void app_ibrt_key4(APP_KEY_STATUS *status, void *param)
{
    TRACE(3,"%s %d,%d",__func__, status->code, status->event);
    TWS_PD_MSG_BLOCK msg;
    msg.msg_body.message_id = 3;
    msg.msg_body.message_ptr = (uint32_t)NULL;
    app_ibrt_peripheral_mailbox_put(&msg);
}

void app_ibrt_key5(APP_KEY_STATUS *status, void *param)
{
    TRACE(3,"%s %d,%d",__func__, status->code, status->event);
    TWS_PD_MSG_BLOCK msg;
    msg.msg_body.message_id = 4;
    msg.msg_body.message_ptr = (uint32_t)NULL;
    app_ibrt_peripheral_mailbox_put(&msg);
}

void app_ibrt_key6(APP_KEY_STATUS *status, void *param)
{
    TRACE(3,"%s %d,%d",__func__, status->code, status->event);
    TWS_PD_MSG_BLOCK msg;
    msg.msg_body.message_id = 5;
    msg.msg_body.message_ptr = (uint32_t)NULL;
    app_ibrt_peripheral_mailbox_put(&msg);
}
#endif
#endif

void app_ibrt_ui_test_key_local(APP_KEY_STATUS *status, void *param)
{
    TRACE(3,"%s %d,%d",__func__, status->code, status->event);
#ifdef IBRT_SEARCH_UI
    app_ibrt_search_ui_handle_key(status,param);
#else
    app_ibrt_normal_ui_handle_key(status,param);
#endif
}

void app_ibrt_ui_test_key(APP_KEY_STATUS *status, void *param)
{
    TRACE(3,"%s %d,%d",__func__, status->code, status->event);
#ifndef TILE_DATAPATH
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    uint8_t shutdown_key = HAL_KEY_EVENT_LONGLONGPRESS;
#endif

#ifndef TILE_DATAPATH
    if (IBRT_SLAVE == p_ibrt_ctrl->current_role && status->event != shutdown_key)
    {
        app_ibrt_if_keyboard_notify(status,param);
    }
    else
#endif
    {
#ifdef IBRT_SEARCH_UI
        app_ibrt_search_ui_handle_key(status,param);
#else
        app_ibrt_normal_ui_handle_key(status,param);
#endif
    }
}

void app_ibrt_ui_test_key_io_event(APP_KEY_STATUS *status, void *param)
{
    TRACE(3,"%s %d,%d",__func__, status->code, status->event);
    switch(status->event)
    {
        case APP_KEY_EVENT_CLICK:
            if (status->code== APP_KEY_CODE_FN1)
            {
                app_ibrt_if_event_entry(IBRT_OPEN_BOX_EVENT);
            }
            else if (status->code== APP_KEY_CODE_FN2)
            {
                app_ibrt_if_event_entry(IBRT_FETCH_OUT_EVENT);
            }
            else
            {
                app_ibrt_if_event_entry(IBRT_WEAR_UP_EVENT);
            }
            break;

        case APP_KEY_EVENT_DOUBLECLICK:
            if (status->code== APP_KEY_CODE_FN1)
            {
                app_ibrt_if_event_entry(IBRT_CLOSE_BOX_EVENT);
            }
            else if (status->code== APP_KEY_CODE_FN2)
            {
                app_ibrt_if_event_entry(IBRT_PUT_IN_EVENT);
            }
            else
            {
               app_ibrt_if_event_entry(IBRT_WEAR_DOWN_EVENT);
            }
            break;

        case APP_KEY_EVENT_LONGPRESS:
            break;

        case APP_KEY_EVENT_TRIPLECLICK:
            break;

        case HAL_KEY_EVENT_LONGLONGPRESS:
            break;

        case APP_KEY_EVENT_ULTRACLICK:
            break;

        case APP_KEY_EVENT_RAMPAGECLICK:
            break;
    }
}

void app_ibrt_ui_test_key_custom_event(APP_KEY_STATUS *status, void *param)
{
    TRACE(3,"%s %d,%d",__func__, status->code, status->event);
    switch(status->event)
    {
        case APP_KEY_EVENT_CLICK:
            break;

        case APP_KEY_EVENT_DOUBLECLICK:
            break;

        case APP_KEY_EVENT_LONGPRESS:
            break;

        case APP_KEY_EVENT_TRIPLECLICK:
            break;

        case HAL_KEY_EVENT_LONGLONGPRESS:
            break;

        case APP_KEY_EVENT_ULTRACLICK:
            break;

        case APP_KEY_EVENT_RAMPAGECLICK:
            break;
    }
}

#ifdef IS_MULTI_AI_ENABLED
#ifdef MAI_TYPE_REBOOT_WITHOUT_OEM_APP
extern "C" bool gva_status_update_start;
extern "C" bool ama_status_update_start;
#endif
#endif
void app_ibrt_ui_test_voice_assistant_key(APP_KEY_STATUS *status, void *param)
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();

    TRACE(2,"%s event %d", __func__, status->event);

    if (p_ibrt_ctrl->current_role != IBRT_MASTER)
    {
        app_ibrt_if_keyboard_notify(status, param);
        TRACE(2,"%s isn't master %d", __func__, p_ibrt_ctrl->current_role);
        return;
    }
#ifdef IS_MULTI_AI_ENABLED
    if (ai_manager_spec_get_status_is_in_invalid()) {
        TRACE(0,"AI feature has been diabled");
        return;
    }

#ifdef MAI_TYPE_REBOOT_WITHOUT_OEM_APP
    if (gva_status_update_start||ama_status_update_start) {
        TRACE(0,"device reboot is ongoing...");
        return;
    }
#endif

    if(ai_voicekey_is_enable()) {
        if (AI_SPEC_GSOUND == ai_manager_get_current_spec()) {
#ifdef VOICE_DATAPATH
            app_voicepath_key(status, param);
#endif
        } else if(AI_SPEC_INIT != ai_manager_get_current_spec()) {
            if (APP_KEY_EVENT_CLICK == status->event)
                ai_function_handle(CALLBACK_WAKE_UP, status, 0);
        }
    }
#else
#if defined(__AI_VOICE__)
    if(ai_struct.wake_up_type == TYPE__PRESS_AND_HOLD) {
        if (APP_KEY_EVENT_FIRST_DOWN == status->event) {
            TRACE(1,"%s wake up AI", __func__);
            ai_function_handle(CALLBACK_WAKE_UP, status, 0);
        } else if (APP_KEY_EVENT_UP == status->event) {
            TRACE(1,"%s stop speech", __func__);
#ifdef __AMA_VOICE__
            ai_function_handle(CALLBACK_ENDPOINT_SPEECH, status, 0);
#elif   defined(__TENCENT_VOICE__)
            ai_function_handle(CALLBACK_REQUEST_FOR_STOP_SPEECH, status, 0);
#else
            ai_function_handle(CALLBACK_STOP_SPEECH, status, 0);
#endif
        }
#ifdef __TENCENT_VOICE__
        else if(APP_KEY_EVENT_DOUBLECLICK == status->event)
            {
                 TRACE(1,"%s stop replying", __func__);
                 ai_function_handle(CALLBACK_STOP_REPLYING, status, 0);
            }
#endif
    } else {
        if (APP_KEY_EVENT_CLICK == status->event) {
            ai_function_handle(CALLBACK_WAKE_UP, status, 0);
        }
    }
#elif defined(VOICE_DATAPATH)
    app_voicepath_key(status, param);
#endif
#endif
}
#ifdef POWERKEY_I2C_SWITCH
extern void app_factorymode_i2c_switch(APP_KEY_STATUS *status, void *param);
#endif

#ifdef TOTA_v2
extern void app_bt_key_enter_tota_mode(APP_KEY_STATUS *status, void *param);
#endif

const APP_KEY_HANDLE  app_ibrt_ui_test_key_cfg[] =
{
#if defined(__AI_VOICE__) || defined(GSOUND_ENABLED)
    {{APP_KEY_CODE_GOOGLE, APP_KEY_EVENT_FIRST_DOWN}, "google assistant key", app_ibrt_ui_test_voice_assistant_key, NULL},
    {{APP_KEY_CODE_GOOGLE, APP_KEY_EVENT_UP}, "google assistant key", app_ibrt_ui_test_voice_assistant_key, NULL},
    {{APP_KEY_CODE_GOOGLE, APP_KEY_EVENT_LONGPRESS}, "google assistant key", app_ibrt_ui_test_voice_assistant_key, NULL},
    {{APP_KEY_CODE_GOOGLE, APP_KEY_EVENT_CLICK}, "google assistant key", app_ibrt_ui_test_voice_assistant_key, NULL},
    {{APP_KEY_CODE_GOOGLE, APP_KEY_EVENT_DOUBLECLICK}, "google assistant key", app_ibrt_ui_test_voice_assistant_key, NULL},
#endif
#ifdef TILE_DATAPATH
    {{APP_KEY_CODE_TILE,APP_KEY_EVENT_DOWN},"tile function key",app_ibrt_ui_test_key, NULL},
    {{APP_KEY_CODE_TILE,APP_KEY_EVENT_UP},"tile function key",app_ibrt_ui_test_key, NULL},
#endif


    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_DOWN},"app_ibrt_ui_test_key_local",app_ibrt_ui_test_key_local, NULL},
#if defined( __BT_ANC_KEY__)&&defined(ANC_APP)
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_CLICK},"bt anc key",app_anc_key, NULL},
#elif TOTA_v2
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_CLICK},"app_bt_key_enter_tota_mode", app_bt_key_enter_tota_mode, NULL},
#else
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_CLICK},"app_ibrt_ui_test_key", app_ibrt_ui_test_key, NULL},
#endif
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_LONGPRESS},"app_ibrt_ui_test_key", app_ibrt_ui_test_key, NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_LONGLONGPRESS},"app_ibrt_ui_test_key", app_ibrt_ui_test_key, NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_DOUBLECLICK},"app_ibrt_ui_test_key", app_ibrt_ui_test_key, NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_TRIPLECLICK},"app_ibrt_ui_test_key", app_ibrt_ui_test_key, NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_ULTRACLICK},"app_ibrt_ui_test_key", app_ibrt_ui_test_key, NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_RAMPAGECLICK},"app_ibrt_ui_test_key", app_ibrt_ui_test_key, NULL},
#ifdef POWERKEY_I2C_SWITCH
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_RAMPAGECLICK},"bt i2c key",app_factorymode_i2c_switch, NULL},
#endif
#if defined(CHIP_BEST1400)
#ifdef IBRT_SEARCH_UI
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_CLICK},"app_ibrt_ui_test_key", app_ibrt_simulate_charger_plugin_key, NULL},
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_DOUBLECLICK},"app_ibrt_ui_test_key",app_ibrt_simulate_tws_role_switch, NULL},
    {{APP_KEY_CODE_FN2,APP_KEY_EVENT_CLICK},"app_ibrt_ui_test_key", app_ibrt_simulate_charger_plugout_key, NULL},
#endif
#elif defined(CHIP_BEST1402)
#ifdef IBRT_SEARCH_UI
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_CLICK},"app_ibrt_ui_test_key", app_ibrt_simulate_charger_plugin_key, NULL},
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_LONGPRESS},"app_ibrt_ui_test_key",app_ibrt_simulate_tws_role_switch, NULL},
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_DOUBLECLICK},"app_ibrt_ui_test_key", app_ibrt_simulate_charger_plugout_key, NULL},
#endif
#else
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_CLICK},"app_ibrt_ui_test_key", app_ibrt_ui_test_key_io_event, NULL},
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_DOUBLECLICK},"app_ibrt_ui_test_key", app_ibrt_ui_test_key_io_event, NULL},
    {{APP_KEY_CODE_FN2,APP_KEY_EVENT_CLICK},"app_ibrt_ui_test_key", app_ibrt_ui_test_key_io_event, NULL},
    {{APP_KEY_CODE_FN2,APP_KEY_EVENT_DOUBLECLICK},"app_ibrt_ui_test_key", app_ibrt_ui_test_key_io_event, NULL},
#endif
    {{APP_KEY_CODE_FN3,APP_KEY_EVENT_CLICK},"app_ibrt_ui_test_key", app_ibrt_ui_test_key_io_event, NULL},
    {{APP_KEY_CODE_FN3,APP_KEY_EVENT_DOUBLECLICK},"app_ibrt_ui_test_key", app_ibrt_ui_test_key_io_event, NULL},
/*
#ifdef BES_AUDIO_DEV_Main_Board_9v0
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_CLICK},"app_ibrt_ui_test_key", app_ibrt_key1, NULL},
    {{APP_KEY_CODE_FN2,APP_KEY_EVENT_CLICK},"app_ibrt_ui_test_key", app_ibrt_key2, NULL},
    {{APP_KEY_CODE_FN3,APP_KEY_EVENT_CLICK},"app_ibrt_ui_test_key", app_ibrt_key3, NULL},
    {{APP_KEY_CODE_FN4,APP_KEY_EVENT_CLICK},"app_ibrt_ui_test_key", app_ibrt_key4, NULL},
    {{APP_KEY_CODE_FN5,APP_KEY_EVENT_CLICK},"app_ibrt_ui_test_key", app_ibrt_key5, NULL},
    {{APP_KEY_CODE_FN6,APP_KEY_EVENT_CLICK},"app_ibrt_ui_test_key", app_ibrt_key6, NULL},
#endif
*/
};

/*
* customer addr config here
*/
ibrt_pairing_info_t g_ibrt_pairing_info[] =
{
    {{0x51, 0x33, 0x33, 0x22, 0x11, 0x11},{0x50, 0x33, 0x33, 0x22, 0x11, 0x11}},
    {{0x53, 0x33, 0x33, 0x22, 0x11, 0x11},{0x52, 0x33, 0x33, 0x22, 0x11, 0x11}}, /*LJH*/
    {{0x61, 0x33, 0x33, 0x22, 0x11, 0x11},{0x60, 0x33, 0x33, 0x22, 0x11, 0x11}},
    {{0x71, 0x33, 0x33, 0x22, 0x11, 0x11},{0x70, 0x33, 0x33, 0x22, 0x11, 0x11}},
    {{0x81, 0x33, 0x33, 0x22, 0x11, 0x11},{0x80, 0x33, 0x33, 0x22, 0x11, 0x11}},
    {{0x91, 0x33, 0x33, 0x22, 0x11, 0x11},{0x90, 0x33, 0x33, 0x22, 0x11, 0x11}}, /*Customer use*/
    {{0x05, 0x33, 0x33, 0x22, 0x11, 0x11},{0x04, 0x33, 0x33, 0x22, 0x11, 0x11}}, /*Rui*/
    {{0x07, 0x33, 0x33, 0x22, 0x11, 0x11},{0x06, 0x33, 0x33, 0x22, 0x11, 0x11}}, /*zsl*/
    {{0x88, 0xaa, 0x33, 0x22, 0x11, 0x11},{0x87, 0xaa, 0x33, 0x22, 0x11, 0x11}}, /*Lufang*/
    {{0x77, 0x22, 0x66, 0x22, 0x11, 0x11},{0x77, 0x33, 0x66, 0x22, 0x11, 0x11}}, /*xiao*/
    {{0xAA, 0x22, 0x66, 0x22, 0x11, 0x11},{0xBB, 0x33, 0x66, 0x22, 0x11, 0x11}}, /*LUOBIN*/
    {{0x08, 0x33, 0x66, 0x22, 0x11, 0x11},{0x07, 0x33, 0x66, 0x22, 0x11, 0x11}}, /*Yangbin1*/
    {{0x0B, 0x33, 0x66, 0x22, 0x11, 0x11},{0x0A, 0x33, 0x66, 0x22, 0x11, 0x11}}, /*Yangbin2*/
    {{0x35, 0x33, 0x66, 0x22, 0x11, 0x11},{0x34, 0x33, 0x66, 0x22, 0x11, 0x11}}, /*Lulu*/
    {{0xF8, 0x33, 0x66, 0x22, 0x11, 0x11},{0xF7, 0x33, 0x66, 0x22, 0x11, 0x11}}, /*jtx*/
    {{0xd3, 0x53, 0x86, 0x42, 0x71, 0x31},{0xd2, 0x53, 0x86, 0x42, 0x71, 0x31}}, /*shhx*/
    {{0xcc, 0xaa, 0x99, 0x88, 0x77, 0x66},{0xbb, 0xaa, 0x99, 0x88, 0x77, 0x66}}, /*mql*/
    {{0x95, 0x33, 0x69, 0x22, 0x11, 0x11},{0x94, 0x33, 0x69, 0x22, 0x11, 0x11}}, /*wyl*/
    {{0x82, 0x35, 0x68, 0x24, 0x19, 0x17},{0x81, 0x35, 0x68, 0x24, 0x19, 0x17}}, /*hy*/
    {{0x66, 0x66, 0x88, 0x66, 0x66, 0x88},{0x65, 0x66, 0x88, 0x66, 0x66, 0x88}}, /*xdl*/
    {{0x61, 0x66, 0x66, 0x66, 0x66, 0x81},{0x16, 0x66, 0x66, 0x66, 0x66, 0x18}}, /*test1*/
    {{0x62, 0x66, 0x66, 0x66, 0x66, 0x82},{0x26, 0x66, 0x66, 0x66, 0x66, 0x28}}, /*test2*/
    {{0x63, 0x66, 0x66, 0x66, 0x66, 0x83},{0x36, 0x66, 0x66, 0x66, 0x66, 0x38}}, /*test3*/
    {{0x64, 0x66, 0x66, 0x66, 0x66, 0x84},{0x46, 0x66, 0x66, 0x66, 0x66, 0x48}}, /*test4*/
    {{0x65, 0x66, 0x66, 0x66, 0x66, 0x85},{0x56, 0x66, 0x66, 0x66, 0x66, 0x58}}, /*test5*/
    {{0xaa, 0x66, 0x66, 0x66, 0x66, 0x86},{0xaa, 0x66, 0x66, 0x66, 0x66, 0x68}}, /*test6*/
    {{0x67, 0x66, 0x66, 0x66, 0x66, 0x87},{0x76, 0x66, 0x66, 0x66, 0x66, 0x78}}, /*test7*/
    {{0x68, 0x66, 0x66, 0x66, 0x66, 0xa8},{0x86, 0x66, 0x66, 0x66, 0x66, 0x8a}}, /*test8*/
    {{0x69, 0x66, 0x66, 0x66, 0x66, 0x89},{0x86, 0x66, 0x66, 0x66, 0x66, 0x18}}, /*test9*/
    {{0x93, 0x33, 0x33, 0x33, 0x33, 0x33},{0x92, 0x33, 0x33, 0x33, 0x33, 0x33}}, /*gxl*/
    {{0xae, 0x28, 0x00, 0xe9, 0xc6, 0x5c},{0xd8, 0x29, 0x00, 0xe9, 0xc6, 0x5c}}, /*lsk*/
    {{0x07, 0x13, 0x66, 0x22, 0x11, 0x11},{0x06, 0x13, 0x66, 0x22, 0x11, 0x11}}, /*yangguo*/
    {{0x02, 0x15, 0x66, 0x22, 0x11, 0x11},{0x01, 0x15, 0x66, 0x22, 0x11, 0x11}}, /*mql fpga*/
	{{0x11, 0x58, 0x00, 0x1e, 0x43, 0x9c},{0x10, 0x58, 0x00, 0x1e, 0x43, 0x9c}}, /*sndp*/

    {{0x55, 0xda, 0x61, 0xe9, 0xc6, 0x5c},{0x56, 0xda, 0x61, 0xe9, 0xc6, 0x5c}}, /*test tile*/
    
};

int app_ibrt_ui_test_config_load(void *config)
{
    ibrt_pairing_info_t *ibrt_pairing_info_lst = g_ibrt_pairing_info;
    uint32_t lst_size = sizeof(g_ibrt_pairing_info)/sizeof(ibrt_pairing_info_t);
    ibrt_config_t *ibrt_config = (ibrt_config_t *)config;
#if defined(SIMPLE_TEST_UI)
    const struct HAL_IOMUX_PIN_FUNCTION_MAP app_tws_side_role_cfg = {
        HAL_IOMUX_PIN_P1_3, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL,
    };
    /**tws side role*/
    hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&app_tws_side_role_cfg, 1);
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)app_tws_side_role_cfg.pin, HAL_GPIO_DIR_IN, 1);

    const struct HAL_IOMUX_PIN_FUNCTION_MAP app_chg_cfg[2] = {
        HAL_IOMUX_PIN_P2_1, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL,
        HAL_IOMUX_PIN_P2_2, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL,
    };

    for (uint8_t i=0; i<sizeof(app_chg_cfg)/sizeof(struct HAL_IOMUX_PIN_FUNCTION_MAP); i++)
    {
        hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&app_chg_cfg[i], 1);
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)app_chg_cfg[i].pin, HAL_GPIO_DIR_OUT, 1);
    };
/*
    if(hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)app_tws_side_role_cfg.pin))
    {
        memcpy(ibrt_config->local_addr.address,ibrt_pairing_info_lst[19].master_bdaddr.address,BD_ADDR_LEN);
    }
    else
    {
        memcpy(ibrt_config->local_addr.address,ibrt_pairing_info_lst[19].slave_bdaddr.address,BD_ADDR_LEN);
    }
#else
*/
#endif
    factory_section_original_btaddr_get(ibrt_config->local_addr.address);

    for(uint32_t i =0; i<lst_size; i++)
    {
        if (!memcmp(ibrt_pairing_info_lst[i].master_bdaddr.address, ibrt_config->local_addr.address, BD_ADDR_LEN))
        {
            ibrt_config->nv_role = IBRT_MASTER;
            ibrt_config->audio_chnl_sel = AUDIO_CHANNEL_SELECT_RCHNL;
            memcpy(ibrt_config->peer_addr.address, ibrt_pairing_info_lst[i].slave_bdaddr.address, BD_ADDR_LEN);
            return 0;
        }
        else if (!memcmp(ibrt_pairing_info_lst[i].slave_bdaddr.address, ibrt_config->local_addr.address, BD_ADDR_LEN))
        {
            ibrt_config->nv_role = IBRT_SLAVE;
            ibrt_config->audio_chnl_sel = AUDIO_CHANNEL_SELECT_LCHNL;
            memcpy(ibrt_config->peer_addr.address, ibrt_pairing_info_lst[i].master_bdaddr.address, BD_ADDR_LEN);
            return 0;
        }
    }
    return -1;
}

void app_ibrt_ui_test_key_init(void)
{
    app_key_handle_clear();
    for (uint8_t i=0; i<ARRAY_SIZE(app_ibrt_ui_test_key_cfg); i++)
    {
        app_key_handle_registration(&app_ibrt_ui_test_key_cfg[i]);
    }
}

void app_ibrt_ui_test_init(void)
{
    TRACE(1,"%s", __func__);

    app_ibrt_ui_box_init(&box_ble_addr);

}

void app_ibrt_ui_sync_anc_status(uint8_t *buf, uint32_t len)
{
#if defined(ANC_APP)
    app_anc_tws_sync_change(buf, len);
#endif
}

void app_ibrt_ui_sync_psap_status(uint8_t *buf, uint32_t len)
{
#if defined(PSAP_APP)
    TRACE(0, "[%s] TODO ...", __func__);
#endif
}

void app_ibrt_ui_sync_anc_assist_status(uint8_t *buf, uint32_t len)
{
#if defined(ANC_ASSIST_ENABLED)
    anc_assist_tws_sync_status_change(buf, len);
#endif
}

#endif
