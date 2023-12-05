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
//#include "mbed.h"
#include <stdio.h>
#include "cmsis.h"
#include "cmsis_os.h"
#include "hal_uart.h"
#include "hal_timer.h"
#include "audioflinger.h"
#include "lockcqueue.h"
#include "hal_trace.h"
#include "hal_cmu.h"
#include "analog.h"


#include "hfp_api.h"
#include "me_api.h"
#include "a2dp_api.h"
#include "avdtp_api.h"
#include "avctp_api.h"
#include "avrcp_api.h"

#include "besbt.h"

#include "cqueue.h"
#include "btapp.h"
#include "app_key.h"
#include "app_audio.h"

#include "apps.h"
#include "app_bt_stream.h"
#include "app_bt_media_manager.h"
#include "app_bt.h"
#include "app_bt_func.h"
#include "app_hfp.h"
#include "bt_if.h"
#if defined(APP_LINEIN_A2DP_SOURCE)||defined(APP_I2S_A2DP_SOURCE)
#include "nvrecord_env.h"
#endif
#if defined(IBRT)
#include "app_ibrt_ui.h"
#endif

#include "os_api.h"
extern struct BT_DEVICE_T  app_bt_device;
//BT_DEVICE_ID_T g_current_device_id=BT_DEVICE_NUM;  //used to change sco by one-bring-two
//BT_DEVICE_ID_T g_another_device_id=BT_DEVICE_NUM;  //used to change sco by one-bring-two


#ifdef SUPPORT_SIRI
extern int app_hfp_siri_report();
extern int app_hfp_siri_voice(bool en);
int open_siri_flag = 0;
void bt_key_handle_siri_key(enum APP_KEY_EVENT_T event)
{
     switch(event)
     {
        case  APP_KEY_EVENT_NONE:
            if(open_siri_flag == 1){
                TRACE(0,"open siri");
                app_hfp_siri_voice(true);
                open_siri_flag = 0;
            } /*else {
                TRACE(0,"evnet none close siri");
                app_hfp_siri_voice(false);
            }*/
            break;
        case  APP_KEY_EVENT_LONGLONGPRESS:
        case  APP_KEY_EVENT_UP:
            //TRACE(0,"long long/up/click event close siri");
            //app_hfp_siri_voice(false);
            break;
        default:
            TRACE(1,"unregister down key event=%x",event);
            break;
        }
}

#endif

//bool hf_mute_flag = 0;

#if defined (__HSP_ENABLE__)

extern XaStatus app_hs_handle_cmd(HsChannel *Chan,uint8_t cmd_type);

void hsp_handle_key(uint8_t hsp_key)
{
    HsCommand *hs_cmd_p;
    HsChannel *hs_channel_tmp = NULL;
    uint8_t ret= 0;
#ifdef __BT_ONE_BRING_TWO__
    enum BT_DEVICE_ID_T another_device_id =  (app_bt_device.curr_hs_channel_id == BT_DEVICE_ID_1) ? BT_DEVICE_ID_2 : BT_DEVICE_ID_1;
    TRACE(1,"!!!hsp_handle_key curr_hf_channel=%d\n",app_bt_device.curr_hs_channel_id);
    hs_channel_tmp = (app_bt_device.curr_hs_channel_id == BT_DEVICE_ID_1) ? &(app_bt_device.hs_channel[BT_DEVICE_ID_1]) : &(app_bt_device.hs_channel[BT_DEVICE_ID_2]);
#else
    hs_channel_tmp = &(app_bt_device.hs_channel[BT_DEVICE_ID_1]);
#endif

    if(hsp_key == HSP_KEY_CKPD_CONTROL)
    {
	TRACE(0,"hsp_key = HSP_KEY_CKPD_CONTROL");
	if(app_hs_handle_cmd(hs_channel_tmp,APP_CPKD_CMD) !=0)
            TRACE(0,"app_hs_handle_cmd err");

    }
    else if(hsp_key == HSP_KEY_CHANGE_TO_PHONE)
    {
           TRACE(0,"hsp_key = HSP_KEY_CHANGE_TO_PHONE");
	HS_DisconnectAudioLink(hs_channel_tmp);
    }
    else if(hsp_key == HSP_KEY_ADD_TO_EARPHONE)
    {
        TRACE(0,"hsp_key = HSP_KEY_ADD_TO_EARPHONE");
	HS_CreateAudioLink(hs_channel_tmp);
    }


}
#endif

void hfp_handle_key(uint8_t hfp_key)
{
    hf_chan_handle_t hf_channel_tmp = NULL;

#ifdef __BT_ONE_BRING_TWO__
    enum BT_DEVICE_ID_T another_device_id =
        (app_bt_device.curr_hf_channel_id == BT_DEVICE_ID_1) ? BT_DEVICE_ID_2 : BT_DEVICE_ID_1;
    hf_chan_handle_t hf_channel_curr = NULL;
    hf_chan_handle_t hf_channel_another = NULL;
    TRACE(1,"!!!hfp_handle_key curr_hf_channel=%d\n",app_bt_device.curr_hf_channel_id);
    hf_channel_tmp = (app_bt_device.curr_hf_channel_id == BT_DEVICE_ID_1) ?
                        app_bt_device.hf_channel[BT_DEVICE_ID_1] :
                        app_bt_device.hf_channel[BT_DEVICE_ID_2];

#else
    hf_channel_tmp = app_bt_device.hf_channel[BT_DEVICE_ID_1];
#endif
    switch(hfp_key)
    {
        case HFP_KEY_ANSWER_CALL:
            ///answer a incomming call
            TRACE(0,"avrcp_key = HFP_KEY_ANSWER_CALL\n");
            //HF_AnswerCall(hf_channel_tmp,&app_bt_device.hf_command[BT_DEVICE_ID_1]);
            btif_hf_answer_call(hf_channel_tmp);
            break;
        case HFP_KEY_HANGUP_CALL:
            TRACE(0,"avrcp_key = HFP_KEY_HANGUP_CALL\n");
            //HF_Hangup(&app_bt_device.hf_channel[app_bt_device.curr_hf_channel_id],&app_bt_device.hf_command[BT_DEVICE_ID_1]);
            hf_channel_tmp = app_bt_device.hf_channel[app_bt_device.curr_hf_channel_id];
            btif_hf_hang_up_call(hf_channel_tmp);
            break;
        case HFP_KEY_REDIAL_LAST_CALL:
            ///redail the last call
            TRACE(0,"avrcp_key = HFP_KEY_REDIAL_LAST_CALL\n");
            //HF_Redial(&app_bt_device.hf_channel[app_bt_device.curr_hf_channel_id],&app_bt_device.hf_command[BT_DEVICE_ID_1]);
            hf_channel_tmp = app_bt_device.hf_channel[app_bt_device.curr_hf_channel_id];
            btif_hf_redial_call(hf_channel_tmp);
            break;
        case HFP_KEY_CHANGE_TO_PHONE:
            ///remove sco and voice change to phone
            if(app_bt_is_hfp_audio_on())
            {
                TRACE(0,"avrcp_key = HFP_KEY_CHANGE_TO_PHONE\n");
                //HF_DisconnectAudioLink(hf_channel_tmp);
                btif_hf_disc_audio_link(hf_channel_tmp);
            }
            break;
        case HFP_KEY_ADD_TO_EARPHONE:
            ///add a sco and voice change to earphone
            if(!app_bt_is_hfp_audio_on())
            {
                TRACE(1,"avrcp_key = HFP_KEY_ADD_TO_EARPHONE ver:%x\n",  btif_hf_get_version(hf_channel_tmp));
#if defined(HFP_1_6_ENABLE)
                //if (hf_channel_tmp->negotiated_codec == HF_SCO_CODEC_MSBC){
                if (btif_hf_get_negotiated_codec(hf_channel_tmp) == BTIF_HF_SCO_CODEC_MSBC) {
                    TRACE(0,"at+bcc");
#ifdef __HFP_OK__
                    hfp_handle_add_to_earphone_1_6(hf_channel_tmp);
#endif
                    TRACE(0,"CreateAudioLink");
                    btif_hf_create_audio_link(hf_channel_tmp);
                } else
#endif
                {
                    TRACE(0,"CreateAudioLink");
                    //HF_CreateAudioLink(hf_channel_tmp);
                    btif_hf_create_audio_link(hf_channel_tmp);
                }
            }
            break;
        case HFP_KEY_MUTE:
            TRACE(0,"avrcp_key = HFP_KEY_MUTE\n");
            app_bt_device.hf_mute_flag = 1;
            break;
        case HFP_KEY_CLEAR_MUTE:
            TRACE(0,"avrcp_key = HFP_KEY_CLEAR_MUTE\n");
            app_bt_device.hf_mute_flag = 0;
            break;
        case HFP_KEY_THREEWAY_HOLD_AND_ANSWER:
            TRACE(0,"avrcp_key = HFP_KEY_THREEWAY_HOLD_AND_ANSWER\n");
            //HF_CallHold(&app_bt_device.hf_channel[app_bt_device.curr_hf_channel_id],HF_HOLD_HOLD_ACTIVE_CALLS,0,&app_bt_device.hf_command[BT_DEVICE_ID_1]);
            hf_channel_tmp = app_bt_device.hf_channel[app_bt_device.curr_hf_channel_id];
            btif_hf_call_hold(hf_channel_tmp, BTIF_HF_HOLD_HOLD_ACTIVE_CALLS, 0);
            break;
        case HFP_KEY_THREEWAY_HANGUP_AND_ANSWER:
            TRACE(0,"avrcp_key = HFP_KEY_THREEWAY_HOLD_SWAP_ANSWER\n");
            //HF_CallHold(&app_bt_device.hf_channel[app_bt_device.curr_hf_channel_id],HF_HOLD_RELEASE_ACTIVE_CALLS,0,&app_bt_device.hf_command[BT_DEVICE_ID_1]);
            hf_channel_tmp = app_bt_device.hf_channel[app_bt_device.curr_hf_channel_id];
            btif_hf_call_hold(hf_channel_tmp, BTIF_HF_HOLD_RELEASE_ACTIVE_CALLS, 0);
            break;
        case HFP_KEY_THREEWAY_HOLD_REL_INCOMING:
            TRACE(0,"avrcp_key = HFP_KEY_THREEWAY_HOLD_REL_INCOMING\n");
            //HF_CallHold(&app_bt_device.hf_channel[app_bt_device.curr_hf_channel_id],HF_HOLD_RELEASE_HELD_CALLS,0,&app_bt_device.hf_command[BT_DEVICE_ID_1]);
            hf_channel_tmp = app_bt_device.hf_channel[app_bt_device.curr_hf_channel_id];
            btif_hf_call_hold(hf_channel_tmp, BTIF_HF_HOLD_RELEASE_HELD_CALLS, 0);
            break;
#ifdef __BT_ONE_BRING_TWO__
        case HFP_KEY_DUAL_HF_HANGUP_ANOTHER:////////double click
            TRACE(0,"avrcp_key = HFP_KEY_DUAL_HF_HANGUP_ANOTHER\n");
            //HF_Hangup(&app_bt_device.hf_channel[another_device_id],&app_bt_device.hf_command[another_device_id]);
            hf_channel_tmp = app_bt_device.hf_channel[another_device_id];
#ifdef __HFP_OK__
            hf_hang_up_call(hf_channel_tmp);
#endif
            break;
        case HFP_KEY_DUAL_HF_HANGUP_CURR_ANSWER_ANOTHER://////////click
            TRACE(0,"avrcp_key = HFP_KEY_DUAL_HF_HANGUP_CURR_ANSWER_ANOTHER\n");
//          HF_Hangup(&app_bt_device.hf_channel[another_device_id],&app_bt_device.hf_command[another_device_id]);
//          HF_AnswerCall(&app_bt_device.hf_channel[app_bt_device.curr_hf_channel_id],&app_bt_device.hf_command[app_bt_device.curr_hf_channel_id]);
            /*
            HF_Hangup(&app_bt_device.hf_channel[app_bt_device.curr_hf_channel_id],&app_bt_device.hf_command[app_bt_device.curr_hf_channel_id]);
            HF_AnswerCall(&app_bt_device.hf_channel[another_device_id],&app_bt_device.hf_command[another_device_id]);
            */
            hf_channel_curr = app_bt_device.hf_channel[app_bt_device.curr_hf_channel_id];
            hf_channel_another = app_bt_device.hf_channel[another_device_id];
            btif_hf_switch_calls(hf_channel_curr, hf_channel_another);
            break;
        case HFP_KEY_DUAL_HF_HOLD_CURR_ANSWER_ANOTHER:///////////long press
            TRACE(0,"avrcp_key = HFP_KEY_DUAL_HF_HOLD_CURR_ANSWER_ANOTHER\n");
            //HF_AnswerCall(&app_bt_device.hf_channel[app_bt_device.curr_hf_channel_id],&app_bt_device.hf_command[BT_DEVICE_ID_1]);
            //HF_AnswerCall(&app_bt_device.hf_channel[another_device_id],&app_bt_device.hf_command[another_device_id]);
            hf_channel_another = app_bt_device.hf_channel[another_device_id];
#ifdef __HFP_OK__
            hf_answer_call(hf_channel_another);
#endif
            break;
#endif

        default :
            break;
    }
}

//bool a2dp_play_pause_flag = 0;
uint8_t get_avrcp_via_a2dp_id(uint8_t a2dp_id);
extern void a2dp_handleKey(uint8_t a2dp_key)
{
    btif_avrcp_channel_t* avrcp_channel_tmp = NULL;
    enum BT_DEVICE_ID_T avrcp_id = BT_DEVICE_NUM;
    if(app_bt_device.a2dp_state[app_bt_device.curr_a2dp_stream_id] == 0)
        return;

#ifdef __BT_ONE_BRING_TWO__
    TRACE(1,"!!!a2dp_handleKey curr_a2dp_stream_id=%d\n",app_bt_device.curr_a2dp_stream_id);
        avrcp_id = (enum BT_DEVICE_ID_T )get_avrcp_via_a2dp_id(app_bt_device.curr_a2dp_stream_id);
        if(avrcp_id == BT_DEVICE_NUM)
            avrcp_id = BT_DEVICE_ID_1;
        avrcp_channel_tmp = app_bt_device.avrcp_channel[avrcp_id];
#else
    avrcp_id = BT_DEVICE_ID_1;
    avrcp_channel_tmp = app_bt_device.avrcp_channel[avrcp_id];
#endif

    if (!btif_avrcp_is_control_channel_connected(avrcp_channel_tmp))
    {
        TRACE(1,"avrcp_key %d the channel is not connected", a2dp_key);
        return;
    }

    switch(a2dp_key)
    {
        case AVRCP_KEY_STOP:
            TRACE(0,"avrcp_key = AVRCP_KEY_STOP");
            btif_avrcp_set_panel_key(avrcp_channel_tmp,BTIF_AVRCP_POP_STOP,TRUE);
            btif_avrcp_set_panel_key(avrcp_channel_tmp,BTIF_AVRCP_POP_STOP,FALSE);
            app_bt_device.a2dp_play_pause_flag = 0;
            break;
            
            /*
             * Wrapper PLAY/PAUSE key as IBRT EVENT to avoid TWS switch interrupt
             */
#if defined(IBRT)
        case AVRCP_KEY_PLAY:
            if (!app_ibrt_ui_event_has_been_queued(IBRT_AUDIO_PLAY))
            {
                TRACE(0,"avrcp_key = AVRCP_KEY_PLAY_INTERNAL event");
                app_ibrt_ui_event_entry(IBRT_AUDIO_PLAY);
            }
            else
            {
                TRACE(0,"avrcp_key = AVRCP_KEY_PLAY_INTERNAL event duplicated");
            }
            break;
            
        case AVRCP_KEY_PAUSE:
            if (!app_ibrt_ui_event_has_been_queued(IBRT_AUDIO_PAUSE))
            {
                TRACE(0,"avrcp_key = AVRCP_KEY_PAUSE_INTERNAL event");
                app_ibrt_ui_event_entry(IBRT_AUDIO_PAUSE);
            }
            else
            {
                TRACE(0,"avrcp_key = AVRCP_KEY_PAUSE_INTERNAL event duplicated");
            }
            break;

        case AVRCP_KEY_PLAY_INTERNAL:
            TRACE(0,"avrcp_key = AVRCP_KEY_PLAY");
            btif_avrcp_set_panel_key(avrcp_channel_tmp,BTIF_AVRCP_POP_PLAY,TRUE);
            btif_avrcp_set_panel_key(avrcp_channel_tmp,BTIF_AVRCP_POP_PLAY,FALSE);
            app_bt_device.a2dp_play_pause_flag = 1;
            break;
        case AVRCP_KEY_PAUSE_INTERNAL:    
            TRACE(0,"avrcp_key = AVRCP_KEY_PAUSE");
            btif_avrcp_set_panel_key(avrcp_channel_tmp,BTIF_AVRCP_POP_PAUSE,TRUE);
            btif_avrcp_set_panel_key(avrcp_channel_tmp,BTIF_AVRCP_POP_PAUSE,FALSE);
            app_bt_device.a2dp_play_pause_flag = 0;
            break;
#else            
        case AVRCP_KEY_PLAY:
            TRACE(0,"avrcp_key = AVRCP_KEY_PLAY");
            btif_avrcp_set_panel_key(avrcp_channel_tmp,BTIF_AVRCP_POP_PLAY,TRUE);
            btif_avrcp_set_panel_key(avrcp_channel_tmp,BTIF_AVRCP_POP_PLAY,FALSE);
            app_bt_device.a2dp_play_pause_flag = 1;
            break;
            
        case AVRCP_KEY_PAUSE:
            TRACE(0,"avrcp_key = AVRCP_KEY_PAUSE");
            btif_avrcp_set_panel_key(avrcp_channel_tmp,BTIF_AVRCP_POP_PAUSE,TRUE);
            btif_avrcp_set_panel_key(avrcp_channel_tmp,BTIF_AVRCP_POP_PAUSE,FALSE);
            app_bt_device.a2dp_play_pause_flag = 0;
            break;
#endif
        case AVRCP_KEY_FORWARD:
            TRACE(0,"avrcp_key = AVRCP_KEY_FORWARD");
            btif_avrcp_set_panel_key(avrcp_channel_tmp,BTIF_AVRCP_POP_FORWARD,TRUE);
            btif_avrcp_set_panel_key(avrcp_channel_tmp,BTIF_AVRCP_POP_FORWARD,FALSE);
            app_bt_device.a2dp_play_pause_flag = 1;
            break;
        case AVRCP_KEY_BACKWARD:
            TRACE(0,"avrcp_key = AVRCP_KEY_BACKWARD");
            btif_avrcp_set_panel_key(avrcp_channel_tmp,BTIF_AVRCP_POP_BACKWARD,TRUE);
            btif_avrcp_set_panel_key(avrcp_channel_tmp,BTIF_AVRCP_POP_BACKWARD,FALSE);
            app_bt_device.a2dp_play_pause_flag = 1;
            break;
        case AVRCP_KEY_VOLUME_UP:
            TRACE(0,"avrcp_key = AVRCP_KEY_VOLUME_UP");
            btif_avrcp_set_panel_key(avrcp_channel_tmp,BTIF_AVRCP_POP_VOLUME_UP,TRUE);
            btif_avrcp_set_panel_key(avrcp_channel_tmp,BTIF_AVRCP_POP_VOLUME_UP,FALSE);
            break;
        case AVRCP_KEY_VOLUME_DOWN:
            TRACE(0,"avrcp_key = AVRCP_KEY_VOLUME_DOWN");
            btif_avrcp_set_panel_key(avrcp_channel_tmp,BTIF_AVRCP_POP_VOLUME_DOWN,TRUE);
            btif_avrcp_set_panel_key(avrcp_channel_tmp,BTIF_AVRCP_POP_VOLUME_DOWN,FALSE);
            break;
        default :
            break;
    }
}

//uint8_t phone_earphone_mark = 0;
#ifdef __BT_ONE_BRING_TWO__
#define HF_CHANNEL_SWITCH_HOOK_USER APP_BT_GOLBAL_HANDLE_HOOK_USER_0

enum HF_CHANNEL_SWITCH_STATUS{
    HF_CHANNEL_SWITCH_STATUS_IDLE,
    HF_CHANNEL_SWITCH_STATUS_ONPROCESS,
};

struct HF_CHANNEL_SWITCH_ENV_T{
    BT_DEVICE_ID_T old_device_id;
    hf_chan_handle_t hf_channel_old;
    BT_DEVICE_ID_T new_device_id;
    hf_chan_handle_t hf_channel_new;
    enum HF_CHANNEL_SWITCH_STATUS status;
}hf_channel_switch_env;

static void bt_key_hf_channel_switch_hook(const btif_event_t *Event);

static void bt_key_hf_channel_switch_stop(void)
{
    hf_channel_switch_env.status = HF_CHANNEL_SWITCH_STATUS_IDLE;
    hf_channel_switch_env.old_device_id = BT_DEVICE_NUM;
    hf_channel_switch_env.hf_channel_old = NULL;
    hf_channel_switch_env.new_device_id = BT_DEVICE_NUM;
    hf_channel_switch_env.hf_channel_new  = NULL;
    app_bt_global_handle_hook_set(HF_CHANNEL_SWITCH_HOOK_USER, NULL);
}

#ifdef __HF_KEEP_ONE_ALIVE__
static void bt_key_hf_channel_switch_active(BT_DEVICE_ID_T current_id, BT_DEVICE_ID_T another_id)
{
    TRACE(2,"switch_active %d-->%d", current_id, another_id);

    hf_channel_switch_env.old_device_id = current_id;
    hf_channel_switch_env.hf_channel_old = (app_bt_device.hf_channel[current_id]);

    hf_channel_switch_env.new_device_id = another_id;
    hf_channel_switch_env.hf_channel_new = (app_bt_device.hf_channel[another_id]);

    app_bt_HF_DisconnectAudioLink(hf_channel_switch_env.hf_channel_old);
    app_bt_HF_CreateAudioLink(hf_channel_switch_env.hf_channel_new);
}
#endif

static void bt_key_hf_channel_switch_start(void)
{
    uint8_t i;

    TRACE(2,"%s status:%d enter",__func__, hf_channel_switch_env.status );
    if (hf_channel_switch_env.status == HF_CHANNEL_SWITCH_STATUS_ONPROCESS){
        return;
    }

    for (i=0; i<BT_DEVICE_NUM; i++){
        if (app_bt_device.hf_voice_en[i] == HF_VOICE_ENABLE){
            break;
        }
    }

    if (i < BT_DEVICE_NUM){
        TRACE(2,"%s switch to %d",__func__, i);
        hf_channel_switch_env.status = HF_CHANNEL_SWITCH_STATUS_ONPROCESS;

        app_audio_manager_set_active_sco_num((enum BT_DEVICE_ID_T)i);
#ifdef __HFP_OK__

        hf_channel_switch_env.old_device_id = (enum BT_DEVICE_ID_T)i;
        hf_channel_switch_env.hf_channel_old = (app_bt_device.hf_channel[i]);

        hf_channel_switch_env.new_device_id = i==BT_DEVICE_ID_1 ? BT_DEVICE_ID_2 : BT_DEVICE_ID_1;

        hf_channel_switch_env.hf_channel_new = (app_bt_device.hf_channel[hf_channel_switch_env.new_device_id]);
#endif
        app_bt_HF_DisconnectAudioLink(hf_channel_switch_env.hf_channel_old);
        app_bt_global_handle_hook_set(HF_CHANNEL_SWITCH_HOOK_USER, bt_key_hf_channel_switch_hook);
    }else{
        hf_channel_switch_env.status = HF_CHANNEL_SWITCH_STATUS_IDLE;
        TRACE(1,"%s not found",__func__);
    }
}

static void bt_key_hf_channel_switch_init(void)
{
    memset(&hf_channel_switch_env, 0, sizeof(struct HF_CHANNEL_SWITCH_ENV_T));
}

static void bt_key_hf_channel_switch_hook(const btif_event_t*Event)
{
    switch ( btif_me_get_callback_event_type( Event)) {
        case BTIF_BTEVENT_LINK_DISCONNECT:
            TRACE(1,"%s DISCONNECT",__func__);
            bt_key_hf_channel_switch_stop();
            break;
        case  BTIF_BTEVENT_SCO_CONNECT_IND:
        case  BTIF_BTEVENT_SCO_CONNECT_CNF:
            TRACE(1,"%s connect another ok",__func__);
            if ((  btif_hf_cmgr_get_remote_device( hf_channel_switch_env.hf_channel_new) ==  btif_me_get_callback_event_rem_dev(Event))||
                (app_bt_device.hf_voice_en[hf_channel_switch_env.new_device_id] == HF_VOICE_ENABLE)){
                app_bt_device.curr_hf_channel_id = hf_channel_switch_env.new_device_id;
                app_bt_device.hf_voice_en[app_bt_device.curr_hf_channel_id] = HF_VOICE_ENABLE;
                app_audio_manager_set_active_sco_num(app_bt_device.curr_hf_channel_id);
                bt_key_hf_channel_switch_stop();
            }
            break;
        case  BTIF_BTEVENT_SCO_DISCONNECT:
            TRACE(1,"%s try connect another",__func__);
            app_audio_manager_set_active_sco_num(BT_DEVICE_NUM);
            app_bt_device.hf_voice_en[hf_channel_switch_env.old_device_id] = HF_VOICE_DISABLE;
            if (app_bt_device.hf_voice_en[hf_channel_switch_env.new_device_id] == HF_VOICE_DISABLE){
                app_bt_HF_CreateAudioLink(hf_channel_switch_env.hf_channel_new);
            }else{
                bt_key_hf_channel_switch_stop();
            }
            break;
    }
}
#endif

void bt_key_handle_func_click()
{
#if defined (__HSP_ENABLE__)
    if((app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1] == BTIF_HF_CALL_SETUP_NONE)&&
            (app_bt_device.hfchan_call[BT_DEVICE_ID_1] == BTIF_HF_CALL_NONE)&&
            (app_bt_device.hf_audio_state[BT_DEVICE_ID_1] == HF_AUDIO_DISCON)&&
            (app_bt_device.hs_conn_flag[app_bt_device.curr_hs_channel_id]  != 1)){
#else
     if((app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1] == BTIF_HF_CALL_SETUP_NONE)&&
            (app_bt_device.hfchan_call[BT_DEVICE_ID_1] == BTIF_HF_CALL_NONE)&&
            (app_bt_device.hf_audio_state[BT_DEVICE_ID_1] == BTIF_HF_AUDIO_DISCON)){
#endif
        if(app_bt_device.a2dp_play_pause_flag == 0){   /////if no connect?
            a2dp_handleKey(AVRCP_KEY_PLAY);
        }else{
            a2dp_handleKey(AVRCP_KEY_PAUSE);
        }
    }
    if((app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1] == BTIF_HF_CALL_SETUP_IN)&&(app_bt_device.hfchan_call[BT_DEVICE_ID_1] == BTIF_HF_CALL_NONE)){
        hfp_handle_key(HFP_KEY_ANSWER_CALL);
    }
    if(app_bt_device.hfchan_call[BT_DEVICE_ID_1] == BTIF_HF_CALL_ACTIVE){
        hfp_handle_key(HFP_KEY_HANGUP_CALL);
    }
    if((app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1] == BTIF_HF_CALL_SETUP_OUT)||(app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1] == BTIF_HF_CALL_SETUP_ALERT)){
        hfp_handle_key(HFP_KEY_HANGUP_CALL);
    }
#if defined (__HSP_ENABLE__)
    if(app_bt_device.hs_conn_flag[app_bt_device.curr_hs_channel_id]  == 1){         //now we know it is HSP active !
            hsp_handle_key(HSP_KEY_CKPD_CONTROL);
    }
#endif
#if HF_CUSTOM_FEATURE_SUPPORT & HF_CUSTOM_FEATURE_SIRI_REPORT
    //TRACE(0,"powerkey close siri");
    //app_hfp_siri_voice(false);
    open_siri_flag = 0;
#endif
}
                        
void bt_key_handle_func_doubleclick()
{
    TRACE(0,"!!!APP_KEY_EVENT_DOUBLECLICK\n");
    if((app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1] == BTIF_HF_CALL_SETUP_NONE)&&(app_bt_device.hfchan_call[BT_DEVICE_ID_1] == BTIF_HF_CALL_NONE)){
        hfp_handle_key(HFP_KEY_REDIAL_LAST_CALL);
    }
    if(app_bt_device.hf_audio_state[BT_DEVICE_ID_1] == BTIF_HF_AUDIO_CON){
        if(app_bt_device.hf_mute_flag == 0){
            hfp_handle_key(HFP_KEY_MUTE);
            app_bt_device.hf_mute_flag = 1;
        }else{
            hfp_handle_key(HFP_KEY_CLEAR_MUTE);
            app_bt_device.hf_mute_flag = 0;
        }
    }
}

void bt_key_handle_func_longpress()
{
#ifdef SUPPORT_SIRI
    open_siri_flag=0;
#endif
    if((app_bt_device.hfchan_call[BT_DEVICE_ID_1] == BTIF_HF_CALL_ACTIVE) &&
    ((app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1] == BTIF_HF_CALL_SETUP_IN))) {
#ifndef FPGA
        app_voice_report(APP_STATUS_INDICATION_WARNING, 0);
#endif
        hfp_handle_key(HFP_KEY_THREEWAY_HOLD_AND_ANSWER);
    } else  if(app_bt_device.hfchan_call[BT_DEVICE_ID_1] == BTIF_HF_CALL_ACTIVE){
        if(app_bt_device.phone_earphone_mark == 0){
            //call is active, switch from earphone to phone
            hfp_handle_key(HFP_KEY_CHANGE_TO_PHONE);
        }else if(app_bt_device.phone_earphone_mark == 1){
            //call is active, switch from phone to earphone
            hfp_handle_key(HFP_KEY_ADD_TO_EARPHONE);
        }
    } else if(app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1] == BTIF_HF_CALL_SETUP_IN){
        hfp_handle_key(HFP_KEY_HANGUP_CALL);
    }
#if defined (__HSP_ENABLE__)
    else if(app_bt_device.hs_conn_flag[app_bt_device.curr_hs_channel_id]  == 1){         //now we know it is HSP active !
        if(app_bt_device.phone_earphone_mark == 0){
            //call is active, switch from earphone to phone
            hsp_handle_key(HSP_KEY_CHANGE_TO_PHONE);
        }else if(app_bt_device.phone_earphone_mark == 1){
            //call is active, switch from phone to earphone
            hsp_handle_key(HSP_KEY_ADD_TO_EARPHONE);
        }
    }
#endif

#ifdef BTIF_HID_DEVICE
    else if((app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1] == BTIF_HF_CALL_SETUP_NONE) &&
    (app_bt_device.hfchan_call[BT_DEVICE_ID_1] == BTIF_HF_CALL_NONE)) {
        Hid_Send_capture(&app_bt_device.hid_channel[BT_DEVICE_ID_1]);
    }
#endif

#if HF_CUSTOM_FEATURE_SUPPORT & HF_CUSTOM_FEATURE_SIRI_REPORT
    else if((app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1] == BTIF_HF_CALL_SETUP_NONE) &&
    (app_bt_device.hfchan_call[BT_DEVICE_ID_1] == BTIF_HF_CALL_NONE) &&
    (open_siri_flag == 0 )){
#ifndef FPGA
        app_voice_report(APP_STATUS_INDICATION_WARNING, 0);
#endif
        open_siri_flag = 1;
    }
#endif

}


void bt_key_handle_func_key(enum APP_KEY_EVENT_T event)
{
#ifdef __BT_ONE_BRING_TWO__
    //if(g_current_device_id == BT_DEVICE_NUM)
    BT_DEVICE_ID_T current_device_id;
    BT_DEVICE_ID_T another_device_id;

    current_device_id = app_bt_device.curr_hf_channel_id;
    another_device_id = (current_device_id == BT_DEVICE_ID_1) ? BT_DEVICE_ID_2 : BT_DEVICE_ID_1;
#endif

    switch(event)
    {		
#ifndef __BT_ONE_BRING_TWO__
        case  APP_KEY_EVENT_UP:
        case  APP_KEY_EVENT_CLICK:
            bt_key_handle_func_click();
            break;
        case  APP_KEY_EVENT_DOUBLECLICK:
            bt_key_handle_func_doubleclick();
            break;
        case  APP_KEY_EVENT_LONGPRESS:
            bt_key_handle_func_longpress();
            break;
#else
        case  APP_KEY_EVENT_UP:
        case  APP_KEY_EVENT_CLICK:
            TRACE(8,"!!!APP_KEY_EVENT_CLICK callsetup %d %d call %d %d held %d %d audio_state %d %d\n",
				app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1], app_bt_device.hfchan_callSetup[BT_DEVICE_ID_2],
                app_bt_device.hfchan_call[BT_DEVICE_ID_1], app_bt_device.hfchan_call[BT_DEVICE_ID_2],
				app_bt_device.hf_callheld[BT_DEVICE_ID_1], app_bt_device.hf_callheld[BT_DEVICE_ID_2],
                app_bt_device.hf_audio_state[BT_DEVICE_ID_1], app_bt_device.hf_audio_state[BT_DEVICE_ID_2]);

            if((app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1] == BTIF_HF_CALL_SETUP_NONE)&&(app_bt_device.hfchan_call[BT_DEVICE_ID_1] ==  BTIF_HF_CALL_NONE)&&(app_bt_device.hf_audio_state[BT_DEVICE_ID_1] == BTIF_HF_AUDIO_DISCON)&&
                (app_bt_device.hfchan_callSetup[BT_DEVICE_ID_2] ==  BTIF_HF_CALL_SETUP_NONE)&&(app_bt_device.hfchan_call[BT_DEVICE_ID_2] ==  BTIF_HF_CALL_NONE)&&(app_bt_device.hf_audio_state[BT_DEVICE_ID_2] == BTIF_HF_AUDIO_DISCON)){
                if(app_bt_device.a2dp_play_pause_flag == 0){
                    a2dp_handleKey(AVRCP_KEY_PLAY);
                }else{
                    a2dp_handleKey(AVRCP_KEY_PAUSE);
                }
            }

            if(((app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1] ==  BTIF_HF_CALL_SETUP_IN)&&(app_bt_device.hfchan_call[BT_DEVICE_ID_1] ==  BTIF_HF_CALL_NONE)&&(app_bt_device.hfchan_call[BT_DEVICE_ID_2] ==  BTIF_HF_CALL_NONE)&&(app_bt_device.hfchan_callSetup[BT_DEVICE_ID_2] !=  BTIF_HF_CALL_SETUP_ALERT))) {
                //hfp_handle_key(HFP_KEY_ANSWER_CALL);
                /* (A incoming && B no active) */
                TRACE(0,"!!!!answer call hf_channel=0\n");
                btif_hf_answer_call(app_bt_device.hf_channel[BT_DEVICE_ID_1]);
            } else if ((app_bt_device.hfchan_callSetup[BT_DEVICE_ID_2] ==  BTIF_HF_CALL_SETUP_IN)&&(app_bt_device.hfchan_call[BT_DEVICE_ID_2] ==  BTIF_HF_CALL_NONE)&&(app_bt_device.hfchan_call[BT_DEVICE_ID_1] ==  BTIF_HF_CALL_NONE)&&(app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1] !=  BTIF_HF_CALL_SETUP_ALERT)) {
                //hfp_handle_key(HFP_KEY_ANSWER_CALL);
                /* B incoming && A no active */
                TRACE(0,"!!!!answer call hf_channel=1\n");
                btif_hf_answer_call(app_bt_device.hf_channel[BT_DEVICE_ID_2]);
            }

            if (((app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1] ==  BTIF_HF_CALL_SETUP_OUT)||(app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1] == BTIF_HF_CALL_SETUP_ALERT))&&(app_bt_device.hfchan_call[BT_DEVICE_ID_2] == BTIF_HF_CALL_NONE)&&(app_bt_device.hfchan_callSetup[BT_DEVICE_ID_2] ==  BTIF_HF_CALL_SETUP_NONE)) {
                TRACE(0,"!!!!call out hangup hf_channel=0\n");
                //hfp_handle_key(HFP_KEY_HANGUP_CALL);
                /* (A outgoing && B no active no setup */
                btif_hf_hang_up_call(app_bt_device.hf_channel[BT_DEVICE_ID_1]);
            } else if (((app_bt_device.hfchan_callSetup[BT_DEVICE_ID_2] ==  BTIF_HF_CALL_SETUP_OUT)||(app_bt_device.hfchan_callSetup[BT_DEVICE_ID_2] == BTIF_HF_CALL_SETUP_ALERT))&&(app_bt_device.hfchan_call[BT_DEVICE_ID_1] == BTIF_HF_CALL_NONE)&&(app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1] ==  BTIF_HF_CALL_SETUP_NONE)) {
                TRACE(0,"!!!!call out hangup hf_channel=1\n");
                //hfp_handle_key(HFP_KEY_HANGUP_CALL);
                /* B outgoing && A no active no setup */
                btif_hf_hang_up_call(app_bt_device.hf_channel[BT_DEVICE_ID_2]);
            }

            if(app_bt_device.hfchan_call[BT_DEVICE_ID_1] == BTIF_HF_CALL_ACTIVE){
                if((app_bt_device.hfchan_call[BT_DEVICE_ID_2] == BTIF_HF_CALL_NONE) &&
                            (app_bt_device.hfchan_callSetup[BT_DEVICE_ID_2] == BTIF_HF_CALL_SETUP_NONE)) {
                    if (app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1] == BTIF_HF_CALL_SETUP_IN) {
                        /* A 1-call active 1-call incoming && B on active no setup */
                        TRACE(0,"!!!!release active calls hf_channel=0\n");
                        btif_hf_call_hold(app_bt_device.hf_channel[BT_DEVICE_ID_1], BTIF_HF_HOLD_RELEASE_ACTIVE_CALLS, 0);
                    } else {
                        /* A active && B no active no setup */
                        //hfp_handle_key(HFP_KEY_HANGUP_CALL);
                        TRACE(0,"!!!!hangup call hf_channel=0\n");
                        if (app_bt_device.hf_callheld[BT_DEVICE_ID_1] == BTIF_HF_CALL_HELD_NO_ACTIVE) {
                            btif_hf_call_hold(app_bt_device.hf_channel[BT_DEVICE_ID_1], BTIF_HF_HOLD_HOLD_ACTIVE_CALLS, 0);
                        }
                        btif_hf_hang_up_call(app_bt_device.hf_channel[BT_DEVICE_ID_1]);
                    }
                }else if(app_bt_device.hfchan_callSetup[BT_DEVICE_ID_2] == BTIF_HF_CALL_SETUP_IN){
                    //hfp_handle_key(HFP_KEY_DUAL_HF_HANGUP_CURR_ANSWER_ANOTHER);
                    /* A active && B incoming */
                    TRACE(0,"!!!!1hangup active and answer incoming\n");
                    btif_hf_hang_up_call(app_bt_device.hf_channel[BT_DEVICE_ID_1]);
                    btif_hf_answer_call(app_bt_device.hf_channel[BT_DEVICE_ID_2]);
                }
            }else if(app_bt_device.hfchan_call[BT_DEVICE_ID_2] == BTIF_HF_CALL_ACTIVE){
                if((app_bt_device.hfchan_call[BT_DEVICE_ID_1] == BTIF_HF_CALL_NONE) &&
                            (app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1] == BTIF_HF_CALL_SETUP_NONE)) {
                    if (app_bt_device.hfchan_callSetup[BT_DEVICE_ID_2] == BTIF_HF_CALL_SETUP_IN) {
                        /* B 1-call active 1-call incoming && A on active no setup */
                        TRACE(0,"!!!!release active calls hf_channel=1\n");
                        btif_hf_call_hold(app_bt_device.hf_channel[BT_DEVICE_ID_2], BTIF_HF_HOLD_RELEASE_ACTIVE_CALLS, 0);
                    } else {
                        /* B active && A no active no setup */
                        //hfp_handle_key(HFP_KEY_HANGUP_CALL);
                        TRACE(0,"!!!!hangup call hf_channel=1\n");
                        if (app_bt_device.hf_callheld[BT_DEVICE_ID_2] == BTIF_HF_CALL_HELD_NO_ACTIVE) {
                            btif_hf_call_hold(app_bt_device.hf_channel[BT_DEVICE_ID_2], BTIF_HF_HOLD_HOLD_ACTIVE_CALLS, 0);
                        }
                        btif_hf_hang_up_call(app_bt_device.hf_channel[BT_DEVICE_ID_2]);
                    }
                }else if(app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1] == BTIF_HF_CALL_SETUP_IN){
                    //hfp_handle_key(HFP_KEY_DUAL_HF_HANGUP_CURR_ANSWER_ANOTHER);
                    /* B active && A incoming */
                    TRACE(0,"!!!!2hangup active and answer incoming\n");
                    btif_hf_hang_up_call(app_bt_device.hf_channel[BT_DEVICE_ID_2]);
                    btif_hf_answer_call(app_bt_device.hf_channel[BT_DEVICE_ID_1]);
                }
            }

            /* A active && B active */
            if((app_bt_device.hfchan_call[BT_DEVICE_ID_1] == BTIF_HF_CALL_ACTIVE) &&
                    (app_bt_device.hfchan_call[BT_DEVICE_ID_2] == BTIF_HF_CALL_ACTIVE)){
                TRACE(0,"!!!two call both active\n");
                hfp_handle_key(HFP_KEY_HANGUP_CALL);
            }
#ifdef SUPPORT_SIRI
            //TRACE(0,"powerkey close siri");
            //app_hfp_siri_voice(false);
            open_siri_flag = 0;
#endif

            break;
        case  APP_KEY_EVENT_DOUBLECLICK:
		    TRACE(8,"!!!APP_KEY_EVENT_DOUBLECLICK callsetup %d %d call %d %d held %d %d audio_state %d %d\n",
				app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1], app_bt_device.hfchan_callSetup[BT_DEVICE_ID_2],
                app_bt_device.hfchan_call[BT_DEVICE_ID_1], app_bt_device.hfchan_call[BT_DEVICE_ID_2],
				app_bt_device.hf_callheld[BT_DEVICE_ID_1], app_bt_device.hf_callheld[BT_DEVICE_ID_2],
                app_bt_device.hf_audio_state[BT_DEVICE_ID_1], app_bt_device.hf_audio_state[BT_DEVICE_ID_2]);

            //if(((app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1] == BTIF_HF_CALL_SETUP_NONE)&&(app_bt_device.hfchan_call[BT_DEVICE_ID_1] == BTIF_HF_CALL_NONE)&&(app_bt_device.hf_channel[BT_DEVICE_ID_2].state == HF_STATE_CLOSED))||
            if(((app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1] == BTIF_HF_CALL_SETUP_NONE)&&(app_bt_device.hfchan_call[BT_DEVICE_ID_1] == BTIF_HF_CALL_NONE)&&
                        (btif_get_hf_chan_state(app_bt_device.hf_channel[BT_DEVICE_ID_2]) == BTIF_HF_STATE_CLOSED))||
                ((app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1] == BTIF_HF_CALL_SETUP_NONE)&&(app_bt_device.hfchan_call[BT_DEVICE_ID_1] == BTIF_HF_CALL_NONE)&&(app_bt_device.hfchan_callSetup[BT_DEVICE_ID_2] == BTIF_HF_CALL_SETUP_NONE)&&(app_bt_device.hfchan_call[BT_DEVICE_ID_2] == BTIF_HF_CALL_NONE))){
                hfp_handle_key(HFP_KEY_REDIAL_LAST_CALL);
            }

            if(((app_bt_device.hf_audio_state[BT_DEVICE_ID_1] == BTIF_HF_AUDIO_CON)&&(app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1] == BTIF_HF_CALL_SETUP_NONE)&&(app_bt_device.hfchan_callSetup[BT_DEVICE_ID_2] == BTIF_HF_CALL_SETUP_NONE))||
                ((app_bt_device.hf_audio_state[BT_DEVICE_ID_2] == BTIF_HF_AUDIO_CON)&&(app_bt_device.hfchan_callSetup[BT_DEVICE_ID_2] == BTIF_HF_CALL_SETUP_NONE)&&(app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1] == BTIF_HF_CALL_SETUP_NONE))){
                if(app_bt_device.hf_mute_flag == 0){
                    hfp_handle_key(HFP_KEY_MUTE);
                }else{
                    hfp_handle_key(HFP_KEY_CLEAR_MUTE);
                }
            }

            if((app_bt_device.hfchan_call[BT_DEVICE_ID_1] == BTIF_HF_CALL_ACTIVE)&&(app_bt_device.hfchan_callSetup[BT_DEVICE_ID_2] == BTIF_HF_CALL_SETUP_IN)){
                /* A active && B incoming */
                //hfp_handle_key(HFP_KEY_DUAL_HF_HANGUP_ANOTHER);
                TRACE(0,"!!!!1keep active and reject incoming\n");
                btif_hf_hang_up_call(app_bt_device.hf_channel[BT_DEVICE_ID_2]);
            } else if ((app_bt_device.hfchan_call[BT_DEVICE_ID_2] == BTIF_HF_CALL_ACTIVE)&&(app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1] == BTIF_HF_CALL_SETUP_IN)){
                /* B active && A incoming */
                //hfp_handle_key(HFP_KEY_DUAL_HF_HANGUP_ANOTHER);
                TRACE(0,"!!!!2keep active and reject incoming\n");
                btif_hf_hang_up_call(app_bt_device.hf_channel[BT_DEVICE_ID_1]);
            } else if ((app_bt_device.hfchan_call[BT_DEVICE_ID_1] == BTIF_HF_CALL_ACTIVE)&&(app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1] == BTIF_HF_CALL_SETUP_IN)) {
                /* A 1-call active and 1-call incoming */
                TRACE(0,"!!!!release incoming call hf_channel=0\n");
                btif_hf_call_hold(app_bt_device.hf_channel[BT_DEVICE_ID_1], BTIF_HF_HOLD_RELEASE_HELD_CALLS, 0);
            } else if ((app_bt_device.hfchan_call[BT_DEVICE_ID_2] == BTIF_HF_CALL_ACTIVE)&&(app_bt_device.hfchan_callSetup[BT_DEVICE_ID_2] == BTIF_HF_CALL_SETUP_IN)) {
                /* B 1-call active and 1-call incoming */
                TRACE(0,"!!!!release incoming call hf_channel=1\n");
                btif_hf_call_hold(app_bt_device.hf_channel[BT_DEVICE_ID_2], BTIF_HF_HOLD_RELEASE_HELD_CALLS, 0);
            }

            break;
        case  APP_KEY_EVENT_LONGPRESS:
#ifdef SUPPORT_SIRI
            open_siri_flag=0;
#endif



//one bring two: long press switch sco
		    TRACE(8,"!!!APP_KEY_EVENT_LONGPRESS callsetup %d %d call %d %d held %d %d audio_state %d %d\n",
				app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1], app_bt_device.hfchan_callSetup[BT_DEVICE_ID_2],
                app_bt_device.hfchan_call[BT_DEVICE_ID_1], app_bt_device.hfchan_call[BT_DEVICE_ID_2],
				app_bt_device.hf_callheld[BT_DEVICE_ID_1], app_bt_device.hf_callheld[BT_DEVICE_ID_2],
                app_bt_device.hf_audio_state[BT_DEVICE_ID_1], app_bt_device.hf_audio_state[BT_DEVICE_ID_2]);

            TRACE(6,"app_bt_device.curr_hf_channel_id=%d, g_curr=%d curr=%d another:%d scoHciHandle:%x/%x",  app_bt_device.curr_hf_channel_id,current_device_id,
                                                                                  current_device_id, another_device_id,
                                                                                  //app_bt_device.hf_channel[current_device_id].cmgrHandler.scoConnect->scoHciHandle
                                                                                  //,app_bt_device.hf_channel[another_device_id].cmgrHandler.scoConnect->scoHciHandle);
                                                                                   btif_hf_get_sco_hcihandle(app_bt_device.hf_channel[current_device_id]),
                                                                                   btif_hf_get_sco_hcihandle(app_bt_device.hf_channel[another_device_id]));
              if((app_bt_device.hfchan_call[current_device_id] == BTIF_HF_CALL_ACTIVE)&&(app_bt_device.hfchan_call[another_device_id] != BTIF_HF_CALL_ACTIVE)
                &&(app_bt_device.hfchan_callSetup[another_device_id] == BTIF_HF_CALL_SETUP_IN)){//A is active, B is incoming call
                TRACE(2,"HFP_KEY_DUAL_HF_HOLD_CURR_ANSWER_ANOTHER: current=%d, g_current_device_id=%d",app_bt_device.curr_hf_channel_id, current_device_id);
#ifndef FPGA
                app_voice_report(APP_STATUS_INDICATION_WARNING, 0);
#endif
                //hfp_handle_key(HFP_KEY_DUAL_HF_HOLD_CURR_ANSWER_ANOTHER); //hold and answer
                //app_bt_device.curr_hf_channel_id = another_device_id;
                btif_hf_answer_call(app_bt_device.hf_channel[another_device_id]);
                btif_hf_call_hold(app_bt_device.hf_channel[current_device_id], BTIF_HF_HOLD_HOLD_ACTIVE_CALLS, 0);
            } else if ((app_bt_device.hfchan_call[another_device_id] == BTIF_HF_CALL_ACTIVE) &&
                     (app_bt_device.hfchan_call[current_device_id] != BTIF_HF_CALL_ACTIVE) &&
                     (app_bt_device.hfchan_callSetup[current_device_id] == BTIF_HF_CALL_SETUP_IN))
            {
                TRACE(2,"HFP_KEY_DUAL_HF_HOLD_CURR_ANSWER_ANOTHER: current=%d, g_current_device_id=%d",app_bt_device.curr_hf_channel_id, current_device_id);
#ifndef FPGA
                app_voice_report(APP_STATUS_INDICATION_WARNING, 0);
#endif
                btif_hf_answer_call(app_bt_device.hf_channel[current_device_id]);
                btif_hf_call_hold(app_bt_device.hf_channel[another_device_id], BTIF_HF_HOLD_HOLD_ACTIVE_CALLS, 0);
            } else if (app_bt_device.hfchan_call[current_device_id] == BTIF_HF_CALL_ACTIVE &&
                     app_bt_device.hf_callheld[current_device_id] == BTIF_HF_CALL_HELD_NONE &&
                     app_bt_device.hf_callheld[another_device_id] == BTIF_HF_CALL_HELD_NO_ACTIVE)
            {
                TRACE(0,"!!!!1switch hold call and active call\n");
#ifndef FPGA
                app_voice_report(APP_STATUS_INDICATION_WARNING, 0);
#endif
                btif_hf_call_hold(app_bt_device.hf_channel[current_device_id], BTIF_HF_HOLD_HOLD_ACTIVE_CALLS, 0);
                btif_hf_call_hold(app_bt_device.hf_channel[another_device_id], BTIF_HF_HOLD_HOLD_ACTIVE_CALLS, 0);
            } else if (app_bt_device.hfchan_call[another_device_id] == BTIF_HF_CALL_ACTIVE &&
                     app_bt_device.hf_callheld[another_device_id] == BTIF_HF_CALL_HELD_NONE &&
                     app_bt_device.hf_callheld[current_device_id] == BTIF_HF_CALL_HELD_NO_ACTIVE)
            {
                TRACE(0,"!!!!2switch hold call and active call\n");
#ifndef FPGA
                app_voice_report(APP_STATUS_INDICATION_WARNING, 0);
#endif
                btif_hf_call_hold(app_bt_device.hf_channel[another_device_id], BTIF_HF_HOLD_HOLD_ACTIVE_CALLS, 0);
                btif_hf_call_hold(app_bt_device.hf_channel[current_device_id], BTIF_HF_HOLD_HOLD_ACTIVE_CALLS, 0);
            } else if((app_bt_device.hfchan_call[current_device_id] == BTIF_HF_CALL_ACTIVE)&&(app_bt_device.hfchan_call[another_device_id] == BTIF_HF_CALL_ACTIVE)
                &&(app_bt_device.hfchan_callSetup[current_device_id] == BTIF_HF_CALL_SETUP_NONE)&&(app_bt_device.hfchan_callSetup[another_device_id] == BTIF_HF_CALL_SETUP_NONE)){//A is active, B is active
                TRACE(2,"AB is active: current=%d, g_current_device_id=%d",app_bt_device.curr_hf_channel_id, current_device_id);
#ifndef FPGA
                app_voice_report(APP_STATUS_INDICATION_WARNING, 0);
#endif
                if (bt_get_sco_number()>1){
#ifdef __HF_KEEP_ONE_ALIVE__
                    bt_key_hf_channel_switch_active(current_device_id, another_device_id);
#else
                    app_audio_manager_sendrequest(APP_BT_STREAM_MANAGER_SWAP_SCO,BT_STREAM_SBC, another_device_id, 0);
#endif
                    app_bt_device.hf_voice_en[current_device_id] = HF_VOICE_DISABLE;
                    app_bt_device.hf_voice_en[another_device_id] = HF_VOICE_ENABLE;
                    app_bt_device.curr_hf_channel_id = another_device_id;
                }else{
                    bt_key_hf_channel_switch_start();
                }
            } else if((app_bt_device.hfchan_call[current_device_id] == BTIF_HF_CALL_ACTIVE)&&(app_bt_device.hfchan_call[another_device_id] == BTIF_HF_CALL_ACTIVE)
                &&((app_bt_device.hfchan_callSetup[current_device_id] == BTIF_HF_CALL_SETUP_IN)||(app_bt_device.hfchan_callSetup[another_device_id] == BTIF_HF_CALL_SETUP_IN))){
                TRACE(2,"AB is active and incoming call: current=%d, g_current_device_id=%d",app_bt_device.curr_hf_channel_id, current_device_id);
#ifndef FPGA
                app_voice_report(APP_STATUS_INDICATION_WARNING, 0);
#endif
                if (bt_get_sco_number()>1){
#ifdef __HF_KEEP_ONE_ALIVE__
                    bt_key_hf_channel_switch_active(current_device_id, another_device_id);
#else
                    app_audio_manager_sendrequest(APP_BT_STREAM_MANAGER_SWAP_SCO, BT_STREAM_SBC, another_device_id, 0);
#endif
                    app_bt_device.hf_voice_en[current_device_id] = HF_VOICE_DISABLE;
                    app_bt_device.hf_voice_en[another_device_id] = HF_VOICE_ENABLE;
                    app_bt_device.curr_hf_channel_id = another_device_id;
                }else{
                    bt_key_hf_channel_switch_start();
                }
            } else if(((app_bt_device.hfchan_call[BT_DEVICE_ID_1] == BTIF_HF_CALL_ACTIVE)&&(app_bt_device.hfchan_callSetup[BT_DEVICE_ID_2] == BTIF_HF_CALL_SETUP_NONE))||
                ((app_bt_device.hfchan_call[BT_DEVICE_ID_2] == BTIF_HF_CALL_ACTIVE)&&(app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1] == BTIF_HF_CALL_SETUP_NONE))){
//three call
                TRACE(0,"three way call");
#ifndef FPGA
                app_voice_report(APP_STATUS_INDICATION_WARNING, 0);
#endif
                hfp_handle_key(HFP_KEY_THREEWAY_HOLD_AND_ANSWER);
#if 0
                if(app_bt_device.phone_earphone_mark == 0){
                    hfp_handle_key(HFP_KEY_CHANGE_TO_PHONE);
                }else if(app_bt_device.phone_earphone_mark == 1){
                    hfp_handle_key(HFP_KEY_ADD_TO_EARPHONE);
                }
#endif
            } else if(((app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1] == BTIF_HF_CALL_SETUP_IN)&&(app_bt_device.hfchan_call[BT_DEVICE_ID_2] == BTIF_HF_CALL_NONE))||
                ((app_bt_device.hfchan_callSetup[BT_DEVICE_ID_2] == BTIF_HF_CALL_SETUP_IN)&&(app_bt_device.hfchan_call[BT_DEVICE_ID_1] == BTIF_HF_CALL_NONE))){
                hfp_handle_key(HFP_KEY_HANGUP_CALL);
            }
#ifdef BTIF_HID_DEVICE
            else if(((app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1] == BTIF_HF_CALL_SETUP_NONE)&&(app_bt_device.hfchan_call[BT_DEVICE_ID_1] == BTIF_HF_CALL_NONE)&&( btif_get_hf_chan_state(app_bt_device.hf_channel[BT_DEVICE_ID_2])== BTIF_HF_STATE_CLOSED))||
                ((app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1] == BTIF_HF_CALL_SETUP_NONE)&&(app_bt_device.hfchan_call[BT_DEVICE_ID_1] == BTIF_HF_CALL_NONE)&&(app_bt_device.hfchan_callSetup[BT_DEVICE_ID_2] == BTIF_HF_CALL_SETUP_NONE)&&(app_bt_device.hfchan_call[BT_DEVICE_ID_2] == BTIF_HF_CALL_NONE))){

                Hid_Send_capture(&app_bt_device.hid_channel[BT_DEVICE_ID_1]);
            }
#endif
#ifdef SUPPORT_SIRI
            else if((open_siri_flag == 0) &&
                    (((app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1] == BTIF_HF_CALL_SETUP_NONE)&&(app_bt_device.hfchan_call[BT_DEVICE_ID_1] == BTIF_HF_CALL_NONE)&&(btif_get_hf_chan_state(app_bt_device.hf_channel[BT_DEVICE_ID_2]) == BTIF_HF_STATE_CLOSED))||
                    //(((app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1] == BTIF_HF_CALL_SETUP_NONE)&&(app_bt_device.hfchan_call[BT_DEVICE_ID_1] == BTIF_HF_CALL_NONE)&&(app_bt_device.hf_channel[BT_DEVICE_ID_2].state == BTIF_HF_STATE_CLOSED))||
                    ((app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1] == BTIF_HF_CALL_SETUP_NONE)&&(app_bt_device.hfchan_call[BT_DEVICE_ID_1] == BTIF_HF_CALL_NONE)&&(app_bt_device.hfchan_callSetup[BT_DEVICE_ID_2] == BTIF_HF_CALL_SETUP_NONE)&&(app_bt_device.hfchan_call[BT_DEVICE_ID_2] == BTIF_HF_CALL_NONE)))){
#ifndef FPGA
                    app_voice_report(APP_STATUS_INDICATION_WARNING, 0);
#endif
                    open_siri_flag = 1;
            }
#endif



 #if 0
            if((app_bt_device.hfchan_call[BT_DEVICE_ID_1] == BTIF_HF_CALL_ACTIVE)&&(app_bt_device.hfchan_callSetup[BT_DEVICE_ID_2] == BTIF_HF_CALL_SETUP_IN)){
                hfp_handle_key(HFP_KEY_DUAL_HF_HOLD_CURR_ANSWER_ANOTHER);
            }else if((app_bt_device.hfchan_call[BT_DEVICE_ID_2] == BTIF_HF_CALL_ACTIVE)&&(app_bt_device.hfchan_callSetup[BT_DEVICE_ID_1] == BTIF_HF_CALL_SETUP_IN)){
                hfp_handle_key(HFP_KEY_DUAL_HF_HOLD_CURR_ANSWER_ANOTHER);
            }
#endif
            break;
#endif/* __BT_ONE_BRING_TWO__ */
        default:
            TRACE(1,"unregister func key event=%x",event);
            break;
    }
}

void app_bt_volumeup()
{
    app_audio_manager_ctrl_volume(APP_AUDIO_MANAGER_VOLUME_CTRL_UP, 0);
}


void app_bt_volumedown()
{
    app_audio_manager_ctrl_volume(APP_AUDIO_MANAGER_VOLUME_CTRL_DOWN, 0);
}

#if defined(__APP_KEY_FN_STYLE_A__)
void bt_key_handle_up_key(enum APP_KEY_EVENT_T event)
{
#if defined(APP_LINEIN_A2DP_SOURCE)||defined(APP_I2S_A2DP_SOURCE)
	struct nvrecord_env_t *nvrecord_env=NULL;
#endif
    switch(event)
    {
        case  APP_KEY_EVENT_UP:
        case  APP_KEY_EVENT_CLICK:
            app_bt_volumeup();
            break;
        case  APP_KEY_EVENT_LONGPRESS:
            a2dp_handleKey(AVRCP_KEY_FORWARD);
            break;
#if defined(APP_LINEIN_A2DP_SOURCE)||defined(APP_I2S_A2DP_SOURCE)
		case  APP_KEY_EVENT_DOUBLECLICK:
			//debug switch src mode
			nv_record_env_get(&nvrecord_env);
			if(app_bt_device.src_or_snk==BT_DEVICE_SRC)
			{
				nvrecord_env->src_snk_flag.src_snk_mode =BT_DEVICE_SNK;
			}
			else
			{
				nvrecord_env->src_snk_flag.src_snk_mode =BT_DEVICE_SRC;
			}
			nv_record_env_set(nvrecord_env);
			app_reset();
			break;
#endif
        default:
            TRACE(1,"unregister up key event=%x",event);
            break;
    }
}


void bt_key_handle_down_key(enum APP_KEY_EVENT_T event)
{
    switch(event)
    {
        case  APP_KEY_EVENT_UP:
        case  APP_KEY_EVENT_CLICK:
            app_bt_volumedown();
            break;
        case  APP_KEY_EVENT_LONGPRESS:
            a2dp_handleKey(AVRCP_KEY_BACKWARD);

            break;
        default:
            TRACE(1,"unregister down key event=%x",event);
            break;
    }
}
#else //#elif defined(__APP_KEY_FN_STYLE_B__)
void bt_key_handle_up_key(enum APP_KEY_EVENT_T event)
{
    TRACE(1,"%s",__func__);
    switch(event)
    {
        case  APP_KEY_EVENT_REPEAT:
            app_bt_volumeup();
            break;
        case  APP_KEY_EVENT_UP:
        case  APP_KEY_EVENT_CLICK:
            a2dp_handleKey(AVRCP_KEY_FORWARD);
            break;
        default:
            TRACE(1,"unregister up key event=%x",event);
            break;
    }
}


void bt_key_handle_down_key(enum APP_KEY_EVENT_T event)
{
    switch(event)
    {
        case  APP_KEY_EVENT_REPEAT:
            app_bt_volumedown();
            break;
        case  APP_KEY_EVENT_UP:
        case  APP_KEY_EVENT_CLICK:
            a2dp_handleKey(AVRCP_KEY_BACKWARD);
            break;
        default:
            TRACE(1,"unregister down key event=%x",event);
            break;
    }
}
#endif

#if defined(APP_LINEIN_A2DP_SOURCE)||defined(APP_I2S_A2DP_SOURCE)
void bt_key_handle_source_func_key(enum APP_KEY_EVENT_T event)
{
    TRACE(2,"%s,%d",__FUNCTION__,event);
    static bool onaudioloop = false;
    switch(event)
    {
        case  APP_KEY_EVENT_UP:
        case  APP_KEY_EVENT_CLICK:
            app_a2dp_source_find_sink();
            break;
        case APP_KEY_EVENT_DOUBLECLICK:
			if(app_bt_device.a2dp_state[0]==1)
			{
				 onaudioloop = onaudioloop?false:true;
				if (onaudioloop)
				{
					app_a2dp_start_stream();

				}
				else
				{
					app_a2dp_suspend_stream();
				}
			}
			break;
        case APP_KEY_EVENT_TRIPLECLICK:
            app_a2dp_start_stream();
            break;
        default:
            TRACE(1,"unregister down key event=%x",event);
            break;
    }
}
#endif

APP_KEY_STATUS bt_key;
static void bt_update_key_event(uint32_t code, uint8_t event)
{
    TRACE(3,"%s code:%d evt:%d",__func__, code, event);

    bt_key.code = code;
    bt_key.event = event;
    osapi_notify_evm();
}

void bt_key_send(APP_KEY_STATUS *status)
{
    uint32_t lock = int_lock();
    bool isKeyBusy = false;
    if (0xff != bt_key.code)
    {
        isKeyBusy = true;
    }
    int_unlock(lock);

    if (!isKeyBusy)
    {
        app_bt_start_custom_function_in_bt_thread(
            (uint32_t)status->code, 
            (uint32_t)status->event,
            (uint32_t)bt_update_key_event);
    }
}

void bt_key_handle(void)
{
    osapi_lock_stack();
    if(bt_key.code != 0xff)
    {
        TRACE(3,"%s code:%d evt:%d",__func__, bt_key.code, bt_key.event);
        switch(bt_key.code)
        {
            case BTAPP_FUNC_KEY:
#if defined(APP_LINEIN_A2DP_SOURCE)||defined(APP_I2S_A2DP_SOURCE)
				if(app_bt_device.src_or_snk==BT_DEVICE_SRC)
				{
					bt_key_handle_source_func_key((enum APP_KEY_EVENT_T)bt_key.event);
				}
				else
#endif
				{
					bt_key_handle_func_key((enum APP_KEY_EVENT_T)bt_key.event);
				}
                break;
            case BTAPP_VOLUME_UP_KEY:
                bt_key_handle_up_key((enum APP_KEY_EVENT_T)bt_key.event);
                break;
            case BTAPP_VOLUME_DOWN_KEY:
                bt_key_handle_down_key((enum APP_KEY_EVENT_T)bt_key.event);
                break;
#ifdef SUPPORT_SIRI
            case BTAPP_RELEASE_KEY:
                bt_key_handle_siri_key((enum APP_KEY_EVENT_T)bt_key.event);
                break;
#endif
            default:
                TRACE(0,"bt_key_handle  undefined key");
                break;
        }
        bt_key.code = 0xff;
    }
    osapi_unlock_stack();
}

void bt_key_init(void)
{
    Besbt_hook_handler_set(BESBT_HOOK_USER_2, bt_key_handle);
#ifdef __BT_ONE_BRING_TWO__
    bt_key_hf_channel_switch_init();
#endif
    bt_key.code = 0xff;
    bt_key.event = 0xff;
}

