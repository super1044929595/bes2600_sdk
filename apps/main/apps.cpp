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

#include "stdio.h"
#include "cmsis_os.h"
#include "list.h"
#include "string.h"

#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_bootmode.h"

#include "audioflinger.h"
#include "apps.h"
#include "app_thread.h"
#include "app_key.h"
#ifdef __PC_CMD_UART__
#include "app_cmd.h"
#endif
#include "app_pwl.h"
#include "app_audio.h"
#include "app_overlay.h"
#include "app_battery.h"
#include "app_utils.h"
#include "app_status_ind.h"
#ifdef __FACTORY_MODE_SUPPORT__
#include "app_factory.h"
#include "app_factory_bt.h"
#endif
#include "bt_drv_interface.h"
#include "besbt.h"
#include "norflash_api.h"
#include "nvrecord.h"
#include "nvrecord_dev.h"
#include "nvrecord_env.h"
#include "crash_dump_section.h"
#include "log_section.h"
#include "factory_section.h"

#include "a2dp_api.h"
#include "me_api.h"
#include "os_api.h"
#include "btapp.h"
#include "app_bt.h"
#include "app_ble_include.h"
#include "bt_if.h"
#ifdef __INTERACTION__
#include "app_interaction.h"
#endif
#ifdef GSOUND_OTA_ENABLED
#include "gsound_ota.h"
#endif

#ifdef BES_OTA_BASIC
#include "ota_bes.h"
#endif
#include "pmu.h"
#ifdef MEDIA_PLAYER_SUPPORT
#include "resources.h"
#include "app_media_player.h"
#endif
#include "app_bt_media_manager.h"
#include "hal_sleep.h"
#ifdef VOICE_DATAPATH
#include "app_voicepath.h"
#endif
#ifdef BT_USB_AUDIO_DUAL_MODE
#include "btusb_audio.h"
#include "usbaudio_thread.h"
#endif

#if defined(IBRT)
#include "app_ibrt_if.h"
#include "app_ibrt_customif_ui.h"
#include "app_ibrt_ui_test.h"
#include "app_ibrt_voice_report.h"
#include "app_ibrt_rssi.h"
#endif

#ifdef APP_TRACE_RX_ENABLE
#include "app_trace_rx.h"
#endif

#ifdef GFPS_ENABLED
#include "app_gfps.h"
#endif

#ifdef TILE_DATAPATH
#include "tile_target_ble.h"
#endif

#ifdef PROMPT_IN_FLASH
#include "nvrecord_prompt.h"
#endif

#ifdef ANC_APP
#include "app_anc.h"
#endif

#ifdef AUDIO_DEBUG
extern "C" int speech_tuning_init(void);
#endif

#if defined(BTUSB_AUDIO_MODE)
extern "C"  bool app_usbaudio_mode_on(void) ;
#endif

#ifdef BES_OTA_BASIC
extern "C" void ota_flash_init(void);
#endif

#ifdef BTIF_BLE_APP_DATAPATH_SERVER
#include "app_ble_cmd_handler.h"
#endif

#if defined(ANC_ASSIST_ENABLED)
#include "app_anc_assist.h"
#include "app_voice_assist_ai_voice.h"
#if defined(ANC_ASSIST_USE_PILOT)
#include "app_voice_assist_wd.h"
#include "app_voice_assist_pilot_anc.h"
#include "app_voice_assist_ultrasound.h"
#include "app_voice_assist_custom_leak_detect.h"
#endif
#include "app_voice_assist_prompt_leak_detect.h"
#if defined(AUDIO_ADAPTIVE_EQ)
#include "app_voice_assist_adaptive_eq.h"
#endif
#include "app_voice_assist_noise_adapt_anc.h"
#endif

#ifdef AUDIO_OUTPUT_DC_AUTO_CALIB
#include "codec_calib.h"
#endif

#ifdef ANC_APP
#include "app_anc.h"
#endif

#ifdef __AI_VOICE__
#include "ai_manager.h"
#include "ai_thread.h"
#include "ai_control.h"
#include "ai_spp.h"
#include "app_ai_voice.h"
#endif

#include "gapm_task.h"
#include "app_ble_mode_switch.h"

#ifdef __THIRDPARTY
#include "app_thirdparty.h"
#endif
#include "app_ibrt_rssi.h"
#include "watchdog/watchdog.h"
#include "app_tota.h"

#if defined(__STDF__)
#include "stdf.h"
#endif

#ifdef APP_TRACE_RX_ENABLE
extern "C"  void app_trace_rx_open(void);
#endif
#define APP_BATTERY_LEVEL_LOWPOWERTHRESHOLD (1)

#ifdef RB_CODEC
extern int rb_ctl_init();
extern bool rb_ctl_is_init_done(void);
extern void app_rbplay_audio_reset_pause_status(void);
#endif

#ifdef __ANC_STICK_SWITCH_USE_GPIO__
extern bool app_anc_switch_get_val(void);
#endif

#ifdef GFPS_ENABLED
extern "C" void app_fast_pairing_timeout_timehandler(void);
#endif

extern "C" void gsound_dump_set_flag(uint8_t is_happend);

uint8_t  app_poweroff_flag = 0;

#ifndef APP_TEST_MODE
static uint8_t app_status_indication_init(void)
{
    struct APP_PWL_CFG_T cfg;
    memset(&cfg, 0, sizeof(struct APP_PWL_CFG_T));
    app_pwl_open();
    app_pwl_setup(APP_PWL_ID_0, &cfg);
    app_pwl_setup(APP_PWL_ID_1, &cfg);
    return 0;
}
#endif

#if defined(__BTIF_EARPHONE__) && defined(__BTIF_AUTOPOWEROFF__)

void PairingTransferToConnectable(void);

typedef void (*APP_10_SECOND_TIMER_CB_T)(void);

void app_pair_timerout(void);
void app_poweroff_timerout(void);
void CloseEarphone(void);

typedef struct
{
    uint8_t timer_id;
    uint8_t timer_en;
    uint8_t timer_count;
    uint8_t timer_period;
    APP_10_SECOND_TIMER_CB_T cb;
}APP_10_SECOND_TIMER_STRUCT;

#define INIT_APP_TIMER(_id, _en, _count, _period, _cb) \
    { \
        .timer_id = _id, \
        .timer_en = _en, \
        .timer_count = _count, \
        .timer_period = _period, \
        .cb = _cb, \
    }

APP_10_SECOND_TIMER_STRUCT app_10_second_array[] =
{
    INIT_APP_TIMER(APP_PAIR_TIMER_ID, 0, 0, 6, PairingTransferToConnectable),
    INIT_APP_TIMER(APP_POWEROFF_TIMER_ID, 0, 0, 90, CloseEarphone),
#ifdef GFPS_ENABLED
    INIT_APP_TIMER(APP_FASTPAIR_LASTING_TIMER_ID, 0, 0, APP_FAST_PAIRING_TIMEOUT_IN_SECOND/10,
        app_fast_pairing_timeout_timehandler),
#endif
};

void app_stop_10_second_timer(uint8_t timer_id)
{
    APP_10_SECOND_TIMER_STRUCT *timer = &app_10_second_array[timer_id];

    timer->timer_en = 0;
    timer->timer_count = 0;
}

void app_start_10_second_timer(uint8_t timer_id)
{
    APP_10_SECOND_TIMER_STRUCT *timer = &app_10_second_array[timer_id];

    timer->timer_en = 1;
    timer->timer_count = 0;
}

void app_set_10_second_timer(uint8_t timer_id, uint8_t enable, uint8_t period)
{
    APP_10_SECOND_TIMER_STRUCT *timer = &app_10_second_array[timer_id];

    timer->timer_en = enable;
    timer->timer_count = period;
}

void app_10_second_timer_check(void)
{
    APP_10_SECOND_TIMER_STRUCT *timer = app_10_second_array;
    unsigned int i;

    for(i = 0; i < ARRAY_SIZE(app_10_second_array); i++) {
        if (timer->timer_en) {
            timer->timer_count++;
            if (timer->timer_count >= timer->timer_period) {
                timer->timer_en = 0;
                if (timer->cb)
                    timer->cb();
            }
        }
        timer++;
    }
}

void CloseEarphone(void)
{
    int activeCons;
    osapi_lock_stack();
    activeCons = btif_me_get_activeCons();
    osapi_unlock_stack();

#ifdef ANC_APP
    if(app_anc_work_status()) {
        app_set_10_second_timer(APP_POWEROFF_TIMER_ID, 1, 30);
        return;
    }
#endif /* ANC_APP */
    if(activeCons == 0) {
        TRACE(0,"!!!CloseEarphone\n");
        app_shutdown();
    }
}
#endif /* #if defined(__BTIF_EARPHONE__) && defined(__BTIF_AUTOPOWEROFF__) */

//#if (HF_CUSTOM_FEATURE_SUPPORT & HF_CUSTOM_FEATURE_BATTERY_REPORT) || (HF_SDK_FEATURES & HF_FEATURE_HF_INDICATORS)
#if defined(SUPPORT_BATTERY_REPORT) || defined(SUPPORT_HF_INDICATORS)
extern void app_hfp_set_battery_level(uint8_t level);
#endif

int app_status_battery_report(uint8_t level)
{
#if defined(__BTIF_EARPHONE__)
    if (app_is_stack_ready())
    {
        app_bt_state_checker();
    }
    app_10_second_timer_check();
#endif
    if (level<=APP_BATTERY_LEVEL_LOWPOWERTHRESHOLD){
        //add something
    }
    if (app_is_stack_ready())
    {
// #if (HF_CUSTOM_FEATURE_SUPPORT & HF_CUSTOM_FEATURE_BATTERY_REPORT) || (HF_SDK_FEATURES & HF_FEATURE_HF_INDICATORS)
#if defined(SUPPORT_BATTERY_REPORT) || defined(SUPPORT_HF_INDICATORS)
#if defined(IBRT)
    if (app_tws_ibrt_mobile_link_connected())
    {
        app_hfp_set_battery_level(level);
    }
#else
    app_hfp_set_battery_level(level);
#endif
#else
    TRACE(1,"[%s] Can not enable SUPPORT_BATTERY_REPORT",__func__);
#endif
    osapi_notify_evm();
    }
    return 0;
}

#ifdef MEDIA_PLAYER_SUPPORT

void app_status_set_num(const char* p)
{
    media_Set_IncomingNumber(p);
}

int app_voice_report_handler(APP_STATUS_INDICATION_T status, uint8_t device_id, uint8_t isMerging, uint8_t islocal)
{
#if (defined(BTUSB_AUDIO_MODE) || defined(BTUSB_AUDIO_MODE))
    if(app_usbaudio_mode_on()) return 0;
#endif
    TRACE(2,"%s %d",__func__, status);
    uint16_t aud_pram = 0;
    AUD_ID_ENUM id = MAX_RECORD_NUM;
    if (app_poweroff_flag == 1){
        switch (status) {
            case APP_STATUS_INDICATION_POWEROFF:
                id = AUD_ID_POWER_OFF;
                break;
            default:
                return 0;
                break;
        }
    }else{
        switch (status) {
            case APP_STATUS_INDICATION_POWERON:
                id = AUD_ID_POWER_ON;
                break;
            case APP_STATUS_INDICATION_POWEROFF:
                id = AUD_ID_POWER_OFF;
                break;
            case APP_STATUS_INDICATION_CONNECTED:
                id = AUD_ID_BT_CONNECTED;
                break;
            case APP_STATUS_INDICATION_DISCONNECTED:
                id = AUD_ID_BT_DIS_CONNECT;
                break;
            case APP_STATUS_INDICATION_CALLNUMBER:
                id = AUD_ID_BT_CALL_INCOMING_NUMBER;
                break;
            case APP_STATUS_INDICATION_CHARGENEED:
                id = AUD_ID_BT_CHARGE_PLEASE;
                break;
            case APP_STATUS_INDICATION_FULLCHARGE:
                id = AUD_ID_BT_CHARGE_FINISH;
                break;
            case APP_STATUS_INDICATION_PAIRSUCCEED:
                id = AUD_ID_BT_PAIRING_SUC;
                break;
            case APP_STATUS_INDICATION_PAIRFAIL:
                id = AUD_ID_BT_PAIRING_FAIL;
                break;

            case APP_STATUS_INDICATION_HANGUPCALL:
                id = AUD_ID_BT_CALL_HUNG_UP;
                break;

            case APP_STATUS_INDICATION_REFUSECALL:
                id = AUD_ID_BT_CALL_REFUSE;
                break;

            case APP_STATUS_INDICATION_ANSWERCALL:
                id = AUD_ID_BT_CALL_ANSWER;
                break;

            case APP_STATUS_INDICATION_CLEARSUCCEED:
                id = AUD_ID_BT_CLEAR_SUCCESS;
                break;

            case APP_STATUS_INDICATION_CLEARFAIL:
                id = AUD_ID_BT_CLEAR_FAIL;
                break;
            case APP_STATUS_INDICATION_INCOMINGCALL:
                id = AUD_ID_BT_CALL_INCOMING_CALL;
                break;
            case APP_STATUS_INDICATION_BOTHSCAN:
                id = AUD_ID_BT_PAIR_ENABLE;
                break;
            case APP_STATUS_INDICATION_WARNING:
                id = AUD_ID_BT_WARNING;
                break;
            case APP_STATUS_INDICATION_ALEXA_START:
                id = AUDIO_ID_BT_ALEXA_START;
                break;
            case APP_STATUS_INDICATION_ALEXA_STOP:
                id = AUDIO_ID_BT_ALEXA_STOP;
                break;
            case APP_STATUS_INDICATION_GSOUND_MIC_OPEN:
                id = AUDIO_ID_BT_GSOUND_MIC_OPEN;
                break;
            case APP_STATUS_INDICATION_GSOUND_MIC_CLOSE:
                id = AUDIO_ID_BT_GSOUND_MIC_CLOSE;
                break;
            case APP_STATUS_INDICATION_GSOUND_NC:
                id = AUDIO_ID_BT_GSOUND_NC;
                break;
#ifdef __BT_WARNING_TONE_MERGE_INTO_STREAM_SBC__
            case APP_STATUS_RING_WARNING:
                id = AUD_ID_RING_WARNING;
                break;
#endif
            case APP_STATUS_INDICATION_MUTE:
                id = AUDIO_ID_BT_MUTE;
                break;
#ifdef __INTERACTION__
            case APP_STATUS_INDICATION_FINDME:
                id = AUD_ID_BT_FINDME;
                break;
#endif
            default:
                break;
        }
    }
    
    if (isMerging){
        aud_pram |= PROMOT_ID_BIT_MASK_MERGING;
    }

    if (islocal){
        aud_pram |= PROMOT_ID_BIT_MASK_CHNLSEl_LOCAL;
    }

#ifdef BT_USB_AUDIO_DUAL_MODE
    if(!btusb_is_usb_mode())
    {
#if defined(IBRT)
        app_ibrt_if_voice_report_handler(id, aud_pram);
#else
        trigger_media_play(id, device_id, aud_pram);
#endif
    }

#else
#if defined(IBRT)
    aud_pram |= PROMOT_ID_BIT_MASK_CHNLSEl_ALL;
    app_ibrt_if_voice_report_handler(id, aud_pram);
#else
    trigger_media_play(id, device_id, aud_pram);
#endif
#endif

    return 0;
}

extern "C" int app_voice_report_local(APP_STATUS_INDICATION_T status, uint8_t device_id)
{
    return app_voice_report_handler(status, device_id, true, true);
}

extern "C" int app_voice_report(APP_STATUS_INDICATION_T status, uint8_t device_id)
{
    return app_voice_report_handler(status, device_id, true, false);
}

extern "C" int app_voice_report_generic(APP_STATUS_INDICATION_T status, uint8_t device_id, uint8_t isMerging, uint8_t islocal)
{
    return app_voice_report_handler(status, device_id, isMerging, islocal);
}

extern "C" int app_voice_stop(APP_STATUS_INDICATION_T status, uint8_t device_id)
{
    AUD_ID_ENUM id = MAX_RECORD_NUM;

    TRACE(2,"%s %d", __func__, status);

    if (status == APP_STATUS_INDICATION_FIND_MY_BUDS)
        id = AUDIO_ID_FIND_MY_BUDS;

    if (id != MAX_RECORD_NUM)
        trigger_media_stop(id, device_id);

    return 0;
}

#endif


#define POWERON_PRESSMAXTIME_THRESHOLD_MS  (5000)

enum APP_POWERON_CASE_T {
    APP_POWERON_CASE_NORMAL = 0,
    APP_POWERON_CASE_DITHERING,
    APP_POWERON_CASE_REBOOT,
    APP_POWERON_CASE_ALARM,
    APP_POWERON_CASE_CALIB,
    APP_POWERON_CASE_BOTHSCAN,
    APP_POWERON_CASE_CHARGING,
    APP_POWERON_CASE_FACTORY,
    APP_POWERON_CASE_TEST,
    APP_POWERON_CASE_INVALID,

    APP_POWERON_CASE_NUM
};


static osThreadId apps_init_tid = NULL;

static enum APP_POWERON_CASE_T g_pwron_case = APP_POWERON_CASE_INVALID;

static void app_poweron_normal(APP_KEY_STATUS *status, void *param)
{
    TRACE(3,"%s %d,%d",__func__, status->code, status->event);
    g_pwron_case = APP_POWERON_CASE_NORMAL;

    osSignalSet(apps_init_tid, 0x2);
}

#if !defined(BLE_ONLY_ENABLED)
static void app_poweron_scan(APP_KEY_STATUS *status, void *param)
{
    TRACE(3,"%s %d,%d",__func__, status->code, status->event);
    g_pwron_case = APP_POWERON_CASE_BOTHSCAN;

    osSignalSet(apps_init_tid, 0x2);
}
#endif

#ifdef __ENGINEER_MODE_SUPPORT__
#if !defined(BLE_ONLY_ENABLED)
static void app_poweron_factorymode(APP_KEY_STATUS *status, void *param)
{
    TRACE(3,"%s %d,%d",__func__, status->code, status->event);
    hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT);
    app_factorymode_enter();
}
#endif
#endif


static bool g_pwron_finished = false;
static void app_poweron_finished(APP_KEY_STATUS *status, void *param)
{
    TRACE(3,"%s %d,%d",__func__, status->code, status->event);
    g_pwron_finished = true;
    osSignalSet(apps_init_tid, 0x2);
}

void app_poweron_wait_finished(void)
{
    if (!g_pwron_finished){
        osSignalWait(0x2, osWaitForever);
    }
}

#if  defined(__POWERKEY_CTRL_ONOFF_ONLY__)
void app_bt_key_shutdown(APP_KEY_STATUS *status, void *param);
const  APP_KEY_HANDLE  pwron_key_handle_cfg[] = {
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_UP},           "power on: shutdown"     , app_bt_key_shutdown, NULL},
};
#elif defined(__ENGINEER_MODE_SUPPORT__)
const  APP_KEY_HANDLE  pwron_key_handle_cfg[] = {
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_INITUP},           "power on: normal"     , app_poweron_normal, NULL},
#if !defined(BLE_ONLY_ENABLED)
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_INITLONGPRESS},    "power on: both scan"  , app_poweron_scan  , NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_INITLONGLONGPRESS},"power on: factory mode", app_poweron_factorymode  , NULL},
#endif
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_INITFINISHED},     "power on: finished"   , app_poweron_finished  , NULL},
};
#else
const  APP_KEY_HANDLE  pwron_key_handle_cfg[] = {
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_INITUP},           "power on: normal"     , app_poweron_normal, NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_INITLONGPRESS},    "power on: both scan"  , app_poweron_scan  , NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_INITFINISHED},     "power on: finished"   , app_poweron_finished  , NULL},
};
#endif

#ifdef SINGLE_WIRE_DOWNLOAD
//Add it where you need it
void app_bt_key_enter_single_dld_mode(APP_KEY_STATUS *status, void *param)
{
    TRACE(1,"%s ",__func__);
    hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT);
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_SINGLE_LINE_DOWNLOAD);
    hal_cmu_sys_reboot();
}
#endif

#ifdef TOTA_v2
void app_bt_key_enter_tota_mode(APP_KEY_STATUS *status, void *param)
{
    reboot_to_enter_tota_mode();    
}
#endif

#ifndef APP_TEST_MODE
static void app_poweron_key_init(void)
{
    uint8_t i = 0;
    TRACE(1,"%s",__func__);

    for (i=0; i<(sizeof(pwron_key_handle_cfg)/sizeof(APP_KEY_HANDLE)); i++){
        app_key_handle_registration(&pwron_key_handle_cfg[i]);
    }
}

static uint8_t app_poweron_wait_case(void)
{
    uint32_t stime = 0, etime = 0;

#ifdef __POWERKEY_CTRL_ONOFF_ONLY__
    g_pwron_case = APP_POWERON_CASE_NORMAL;
#else
    TRACE(0,"poweron_wait_case enter", g_pwron_case);
    if (g_pwron_case == APP_POWERON_CASE_INVALID){
        stime = hal_sys_timer_get();
        osSignalWait(0x2, POWERON_PRESSMAXTIME_THRESHOLD_MS);
        etime = hal_sys_timer_get();
    }
    TRACE(2,"powon raw case:%d time:%d", g_pwron_case, TICKS_TO_MS(etime - stime));
#endif
    return g_pwron_case;

}
#endif

uint8_t stack_ready_flag = 0;
void app_notify_stack_ready(uint8_t ready_flag)
{
    TRACE(2,"app_notify_stack_ready %d %d", stack_ready_flag, ready_flag);

    stack_ready_flag |= ready_flag;

#ifdef __IAG_BLE_INCLUDE__
    if(stack_ready_flag == (STACK_READY_BT|STACK_READY_BLE))
#endif
    {
        osSignalSet(apps_init_tid, 0x3);
    }
}

bool app_is_stack_ready(void)
{
    bool ret = false;

    if (stack_ready_flag == (STACK_READY_BT
#ifdef __IAG_BLE_INCLUDE__
                             | STACK_READY_BLE
#endif
                             ))
    {
        ret = true;
    }

    return ret;
}

static void app_stack_ready_cb(void)
{
    TRACE(0,"stack init done");
#ifdef __IAG_BLE_INCLUDE__
    app_ble_stub_user_init();
    app_ble_start_connectable_adv(BLE_ADVERTISING_INTERVAL);
#endif
}

static void app_wait_stack_ready(void)
{
    uint32_t stime, etime;
    stime = hal_sys_timer_get();
    osSignalWait(0x3, 1000);
    etime = hal_sys_timer_get();
    TRACE(1,"app_wait_stack_ready: wait:%d ms", TICKS_TO_MS(etime - stime));

    app_stack_ready_cb();
}

extern "C" int system_shutdown(void);
int app_shutdown(void)
{
    system_shutdown();
    return 0;
}

int system_reset(void);
int app_reset(void)
{
    system_reset();
    return 0;
}

static void app_postponed_reset_timer_handler(void const *param);
osTimerDef(APP_POSTPONED_RESET_TIMER, app_postponed_reset_timer_handler);
static osTimerId app_postponed_reset_timer = NULL;
#define APP_RESET_PONTPONED_TIME_IN_MS  2000
static void app_postponed_reset_timer_handler(void const *param)
{
    system_reset();
}

void app_start_postponed_reset(void)
{
    if (NULL == app_postponed_reset_timer)
    {
        app_postponed_reset_timer = osTimerCreate(osTimer(APP_POSTPONED_RESET_TIMER), osTimerOnce, NULL);
    }

    hal_sw_bootmode_set(HAL_SW_BOOTMODE_ENTER_HIDE_BOOT);

    osTimerStart(app_postponed_reset_timer, APP_RESET_PONTPONED_TIME_IN_MS);
}

void app_bt_key_shutdown(APP_KEY_STATUS *status, void *param)
{
    TRACE(3,"%s %d,%d",__func__, status->code, status->event);
#ifdef __POWERKEY_CTRL_ONOFF_ONLY__
    hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT);
    app_reset();
#else
    app_shutdown();
#endif
}

void app_bt_key_enter_testmode(APP_KEY_STATUS *status, void *param)
{
    TRACE(1,"%s\n",__FUNCTION__);

    if(app_status_indication_get() == APP_STATUS_INDICATION_BOTHSCAN){
#ifdef __FACTORY_MODE_SUPPORT__
        app_factorymode_bt_signalingtest(status, param);
#endif
    }
}

void app_bt_key_enter_nosignal_mode(APP_KEY_STATUS *status, void *param)
{
    TRACE(1,"%s\n",__FUNCTION__);
    if(app_status_indication_get() == APP_STATUS_INDICATION_BOTHSCAN){
#ifdef __FACTORY_MODE_SUPPORT__
        app_factorymode_bt_nosignalingtest(status, param);
#endif
    }
}

void app_bt_singleclick(APP_KEY_STATUS *status, void *param)
{
    TRACE(3,"%s %d,%d",__func__, status->code, status->event);
}

void app_bt_doubleclick(APP_KEY_STATUS *status, void *param)
{
    TRACE(3,"%s %d,%d",__func__, status->code, status->event);
}

void app_power_off(APP_KEY_STATUS *status, void *param)
{
    TRACE(0,"app_power_off\n");
}

extern "C" void OS_NotifyEvm(void);

#define PRESS_KEY_TO_ENTER_OTA_INTERVEL    (15000)          // press key 15s enter to ota
#define PRESS_KEY_TO_ENTER_OTA_REPEAT_CNT    ((PRESS_KEY_TO_ENTER_OTA_INTERVEL - 2000) / 500)
void app_otaMode_enter(APP_KEY_STATUS *status, void *param)
{
    TRACE(1,"%s",__func__);

    hal_norflash_disable_protection(HAL_FLASH_ID_0);

    hal_sw_bootmode_set(HAL_SW_BOOTMODE_ENTER_HIDE_BOOT);
#ifdef __KMATE106__
    app_status_indication_set(APP_STATUS_INDICATION_OTA);
    app_voice_report(APP_STATUS_INDICATION_WARNING, 0);
    osDelay(1200);
#endif
    hal_cmu_sys_reboot();
}

#ifdef __USB_COMM__
void app_usb_cdc_comm_key_handler(APP_KEY_STATUS *status, void *param)
{
    TRACE(3,"%s %d,%d", __func__, status->code, status->event);
    hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT);
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_CDC_COMM);
    pmu_usb_config(PMU_USB_CONFIG_TYPE_DEVICE);
    hal_cmu_reset_set(HAL_CMU_MOD_GLOBAL);
}
#endif

#if 0
void app_dfu_key_handler(APP_KEY_STATUS *status, void *param)
{
    TRACE(1,"%s ",__func__);
    hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT);
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_FORCE_USB_DLD);
    pmu_usb_config(PMU_USB_CONFIG_TYPE_DEVICE);
    hal_cmu_reset_set(HAL_CMU_MOD_GLOBAL);
}
#else
void app_dfu_key_handler(APP_KEY_STATUS *status, void *param)
{
    TRACE(1,"%s ",__func__);
    hal_sw_bootmode_clear(0xffffffff);
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_FORCE_USB_DLD | HAL_SW_BOOTMODE_SKIP_FLASH_BOOT);
    pmu_usb_config(PMU_USB_CONFIG_TYPE_DEVICE);
    hal_cmu_reset_set(HAL_CMU_MOD_GLOBAL);
}
#endif

void app_ota_key_handler(APP_KEY_STATUS *status, void *param)
{
    static uint32_t time = hal_sys_timer_get();
    static uint16_t cnt = 0;

    TRACE(3,"%s %d,%d",__func__, status->code, status->event);

    if (TICKS_TO_MS(hal_sys_timer_get() - time) > 600) // 600 = (repeat key intervel)500 + (margin)100
        cnt = 0;
    else
        cnt++;

    if (cnt == PRESS_KEY_TO_ENTER_OTA_REPEAT_CNT) {
        app_otaMode_enter(NULL, NULL);
    }

    time = hal_sys_timer_get();
}
extern "C" void app_bt_key(APP_KEY_STATUS *status, void *param)
{
    TRACE(3,"%s %d,%d",__func__, status->code, status->event);
#define DEBUG_CODE_USE 0
    switch(status->event)
    {
        case APP_KEY_EVENT_CLICK:
            TRACE(0,"first blood!");
#if DEBUG_CODE_USE
            if (status->code == APP_KEY_CODE_PWR)
            {
                static int m = 0;
                if (m == 0) {
                    m = 1;
                    hal_iomux_set_analog_i2c();
                }
                else {
                    m = 0;
                    hal_iomux_set_uart0();
                }
            }
#endif
            break;
        case APP_KEY_EVENT_DOUBLECLICK:
            TRACE(0,"double kill");
#if DEBUG_CODE_USE
            if (status->code == APP_KEY_CODE_PWR)
            {
                app_otaMode_enter(NULL, NULL);
                return;
            }
            break;
        case APP_KEY_EVENT_TRIPLECLICK:
            TRACE(0,"triple kill");
            if (status->code == APP_KEY_CODE_PWR)
            {
                app_bt_accessmode_set(BTIF_BT_DEFAULT_ACCESS_MODE_PAIR);
#ifdef GFPS_ENABLED
                app_enter_fastpairing_mode();
#endif
                return;
            }
#endif
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
#ifdef __FACTORY_MODE_SUPPORT__
    if (app_status_indication_get() == APP_STATUS_INDICATION_BOTHSCAN && (status->event == APP_KEY_EVENT_DOUBLECLICK)){
        app_factorymode_languageswitch_proc();
    }else
#endif
    {
        bt_key_send(status);
    }
}

#ifdef RB_CODEC
extern bool app_rbcodec_check_hfp_active(void );
void app_switch_player_key(APP_KEY_STATUS *status, void *param)
{
    TRACE(3,"%s %d,%d",__func__, status->code, status->event);

    if(!rb_ctl_is_init_done()) {
        TRACE(0,"rb ctl not init done");
        return ;
    }

    if( app_rbcodec_check_hfp_active() ) {
        app_bt_key(status,param);
        return;
    }

    app_rbplay_audio_reset_pause_status();

    if(app_rbplay_mode_switch()) {
        app_voice_report(APP_STATUS_INDICATION_POWERON, 0);
        app_rbcodec_ctr_play_onoff(true);
    } else {
        app_rbcodec_ctr_play_onoff(false);
        app_voice_report(APP_STATUS_INDICATION_POWEROFF, 0);
    }
    return ;

}
#endif

#ifdef IS_MULTI_AI_ENABLED
#ifdef MAI_TYPE_REBOOT_WITHOUT_OEM_APP
extern "C" bool gva_status_update_start;
extern "C" bool ama_status_update_start;
#endif
#endif
void app_voice_assistant_key(APP_KEY_STATUS *status, void *param)
{
    TRACE(2,"%s event %d", __func__, status->event);
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

#ifdef IS_MULTI_AI_ENABLED
void app_voice_gva_onoff_key(APP_KEY_STATUS *status, void *param)
{
    uint8_t current_ai_spec = ai_manager_get_current_spec();

    TRACE(2,"%s current_ai_spec %d", __func__, current_ai_spec);
    if (current_ai_spec == AI_SPEC_INIT)
    {
        ai_manager_enable(true, AI_SPEC_GSOUND);
    }
    else if(current_ai_spec == AI_SPEC_GSOUND)
    {
        ai_manager_enable(false, AI_SPEC_GSOUND);
    }
    else if(current_ai_spec == AI_SPEC_AMA)
    {
        ai_manager_switch_spec(AI_SPEC_GSOUND);
    }
    app_start_ble_advertising(GAPM_ADV_UNDIRECT, BLE_ADVERTISING_INTERVAL);
}

void app_voice_ama_onoff_key(APP_KEY_STATUS *status, void *param)
{
    uint8_t current_ai_spec = ai_manager_get_current_spec();

    TRACE(2,"%s current_ai_spec %d", __func__, current_ai_spec);
    if (current_ai_spec == AI_SPEC_INIT)
    {
        ai_manager_enable(true, AI_SPEC_AMA);
    }
    else if(current_ai_spec == AI_SPEC_AMA)
    {
        ai_manager_enable(false, AI_SPEC_AMA);
    }
    else if(current_ai_spec == AI_SPEC_GSOUND)
    {
        ai_manager_switch_spec(AI_SPEC_AMA);
    }
    app_start_ble_advertising(GAPM_ADV_UNDIRECT, BLE_ADVERTISING_INTERVAL);
}
#endif

#if defined(BT_USB_AUDIO_DUAL_MODE_TEST) && defined(BT_USB_AUDIO_DUAL_MODE)
extern "C" void test_btusb_switch(void);
void app_btusb_audio_dual_mode_test(APP_KEY_STATUS *status, void *param)
{
    TRACE(0,"test_btusb_switch");
    test_btusb_switch();
}
#endif

extern void switch_dualmic_status(void);

void app_switch_dualmic_key(APP_KEY_STATUS *status, void *param)
{
    switch_dualmic_status();
}

#if defined(ANC_APP)
void app_anc_key(APP_KEY_STATUS *status, void *param)
{
    app_anc_loop_switch();
}
#endif

#ifdef POWERKEY_I2C_SWITCH
extern void app_factorymode_i2c_switch(APP_KEY_STATUS *status, void *param);
#endif

#ifdef TILE_DATAPATH
extern "C" void app_tile_key_handler(APP_KEY_STATUS *status, void *param);
#endif


#ifdef __POWERKEY_CTRL_ONOFF_ONLY__
#if defined(__APP_KEY_FN_STYLE_A__)
const APP_KEY_HANDLE  app_key_handle_cfg[] = {
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_UP},"bt function key",app_bt_key_shutdown, NULL},
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_LONGPRESS},"bt function key",app_bt_key, NULL},
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_UP},"bt function key",app_bt_key, NULL},
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_DOUBLECLICK},"bt function key",app_bt_key, NULL},
    {{APP_KEY_CODE_FN2,APP_KEY_EVENT_UP},"bt volume up key",app_bt_key, NULL},
    {{APP_KEY_CODE_FN2,APP_KEY_EVENT_LONGPRESS},"bt play backward key",app_bt_key, NULL},
    {{APP_KEY_CODE_FN3,APP_KEY_EVENT_UP},"bt volume down key",app_bt_key, NULL},
    {{APP_KEY_CODE_FN3,APP_KEY_EVENT_LONGPRESS},"bt play forward key",app_bt_key, NULL},
#ifdef SUPPORT_SIRI
    {{APP_KEY_CODE_NONE ,APP_KEY_EVENT_NONE},"none function key",app_bt_key, NULL},
#endif

};
#else //#elif defined(__APP_KEY_FN_STYLE_B__)
const APP_KEY_HANDLE  app_key_handle_cfg[] = {
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_UP},"bt function key",app_bt_key_shutdown, NULL},
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_LONGPRESS},"bt function key",app_bt_key, NULL},
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_UP},"bt function key",app_bt_key, NULL},
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_DOUBLECLICK},"bt function key",app_bt_key, NULL},
    {{APP_KEY_CODE_FN2,APP_KEY_EVENT_REPEAT},"bt volume up key",app_bt_key, NULL},
    {{APP_KEY_CODE_FN2,APP_KEY_EVENT_UP},"bt play backward key",app_bt_key, NULL},
    {{APP_KEY_CODE_FN3,APP_KEY_EVENT_REPEAT},"bt volume down key",app_bt_key, NULL},
    {{APP_KEY_CODE_FN3,APP_KEY_EVENT_UP},"bt play forward key",app_bt_key, NULL},
#ifdef SUPPORT_SIRI
    {{APP_KEY_CODE_NONE ,APP_KEY_EVENT_NONE},"none function key",app_bt_key, NULL},
#endif

};
#endif
#else
#if defined(__APP_KEY_FN_STYLE_A__)
//--
const APP_KEY_HANDLE  app_key_handle_cfg[] = {
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_LONGLONGPRESS},"bt function key",app_bt_key_shutdown, NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_LONGPRESS},"bt function key",app_bt_key, NULL},
#if defined(BT_USB_AUDIO_DUAL_MODE_TEST) && defined(BT_USB_AUDIO_DUAL_MODE)
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_CLICK},"bt function key",app_bt_key, NULL},
#ifdef RB_CODEC
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_CLICK},"bt function key",app_switch_player_key, NULL},
#else
    //{{APP_KEY_CODE_PWR,APP_KEY_EVENT_CLICK},"btusb mode switch key.",app_btusb_audio_dual_mode_test, NULL},
#endif
#endif
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_DOUBLECLICK},"bt function key",app_bt_key, NULL},
#ifdef TILE_DATAPATH
     {{APP_KEY_CODE_PWR,APP_KEY_EVENT_TRIPLECLICK},"bt function key",app_tile_key_handler, NULL},
#else
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_TRIPLECLICK},"bt function key",app_bt_key, NULL},
#endif
#if RAMPAGECLICK_TEST_MODE
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_ULTRACLICK},"bt function key",app_bt_key_enter_nosignal_mode, NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_RAMPAGECLICK},"bt function key",app_bt_key_enter_testmode, NULL},
#endif
#ifdef POWERKEY_I2C_SWITCH
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_RAMPAGECLICK},"bt i2c key",app_factorymode_i2c_switch, NULL},
#endif
    //{{APP_KEY_CODE_FN1,APP_KEY_EVENT_UP},"bt volume up key",app_bt_key, NULL},
    //{{APP_KEY_CODE_FN1,APP_KEY_EVENT_LONGPRESS},"bt play backward key",app_bt_key, NULL},
#if defined(APP_LINEIN_A2DP_SOURCE)||defined(APP_I2S_A2DP_SOURCE)
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_DOUBLECLICK},"bt mode src snk key",app_bt_key, NULL},
#endif
    //{{APP_KEY_CODE_FN2,APP_KEY_EVENT_UP},"bt volume down key",app_bt_key, NULL},
    //{{APP_KEY_CODE_FN2,APP_KEY_EVENT_LONGPRESS},"bt play forward key",app_bt_key, NULL},
    //{{APP_KEY_CODE_FN15,APP_KEY_EVENT_UP},"bt volume down key",app_bt_key, NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_CLICK},"bt function key",app_bt_key, NULL},

#ifdef SUPPORT_SIRI
    {{APP_KEY_CODE_NONE ,APP_KEY_EVENT_NONE},"none function key",app_bt_key, NULL},
#endif
#if defined( __BT_ANC_KEY__)&&defined(ANC_APP)
     {{APP_KEY_CODE_PWR,APP_KEY_EVENT_CLICK},"bt anc key",app_anc_key, NULL},
#endif
#if defined(VOICE_DATAPATH) || defined(__AI_VOICE__)
    {{APP_KEY_CODE_GOOGLE, APP_KEY_EVENT_FIRST_DOWN}, "google assistant key", app_voice_assistant_key, NULL},
#if defined(IS_GSOUND_BUTTION_HANDLER_WORKAROUND_ENABLED) || defined(PUSH_AND_HOLD_ENABLED)
    {{APP_KEY_CODE_GOOGLE, APP_KEY_EVENT_UP}, "google assistant key", app_voice_assistant_key, NULL},
#endif
#ifdef __TENCENT_VOICE__
     {{APP_KEY_CODE_GOOGLE, APP_KEY_EVENT_UP}, "google assistant key", app_voice_assistant_key, NULL},
#endif
    {{APP_KEY_CODE_GOOGLE, APP_KEY_EVENT_UP_AFTER_LONGPRESS}, "google assistant key", app_voice_assistant_key, NULL},
    {{APP_KEY_CODE_GOOGLE, APP_KEY_EVENT_LONGPRESS}, "google assistant key", app_voice_assistant_key, NULL},
    {{APP_KEY_CODE_GOOGLE, APP_KEY_EVENT_CLICK}, "google assistant key", app_voice_assistant_key, NULL},
    {{APP_KEY_CODE_GOOGLE, APP_KEY_EVENT_DOUBLECLICK}, "google assistant key", app_voice_assistant_key, NULL},
#endif
#ifdef TILE_DATAPATH
    {{APP_KEY_CODE_TILE, APP_KEY_EVENT_DOWN}, "tile function key", app_tile_key_handler, NULL},
    {{APP_KEY_CODE_TILE, APP_KEY_EVENT_UP}, "tile function key", app_tile_key_handler, NULL},
#endif
#ifdef IS_MULTI_AI_ENABLED
    {{APP_KEY_CODE_FN13, APP_KEY_EVENT_CLICK}, "gva on-off key", app_voice_gva_onoff_key, NULL},
    {{APP_KEY_CODE_FN14, APP_KEY_EVENT_CLICK}, "ama on-off key", app_voice_ama_onoff_key, NULL},
#endif
#if defined(BT_USB_AUDIO_DUAL_MODE_TEST) && defined(BT_USB_AUDIO_DUAL_MODE)
    {{APP_KEY_CODE_FN15, APP_KEY_EVENT_CLICK}, "btusb mode switch key.", app_btusb_audio_dual_mode_test, NULL},
#endif
};
#else //#elif defined(__APP_KEY_FN_STYLE_B__)
const APP_KEY_HANDLE  app_key_handle_cfg[] = {
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_LONGLONGPRESS},"bt function key",app_bt_key_shutdown, NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_LONGPRESS},"bt function key",app_bt_key, NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_CLICK},"bt function key",app_bt_key, NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_DOUBLECLICK},"bt function key",app_bt_key, NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_TRIPLECLICK},"bt function key",app_bt_key, NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_ULTRACLICK},"bt function key",app_bt_key_enter_nosignal_mode, NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_RAMPAGECLICK},"bt function key",app_bt_key_enter_testmode, NULL},
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_REPEAT},"bt volume up key",app_bt_key, NULL},
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_UP},"bt play backward key",app_bt_key, NULL},
    {{APP_KEY_CODE_FN2,APP_KEY_EVENT_REPEAT},"bt volume down key",app_bt_key, NULL},
    {{APP_KEY_CODE_FN2,APP_KEY_EVENT_UP},"bt play forward key",app_bt_key, NULL},
#ifdef SUPPORT_SIRI
    {{APP_KEY_CODE_NONE ,APP_KEY_EVENT_NONE},"none function key",app_bt_key, NULL},
#endif
};
#endif
#endif

void app_key_init(void)
{
#if defined(IBRT)
    app_ibrt_ui_test_key_init();
#else
    uint8_t i = 0;
    TRACE(1,"%s",__func__);

    app_key_handle_clear();
    for (i=0; i<(sizeof(app_key_handle_cfg)/sizeof(APP_KEY_HANDLE)); i++){
        app_key_handle_registration(&app_key_handle_cfg[i]);
    }
#endif
}
void app_key_init_on_charging(void)
{
    uint8_t i = 0;
    const APP_KEY_HANDLE  key_cfg[] = {
        {{APP_KEY_CODE_PWR,APP_KEY_EVENT_REPEAT},"ota function key",app_ota_key_handler, NULL},
        {{APP_KEY_CODE_PWR,APP_KEY_EVENT_CLICK},"bt function key",app_dfu_key_handler, NULL},
#ifdef __USB_COMM__
        {{APP_KEY_CODE_PWR,APP_KEY_EVENT_LONGPRESS},"usb cdc key",app_usb_cdc_comm_key_handler, NULL},
#endif
    };

    TRACE(1,"%s",__func__);
    for (i=0; i<(sizeof(key_cfg)/sizeof(APP_KEY_HANDLE)); i++){
        app_key_handle_registration(&key_cfg[i]);
    }
}

extern bt_status_t LinkDisconnectDirectly(void);
void a2dp_suspend_music_force(void);

bool app_is_power_off_in_progress(void)
{
    return app_poweroff_flag?TRUE:FALSE;
}

#if GFPS_ENABLED
#define APP_GFPS_BATTERY_TIMEROUT_VALUE             (10000)
static void app_gfps_battery_show_timeout_timer_cb(void const *n);
osTimerDef (GFPS_BATTERY_SHOW_TIMEOUT_TIMER, app_gfps_battery_show_timeout_timer_cb);
static osTimerId app_gfps_battery_show_timer_id = NULL;
static void app_gfps_battery_show_timeout_timer_cb(void const *n)
{
    TRACE(1,"%s", __func__);
    app_gfps_set_battery_datatype(HIDE_UI_INDICATION);
}

void app_gfps_battery_show_timer_start()
{
    if (app_gfps_battery_show_timer_id == NULL)
        app_gfps_battery_show_timer_id = osTimerCreate(osTimer(GFPS_BATTERY_SHOW_TIMEOUT_TIMER), osTimerOnce, NULL);
    osTimerStart(app_gfps_battery_show_timer_id, APP_GFPS_BATTERY_TIMEROUT_VALUE);
}

void app_gfps_battery_show_timer_stop()
{
    if (app_gfps_battery_show_timer_id)
        osTimerStop(app_gfps_battery_show_timer_id);
}

enum GFPS_FIND_BUDS_STATUS {
    GFPS_MUSIC_OFF_FIND_OFF,
    GFPS_MUSIC_OFF_FIND_ON,
    GFPS_MUSIC_ON_FIND_OFF,
    GFPS_MUSIC_ON_FIND_ON,
};

static void app_gfps_find_mybuds_timer_cb(void const *n);
osTimerDef (GFPS_FIND_MYBUDS_TIMER, app_gfps_find_mybuds_timer_cb);
static osTimerId app_gfps_find_mybuds_timer_id = NULL;
static uint8_t current_gfps_find_state = GFPS_MUSIC_OFF_FIND_OFF;
static bool gfps_find_state = 0;
extern struct BT_DEVICE_T  app_bt_device;
uint8_t gfps_ring_mode = GFPS_RING_MODE_BOTH_OFF;

void app_gfps_ring_mode_set(uint8_t ring_mode)
{
    gfps_ring_mode = ring_mode;
}

uint8_t app_gfps_ring_mode_get()
{
    return gfps_ring_mode;
}

void app_voice_start_gfps_find(void)
{
    TRACE(1, "%s", __func__);
    gfps_find_state=1;
    app_voice_report(APP_STATUS_INDICATION_FIND_MY_BUDS, 0);
}

void app_voice_stop_gfps_find(void)
{
    TRACE(1, "%s", __func__);
    gfps_find_state=0;
    app_voice_stop(APP_STATUS_INDICATION_FIND_MY_BUDS, 0);
}

bool app_voice_gfps_find_state(void)
{
    return gfps_find_state;
}

static void app_gfps_find_mybuds_timer_cb(void const *n)
{
    app_gfps_find_sm(1);
}

static void app_gfps_delay_to_start_find_mybuds()
{
    if (app_gfps_find_mybuds_timer_id == NULL)
        app_gfps_find_mybuds_timer_id = osTimerCreate(osTimer(GFPS_FIND_MYBUDS_TIMER), osTimerOnce, NULL);
    osTimerStart(app_gfps_find_mybuds_timer_id, 500);
}

void app_gfps_find_sm(bool find_on_off)
{
    bool music_on_off = app_bt_device.a2dp_streamming[0];
    osTimerStop(app_gfps_find_mybuds_timer_id);
    TRACE(3, "%s %d %d", __func__, find_on_off, current_gfps_find_state);
    switch(current_gfps_find_state)
    {
        case GFPS_MUSIC_OFF_FIND_OFF:
            if(find_on_off)
            {
                app_voice_start_gfps_find();
                if(music_on_off)
                    current_gfps_find_state=GFPS_MUSIC_ON_FIND_ON;
                else
                    current_gfps_find_state=GFPS_MUSIC_OFF_FIND_ON;
            }
            else
            {
                if(music_on_off)
                    current_gfps_find_state=GFPS_MUSIC_ON_FIND_OFF;
                if (AUDIO_ID_FIND_MY_BUDS == app_play_audio_get_aud_id())
                    app_voice_stop_gfps_find();
            }
            break;

        case GFPS_MUSIC_OFF_FIND_ON:
            if(find_on_off)
            {
                if(music_on_off)
                {
                    app_voice_stop_gfps_find();
                    app_gfps_delay_to_start_find_mybuds();
                    current_gfps_find_state=GFPS_MUSIC_ON_FIND_OFF;//DELAY TO SATRT SM,TO OPEN FIND
                }
                else
                {
                    app_voice_start_gfps_find();
                }
            }
            else
            {
                if (AUDIO_ID_FIND_MY_BUDS == app_play_audio_get_aud_id())
                    app_voice_stop_gfps_find();
                if(music_on_off)
                    current_gfps_find_state=GFPS_MUSIC_ON_FIND_OFF;
                else
                    current_gfps_find_state=GFPS_MUSIC_OFF_FIND_OFF;
            }
            break;

        case GFPS_MUSIC_ON_FIND_OFF:
            if(find_on_off)
            {
                if(music_on_off)
                {
                    app_audio_manager_sendrequest(APP_BT_STREAM_MANAGER_STOP,BT_STREAM_SBC,BT_DEVICE_ID_1,MAX_RECORD_NUM);
                    current_gfps_find_state=GFPS_MUSIC_ON_FIND_ON;
                }
                else
                {
                    current_gfps_find_state=GFPS_MUSIC_OFF_FIND_ON;
                }
                app_voice_start_gfps_find();
            }
            else
            {
                if(!music_on_off)
                    current_gfps_find_state=GFPS_MUSIC_OFF_FIND_OFF;
                if (AUDIO_ID_FIND_MY_BUDS == app_play_audio_get_aud_id())
                    app_voice_stop_gfps_find();
            }
            break;

        case GFPS_MUSIC_ON_FIND_ON:
            if(find_on_off)
            {
                if(!music_on_off)
                    current_gfps_find_state=GFPS_MUSIC_OFF_FIND_ON;
                app_voice_start_gfps_find();
            }
            else
            {
                if (AUDIO_ID_FIND_MY_BUDS == app_play_audio_get_aud_id())
                    app_voice_stop_gfps_find();
                if(music_on_off)
                {
                    current_gfps_find_state=GFPS_MUSIC_ON_FIND_OFF;
                    app_audio_manager_sendrequest(APP_BT_STREAM_MANAGER_START,BT_STREAM_SBC,BT_DEVICE_ID_1,MAX_RECORD_NUM);
                }
                else
                {
                    current_gfps_find_state=GFPS_MUSIC_OFF_FIND_OFF;
                }
            }
            break;
    }
}
#endif

int app_deinit(int deinit_case)
{
    int nRet = 0;
    TRACE(2,"%s case:%d",__func__, deinit_case);

#ifdef __PC_CMD_UART__
    app_cmd_close();
#endif
#if defined(BTUSB_AUDIO_MODE)
    if(app_usbaudio_mode_on()) return 0;
#endif
    if (!deinit_case){
#if defined(ANC_APP)
        app_anc_deinit();
#endif
#if defined(ANC_ASSIST_ENABLED)
        app_anc_assist_deinit();
#endif
        app_poweroff_flag = 1;
#if defined(APP_LINEIN_A2DP_SOURCE)
        app_audio_sendrequest(APP_A2DP_SOURCE_LINEIN_AUDIO, (uint8_t)APP_BT_SETTING_CLOSE,0);
#endif
#if defined(APP_I2S_A2DP_SOURCE)
        app_audio_sendrequest(APP_A2DP_SOURCE_I2S_AUDIO, (uint8_t)APP_BT_SETTING_CLOSE,0);
#endif
        app_status_indication_filter_set(APP_STATUS_INDICATION_BOTHSCAN);
        app_audio_sendrequest(APP_BT_STREAM_INVALID, (uint8_t)APP_BT_SETTING_CLOSEALL, 0);
        osDelay(500);
        LinkDisconnectDirectly();
        osDelay(500);
        app_status_indication_set(APP_STATUS_INDICATION_POWEROFF);
#ifdef MEDIA_PLAYER_SUPPORT
        app_voice_report(APP_STATUS_INDICATION_POWEROFF, 0);
#endif
        osDelay(2000);
        af_close();
#ifdef __THIRDPARTY
        app_thirdparty_specific_lib_event_handle(THIRDPARTY_ID_NO1,THIRDPARTY_DEINIT);
#endif
#if FPGA==0
        nv_record_flash_flush();
        norflash_flush_all_pending_op();
#endif
    }

    return nRet;
}

#ifdef APP_TEST_MODE
extern void app_test_init(void);
int app_init(void)
{
    int nRet = 0;
    //uint8_t pwron_case = APP_POWERON_CASE_INVALID;
    TRACE(1,"%s",__func__);
    app_poweroff_flag = 0;

#ifdef APP_TRACE_RX_ENABLE
    app_trace_rx_open();
#endif
    apps_init_tid = osThreadGetId();
    app_sysfreq_req(APP_SYSFREQ_USER_APP_INIT, APP_SYSFREQ_52M);
    list_init();
    af_open();
    app_os_init();
    app_pwl_open();
    app_audio_open();
    app_audio_manager_open();
    app_overlay_open();
    if (app_key_open(true))
    {
        nRet = -1;
        goto exit;
    }

    app_test_init();
exit:
    app_sysfreq_req(APP_SYSFREQ_USER_APP_INIT, APP_SYSFREQ_32K);
    return nRet;
}
#else /* !defined(APP_TEST_MODE) */

#define NVRAM_ENV_FACTORY_TESTER_STATUS_TEST_PASS (0xffffaa55)

int app_bt_connect2tester_init(void)
{
    btif_device_record_t rec;
    bt_bdaddr_t tester_addr;
    uint8_t i;
    bool find_tester = false;
    struct nvrecord_env_t *nvrecord_env;
    btdevice_profile *btdevice_plf_p;
    nv_record_env_get(&nvrecord_env);

    if (nvrecord_env->factory_tester_status.status != NVRAM_ENV_FACTORY_TESTER_STATUS_DEFAULT)
        return 0;

    if (!nvrec_dev_get_dongleaddr(&tester_addr)){
        nv_record_open(section_usrdata_ddbrecord);
        for (i = 0; nv_record_enum_dev_records(i, &rec) == BT_STS_SUCCESS; i++) {
            if (!memcmp(rec.bdAddr.address, tester_addr.address, BTIF_BD_ADDR_SIZE)){
                find_tester = true;
            }
        }
        if(i==0 && !find_tester){
            memset(&rec, 0, sizeof(btif_device_record_t));
            memcpy(rec.bdAddr.address, tester_addr.address, BTIF_BD_ADDR_SIZE);
            nv_record_add(section_usrdata_ddbrecord, &rec);
            btdevice_plf_p = (btdevice_profile *)app_bt_profile_active_store_ptr_get(rec.bdAddr.address);
            nv_record_btdevicerecord_set_hfp_profile_active_state(btdevice_plf_p, true);
            nv_record_btdevicerecord_set_a2dp_profile_active_state(btdevice_plf_p, true);
        }
        if (find_tester && i>2){
            nv_record_ddbrec_delete(&tester_addr);
            nvrecord_env->factory_tester_status.status = NVRAM_ENV_FACTORY_TESTER_STATUS_TEST_PASS;
            nv_record_env_set(nvrecord_env);
        }
    }

    return 0;
}

int app_nvrecord_rebuild(void)
{
    struct nvrecord_env_t *nvrecord_env;
    nv_record_env_get(&nvrecord_env);

    nv_record_sector_clear();
    nv_record_env_init();
    nv_record_update_factory_tester_status(NVRAM_ENV_FACTORY_TESTER_STATUS_TEST_PASS);
    nv_record_env_set(nvrecord_env);
    nv_record_flash_flush();

    return 0;
}

#if (defined(BTUSB_AUDIO_MODE) || defined(BTUSB_AUDIO_MODE))
#include "app_audio.h"
#include "usb_audio_frm_defs.h"
#include "usb_audio_app.h"

static bool app_usbaudio_mode = false;

extern "C" void btusbaudio_entry(void);
void app_usbaudio_entry(void)
{
    btusbaudio_entry();
    app_usbaudio_mode = true ;
}

bool app_usbaudio_mode_on(void)
{
    return app_usbaudio_mode;
}

void app_usb_key(APP_KEY_STATUS *status, void *param)
{
    TRACE(3,"%s %d,%d",__func__, status->code, status->event);

}

const APP_KEY_HANDLE  app_usb_handle_cfg[] = {
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_UP},"USB HID FN1 UP key",app_usb_key, NULL},
    {{APP_KEY_CODE_FN2,APP_KEY_EVENT_UP},"USB HID FN2 UP key",app_usb_key, NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_UP},"USB HID PWR UP key",app_usb_key, NULL},
};

void app_usb_key_init(void)
{
    uint8_t i = 0;
    TRACE(1,"%s",__func__);
    for (i=0; i<(sizeof(app_usb_handle_cfg)/sizeof(APP_KEY_HANDLE)); i++){
        app_key_handle_registration(&app_usb_handle_cfg[i]);
    }
}
#endif /* BTUSB_AUDIO_MODE */

//#define OS_HAS_CPU_STAT 1
#if OS_HAS_CPU_STAT
extern "C" void rtx_show_all_threads_usage(void);
#define _CPU_STATISTICS_PEROID_ 6000
#define CPU_USAGE_TIMER_TMO_VALUE (_CPU_STATISTICS_PEROID_/3)
static void cpu_usage_timer_handler(void const *param);
osTimerDef(cpu_usage_timer, cpu_usage_timer_handler);
static osTimerId cpu_usage_timer_id = NULL;
static void cpu_usage_timer_handler(void const *param)
{
    rtx_show_all_threads_usage();
}
#endif

extern "C" void btdrv_set_testmode(bool test);
extern void  btif_me_write_bt_sleep_enable(uint8_t sleep_en);
extern "C" int app_bt_start_custom_function_in_bt_thread(
    uint32_t param0, uint32_t param1, uint32_t funcPtr);
int btdrv_tportopen(void);


void app_ibrt_init(void)
{
        app_bt_global_handle_init();
#if defined(IBRT)
        ibrt_config_t config;
        app_tws_ibrt_init();
        app_ibrt_ui_init();
        app_ibrt_ui_test_init();
        app_ibrt_if_config_load(&config);
        app_ibrt_customif_ui_start();
        app_ibrt_ui_init_after_customif_ui_start();
#ifdef IBRT_SEARCH_UI
        app_tws_ibrt_start(&config, true);
#ifdef __STDF__
        // do nothing
#else
        app_ibrt_search_ui_init(false,IBRT_NONE_EVENT);
#endif
#else
        app_tws_ibrt_start(&config, false);
#endif

#endif

}

#ifdef GFPS_ENABLED
static void app_tell_battery_info_handler(uint8_t *batteryValueCount,
                                          uint8_t *batteryValue)
{
    GFPS_BATTERY_STATUS_E status;
    if (app_battery_is_charging())
    {
        status = BATTERY_CHARGING;
    }
    else
    {
        status = BATTERY_NOT_CHARGING;
    }

    // TODO: add the charger case's battery level
#ifdef IBRT
    if (app_tws_ibrt_tws_link_connected())
    {
        *batteryValueCount = 2;
    }
    else
    {
        *batteryValueCount = 1;
    }
#else
    *batteryValueCount = 1;
#endif

    TRACE(2,"%s,*batteryValueCount is %d",__func__,*batteryValueCount);
    if (1 == *batteryValueCount)
    {
        batteryValue[0] = ((app_battery_current_level()+1) * 10) | (status << 7);
    }
    else
    {
        batteryValue[0] = ((app_battery_current_level()+1) * 10) | (status << 7);
        batteryValue[1] = ((app_battery_current_level()+1) * 10) | (status << 7);

    }
}
#endif
extern uint32_t __prompt_section_start[];
extern uint32_t __coredump_section_start[];
extern uint32_t __ota_upgrade_log_start[];
extern uint32_t __log_dump_start[];
extern uint32_t __crash_dump_start[];
extern uint32_t __custom_parameter_start[];
extern uint32_t __aud_start[];
extern uint32_t __userdata_start[];
extern uint32_t __factory_start[];

#if defined(USE_LOWLATENCY_LIB)
extern void app_lowlatency_custom_lib_init(void);
#endif
int app_init(void)
{
    int nRet = 0;
#ifdef __AI_VOICE__
    uint8_t ai_spec = AI_SPEC_INIT;
#endif
    struct nvrecord_env_t *nvrecord_env;
#if defined(SIMPLE_TEST_UI)
    bool need_check_key = false;
#else	
    bool need_check_key = true;
#endif
    uint8_t pwron_case = APP_POWERON_CASE_INVALID;
#ifdef BT_USB_AUDIO_DUAL_MODE
    uint8_t usb_plugin = 0;
#endif
#ifdef IBRT_SEARCH_UI
    bool is_charging_poweron=false;
#endif
	

    TRACE(0,"please check all sections sizes and heads is correct ........");
    TRACE(2,"__prompt_section_start: 0x%08x length: 0x%x", __prompt_section_start, PROMPT_SECTION_SIZE);
    TRACE(2,"__coredump_section_start: 0x%08x length: 0x%x", __coredump_section_start, CORE_DUMP_SECTION_SIZE);
    TRACE(2,"__ota_upgrade_log_start: 0x%08x length: 0x%x", __ota_upgrade_log_start, OTA_UPGRADE_LOG_SIZE);
    TRACE(2,"__log_dump_start: 0x%08x length: 0x%x", __log_dump_start, LOG_DUMP_SECTION_SIZE);
    TRACE(2,"__crash_dump_start: 0x%08x length: 0x%x", __crash_dump_start, CRASH_DUMP_SECTION_SIZE);
    TRACE(2,"__custom_parameter_start: 0x%08x length: 0x%x", __custom_parameter_start, CUSTOM_PARAMETER_SECTION_SIZE);
    TRACE(2,"__userdata_start: 0x%08x length: 0x%x", __userdata_start, USERDATA_SECTION_SIZE*2);
    TRACE(2,"__aud_start: 0x%08x length: 0x%x", __aud_start, AUD_SECTION_SIZE);
    TRACE(2,"__factory_start: 0x%08x length: 0x%x", __factory_start, FACTORY_SECTION_SIZE);

    TRACE(0,"app_init\n");

#ifdef APP_TRACE_RX_ENABLE
    app_trace_rx_open();
#endif

    nv_record_init();
    factory_section_init();

#ifdef AUDIO_OUTPUT_DC_AUTO_CALIB
    /* DAC DC and Dig gain calibration
     * This subroutine will try to load DC and gain calibration
     * parameters from user data section located at NV record;
     * If failed to loading parameters, It will open AF and start
     * to calibration DC and gain;
     * After accomplished new calibrtion parameters, it will try
     * to save these data into user data section at NV record if
     * CODEC_DAC_DC_NV_DATA defined;
     */
    codec_dac_dc_auto_load(true, false, false);
#endif

    app_sysfreq_req(APP_SYSFREQ_USER_APP_INIT, APP_SYSFREQ_104M);
#if defined(MCU_HIGH_PERFORMANCE_MODE)
    TRACE(1,"sys freq calc : %d\n", hal_sys_timer_calc_cpu_freq(5, 0));
#endif
    apps_init_tid = osThreadGetId();
    list_init();
    nRet = app_os_init();
    if (nRet) {
        goto exit;
    }
#if OS_HAS_CPU_STAT
        cpu_usage_timer_id = osTimerCreate(osTimer(cpu_usage_timer), osTimerPeriodic, NULL);
        if (cpu_usage_timer_id != NULL) {
            osTimerStart(cpu_usage_timer_id, CPU_USAGE_TIMER_TMO_VALUE);
        }
#endif

    app_status_indication_init();

#ifdef BTADDR_FOR_DEBUG
    gen_bt_addr_for_debug();
#endif

#ifdef __STDF__
    stdf_init_fast();
#endif

#ifdef FORCE_SIGNALINGMODE
    hal_sw_bootmode_clear(HAL_SW_BOOTMODE_TEST_NOSIGNALINGMODE);
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_TEST_MODE | HAL_SW_BOOTMODE_TEST_SIGNALINGMODE);
#elif defined FORCE_NOSIGNALINGMODE
    hal_sw_bootmode_clear(HAL_SW_BOOTMODE_TEST_SIGNALINGMODE);
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_TEST_MODE | HAL_SW_BOOTMODE_TEST_NOSIGNALINGMODE);
#endif

    if (hal_sw_bootmode_get() & HAL_SW_BOOTMODE_REBOOT_FROM_CRASH){
        hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT_FROM_CRASH);
        TRACE(0,"Crash happened!!!");
    #ifdef VOICE_DATAPATH
        gsound_dump_set_flag(true);
    #endif
    }

#ifdef TOTA_v2
    if (hal_sw_bootmode_get() & HAL_SW_BOOTMODE_TOTA_REBOOT){
        hal_sw_bootmode_clear(HAL_SW_BOOTMODE_TOTA_REBOOT);
        set_in_tota_mode(true);
        TRACE(0,"To enter TOTA mode!!!");
    }
#endif

    if (hal_sw_bootmode_get() & HAL_SW_BOOTMODE_REBOOT){
        hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT);
        pwron_case = APP_POWERON_CASE_REBOOT;
        need_check_key = false;
        TRACE(0,"Initiative REBOOT happens!!!");
    }

    if (hal_sw_bootmode_get() & HAL_SW_BOOTMODE_TEST_MODE){
        hal_sw_bootmode_clear(HAL_SW_BOOTMODE_TEST_MODE);
        pwron_case = APP_POWERON_CASE_TEST;
        need_check_key = false;
        TRACE(0,"To enter test mode!!!");
    }
    

#ifdef BT_USB_AUDIO_DUAL_MODE
    usb_os_init();
#endif

#ifdef __STDF_EXTERNAL_CHAGER_ENABLE__
    need_check_key = false;
    nRet = 0;
#ifdef IBRT_SEARCH_UI
    is_charging_poweron = false; // default not enter tws pairing
#endif
#else /* __STDF_EXTERNAL_CHAGER_ENABLE__ */

    nRet = app_battery_open();
    TRACE(1,"BATTERY %d",nRet);
    if (pwron_case != APP_POWERON_CASE_TEST){
        switch (nRet) {
            case APP_BATTERY_OPEN_MODE_NORMAL:
                nRet = 0;
                break;
            case APP_BATTERY_OPEN_MODE_CHARGING:
                app_status_indication_set(APP_STATUS_INDICATION_CHARGING);
                TRACE(0,"CHARGING!");
                app_battery_start();
#if !defined(BTUSB_AUDIO_MODE)
                btdrv_start_bt();
                btdrv_hciopen();
                btdrv_sleep_config(1);
                btdrv_hcioff();
#endif
                app_key_open(false);
                app_key_init_on_charging();
                nRet = 0;
                goto exit;
                break;
            case APP_BATTERY_OPEN_MODE_CHARGING_PWRON:
                TRACE(0,"CHARGING PWRON!");
#ifdef IBRT_SEARCH_UI
                is_charging_poweron=true;
#endif
                need_check_key = false;
                nRet = 0;
                break;
            case APP_BATTERY_OPEN_MODE_INVALID:
            default:
                nRet = -1;
                goto exit;
                break;
        }
    }
#endif /* __STDF_EXTERNAL_CHAGER_ENABLE__ */

    if (app_key_open(need_check_key)){
        TRACE(0,"PWR KEY DITHER!");
        nRet = -1;
        goto exit;
    }

    hal_sw_bootmode_set(HAL_SW_BOOTMODE_REBOOT);
    app_poweron_key_init();
#if defined(_AUTO_TEST_)
    AUTO_TEST_SEND("Power on.");
#endif
    app_bt_init();
    af_open();
    app_audio_open();
    app_audio_manager_open();
    app_overlay_open();

    nv_record_env_init();
    nvrec_dev_data_open();
    factory_section_open();
//    app_bt_connect2tester_init();
    nv_record_env_get(&nvrecord_env);

#ifdef PROMPT_IN_FLASH
    nv_record_prompt_rec_init();
#endif

#ifdef __IAG_BLE_INCLUDE__
    app_ble_mode_init();
    app_ble_customif_init();
#endif

#ifdef IS_MULTI_AI_ENABLED
    init_ai_mode();
#endif

#ifdef __AI_VOICE__
#ifdef __AMA_VOICE__
        ai_spec = AI_SPEC_AMA;
#elif defined (__DMA_VOICE__)
        ai_spec = AI_SPEC_BAIDU;
#elif defined (__SMART_VOICE__)
        ai_spec = AI_SPEC_BES;
#elif defined (__TENCENT_VOICE__)
        ai_spec = AI_SPEC_TENCENT;
#endif

        if(ai_open(ai_spec)) {
            nRet = -1;
            goto exit;
        }

#ifdef KNOWLES_UART_DATA
        app_ai_voice_uart_audio_init();
#endif
#endif

#ifdef __PC_CMD_UART__
    app_cmd_open();
#endif

#ifdef AUDIO_DEBUG
    speech_tuning_init();
#endif

#if defined(ANC_ASSIST_ENABLED)
    app_anc_assist_init();
    app_voice_assist_ai_voice_init();
#if defined(ANC_ASSIST_USE_PILOT)
    app_voice_assist_wd_init();
#if defined(VOICE_ASSIST_PILOT_ANC_ENABLED)
    app_voice_assist_pilot_anc_init();
#endif
#if defined(VOICE_ASSIST_WD_ENABLED)
	app_voice_assist_ultrasound_init();
#endif
    app_voice_assist_custom_leak_detect_init();
#endif
    app_voice_assist_prompt_leak_detect_init();
#if defined(AUDIO_ADAPTIVE_EQ)
    app_voice_assist_adaptive_eq_init();
#endif
#if defined(VOICE_ASSIST_NOISE_ADAPT_ANC_ENABLED)
    app_voice_assist_noise_adapt_anc_init();
#endif
#endif

#ifdef ANC_APP
    app_anc_init();
#endif

#if defined(MEDIA_PLAYER_SUPPORT) && !defined(PROMPT_IN_FLASH)
    app_play_audio_set_lang(nvrecord_env->media_language.language);
#endif

    app_bt_stream_volume_ptr_update(NULL);

#ifdef __THIRDPARTY
    app_thirdparty_specific_lib_event_handle(THIRDPARTY_ID_NO2,THIRDPARTY_INIT);
#endif

#if defined(BES_OTA_BASIC)
    ota_flash_init();
#endif

#if defined(GSOUND_OTA_ENABLED)
    GSoundOtaHandlerInit();
#endif

#ifdef __STDF__
    stdf_init();
#endif

    btdrv_start_bt();
    if (pwron_case != APP_POWERON_CASE_TEST) {
        BesbtInit();
        app_wait_stack_ready();
        bt_drv_extra_config_after_init();
        bt_generate_ecdh_key_pair();
        app_bt_start_custom_function_in_bt_thread((uint32_t)0,
                    0, (uint32_t)app_ibrt_init);
#if defined(USE_LOWLATENCY_LIB)
        app_bt_start_custom_function_in_bt_thread( 0, 0, (uint32_t)app_lowlatency_custom_lib_init );
#endif
    }

    app_sysfreq_req(APP_SYSFREQ_USER_APP_INIT, APP_SYSFREQ_52M);
    TRACE(1,"\n\n\nbt_stack_init_done:%d\n\n\n", pwron_case);
		
    if (pwron_case == APP_POWERON_CASE_REBOOT){

        app_bt_sleep_init();

#ifdef __AI_VOICE__
        app_ai_spp_server_init();
#endif

#if defined(BES_OTA_BASIC)
        bes_ota_init();
#endif

#ifdef __INTERACTION__
        app_interaction_init();
#endif

#if defined(__IAG_BLE_INCLUDE__) && defined(BTIF_BLE_APP_DATAPATH_SERVER)
        BLE_custom_command_init();
#endif
        app_bt_accessmode_set(BTIF_BAM_NOT_ACCESSIBLE);

        app_key_init();
        app_battery_start();
#if defined(__BTIF_EARPHONE__) && defined(__BTIF_AUTOPOWEROFF__)
        app_start_10_second_timer(APP_POWEROFF_TIMER_ID);
#endif

#if defined(__IAG_BLE_INCLUDE__) && defined(BTIF_BLE_APP_DATAPATH_SERVER)
        BLE_custom_command_init();
#endif
#ifdef __THIRDPARTY
        app_thirdparty_specific_lib_event_handle(THIRDPARTY_ID_NO1,THIRDPARTY_INIT);
        app_thirdparty_specific_lib_event_handle(THIRDPARTY_ID_NO1,THIRDPARTY_START);
        app_thirdparty_specific_lib_event_handle(THIRDPARTY_ID_NO2,THIRDPARTY_BT_CONNECTABLE);
        app_thirdparty_specific_lib_event_handle(THIRDPARTY_ID_NO3,THIRDPARTY_START);
#endif
#if defined( __BTIF_EARPHONE__) && defined(__BTIF_BT_RECONNECT__)
#if !defined(IBRT)
        app_bt_profile_connect_manager_opening_reconnect();
#endif
#endif
    }
#ifdef __ENGINEER_MODE_SUPPORT__
    else if(pwron_case == APP_POWERON_CASE_TEST){
        app_factorymode_set(true);
        app_status_indication_set(APP_STATUS_INDICATION_POWERON);
#ifdef MEDIA_PLAYER_SUPPORT
        app_voice_report(APP_STATUS_INDICATION_POWERON, 0);
#endif
#ifdef __WATCHER_DOG_RESET__
        app_wdt_close();
#endif
        TRACE(0,"!!!!!ENGINEER_MODE!!!!!\n");
        nRet = 0;
        app_nvrecord_rebuild();
        app_factorymode_key_init();
        if (hal_sw_bootmode_get() & HAL_SW_BOOTMODE_TEST_SIGNALINGMODE){
            hal_sw_bootmode_clear(HAL_SW_BOOTMODE_TEST_MASK);
            app_factorymode_bt_signalingtest(NULL, NULL);
        }
        if (hal_sw_bootmode_get() & HAL_SW_BOOTMODE_TEST_NOSIGNALINGMODE){
            hal_sw_bootmode_clear(HAL_SW_BOOTMODE_TEST_MASK);
            app_factorymode_bt_nosignalingtest(NULL, NULL);
        }
    }
#endif
    else{
        app_status_indication_set(APP_STATUS_INDICATION_POWERON);
#ifdef MEDIA_PLAYER_SUPPORT
        app_voice_report(APP_STATUS_INDICATION_POWERON, 0);
#endif
        if (need_check_key){
            pwron_case = app_poweron_wait_case();
        }
        else
        {
            pwron_case = APP_POWERON_CASE_NORMAL;
        }
        if (pwron_case != APP_POWERON_CASE_INVALID && pwron_case != APP_POWERON_CASE_DITHERING){
            TRACE(1,"hello world i'm bes1000 hahaha case:%d\n", pwron_case);
            nRet = 0;
#ifndef __POWERKEY_CTRL_ONOFF_ONLY__
            app_status_indication_set(APP_STATUS_INDICATION_INITIAL);
#endif
            app_bt_sleep_init();
#ifdef __AI_VOICE__
            app_ai_spp_server_init();
#endif

#ifdef BES_OTA_BASIC
            bes_ota_init();
#endif

#ifdef __INTERACTION__
            app_interaction_init();
#endif

#if defined(__IAG_BLE_INCLUDE__) && defined(BTIF_BLE_APP_DATAPATH_SERVER)
            BLE_custom_command_init();
#endif
#ifdef GFPS_ENABLED
             app_gfps_set_battery_info_acquire_handler(app_tell_battery_info_handler);
             app_gfps_set_battery_datatype(SHOW_UI_INDICATION);
#endif
            switch (pwron_case) {
                case APP_POWERON_CASE_CALIB:
                    break;
                case APP_POWERON_CASE_BOTHSCAN:
                    app_status_indication_set(APP_STATUS_INDICATION_BOTHSCAN);
#ifdef MEDIA_PLAYER_SUPPORT
                    app_voice_report(APP_STATUS_INDICATION_BOTHSCAN,0);
#endif
#if defined( __BTIF_EARPHONE__)
#if defined(IBRT)
#ifdef IBRT_SEARCH_UI
                    if(false==is_charging_poweron)
                        app_ibrt_enter_limited_mode();
#endif
#else
                    app_bt_accessmode_set(BTIF_BT_DEFAULT_ACCESS_MODE_PAIR);
#endif
#ifdef GFPS_ENABLED
                    app_enter_fastpairing_mode();
#endif
#if defined(__BTIF_AUTOPOWEROFF__)
                    app_start_10_second_timer(APP_PAIR_TIMER_ID);
#endif
#endif
#ifdef __THIRDPARTY
                    app_thirdparty_specific_lib_event_handle(THIRDPARTY_ID_NO2,THIRDPARTY_BT_DISCOVERABLE);
#endif
                    break;
                case APP_POWERON_CASE_NORMAL:
#if defined( __BTIF_EARPHONE__ ) && !defined(__EARPHONE_STAY_BOTH_SCAN__)
#if defined(IBRT)
#ifdef IBRT_SEARCH_UI
                    if(is_charging_poweron==false)
                    {
                        if(IBRT_UNKNOW == nvrecord_env->ibrt_mode.mode)
                        {
                            TRACE(0,"ibrt_ui_log:power on unknow mode");
                            app_ibrt_enter_limited_mode();
                        }
                        else
                        {
                            TRACE(1,"ibrt_ui_log:power on %d fetch out", nvrecord_env->ibrt_mode.mode);
                            app_ibrt_ui_event_entry(IBRT_FETCH_OUT_EVENT);
                        }
                    }
#endif
#else
                    app_bt_accessmode_set(BTIF_BAM_NOT_ACCESSIBLE);
#endif
#endif
                case APP_POWERON_CASE_REBOOT:
                case APP_POWERON_CASE_ALARM:
                default:
                    app_status_indication_set(APP_STATUS_INDICATION_PAGESCAN);
#if defined( __BTIF_EARPHONE__) && defined(__BTIF_BT_RECONNECT__) && !defined(IBRT)
                    app_bt_profile_connect_manager_opening_reconnect();
#endif
#ifdef __THIRDPARTY
                    app_thirdparty_specific_lib_event_handle(THIRDPARTY_ID_NO2,THIRDPARTY_BT_CONNECTABLE);
#endif

                    break;
            }
            if (need_check_key)
            {
#ifndef __POWERKEY_CTRL_ONOFF_ONLY__
                app_poweron_wait_finished();
#endif
            }
            app_key_init();
            app_battery_start();
#if defined(__BTIF_EARPHONE__) && defined(__BTIF_AUTOPOWEROFF__)
            app_start_10_second_timer(APP_POWEROFF_TIMER_ID);
#endif
#ifdef __THIRDPARTY
            app_thirdparty_specific_lib_event_handle(THIRDPARTY_ID_NO1,THIRDPARTY_INIT);
            app_thirdparty_specific_lib_event_handle(THIRDPARTY_ID_NO1,THIRDPARTY_START);
            app_thirdparty_specific_lib_event_handle(THIRDPARTY_ID_NO3,THIRDPARTY_START);
#endif

#ifdef RB_CODEC
            rb_ctl_init();
#endif
        }else{
            af_close();
            app_key_close();
            nRet = -1;
        }
    }
exit:

#ifdef __BT_DEBUG_TPORTS__
    {
       extern void bt_enable_tports(void);
       bt_enable_tports();
       //hal_iomux_tportopen();
    }
#endif

#ifdef BT_USB_AUDIO_DUAL_MODE
    btusb_init();
    if(usb_plugin)
    {
        btusb_start_switch_to(BTUSB_MODE_USB);
    }
    else
    {
        btusb_switch(BTUSB_MODE_BT);
    }
#else //BT_USB_AUDIO_DUAL_MODE
#if defined(BTUSB_AUDIO_MODE)
    if(pwron_case == APP_POWERON_CASE_CHARGING) {
        app_wdt_close();
        af_open();
        app_key_handle_clear();
        app_usb_key_init();
        app_usbaudio_entry();
    }

#endif // BTUSB_AUDIO_MODE
#endif // BT_USB_AUDIO_DUAL_MODE
#if defined(IBRT)
    app_ibrt_ui_rssi_reset();
#endif

#if defined(SIMPLE_TEST_UI)
    app_ibrt_if_enter_pairing_after_tws_connected();
#endif

#if defined(__STDF__)
    //stdf_init();
#endif
    
    app_sysfreq_req(APP_SYSFREQ_USER_APP_INIT, APP_SYSFREQ_32K);

    return nRet;
}

#endif /* APP_TEST_MODE */
