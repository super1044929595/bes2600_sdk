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
#include <assert.h>

#include "cmsis_os.h"
#include "tgt_hardware.h"
#include "hal_uart.h"
#include "hal_timer.h"
#include "audioflinger.h"
#include "lockcqueue.h"
#include "hal_trace.h"
#include "hal_cmu.h"
#include "hal_overlay.h"
#include "analog.h"
#include "app_bt_stream.h"
#include "app_overlay.h"
#include "app_audio.h"
#include "app_utils.h"
#ifdef ANC_APP
#include "app_anc.h"
#endif
#include "bluetooth.h"
#include "nvrecord.h"
#include "nvrecord_env.h"
#include "nvrecord_dev.h"
#include "resample_coef.h"
#include "hal_codec.h"
#ifdef MEDIA_PLAYER_SUPPORT
#include "resources.h"
#include "app_media_player.h"
#endif
#ifdef __FACTORY_MODE_SUPPORT__
#include "app_factory_audio.h"
#endif
#ifdef TX_RX_PCM_MASK
#include "hal_chipid.h"
#endif

#ifdef  __IAG_BLE_INCLUDE__
#include "app_ble_mode_switch.h"
#endif

#ifdef VOICE_DATAPATH
#include "app_voicepath.h"
#endif


#include "app_ring_merge.h"
#include "bt_drv.h"
#include "bt_xtal_sync.h"
#include "bt_drv_reg_op.h"
#include "besbt.h"
#include "hal_chipid.h"
#include "cqueue.h"
#include "btapp.h"
#include "app_hfp.h"
#include "app_bt.h"
#include "os_api.h"
#include "audio_process.h"
#include "app_a2dp.h"
#include "a2dp_decoder.h"
#include "codec_int.h"
#if defined(__AUDIO_SPECTRUM__)
#include "audio_spectrum.h"
#endif

#ifdef __AI_VOICE__
#include "app_ai_voice.h"
#include "ai_manager.h"
#include "ai_thread.h"
#endif

#if defined(BONE_SENSOR_TDM)
#include "speech_utils.h"
#include "lis25ba.h"
#include "tdm_stream.h"
#endif

#if defined(ANC_NOISE_TRACKER)
#include "noise_tracker.h"
#include "noise_tracker_callback.h"
#endif

#if defined(IBRT)
#include "app_ibrt_if.h"
#include "app_tws_ctrl_thread.h"
#include "app_tws_ibrt_audio_analysis.h"
#include "app_tws_ibrt_audio_sync.h"
#include "app_ibrt_a2dp.h"
#include "app_ibrt_rssi.h"
#undef MUSIC_DELAY_CONTROL
#endif


#if defined(ANC_ASSIST_ENABLED)
#include "app_anc_assist.h"
#if defined(AUDIO_ADAPTIVE_EQ)
#include "app_voice_assist_adaptive_eq.h"
#endif
#if defined(ASSIST_LOW_RAM_MOD)
#include "custom_allocator.h"
#include "anc_assist_resample.h"
int32_t *anc_assist_resample_buf = NULL;
#endif
#endif

#ifdef SPEECH_SIDETONE
#ifdef HW_SIDETONE_IIR_PROCESS
#include "sidetone_process.h"
extern const IIR_CFG_T * const  audio_sidetone_iir_cfg_list[];
int sidetone_opened=0;
#endif
#endif

uint16_t app_bt_stream_get_skip_frame();

#if defined(USE_LOWLATENCY_LIB)
#include "app_ally.h"
extern void a2dp_audio_set_dest_packet_mut(uint16_t dest_packet_mut);
#include "app_ibrt_customif_cmd.h"
#include "app_a2dp_adbf.h"
uint32_t app_adbuf_get_adbuf_delay();
#include "app_adpc.h"
#include "app_a2dp_packet_sync.h"
extern int a2dp_audio_aac_add_frame(uint16_t nframe);
extern int a2dp_audio_aac_remove_frame(uint16_t nframe);
extern int a2dp_audio_sbc_add_frame(uint16_t nframe);
extern int a2dp_audio_sbc_remove_frame(uint16_t nframe);
#endif
#if defined(__SW_IIR_EQ_PROCESS__)
static uint8_t audio_eq_sw_iir_index = 0;
extern const IIR_CFG_T * const audio_eq_sw_iir_cfg_list[];
#endif

#if defined(__HW_FIR_EQ_PROCESS__)
static uint8_t audio_eq_hw_fir_index = 0;
extern const FIR_CFG_T * const audio_eq_hw_fir_cfg_list[];
#if defined(AUDIO_ADAPTIVE_FIR_EQ)
extern FIR_CFG_T audio_eq_hw_fir_adaptive_eq_cfg;
#endif
#endif

#if defined(__HW_DAC_IIR_EQ_PROCESS__)
static uint8_t audio_eq_hw_dac_iir_index = 0;
extern const IIR_CFG_T * const audio_eq_hw_dac_iir_cfg_list[];
#if defined(AUDIO_ADAPTIVE_IIR_EQ)
extern IIR_CFG_T audio_eq_hw_dac_iir_adaptive_eq_cfg;
#endif
#endif

#include "audio_prompt_sbc.h"

#if defined(__HW_IIR_EQ_PROCESS__)
static uint8_t audio_eq_hw_iir_index = 0;
extern const IIR_CFG_T * const audio_eq_hw_iir_cfg_list[];
#endif

#if defined(HW_DC_FILTER_WITH_IIR)
#include "hw_filter_codec_iir.h"
#include "hw_codec_iir_process.h"

hw_filter_codec_iir_cfg POSSIBLY_UNUSED adc_iir_cfg = {
    .bypass = 0,
    .iir_device = HW_CODEC_IIR_ADC,
#if 1
    .iir_cfg = {
        .iir_filtes_l = {
            .iir_bypass_flag = 0,
            .iir_counter = 2,
            .iir_coef = {
                    {{0.994406, -1.988812, 0.994406}, {1.000000, -1.988781, 0.988843}}, // iir_designer('highpass', 0, 20, 0.7, 16000);
                    {{4.0, 0.0, 0.0}, {1.0, 0.0, 0.0}},
            }
        },
        .iir_filtes_r = {
            .iir_bypass_flag = 0,
            .iir_counter = 2,
            .iir_coef = {
                    {{0.994406, -1.988812, 0.994406}, {1.000000, -1.988781, 0.988843}},
                    {{4.0, 0.0, 0.0}, {1.0, 0.0, 0.0}},
            }
        }
    }
#else
    .iir_cfg = {
        .gain0 = 0,
        .gain1 = 0,
        .num = 1,
        .param = {
            {IIR_TYPE_HIGH_PASS, 0,   20.0,   0.7},
        }
    }
#endif
};

hw_filter_codec_iir_state *hw_filter_codec_iir_st;
#endif

#if defined(CHIP_BEST2300) || defined(CHIP_BEST2300P) || defined(CHIP_BEST3003)|| \
     defined(CHIP_BEST1400) || defined(CHIP_BEST1402)   || defined(CHIP_BEST1000) || \
     defined(CHIP_BEST2000) || defined(CHIP_BEST3001)   || defined(CHIP_BEST2001) \

#undef AUDIO_RESAMPLE_ANTI_DITHER

#else
#define  AUDIO_RESAMPLE_ANTI_DITHER
#endif

#include "audio_cfg.h"

extern uint8_t bt_audio_get_eq_index(AUDIO_EQ_TYPE_T audio_eq_type,uint8_t anc_status);
extern uint32_t bt_audio_set_eq(AUDIO_EQ_TYPE_T audio_eq_type,uint8_t index);
extern uint8_t bt_audio_updata_eq_for_anc(uint8_t anc_status);

#include "app_bt_media_manager.h"

#include "string.h"
#include "hal_location.h"

#include "bt_drv_interface.h"

#include "audio_resample_ex.h"

#if defined(CHIP_BEST1400) || defined(CHIP_BEST1402)
#define BT_INIT_XTAL_SYNC_FCAP_RANGE (0x1FF)
#else
#define BT_INIT_XTAL_SYNC_FCAP_RANGE (0xFF)
#endif
#define BT_INIT_XTAL_SYNC_MIN (20)
#define BT_INIT_XTAL_SYNC_MAX (BT_INIT_XTAL_SYNC_FCAP_RANGE - BT_INIT_XTAL_SYNC_MIN)

#ifdef __THIRDPARTY
#include "app_thirdparty.h"
#endif

#if defined(AUDIO_ANC_FB_MC) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
#include"anc_process.h"

#ifdef ANC_FB_MC_96KHZ
#define DELAY_SAMPLE_MC (29*2)     //  2:ch
#define SAMPLERATE_RATIO_THRESHOLD (4)
#else
#define DELAY_SAMPLE_MC (31*2)     //  2:ch
#define SAMPLERATE_RATIO_THRESHOLD (8)
#endif

static int32_t delay_buf_bt[DELAY_SAMPLE_MC];
#endif

#ifdef ANC_APP
static uint8_t anc_status_record = 0xff;
#endif

#if defined(SCO_DMA_SNAPSHOT)

#define BTCLK_CNT_REF_UNIT (625.0f)

#if defined(CHIP_BEST1501) || defined(CHIP_BEST2003)
#define BTCLK_CNT_UNIT (312.5f)
#define MAX_BT_CLOCK   ((1L<<28) - 1)
#else
#define BTCLK_CNT_UNIT (625.0f)
#define MAX_BT_CLOCK   ((1L<<27) - 1)
#endif

#ifdef PCM_FAST_MODE
#define MASTER_MOBILE_BTCLK_OFFSET (4)
#elif TX_RX_PCM_MASK
#define MASTER_MOBILE_BTCLK_OFFSET (8)
#else
#define MASTER_MOBILE_BTCLK_OFFSET (4)
#endif

#if defined(LOW_DELAY_SCO)
#define BUF_BTCLK_NUM ((uint32_t)(12*(BTCLK_CNT_REF_UNIT/BTCLK_CNT_UNIT)))
#else
#define BUF_BTCLK_NUM ((uint32_t)(24*(BTCLK_CNT_REF_UNIT/BTCLK_CNT_UNIT)))
#endif

#define WRAPED_CLK_OFFSET  (((MAX_BT_CLOCK/BUF_BTCLK_NUM+1)*BUF_BTCLK_NUM)&MAX_BT_CLOCK)

#define MASTER_MOBILE_BTCNT_OFFSET (MASTER_MOBILE_BTCLK_OFFSET*BTCLK_CNT_REF_UNIT)

#if defined(CHIP_BEST1400) || defined(CHIP_BEST1402) || defined(CHIP_BEST2300P) || defined(CHIP_BEST2300A) || defined(CHIP_BEST2001)|| defined(CHIP_BEST1501) || defined(CHIP_BEST2003) || defined(CHIP_BEST1305)
#define MUTE_PATTERN (0x55)
#else
#define MUTE_PATTERN (0x00)
#endif

extern void  app_tws_ibrt_audio_mobile_clkcnt_get(uint32_t btclk, uint16_t btcnt,
                                                     uint32_t *mobile_master_clk, uint16_t *mobile_master_cnt);

static uint8_t *playback_buf_codecpcm;
static uint32_t playback_size_codecpcm;
static uint8_t *capture_buf_codecpcm;
static uint32_t capture_size_codecpcm;

static uint8_t *playback_buf_btpcm;
static uint32_t playback_size_btpcm;
static uint8_t *capture_buf_btpcm;
static uint32_t capture_size_btpcm;

#ifdef TX_RX_PCM_MASK
static uint8_t *playback_buf_btpcm_copy=NULL;
static uint32_t playback_size_btpcm_copy=0;
static uint8_t *capture_buf_btpcm_copy=NULL;
static uint32_t capture_size_btpcm_copy=0;
#endif

volatile int sco_btpcm_mute_flag=0;
volatile int sco_disconnect_mute_flag=0;

static uint8_t *playback_buf_btpcm_cache=NULL;

static enum AUD_SAMPRATE_T playback_samplerate_codecpcm;
static int32_t mobile_master_clk_offset_init;
static uint32_t last_mobile_master_clk=0;
#endif

#if defined(TX_RX_PCM_MASK) || defined(PCM_PRIVATE_DATA_FLAG)
struct PCM_DATA_FLAG_T pcm_data_param[4];
#define PCM_PRIVATE_DATA_LENGTH 11
#define BTPCM_PRIVATE_DATA_LENGTH 32
#define BTPCM_PUBLIC_DATA_LENGTH 120
#define BTPCM_TOTAL_DATA_LENGTH (BTPCM_PRIVATE_DATA_LENGTH+BTPCM_PUBLIC_DATA_LENGTH)
#endif

enum PLAYER_OPER_T
{
    PLAYER_OPER_START,
    PLAYER_OPER_STOP,
    PLAYER_OPER_RESTART,
};

#if defined(AF_ADC_I2S_SYNC)
extern "C" void hal_codec_capture_enable(void);
extern "C" void hal_codec_capture_enable_delay(void);

static uint32_t codec_capture_cnt = 0;
static uint32_t codec_playback_cnt = 0;
#endif

#if (AUDIO_OUTPUT_VOLUME_DEFAULT < 1) || (AUDIO_OUTPUT_VOLUME_DEFAULT > 17)
#error "AUDIO_OUTPUT_VOLUME_DEFAULT out of range"
#endif
int8_t stream_local_volume = (AUDIO_OUTPUT_VOLUME_DEFAULT);
#ifdef AUDIO_LINEIN
int8_t stream_linein_volume = (AUDIO_OUTPUT_VOLUME_DEFAULT);
#endif

struct btdevice_volume *btdevice_volume_p;
struct btdevice_volume current_btdevice_volume;

#ifdef __BT_ANC__
uint8_t bt_sco_samplerate_ratio = 0;
static uint8_t *bt_anc_sco_dec_buf;
extern void us_fir_init(void);
extern uint32_t voicebtpcm_pcm_resample (short* src_samp_buf, uint32_t src_smpl_cnt, short* dst_samp_buf);
#endif

#if defined(AUDIO_ANC_FB_MC) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
static enum AUD_BITS_T sample_size_play_bt;
static enum AUD_SAMPRATE_T sample_rate_play_bt;
static uint32_t data_size_play_bt;

static uint8_t *playback_buf_bt;
static uint32_t playback_size_bt;
static int32_t playback_samplerate_ratio_bt;

static uint8_t *playback_buf_mc;
static uint32_t playback_size_mc;
static enum AUD_CHANNEL_NUM_T  playback_ch_num_bt;
#endif

#if defined(MUSIC_DELAY_CONTROL) && (defined(CHIP_BEST2300) || defined(CHIP_BEST2300P)|| defined(CHIP_BEST2300A))
static enum AUD_BITS_T lowdelay_sample_size_play_bt;
static enum AUD_SAMPRATE_T lowdelay_sample_rate_play_bt;
static uint32_t lowdelay_data_size_play_bt;
static enum AUD_CHANNEL_NUM_T  lowdelay_playback_ch_num_bt;
#endif

extern "C" uint8_t is_sbc_mode (void);
uint8_t bt_sbc_mode;
extern "C" uint8_t __attribute__((section(".fast_text_sram")))  is_sbc_mode(void)
{
    return bt_sbc_mode;
}

extern "C" uint8_t is_sco_mode (void);

uint8_t bt_sco_mode;
extern "C"   uint8_t __attribute__((section(".fast_text_sram")))  is_sco_mode(void)
{
    return bt_sco_mode;
}

#define APP_BT_STREAM_TRIGGER_TIMEROUT (3000)

#define TRIGGER_CHECKER_A2DP_PLAYERBLACK        (1<<0)
#define TRIGGER_CHECKER_A2DP_DONE               (TRIGGER_CHECKER_A2DP_PLAYERBLACK)

#define TRIGGER_CHECKER_HFP_BTPCM_PLAYERBLACK   (1<<1)
#define TRIGGER_CHECKER_HFP_BTPCM_CAPTURE       (1<<2)
#define TRIGGER_CHECKER_HFP_AUDPCM_PLAYERBLACK  (1<<3)
#define TRIGGER_CHECKER_HFP_AUDPCM_CAPTURE      (1<<4)
#define TRIGGER_CHECKER_HFP_DONE                (TRIGGER_CHECKER_HFP_BTPCM_PLAYERBLACK|TRIGGER_CHECKER_HFP_BTPCM_CAPTURE|TRIGGER_CHECKER_HFP_AUDPCM_PLAYERBLACK|TRIGGER_CHECKER_HFP_AUDPCM_CAPTURE)

extern uint32_t app_bt_stream_get_dma_buffer_samples(void);
static bool is_need_discards_samples =false;
static bool app_bt_stream_trigger_enable = 0;
static uint32_t app_bt_stream_trigger_checker = 0;
static void app_bt_stream_trigger_timeout_cb(void const *n);
osTimerDef(APP_BT_STREAM_TRIGGER_TIMEOUT, app_bt_stream_trigger_timeout_cb); 
osTimerId app_bt_stream_trigger_timeout_id = NULL;

static void app_bt_stream_trigger_timeout_cb(void const *n)
{
    TRACE(1,"%s\n", __func__);
    if (app_bt_stream_isrun(APP_BT_STREAM_A2DP_SBC)){
        TRACE(1,"%s-->A2DP_SBC\n", __func__);
#if defined(IBRT)
        app_ibrt_if_force_audio_retrigger();
#else
        app_audio_sendrequest_param(APP_BT_STREAM_A2DP_SBC, (uint8_t)APP_BT_SETTING_RESTART, 0, 0);
#endif
    }else if (app_bt_stream_isrun(APP_BT_STREAM_HFP_PCM)){
        TRACE(1,"%s-->HFP_PCM\n", __func__);
        app_audio_sendrequest(APP_BT_STREAM_HFP_PCM, (uint8_t)APP_BT_SETTING_RESTART, 0);
    }

}

static int app_bt_stream_trigger_checker_init(void)
{
    if (app_bt_stream_trigger_timeout_id == NULL){
        app_bt_stream_trigger_enable = false;
        app_bt_stream_trigger_checker = 0;
        app_bt_stream_trigger_timeout_id = osTimerCreate(osTimer(APP_BT_STREAM_TRIGGER_TIMEOUT), osTimerOnce, NULL);
    }

    return 0;
}

static int app_bt_stream_trigger_checker_start(void)
{
    app_bt_stream_trigger_checker = 0;
    app_bt_stream_trigger_enable = true;
    osTimerStart(app_bt_stream_trigger_timeout_id, APP_BT_STREAM_TRIGGER_TIMEROUT);
    return 0;
}

static int app_bt_stream_trigger_checker_stop(void)
{
    app_bt_stream_trigger_enable = false;
    app_bt_stream_trigger_checker = 0;
    osTimerStop(app_bt_stream_trigger_timeout_id);
    return 0;
}

int app_bt_stream_trigger_checker_handler(uint32_t trigger_checker)
{
    bool trigger_ok = false;
    
    if (app_bt_stream_trigger_enable){
        app_bt_stream_trigger_checker |= trigger_checker;
        if (app_bt_stream_isrun(APP_BT_STREAM_A2DP_SBC)){
            if (app_bt_stream_trigger_checker == TRIGGER_CHECKER_A2DP_DONE){
                trigger_ok = true;
                if(is_need_discards_samples){
                    is_need_discards_samples = false;
                    uint32_t list_samples = 0;
                    uint32_t limter_water_line_samples = 0;
                    A2DP_AUDIO_LASTFRAME_INFO_T lastframe_info;
                    uint32_t dest_discards_samples = app_bt_stream_get_dma_buffer_samples()/2 * a2dp_audio_frame_delay_get();
                    if (a2dp_audio_lastframe_info_get(&lastframe_info)<0) {
                        app_ibrt_if_force_audio_retrigger();
                        return 0;
                    }

                    limter_water_line_samples = (a2dp_audio_dest_packet_mut_get() * lastframe_info.list_samples);
                    a2dp_audio_convert_list_to_samples(&list_samples);
                    if (list_samples > limter_water_line_samples - dest_discards_samples)
                        a2dp_audio_discards_samples(list_samples - limter_water_line_samples + dest_discards_samples);
                }
            }
        }else if (app_bt_stream_isrun(APP_BT_STREAM_HFP_PCM)){
            if (app_bt_stream_trigger_checker == TRIGGER_CHECKER_HFP_DONE){
                trigger_ok = true;
            }
        }
        if (trigger_ok){
            TRACE(1,"%s trigger ok\n", __func__);
            app_bt_stream_trigger_checker_stop();
        }
    }    
    return 0;
}

#ifdef A2DP_LHDC_ON
extern struct BT_DEVICE_T  app_bt_device;
#endif
uint16_t gStreamplayer = APP_BT_STREAM_INVALID;

uint32_t a2dp_audio_more_data(uint8_t codec_type, uint8_t *buf, uint32_t len);
int a2dp_audio_init(void);
int a2dp_audio_deinit(void);
enum AUD_SAMPRATE_T a2dp_sample_rate = AUD_SAMPRATE_48000;
uint32_t a2dp_data_buf_size;
#ifdef RB_CODEC
extern int app_rbplay_audio_onoff(bool onoff, uint16_t aud_id);
#endif

#if defined(APP_LINEIN_A2DP_SOURCE)||defined(APP_I2S_A2DP_SOURCE)
int app_a2dp_source_linein_on(bool on);
#endif
#if defined(APP_I2S_A2DP_SOURCE)
#include "app_status_ind.h"
#include "app_a2dp_source.h"
//player channel should <= capture channel number
//player must be 2 channel
#define LINEIN_PLAYER_CHANNEL (2)
#ifdef __AUDIO_INPUT_MONO_MODE__
#define LINEIN_CAPTURE_CHANNEL (1)
#else
#define LINEIN_CAPTURE_CHANNEL (2)
#endif

#if (LINEIN_CAPTURE_CHANNEL == 1)
#define LINEIN_PLAYER_BUFFER_SIZE (1024*LINEIN_PLAYER_CHANNEL)
#define LINEIN_CAPTURE_BUFFER_SIZE (LINEIN_PLAYER_BUFFER_SIZE/2)
#elif (LINEIN_CAPTURE_CHANNEL == 2)
#define LINEIN_PLAYER_BUFFER_SIZE (1024*LINEIN_PLAYER_CHANNEL)
//#define LINEIN_CAPTURE_BUFFER_SIZE (LINEIN_PLAYER_BUFFER_SIZE)
#define LINEIN_CAPTURE_BUFFER_SIZE (1024*10)
#endif

static int16_t *app_linein_play_cache = NULL;

int8_t app_linein_buffer_is_empty(void)
{
    if (app_audio_pcmbuff_length())
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

uint32_t app_linein_pcm_come(uint8_t * pcm_buf, uint32_t len)
{
    //DUMP16("%d ", pcm_buf, 10);
    DUMP8("0x%02x ", pcm_buf, 10);
    TRACE(0,"app_linein_pcm_come");
    app_audio_pcmbuff_put(pcm_buf, len);

    return len;
}

uint32_t app_linein_need_pcm_data(uint8_t* pcm_buf, uint32_t len)
{

#if (LINEIN_CAPTURE_CHANNEL == 1)
    app_audio_pcmbuff_get((uint8_t *)app_linein_play_cache, len/2);
    //app_play_audio_lineinmode_more_data((uint8_t *)app_linein_play_cache,len/2);
    app_bt_stream_copy_track_one_to_two_16bits((int16_t *)pcm_buf, app_linein_play_cache, len/2/2);
#elif (LINEIN_CAPTURE_CHANNEL == 2)
    app_audio_pcmbuff_get((uint8_t *)pcm_buf, len);
    //app_play_audio_lineinmode_more_data((uint8_t *)pcm_buf, len);
#endif

#if defined(__AUDIO_OUTPUT_MONO_MODE__)
    merge_stereo_to_mono_16bits((int16_t *)buf, (int16_t *)pcm_buf, len/2);
#endif

#ifdef ANC_APP
    bt_audio_updata_eq_for_anc(app_anc_work_status());
#endif

    audio_process_run(pcm_buf, len);

    return len;
}
extern "C" void pmu_linein_onoff(unsigned char en);
extern "C" int hal_analogif_reg_read(unsigned short reg, unsigned short *val);
int app_a2dp_source_I2S_onoff(bool onoff)
{
    static bool isRun =  false;
    uint8_t *linein_audio_cap_buff = 0;
    uint8_t *linein_audio_play_buff = 0;
    uint8_t *linein_audio_loop_buf = NULL;
    struct AF_STREAM_CONFIG_T stream_cfg;

    TRACE(2,"app_a2dp_source_I2S_onoff work:%d op:%d", isRun, onoff);

    if (isRun == onoff)
        return 0;

    if (onoff)
    {
        app_sysfreq_req(APP_SYSFREQ_USER_APP_0, APP_SYSFREQ_104M);
        app_overlay_select(APP_OVERLAY_A2DP);
        app_audio_mempool_init();
        app_audio_mempool_get_buff(&linein_audio_cap_buff, LINEIN_CAPTURE_BUFFER_SIZE);
//        app_audio_mempool_get_buff(&linein_audio_play_buff, LINEIN_PLAYER_BUFFER_SIZE);
//        app_audio_mempool_get_buff(&linein_audio_loop_buf, LINEIN_PLAYER_BUFFER_SIZE<<2);
//        app_audio_pcmbuff_init(linein_audio_loop_buf, LINEIN_PLAYER_BUFFER_SIZE<<2);

#if (LINEIN_CAPTURE_CHANNEL == 1)
        app_audio_mempool_get_buff((uint8_t **)&app_linein_play_cache, LINEIN_PLAYER_BUFFER_SIZE/2/2);
        //app_play_audio_lineinmode_init(LINEIN_CAPTURE_CHANNEL, LINEIN_PLAYER_BUFFER_SIZE/2/2);
#elif (LINEIN_CAPTURE_CHANNEL == 2)
        //app_play_audio_lineinmode_init(LINEIN_CAPTURE_CHANNEL, LINEIN_PLAYER_BUFFER_SIZE/2);
#endif

        memset(&stream_cfg, 0, sizeof(stream_cfg));

        stream_cfg.bits = AUD_BITS_16;
        stream_cfg.channel_num = (enum AUD_CHANNEL_NUM_T)LINEIN_PLAYER_CHANNEL;
        stream_cfg.sample_rate = AUD_SAMPRATE_44100;

#if 0
#if FPGA==0
        stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
#else
        stream_cfg.device = AUD_STREAM_USE_EXT_CODEC;
#endif

        stream_cfg.vol = 10;//stream_linein_volume;
        //TRACE(1,"vol = %d",stream_linein_volume);
        stream_cfg.io_path = AUD_OUTPUT_PATH_SPEAKER;
        stream_cfg.handler = app_linein_need_pcm_data;
        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(linein_audio_play_buff);
        stream_cfg.data_size = LINEIN_PLAYER_BUFFER_SIZE;
        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg);
        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
#endif

#if 1
        stream_cfg.device = AUD_STREAM_USE_I2S_SLAVE;
//     stream_cfg.io_path = AUD_INPUT_PATH_LINEIN;
//      stream_cfg.handler = app_linein_pcm_come;
        stream_cfg.handler = a2dp_source_linein_more_pcm_data;
//      stream_cfg.handler = app_linein_pcm_come;
        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(linein_audio_cap_buff);
        stream_cfg.data_size = LINEIN_CAPTURE_BUFFER_SIZE;//2k

//        pmu_linein_onoff(1);
        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE, &stream_cfg);

        audio_process_init();
    
#if defined(IBRT)
        enum AUD_CHANNEL_NUM_T sw_ch_num = AUD_CHANNEL_NUM_1;
#else
        enum AUD_CHANNEL_NUM_T sw_ch_num = stream_cfg.channel_num;
#endif
        audio_process_open(stream_cfg.sample_rate, stream_cfg.bits, sw_ch_num, stream_cfg.channel_num, NULL, 0);

        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
#endif
        //app_status_indication_set(APP_STATUS_INDICATION_LINEIN_ON);
    }
    else
    {
//       clear buffer data
        a2dp_source.pcm_queue.write=0;
        a2dp_source.pcm_queue.len=0;
        a2dp_source.pcm_queue.read=0;
//       pmu_linein_onoff(0);
        af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);

        af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        //app_status_indication_set(APP_STATUS_INDICATION_LINEIN_OFF);
        app_overlay_unloadall();
        app_sysfreq_req(APP_SYSFREQ_USER_APP_0, APP_SYSFREQ_32K);
    }

    isRun = onoff;
    TRACE(1,"%s end!\n", __func__);
    return 0;
}
#endif


enum AUD_SAMPRATE_T bt_parse_sbc_sample_rate(uint8_t sbc_samp_rate)
{
    enum AUD_SAMPRATE_T sample_rate;
    sbc_samp_rate = sbc_samp_rate & A2D_STREAM_SAMP_FREQ_MSK;

    switch (sbc_samp_rate)
    {
        case A2D_SBC_IE_SAMP_FREQ_16:
//            sample_rate = AUD_SAMPRATE_16000;
//            break;
        case A2D_SBC_IE_SAMP_FREQ_32:
//            sample_rate = AUD_SAMPRATE_32000;
//            break;
        case A2D_SBC_IE_SAMP_FREQ_48:
            sample_rate = AUD_SAMPRATE_48000;
            break;
        case A2D_SBC_IE_SAMP_FREQ_44:
            sample_rate = AUD_SAMPRATE_44100;
            break;
#if defined(A2DP_LHDC_ON) ||defined(A2DP_SCALABLE_ON)
        case A2D_SBC_IE_SAMP_FREQ_96:
            sample_rate = AUD_SAMPRATE_96000;
            break;
#endif
#if defined(A2DP_LDAC_ON)
        case A2D_SBC_IE_SAMP_FREQ_96:
            sample_rate = AUD_SAMPRATE_96000;
            break;
#endif

        default:
            ASSERT(0, "[%s] 0x%x is invalid", __func__, sbc_samp_rate);
            break;
    }
    return sample_rate;
}

void bt_store_sbc_sample_rate(enum AUD_SAMPRATE_T sample_rate)
{
    a2dp_sample_rate = sample_rate;
}

enum AUD_SAMPRATE_T bt_get_sbc_sample_rate(void)
{
    return a2dp_sample_rate;
}

enum AUD_SAMPRATE_T bt_parse_store_sbc_sample_rate(uint8_t sbc_samp_rate)
{
    enum AUD_SAMPRATE_T sample_rate;

    sample_rate = bt_parse_sbc_sample_rate(sbc_samp_rate);
    bt_store_sbc_sample_rate(sample_rate);

    return sample_rate;
}

int bt_sbc_player_setup(uint8_t freq)
{
    struct AF_STREAM_CONFIG_T *stream_cfg = NULL;
    static uint8_t sbc_samp_rate = 0xff;
    uint32_t ret;

    if (sbc_samp_rate == freq)
        return 0;

    switch (freq)
    {
        case A2D_SBC_IE_SAMP_FREQ_16:
        case A2D_SBC_IE_SAMP_FREQ_32:
        case A2D_SBC_IE_SAMP_FREQ_48:
            a2dp_sample_rate = AUD_SAMPRATE_48000;
            break;
#if defined(A2DP_LHDC_ON) ||defined(A2DP_SCALABLE_ON)
        case A2D_SBC_IE_SAMP_FREQ_96:
            a2dp_sample_rate = AUD_SAMPRATE_96000;
            TRACE(2,"%s:Sample rate :%d", __func__, freq);
            break;
#endif
#ifdef  A2DP_LDAC_ON
        case A2D_SBC_IE_SAMP_FREQ_96:
            a2dp_sample_rate = AUD_SAMPRATE_96000;
            TRACE(2,"%s:Sample rate :%d", __func__, freq);
            break;
#endif

        case A2D_SBC_IE_SAMP_FREQ_44:
            a2dp_sample_rate = AUD_SAMPRATE_44100;
            break;
        default:
            break;
    }

    ret = af_stream_get_cfg(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg, true);
    if (ret == 0) {
        stream_cfg->sample_rate = a2dp_sample_rate;
        af_stream_setup(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, stream_cfg);
    }

#if defined(AUDIO_ANC_FB_MC) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
    ret = af_stream_get_cfg(AUD_STREAM_ID_2, AUD_STREAM_PLAYBACK, &stream_cfg, true);
    if (ret == 0) {
        stream_cfg->sample_rate = a2dp_sample_rate;
        sample_rate_play_bt=stream_cfg->sample_rate;
        af_stream_setup(AUD_STREAM_ID_2, AUD_STREAM_PLAYBACK, stream_cfg);
        anc_mc_run_setup(hal_codec_anc_convert_rate(sample_rate_play_bt));
    }
#endif

    sbc_samp_rate = freq;

#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
    if (audio_prompt_is_playing_ongoing())
    {
        audio_prompt_forcefully_stop();
    }
#endif

    return 0;
}

void merge_stereo_to_mono_16bits(int16_t *src_buf, int16_t *dst_buf,  uint32_t src_len)
{
    uint32_t i = 0;
    for (i = 0; i < src_len; i+=2)
    {
        dst_buf[i] = (src_buf[i]>>1) + (src_buf[i+1]>>1);
        dst_buf[i+1] = dst_buf[i];
    }
}

void merge_stereo_to_mono_24bits(int32_t *src_buf, int32_t *dst_buf,  uint32_t src_len)
{
    uint32_t i = 0;
    for (i = 0; i < src_len; i+=2)
    {
        dst_buf[i] = (src_buf[i]>>1) + (src_buf[i+1]>>1);
        dst_buf[i+1] = dst_buf[i];
    }
}

static char _player_type_str[168];
static char *_catstr(char *dst, const char *src) {
     while(*dst) dst++;
     while((*dst++ = *src++));
     return --dst;
}
static const char *player2str(uint16_t player_type) {
    const char *s = NULL;
    char _cat = 0, first = 1, *d = NULL;
    _player_type_str[0] = '\0';
    d = _player_type_str;
    d = _catstr(d, "[");
    if (player_type != 0) {
        for (int i = 15 ; i >= 0; i--) {
            _cat = 1;
            //TRACE(3,"i=%d,player_type=0x%d,player_type&(1<<i)=0x%x", i, player_type, player_type&(1<<i));
            switch(player_type&(1<<i)) {
                case 0: _cat = 0; break;
                case APP_BT_STREAM_HFP_PCM: s = "HFP_PCM"; break;
                case APP_BT_STREAM_HFP_CVSD: s = "HFP_CVSD"; break;
                case APP_BT_STREAM_HFP_VENDOR: s = "HFP_VENDOR"; break;
                case APP_BT_STREAM_A2DP_SBC: s = "A2DP_SBC"; break;
                case APP_BT_STREAM_A2DP_AAC: s = "A2DP_AAC"; break;
                case APP_BT_STREAM_A2DP_VENDOR: s = "A2DP_VENDOR"; break;
                #ifdef __FACTORY_MODE_SUPPORT__
                case APP_FACTORYMODE_AUDIO_LOOP: s = "AUDIO_LOOP"; break;
                #endif
                case APP_PLAY_BACK_AUDIO: s = "BACK_AUDIO"; break;
                #ifdef RB_CODEC
                case APP_BT_STREAM_RBCODEC: s = "RBCODEC"; break;
                #endif
                #ifdef AUDIO_LINEIN
                case APP_PLAY_LINEIN_AUDIO: s = "LINEIN_AUDIO"; break;
                #endif
                #if defined(APP_LINEIN_A2DP_SOURCE)||defined(__APP_A2DP_SOURCE__)||(APP_I2S_A2DP_SOURCE)
                case APP_A2DP_SOURCE_LINEIN_AUDIO: s = "SRC_LINEIN_AUDIO"; break;
                case APP_A2DP_SOURCE_I2S_AUDIO: s = "I2S_AUDIO"; break;
                #endif
                #ifdef VOICE_DATAPATH
                case APP_BT_STREAM_VOICEPATH: s = "VOICEPATH"; break;
                #endif
                default:  s = "UNKNOWN"; break;
            }
            if (_cat) {
                if (!first)
                    d = _catstr(d, "|");
                //TRACE(2,"d=%s,s=%s", d, s);
                d = _catstr(d, s);
                first = 0;
            }
        }
    }

    _catstr(d, "]");

    return _player_type_str;
}


#ifdef __HEAR_THRU_PEAK_DET__
#include "peak_detector.h"
// Depend on codec_dac_vol
const float pkd_vol_multiple[18] = {0.281838, 0.000010, 0.005623, 0.007943, 0.011220, 0.015849, 0.022387, 0.031623, 0.044668, 0.063096, 0.089125, 0.125893, 0.177828, 0.251189, 0.354813, 0.501187, 0.707946, 1.000000};
int app_bt_stream_local_volume_get(void);
#endif
//extern void a2dp_get_curStream_remDev(btif_remote_device_t   **           p_remDev);
uint16_t a2dp_Get_curr_a2dp_conhdl(void);
uint32_t audio_moredata_cnt;

#ifdef PLAYBACK_FORCE_48K
static struct APP_RESAMPLE_T *force48k_resample;
static int app_force48k_resample_iter(uint8_t *buf, uint32_t len);
struct APP_RESAMPLE_T *app_force48k_resample_any_open(enum AUD_CHANNEL_NUM_T chans,
        APP_RESAMPLE_ITER_CALLBACK cb, uint32_t iter_len,
        float ratio_step);
int app_playback_resample_run(struct APP_RESAMPLE_T *resamp, uint8_t *buf, uint32_t len);
#endif

#if defined(MUSIC_DELAY_CONTROL) && (defined(CHIP_BEST2300) || defined(CHIP_BEST2300P) || defined(CHIP_BEST2300A))

#define BT_USPERCLK (625)
#define BT_MUTIUSPERSECOND (1000000/BT_USPERCLK)

#define CALIB_DEVIATION_MS (2)
#define CALIB_FACTOR_MAX_THRESHOLD (0.0001f)
#define CALIB_BT_CLOCK_FACTOR_STEP (0.0000005f)

#define CALIB_FACTOR_DELAY (0.001f)

//bt time
static int32_t  bt_old_clock_us=0;
static uint32_t bt_old_clock_mutius=0;
static int32_t  bt_old_offset_us=0;

static int32_t  bt_clock_us=0;
static uint32_t bt_clock_total_mutius=0;
static int32_t  bt_total_offset_us=0;

static int32_t bt_clock_ms=0;

//local time
static uint32_t local_total_samples=0;
static uint32_t local_total_frames=0;

//static uint32_t local_clock_us=0;
static int32_t local_clock_ms=0;

//bt and local time
static uint32_t bt_local_clock_s=0;

//calib time
static int32_t calib_total_delay=0;
static int32_t calib_flag=0;

//calib factor
static float   calib_factor_offset=0.0f;
static int32_t calib_factor_flag=0;
static volatile int calib_reset=1;
#endif

bool process_delay(int32_t delay_ms)
{
#if defined(MUSIC_DELAY_CONTROL) && (defined(CHIP_BEST2300) || defined(CHIP_BEST2300P) || defined(CHIP_BEST2300A))
    if (delay_ms == 0)return 0;

    TRACE(1,"delay_ms:%d", delay_ms);

    if(calib_flag==0)
    {
        calib_total_delay=calib_total_delay+delay_ms;
        calib_flag=1;
        return 1;
    }
    else
    {
        return 0;
    }
#else
    return 0;
#endif
}

#if defined(MUSIC_DELAY_CONTROL) && (defined(CHIP_BEST2300) || defined(CHIP_BEST2300P) || defined(CHIP_BEST2300A))
void a2dp_clock_calib_process(uint32_t len)
{
//    btif_remote_device_t   * p_a2dp_remDev=NULL;
    uint32_t smplcnt = 0;
    int32_t btoffset = 0;

    uint32_t btclk = 0;
    uint32_t btcnt = 0;
    uint32_t btofs = 0;
    btclk = *((volatile uint32_t*)0xd02201fc);
    btcnt = *((volatile uint32_t*)0xd02201f8);
    btcnt=0;

   // TRACE(2,"bt_sbc_player_more_data btclk:%08x,btcnt:%08x\n", btclk, btcnt);


 //  a2dp_get_curStream_remDev(&p_a2dp_remDev);
    if(a2dp_Get_curr_a2dp_conhdl() >=0x80 && a2dp_Get_curr_a2dp_conhdl()<=0x82)
    {
        btofs = btdrv_rf_bitoffset_get( a2dp_Get_curr_a2dp_conhdl() -0x80);

        if(calib_reset==1)
        {
            calib_reset=0;

            bt_clock_total_mutius=0;

            bt_old_clock_us=btcnt;
            bt_old_clock_mutius=btclk;

            bt_total_offset_us=0;


            local_total_samples=0;
            local_total_frames=0;
            local_clock_ms = 0;

            bt_local_clock_s=0;
            bt_clock_us = 0;
            bt_clock_ms = 0;

            bt_old_offset_us=btofs;

            calib_factor_offset=0.0f;
            calib_factor_flag=0;
            calib_total_delay=0;
            calib_flag=0;
        }
        else
        {
            btoffset=btofs-bt_old_offset_us;

            if(btoffset<-BT_USPERCLK/3)
            {
                btoffset=btoffset+BT_USPERCLK;
            }
            else if(btoffset>BT_USPERCLK/3)
            {
                btoffset=btoffset-BT_USPERCLK;
            }

            bt_total_offset_us=bt_total_offset_us+btoffset;
            bt_old_offset_us=btofs;

            local_total_frames++;
            if(lowdelay_sample_size_play_bt==AUD_BITS_16)
            {
                smplcnt=len/(2*lowdelay_playback_ch_num_bt);
            }
            else
            {
                smplcnt=len/(4*lowdelay_playback_ch_num_bt);
            }

            local_total_samples=local_total_samples+smplcnt;

            bt_clock_us=btcnt-bt_old_clock_us-bt_total_offset_us;

            btoffset=btclk-bt_old_clock_mutius;
            if(btoffset<0)
            {
                btoffset=0;
            }
            bt_clock_total_mutius=bt_clock_total_mutius+btoffset;

            bt_old_clock_us=btcnt;
            bt_old_clock_mutius=btclk;

            if((bt_clock_total_mutius>BT_MUTIUSPERSECOND)&&(local_total_samples>lowdelay_sample_rate_play_bt))
            {
                bt_local_clock_s++;
                bt_clock_total_mutius=bt_clock_total_mutius-BT_MUTIUSPERSECOND;
                local_total_samples=local_total_samples-lowdelay_sample_rate_play_bt;
            }

            bt_clock_ms=(bt_clock_total_mutius*BT_USPERCLK/1000)+bt_clock_us/625;
            local_clock_ms=(local_total_samples*1000)/lowdelay_sample_rate_play_bt;

            local_clock_ms=local_clock_ms+calib_total_delay;

            //TRACE(3,"A2DP bt_clock_ms:%8d,local_clock_ms:%8d,bt_total_offset_us:%8d\n",bt_clock_ms, local_clock_ms,bt_total_offset_us);

            if(bt_clock_ms>(local_clock_ms+CALIB_DEVIATION_MS))
            {
#if defined(__AUDIO_RESAMPLE__) && defined(SW_PLAYBACK_RESAMPLE)
                app_resample_tune(a2dp_resample, CALIB_FACTOR_DELAY);
#else
                af_codec_tune(AUD_STREAM_PLAYBACK, CALIB_FACTOR_DELAY);
#endif
                calib_factor_flag=1;
                //TRACE(0,"*************1***************");
            }
            else if(bt_clock_ms<(local_clock_ms-CALIB_DEVIATION_MS))
            {
#if defined(__AUDIO_RESAMPLE__) && defined(SW_PLAYBACK_RESAMPLE)
                app_resample_tune(a2dp_resample, -CALIB_FACTOR_DELAY);
#else
                af_codec_tune(AUD_STREAM_PLAYBACK, -CALIB_FACTOR_DELAY);
#endif
                calib_factor_flag=-1;
                //TRACE(0,"*************-1***************");
            }
            else
            {
                if((calib_factor_flag==1||calib_factor_flag==-1)&&(bt_clock_ms==local_clock_ms))
                {
                    if(calib_factor_offset<CALIB_FACTOR_MAX_THRESHOLD&&calib_flag==0)
                    {
                        if(calib_factor_flag==1)
                        {
                            calib_factor_offset=calib_factor_offset+CALIB_BT_CLOCK_FACTOR_STEP;
                        }
                        else
                        {
                            calib_factor_offset=calib_factor_offset-CALIB_BT_CLOCK_FACTOR_STEP;
                        }

                    }
#if defined(__AUDIO_RESAMPLE__) && defined(SW_PLAYBACK_RESAMPLE)
                    app_resample_tune(a2dp_resample, calib_factor_offset);
#else
                    af_codec_tune(AUD_STREAM_PLAYBACK, calib_factor_offset);
#endif
                    calib_factor_flag=0;
                    calib_flag=0;
                    //TRACE(0,"*************0***************");
                }
            }
          //  TRACE(1,"factoroffset:%d\n",(int32_t)((factoroffset)*(float)10000000.0f));
        }
    }

    return;
}

#endif

bool app_if_need_fix_target_rxbit(void)
{
    return (!bt_drv_is_enhanced_ibrt_rom());
}

static uint8_t isBtPlaybackTriggered = false;

bool bt_is_playback_triggered(void)
{
    return isBtPlaybackTriggered;
}

static void bt_set_playback_triggered(bool isEnable)
{
    isBtPlaybackTriggered = isEnable;
}

FRAM_TEXT_LOC uint32_t bt_sbc_player_more_data(uint8_t *buf, uint32_t len)
{
    audio_moredata_cnt++;

    app_bt_stream_trigger_checker_handler(TRIGGER_CHECKER_A2DP_PLAYERBLACK);

    bt_set_playback_triggered(true);
#if defined(IBRT)
    app_ibrt_ui_rssi_process();
#endif
#if defined(MUSIC_DELAY_CONTROL) && (defined(CHIP_BEST2300) || defined(CHIP_BEST2300P) || defined(CHIP_BEST2300A))
    a2dp_clock_calib_process(len);
#endif

#ifdef VOICE_DATAPATH
    //if (AI_SPEC_GSOUND == ai_manager_get_current_spec())
    {
        if (app_voicepath_get_stream_pending_state(VOICEPATH_STREAMING))
        {
            af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        #ifdef MIX_MIC_DURING_MUSIC
            app_voicepath_enable_hw_sidetone(0, HW_SIDE_TONE_MAX_ATTENUATION_COEF);
        #endif
            app_voicepath_set_stream_state(VOICEPATH_STREAMING, true);
            app_voicepath_set_pending_started_stream(VOICEPATH_STREAMING, false);
        }
    }
#endif
#ifdef BT_XTAL_SYNC
#ifdef BT_XTAL_SYNC_NEW_METHOD
    if(a2dp_Get_curr_a2dp_conhdl() >=0x80 && a2dp_Get_curr_a2dp_conhdl()<=0x82)
    {
        uint32_t bitoffset = btdrv_rf_bitoffset_get( a2dp_Get_curr_a2dp_conhdl() -0x80);
        if(app_if_need_fix_target_rxbit() == false)
        {
            if(bitoffset<XTAL_OFFSET)
                bitoffset = XTAL_OFFSET;
            else if(bitoffset>SLOT_SIZE-XTAL_OFFSET)
                bitoffset = SLOT_SIZE-XTAL_OFFSET;
        }
#ifdef BT_XTAL_SYNC_SLOW        
        bt_xtal_sync_new(bitoffset,app_if_need_fix_target_rxbit(),BT_XTAL_SYNC_MODE_WITH_MOBILE);
#else
        bt_xtal_sync_new_new(bitoffset,app_if_need_fix_target_rxbit(),BT_XTAL_SYNC_MODE_WITH_MOBILE);
#endif
    }
    
#else
    bt_xtal_sync(BT_XTAL_SYNC_MODE_MUSIC);
#endif
#endif

#ifndef FPGA
    uint8_t codec_type = bt_sbc_player_get_codec_type();
    uint32_t overlay_id = 0;
    if(codec_type ==  BTIF_AVDTP_CODEC_TYPE_MPEG2_4_AAC)
    {
        overlay_id = APP_OVERLAY_A2DP_AAC;
    }

#if defined(A2DP_LHDC_ON)
    else if(codec_type ==  BTIF_AVDTP_CODEC_TYPE_NON_A2DP)
    {
        overlay_id = APP_OVERLAY_A2DP_LHDC;
    }
#endif
#if defined(A2DP_LDAC_ON)
    else if(codec_type ==  BTIF_AVDTP_CODEC_TYPE_NON_A2DP)
    {
        overlay_id = APP_OVERLAY_A2DP_LDAC;
    }
#endif


#if defined(A2DP_SCALABLE_ON)
    else if(codec_type ==  BTIF_AVDTP_CODEC_TYPE_NON_A2DP)
    {
        overlay_id = APP_OVERLAY_A2DP_SCALABLE;
    }
#endif
    else
        overlay_id = APP_OVERLAY_A2DP;

    memset(buf, 0, len);
    
    if(app_get_current_overlay() != overlay_id)
    {
        return len;
    }
#endif

#ifdef PLAYBACK_FORCE_48K
    app_playback_resample_run(force48k_resample, buf, len);
#else
    a2dp_audio_playback_handler(buf, len, audio_moredata_cnt);
#endif

#ifdef __AUDIO_SPECTRUM__
    audio_spectrum_run(buf, len);
#endif

#ifdef __AUDIO_OUTPUT_MONO_MODE__
#ifdef A2DP_EQ_24BIT
    merge_stereo_to_mono_24bits((int32_t *)buf, (int32_t *)buf, len/sizeof(int32_t));
#else
    merge_stereo_to_mono_16bits((int16_t *)buf, (int16_t *)buf, len/sizeof(int16_t));
#endif
#endif


#ifdef __HEAR_THRU_PEAK_DET__
#ifdef ANC_APP
    if(app_anc_work_status())
#endif
    {
        int vol_level = 0;
        vol_level = app_bt_stream_local_volume_get();
        peak_detector_run(buf, len, pkd_vol_multiple[vol_level]);
    }
#endif

#ifdef ANC_APP
    bt_audio_updata_eq_for_anc(app_anc_work_status());
#endif

    audio_process_run(buf, len);

#if defined(USE_LOWLATENCY_LIB)
    if(app_get_ally_flag())
        app_ibrt_a2dp_sync_analysis_before();
#endif



#if defined(IBRT) && !defined(FPGA)
    app_tws_ibrt_audio_analysis_audiohandler_tick();
#endif

    osapi_notify_evm();

    return len;
}

FRAM_TEXT_LOC void bt_sbc_player_playback_post_handler(uint8_t *buf, uint32_t len, void *cfg)
{
    POSSIBLY_UNUSED struct AF_STREAM_CONFIG_T *config = (struct AF_STREAM_CONFIG_T *)cfg;

#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
    if (audio_prompt_is_playing_ongoing())
    {
        audio_prompt_processing_handler(len, buf);
    }
#else
    app_ring_merge_more_data(buf, len);
#endif
}

#ifdef __THIRDPARTY
bool start_by_sbc = false;
#endif

#if defined(AUDIO_ANC_FB_MC) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
static int32_t mid_p_8_old_l=0;
static int32_t mid_p_8_old_r=0;
static uint32_t audio_mc_data_playback_a2dp(uint8_t *buf, uint32_t mc_len_bytes)
{
   uint32_t begin_time;
   //uint32_t end_time;
   begin_time = hal_sys_timer_get();
   TRACE(1,"music cancel: %d",begin_time);

   float left_gain;
   float right_gain;
   int playback_len_bytes,mc_len_bytes_run;
   int i,j,k;
   int delay_sample;

   hal_codec_get_dac_gain(&left_gain,&right_gain);

   //TRACE(1,"playback_samplerate_ratio:  %d",playback_samplerate_ratio);

  // TRACE(1,"left_gain:  %d",(int)(left_gain*(1<<12)));
  // TRACE(1,"right_gain: %d",(int)(right_gain*(1<<12)));

   playback_len_bytes=mc_len_bytes/playback_samplerate_ratio_bt;

   mc_len_bytes_run=mc_len_bytes/SAMPLERATE_RATIO_THRESHOLD;

    if (sample_size_play_bt == AUD_BITS_16)
    {
        int16_t *sour_p=(int16_t *)(playback_buf_bt+playback_size_bt/2);
        int16_t *mid_p=(int16_t *)(buf);
        int16_t *mid_p_8=(int16_t *)(buf+mc_len_bytes-mc_len_bytes_run);
        int16_t *dest_p=(int16_t *)buf;

        if(buf == playback_buf_mc)
        {
            sour_p=(int16_t *)playback_buf_bt;
        }

        delay_sample=DELAY_SAMPLE_MC;

        for(i=0,j=0;i<delay_sample;i=i+2)
        {
            mid_p[j++]=delay_buf_bt[i];
            mid_p[j++]=delay_buf_bt[i+1];
        }

        for(i=0;i<playback_len_bytes/2-delay_sample;i=i+2)
        {
            mid_p[j++]=sour_p[i];
            mid_p[j++]=sour_p[i+1];
        }

        for(j=0;i<playback_len_bytes/2;i=i+2)
        {
            delay_buf_bt[j++]=sour_p[i];
            delay_buf_bt[j++]=sour_p[i+1];
        }

        if(playback_samplerate_ratio_bt<=SAMPLERATE_RATIO_THRESHOLD)
        {
            for(i=0,j=0;i<playback_len_bytes/2;i=i+2*(SAMPLERATE_RATIO_THRESHOLD/playback_samplerate_ratio_bt))
            {
                mid_p_8[j++]=mid_p[i];
                mid_p_8[j++]=mid_p[i+1];
            }
        }
        else
        {
            for(i=0,j=0;i<playback_len_bytes/2;i=i+2)
            {
                for(k=0;k<playback_samplerate_ratio_bt/SAMPLERATE_RATIO_THRESHOLD;k++)
                {
                    mid_p_8[j++]=mid_p[i];
                    mid_p_8[j++]=mid_p[i+1];
                }
            }
        }

        anc_mc_run_stereo((uint8_t *)mid_p_8,mc_len_bytes_run,left_gain,right_gain,AUD_BITS_16);

        for(i=0,j=0;i<(mc_len_bytes_run)/2;i=i+2)
        {
           float delta_l=(mid_p_8[i]-mid_p_8_old_l)/(float)SAMPLERATE_RATIO_THRESHOLD;
           float delta_r=(mid_p_8[i+1]-mid_p_8_old_r)/(float)SAMPLERATE_RATIO_THRESHOLD;
            for(k=1;k<=SAMPLERATE_RATIO_THRESHOLD;k++)
            {
                dest_p[j++]=mid_p_8_old_l+(int32_t)(delta_l*k);
                dest_p[j++]=mid_p_8_old_r+(int32_t)(delta_r*k);
            }
            mid_p_8_old_l=mid_p_8[i];
            mid_p_8_old_r=mid_p_8[i+1];
        }


    }
    else if (sample_size_play_bt == AUD_BITS_24)
    {
        int32_t *sour_p=(int32_t *)(playback_buf_bt+playback_size_bt/2);
        int32_t *mid_p=(int32_t *)(buf);
        int32_t *mid_p_8=(int32_t *)(buf+mc_len_bytes-mc_len_bytes_run);
        int32_t *dest_p=(int32_t *)buf;

        if(buf == (playback_buf_mc))
        {
            sour_p=(int32_t *)playback_buf_bt;
        }

        delay_sample=DELAY_SAMPLE_MC;

        for(i=0,j=0;i<delay_sample;i=i+2)
        {
            mid_p[j++]=delay_buf_bt[i];
            mid_p[j++]=delay_buf_bt[i+1];
        }

         for(i=0;i<playback_len_bytes/4-delay_sample;i=i+2)
        {
            mid_p[j++]=sour_p[i];
            mid_p[j++]=sour_p[i+1];
        }

         for(j=0;i<playback_len_bytes/4;i=i+2)
        {
            delay_buf_bt[j++]=sour_p[i];
            delay_buf_bt[j++]=sour_p[i+1];
        }

        if(playback_samplerate_ratio_bt<=SAMPLERATE_RATIO_THRESHOLD)
        {
            for(i=0,j=0;i<playback_len_bytes/4;i=i+2*(SAMPLERATE_RATIO_THRESHOLD/playback_samplerate_ratio_bt))
            {
                mid_p_8[j++]=mid_p[i];
                mid_p_8[j++]=mid_p[i+1];
            }
        }
        else
        {
            for(i=0,j=0;i<playback_len_bytes/4;i=i+2)
            {
                for(k=0;k<playback_samplerate_ratio_bt/SAMPLERATE_RATIO_THRESHOLD;k++)
                {
                    mid_p_8[j++]=mid_p[i];
                    mid_p_8[j++]=mid_p[i+1];
                }
            }
        }

        anc_mc_run_stereo((uint8_t *)mid_p_8,mc_len_bytes_run,left_gain,right_gain,AUD_BITS_24);

        for(i=0,j=0;i<(mc_len_bytes_run)/4;i=i+2)
        {
           float delta_l=(mid_p_8[i]-mid_p_8_old_l)/(float)SAMPLERATE_RATIO_THRESHOLD;
           float delta_r=(mid_p_8[i+1]-mid_p_8_old_r)/(float)SAMPLERATE_RATIO_THRESHOLD;
            for(k=1;k<=SAMPLERATE_RATIO_THRESHOLD;k++)
            {
                dest_p[j++]=mid_p_8_old_l+(int32_t)(delta_l*k);
                dest_p[j++]=mid_p_8_old_r+(int32_t)(delta_r*k);
            }
            mid_p_8_old_l=mid_p_8[i];
            mid_p_8_old_r=mid_p_8[i+1];
        }
    }

  //  end_time = hal_sys_timer_get();

 //   TRACE(2,"%s:run time: %d", __FUNCTION__, end_time-begin_time);

    return 0;
}
#endif

static uint8_t g_current_eq_index = 0xff;
static bool isMeridianEQON = false;

bool app_is_meridian_on()
{
    return isMeridianEQON;
}

uint8_t app_audio_get_eq()
{
    return g_current_eq_index;
}

bool app_meridian_eq(bool onoff)
{
    isMeridianEQON = onoff;
    return onoff;
}

int app_audio_set_eq(uint8_t index)
{
#ifdef __SW_IIR_EQ_PROCESS__
    if (index >=EQ_SW_IIR_LIST_NUM)
        return -1;
#endif
#ifdef __HW_FIR_EQ_PROCESS__
    if (index >=EQ_HW_FIR_LIST_NUM)
        return -1;
#endif
#ifdef __HW_DAC_IIR_EQ_PROCESS__
    if (index >=EQ_HW_DAC_IIR_LIST_NUM)
        return -1;
#endif
#ifdef __HW_IIR_EQ_PROCESS__
    if (index >=EQ_HW_IIR_LIST_NUM)
        return -1;
#endif
    g_current_eq_index = index;
    return index;
}

void bt_audio_updata_eq(uint8_t index)
{
    TRACE(2,"[%s]anc_status = %d", __func__, index);    
#if defined(__SW_IIR_EQ_PROCESS__) || defined(__HW_FIR_EQ_PROCESS__)|| defined(__HW_DAC_IIR_EQ_PROCESS__)|| defined(__HW_IIR_EQ_PROCESS__)
    AUDIO_EQ_TYPE_T audio_eq_type;
#ifdef __SW_IIR_EQ_PROCESS__
    audio_eq_type = AUDIO_EQ_TYPE_SW_IIR;
#endif

#ifdef __HW_FIR_EQ_PROCESS__
    audio_eq_type = AUDIO_EQ_TYPE_HW_FIR;
#endif

#ifdef __HW_DAC_IIR_EQ_PROCESS__
    audio_eq_type = AUDIO_EQ_TYPE_HW_DAC_IIR;
#endif

#ifdef __HW_IIR_EQ_PROCESS__
    audio_eq_type = AUDIO_EQ_TYPE_HW_IIR;
#endif
    bt_audio_set_eq(audio_eq_type,index);
#endif
}


#ifdef ANC_APP
uint8_t bt_audio_updata_eq_for_anc(uint8_t anc_status)
{
    anc_status = app_anc_work_status();
    if(anc_status_record != anc_status)
    {
        anc_status_record = anc_status;
        TRACE(2,"[%s]anc_status = %d", __func__, anc_status);
#ifdef __SW_IIR_EQ_PROCESS__
        bt_audio_set_eq(AUDIO_EQ_TYPE_SW_IIR,bt_audio_get_eq_index(AUDIO_EQ_TYPE_SW_IIR,anc_status));
#endif

#ifdef __HW_FIR_EQ_PROCESS__
        bt_audio_set_eq(AUDIO_EQ_TYPE_HW_FIR,bt_audio_get_eq_index(AUDIO_EQ_TYPE_HW_FIR,anc_status));
#endif

#ifdef __HW_DAC_IIR_EQ_PROCESS__
        bt_audio_set_eq(AUDIO_EQ_TYPE_HW_DAC_IIR,bt_audio_get_eq_index(AUDIO_EQ_TYPE_HW_DAC_IIR,anc_status));
#endif

#ifdef __HW_IIR_EQ_PROCESS__
        bt_audio_set_eq(AUDIO_EQ_TYPE_HW_IIR,bt_audio_get_eq_index(AUDIO_EQ_TYPE_HW_IIR,anc_status));
#endif
    }

    return 0;
}
#endif


uint8_t bt_audio_get_eq_index(AUDIO_EQ_TYPE_T audio_eq_type,uint8_t anc_status)
{
    uint8_t index_eq=0;

#if defined(__SW_IIR_EQ_PROCESS__) || defined(__HW_FIR_EQ_PROCESS__)|| defined(__HW_DAC_IIR_EQ_PROCESS__)|| defined(__HW_IIR_EQ_PROCESS__)
    switch (audio_eq_type)
    {
#if defined(__SW_IIR_EQ_PROCESS__)
        case AUDIO_EQ_TYPE_SW_IIR:
        {
            if(anc_status)
            {
                index_eq=audio_eq_sw_iir_index+1;
            }
            else
            {
                index_eq=audio_eq_sw_iir_index;
            }

        }
        break;
#endif

#if defined(__HW_FIR_EQ_PROCESS__)
        case AUDIO_EQ_TYPE_HW_FIR:
        {
            if(a2dp_sample_rate == AUD_SAMPRATE_44100) {
                index_eq = 0;
            } else if(a2dp_sample_rate == AUD_SAMPRATE_48000) {
                index_eq = 1;
            } else if(a2dp_sample_rate == AUD_SAMPRATE_96000) {
                index_eq = 2;
            } else {
                ASSERT(0, "[%s] sample_rate_recv(%d) is not supported", __func__, a2dp_sample_rate);
            }
            audio_eq_hw_fir_index=index_eq;

            if(anc_status)
            {
                index_eq=index_eq+3;
            }
        }
        break;
#endif

#if defined(__HW_DAC_IIR_EQ_PROCESS__)
        case AUDIO_EQ_TYPE_HW_DAC_IIR:
        {
            if(anc_status)
            {
                index_eq=audio_eq_hw_dac_iir_index+1;
            }
            else
            {
                index_eq=audio_eq_hw_dac_iir_index;
            }
        }
        break;
#endif

#if defined(__HW_IIR_EQ_PROCESS__)
        case AUDIO_EQ_TYPE_HW_IIR:
        {
            if(anc_status)
            {
                index_eq=audio_eq_hw_iir_index+1;
            }
            else
            {
                index_eq=audio_eq_hw_iir_index;
            }
        }
        break;
#endif
        default:
        {
            ASSERT(false,"[%s]Error eq type!",__func__);
        }
    }
#endif
    return index_eq;
}


uint32_t bt_audio_set_eq(AUDIO_EQ_TYPE_T audio_eq_type, uint8_t index)
{
    const FIR_CFG_T *fir_cfg=NULL;
    const IIR_CFG_T *iir_cfg=NULL;

    TRACE(3,"[%s]audio_eq_type=%d,index=%d", __func__, audio_eq_type,index);

#if defined(__SW_IIR_EQ_PROCESS__) || defined(__HW_FIR_EQ_PROCESS__)|| defined(__HW_DAC_IIR_EQ_PROCESS__)|| defined(__HW_IIR_EQ_PROCESS__)
    switch (audio_eq_type)
    {
#if defined(__SW_IIR_EQ_PROCESS__)
        case AUDIO_EQ_TYPE_SW_IIR:
        {
            if(index >= EQ_SW_IIR_LIST_NUM)
            {
                TRACE(1,"[%s] index > EQ_SW_IIR_LIST_NUM", __func__, index);
                return 1;
            }

            iir_cfg=audio_eq_sw_iir_cfg_list[index];
        }
        break;
#endif

#if defined(__HW_FIR_EQ_PROCESS__)
        case AUDIO_EQ_TYPE_HW_FIR:
        {
            if(index >= EQ_HW_FIR_LIST_NUM)
            {
                TRACE(1,"[%s] index > EQ_HW_FIR_LIST_NUM", __func__, index);
                return 1;
            }

            fir_cfg=audio_eq_hw_fir_cfg_list[index];
        }
        break;
#endif

#if defined(__HW_DAC_IIR_EQ_PROCESS__)
        case AUDIO_EQ_TYPE_HW_DAC_IIR:
        {
            if(index >= EQ_HW_DAC_IIR_LIST_NUM)
            {
                TRACE(1,"[%s] index > EQ_HW_DAC_IIR_LIST_NUM", __func__, index);
                return 1;
            }

            iir_cfg=audio_eq_hw_dac_iir_cfg_list[index];
        }
        break;
#endif

#if defined(__HW_IIR_EQ_PROCESS__)
        case AUDIO_EQ_TYPE_HW_IIR:
        {
            if(index >= EQ_HW_IIR_LIST_NUM)
            {
                TRACE(1,"[%s] index > EQ_HW_IIR_LIST_NUM", __func__, index);
                return 1;
            }

            iir_cfg=audio_eq_hw_iir_cfg_list[index];
        }
        break;
#endif
        default:
        {
            ASSERT(false,"[%s]Error eq type!",__func__);
        }
    }
#endif

#ifdef AUDIO_SECTION_ENABLE
    const IIR_CFG_T *iir_cfg_from_audio_section = (const IIR_CFG_T *)load_audio_cfg_from_audio_section(AUDIO_PROCESS_TYPE_IIR_EQ, index);
    if (iir_cfg_from_audio_section)
    {
        iir_cfg = iir_cfg_from_audio_section;
        for(int i = 0;i < IIR_PARAM_NUM;i++)
        {
            TRACE(3,"[%s] gain:%d,fc:%d",__func__,(int)iir_cfg->param[i].gain,(int)iir_cfg->param[i].fc);
        }        
    }
#endif

    return audio_eq_set_cfg(fir_cfg,iir_cfg,audio_eq_type);
}

/********************************
        AUD_BITS_16
        dma_buffer_delay_us = stream_cfg->data_size/stream_cfg->channel_num/2*1000000LL/stream_cfg->sample_rate;
        AUD_BITS_24
        dma_buffer_delay_us = stream_cfg->data_size/stream_cfg->channel_num/4*1000000LL/stream_cfg->sample_rate;

        dma_buffer_delay_us
        scalable delay = 864/sample*1000*n ms
        scalable delay = 864/44100*1000*13 = 117ms
        scalable delay = 864/96000*1000*6 = 118ms
        waterline delay = 864/sample*1000*n ms
        waterline delay = 864/44100*1000*3 = 58ms
        waterline delay = 864/96000*1000*3 = 27ms
        audio_delay = scalable delay + waterline delay
 *********************************/
#define A2DP_PLAYER_PLAYBACK_DMA_SCALABLE_MTU (1)
#define A2DP_PLAYER_PLAYBACK_DELAY_SCALABLE_HIRES_MTU (13)
#define A2DP_PLAYER_PLAYBACK_DELAY_SCALABLE_HIRES_BASE (9000)
#define A2DP_PLAYER_PLAYBACK_DELAY_SCALABLE_HIRES_US (A2DP_PLAYER_PLAYBACK_DELAY_SCALABLE_HIRES_BASE*A2DP_PLAYER_PLAYBACK_DELAY_SCALABLE_HIRES_MTU)

#define A2DP_PLAYER_PLAYBACK_DELAY_SCALABLE_BASERES_MTU  (6)
#define A2DP_PLAYER_PLAYBACK_DELAY_SCALABLE_BASERES_BASE (19500)
#define A2DP_PLAYER_PLAYBACK_DELAY_SCALABLE_BASERES_US (A2DP_PLAYER_PLAYBACK_DELAY_SCALABLE_BASERES_BASE*A2DP_PLAYER_PLAYBACK_DELAY_SCALABLE_BASERES_MTU)

/********************************
        AUD_BITS_16
        dma_buffer_delay_us = stream_cfg->data_size/stream_cfg->channel_num/2*1000000LL/stream_cfg->sample_rate;
        AUD_BITS_24
        dma_buffer_delay_us = stream_cfg->data_size/stream_cfg->channel_num/4*1000000LL/stream_cfg->sample_rate;

        dma_buffer_delay_us
        aac delay = 1024/sample*1000*n ms
        aac delay = 1024/44100*1000*5 = 116ms
        waterline delay = 1024/sample*1000*n ms
        waterline delay = 1024/44100*1000*3 = 69ms
        audio_delay = aac delay + waterline delay
 *********************************/
#define A2DP_PLAYER_PLAYBACK_DMA_AAC_MTU (1)
#ifndef A2DP_PLAYER_PLAYBACK_DELAY_AAC_MTU
#define A2DP_PLAYER_PLAYBACK_DELAY_AAC_MTU (5)
#endif
#define A2DP_PLAYER_PLAYBACK_DELAY_AAC_BASE (23000)
/********************************
    AUD_BITS_16
    dma_buffer_delay_us = stream_cfg->data_size/stream_cfg->channel_num/2*1000000LL/stream_cfg->sample_rate;
    AUD_BITS_24
    dma_buffer_delay_us = stream_cfg->data_size/stream_cfg->channel_num/4*1000000LL/stream_cfg->sample_rate;

    sbc delay = 128/sample*n ms
    sbc delay = 128/44100*45 = 130ms
    sbc_delay = sbc delay(23219us)
    waterline delay = 128/sample*SBC_FRAME_MTU*n ms
    waterline delay = 128/44100*5*3 = 43ms
    audio_delay = aac delay + waterline delay
*********************************/
#define A2DP_PLAYER_PLAYBACK_DMA_SBC_MTU (7)
#ifndef A2DP_PLAYER_PLAYBACK_DELAY_SBC_MTU
#define A2DP_PLAYER_PLAYBACK_DELAY_SBC_MTU (70)
#endif
#define A2DP_PLAYER_PLAYBACK_DELAY_SBC_BASE (2902)
#if defined(A2DP_LHDC_ON)
/********************************
    AUD_BITS_16
    dma_buffer_delay_us = stream_cfg->data_size/stream_cfg->channel_num/2*1000000LL/stream_cfg->sample_rate;
    AUD_BITS_24
    dma_buffer_delay_us = stream_cfg->data_size/stream_cfg->channel_num/4*1000000LL/stream_cfg->sample_rate;

    lhdc delay = 512/sample*1000*n ms
    lhdc delay =    *28 = 149ms
    audio_delay = lhdc delay

    lhdc_v2 delay = 512/96000*1000*38 =  202ms
    lhdc_v3 delay = 256/96000*1000*58 = 154ms
    audio_delay = lhdc_v3 delay
*********************************/
#if defined(IBRT)
#if defined(A2DP_LHDC_V3)
#define A2DP_PLAYER_PLAYBACK_DMA_LHDC_HIRES_MTU (8)
#define A2DP_PLAYER_PLAYBACK_DMA_LHDC_BASERES_MTU (4)
#define A2DP_PLAYER_PLAYBACK_DMA_LHDC_LLC_MTU (1)
#else
#define A2DP_PLAYER_PLAYBACK_DMA_LHDC_HIRES_MTU (1)
#define A2DP_PLAYER_PLAYBACK_DMA_LHDC_BASERES_MTU (1)
#define A2DP_PLAYER_PLAYBACK_DMA_LHDC_LLC_MTU (1)
#endif
#else
#define A2DP_PLAYER_PLAYBACK_DMA_LHDC_HIRES_MTU (1)
#define A2DP_PLAYER_PLAYBACK_DMA_LHDC_BASERES_MTU (1)
#define A2DP_PLAYER_PLAYBACK_DMA_LHDC_LLC_MTU (1)
#endif

#if defined(IBRT)
#if defined(A2DP_LHDC_V3)
#define A2DP_PLAYER_PLAYBACK_DELAY_LHDC_HIRES_MTU (58)
#else
#define A2DP_PLAYER_PLAYBACK_DELAY_LHDC_HIRES_MTU (38)
#endif
#else
#define A2DP_PLAYER_PLAYBACK_DELAY_LHDC_HIRES_MTU (28)
#endif

#if  defined(A2DP_LHDC_V3)
#define A2DP_PLAYER_PLAYBACK_DELAY_LHDC_HIRES_BASE (2666)
#else
#define A2DP_PLAYER_PLAYBACK_DELAY_LHDC_HIRES_BASE (5333)
#endif
#define A2DP_PLAYER_PLAYBACK_DELAY_LHDC_HIRES_US (A2DP_PLAYER_PLAYBACK_DELAY_LHDC_HIRES_MTU*A2DP_PLAYER_PLAYBACK_DELAY_LHDC_HIRES_BASE)

/********************************
    AUD_BITS_16
    dma_buffer_delay_us = stream_cfg->data_size/stream_cfg->channel_num/2*1000000LL/stream_cfg->sample_rate;
    AUD_BITS_24
    dma_buffer_delay_us = stream_cfg->data_size/stream_cfg->channel_num/4*1000000LL/stream_cfg->sample_rate;

    lhdc delay = 512/sample*1000*n ms
    lhdc_v2 delay = 512/48000*1000*14 = 149ms
    lhdc_v3 delay = 256/48000*1000*28 = 149ms
    audio_delay = lhdc delay
*********************************/
#if defined(A2DP_LHDC_V3)
#define A2DP_PLAYER_PLAYBACK_DELAY_LHDC_BASERES_MTU (28)
#else
#define A2DP_PLAYER_PLAYBACK_DELAY_LHDC_BASERES_MTU (14)
#endif

#if  defined(A2DP_LHDC_V3)
#define A2DP_PLAYER_PLAYBACK_DELAY_LHDC_BASERES_BASE (5333)
#else
#define A2DP_PLAYER_PLAYBACK_DELAY_LHDC_BASERES_BASE (10666)
#endif
#define A2DP_PLAYER_PLAYBACK_DELAY_LHDC_BASERES_US (A2DP_PLAYER_PLAYBACK_DELAY_LHDC_BASERES_MTU*A2DP_PLAYER_PLAYBACK_DELAY_LHDC_BASERES_BASE)

/********************************
    AUD_BITS_16
    dma_buffer_delay_us = stream_cfg->data_size/stream_cfg->channel_num/2*1000000LL/stream_cfg->sample_rate;
    AUD_BITS_24
    dma_buffer_delay_us = stream_cfg->data_size/stream_cfg->channel_num/4*1000000LL/stream_cfg->sample_rate;

    lhdc delay = 512/sample*1000*n ms
    lhdc_v2 delay = 512/48000*1000*9 = 96ms
    lhdc_v3 delay = 256/48000*1000*19 = 101ms
    audio_delay = lhdc delay
*********************************/
#if defined(IBRT)
#if  defined(A2DP_LHDC_V3)
#define A2DP_PLAYER_PLAYBACK_DELAY_LHDC_LLC_MTU (15)
#else
#define A2DP_PLAYER_PLAYBACK_DELAY_LHDC_LLC_MTU (9)
#endif
#else
#define A2DP_PLAYER_PLAYBACK_DELAY_LHDC_LLC_MTU (6)
#endif

#if  defined(A2DP_LHDC_V3)
#define A2DP_PLAYER_PLAYBACK_DELAY_LHDC_LLC_BASE (5333)
#else
#define A2DP_PLAYER_PLAYBACK_DELAY_LHDC_LLC_BASE (10666)
#endif
#define A2DP_PLAYER_PLAYBACK_DELAY_LHDC_LLC_US (A2DP_PLAYER_PLAYBACK_DELAY_LHDC_LLC_MTU*A2DP_PLAYER_PLAYBACK_DELAY_LHDC_LLC_BASE)
#endif
#if defined(USE_LOWLATENCY_LIB)
#define ADBUF_MAX_BUF_DELAY                                     (600000) //600 ms
#define GAME_MODE_MAX_DELAY                                     (200000) //200ms
#endif

enum BT_STREAM_TRIGGER_STATUS_T {
    BT_STREAM_TRIGGER_STATUS_NULL = 0,
    BT_STREAM_TRIGGER_STATUS_INIT,
    BT_STREAM_TRIGGER_STATUS_WAIT,
    BT_STREAM_TRIGGER_STATUS_OK,
};

static uint32_t tg_acl_trigger_time = 0;
static uint32_t tg_acl_trigger_start_time = 0;
static uint32_t tg_acl_trigger_init_time = 0;
static enum BT_STREAM_TRIGGER_STATUS_T bt_stream_trigger_status = BT_STREAM_TRIGGER_STATUS_NULL;

void app_bt_stream_playback_irq_notification(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream);

inline void app_bt_stream_trigger_stauts_set(enum BT_STREAM_TRIGGER_STATUS_T stauts)
{
    TRACE(2,"bt_stream_trigger_stauts_set:%d->%d", bt_stream_trigger_status,stauts);
    bt_stream_trigger_status = stauts;
}

inline enum BT_STREAM_TRIGGER_STATUS_T app_bt_stream_trigger_stauts_get(void)
{
    TRACE(1,"bt_stream_trigger_stauts_get:%d", bt_stream_trigger_status);
    return bt_stream_trigger_status;
}

uint32_t app_bt_stream_get_dma_buffer_delay_us(void)
{
    uint32_t dma_buffer_delay_us = 0;
    struct AF_STREAM_CONFIG_T *stream_cfg = NULL;
    if (!af_stream_get_cfg(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg, false)){
        if (stream_cfg->bits <= AUD_BITS_16){
            dma_buffer_delay_us = stream_cfg->data_size/stream_cfg->channel_num/2*1000000LL/stream_cfg->sample_rate;
        }else{
            dma_buffer_delay_us = stream_cfg->data_size/stream_cfg->channel_num/4*1000000LL/stream_cfg->sample_rate;
        }
    }
    return dma_buffer_delay_us;
}

uint32_t app_bt_stream_get_dma_buffer_samples(void)
{
    uint32_t dma_buffer_delay_samples = 0;
    struct AF_STREAM_CONFIG_T *stream_cfg = NULL;
    if (!af_stream_get_cfg(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg, false)){
        if (stream_cfg->bits <= AUD_BITS_16){
            dma_buffer_delay_samples = stream_cfg->data_size/stream_cfg->channel_num/2;
        }else{
            dma_buffer_delay_samples = stream_cfg->data_size/stream_cfg->channel_num/4;
        }
    }
    return dma_buffer_delay_samples;
}

#if defined(IBRT)
typedef enum {
    APP_TWS_IBRT_AUDIO_SYNCHRONIZE_STATUS_IDLE,
    APP_TWS_IBRT_AUDIO_SYNCHRONIZE_STATUS_ONPROCESS,
    APP_TWS_IBRT_AUDIO_SYNCHRONIZE_STATUS_SYNCOK,
}APP_TWS_IBRT_AUDIO_SYNCHRONIZE_STATUS_TYPE;

#define APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_CNT_LIMIT (100)
void app_bt_stream_ibrt_set_trigger_time(APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T *sync_trigger);
void app_bt_stream_ibrt_auto_synchronize_initsync_start(APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T *sync_trigger);
static APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T app_bt_stream_ibrt_auto_synchronize_trigger;
static uint32_t app_bt_stream_ibrt_auto_synchronize_cnt = 0;
int app_bt_stream_ibrt_audio_mismatch_stopaudio(void);
void app_bt_stream_ibrt_auto_synchronize_hungup(void);
void app_bt_stream_ibrt_auto_synchronize_stop(void);
static APP_TWS_IBRT_AUDIO_SYNCHRONIZE_STATUS_TYPE ibrt_auto_synchronize_status = APP_TWS_IBRT_AUDIO_SYNCHRONIZE_STATUS_IDLE;

int app_bt_stream_ibrt_auto_synchronize_status_set(APP_TWS_IBRT_AUDIO_SYNCHRONIZE_STATUS_TYPE status)
{
    TRACE(2,"[%s] status:%d", __func__, status);
    ibrt_auto_synchronize_status = status;
    return 0;
}

APP_TWS_IBRT_AUDIO_SYNCHRONIZE_STATUS_TYPE app_bt_stream_ibrt_auto_synchronize_status_get(void)
{
    TRACE(2,"[%s] status:%d", __func__, ibrt_auto_synchronize_status);
    return ibrt_auto_synchronize_status;
}

int app_bt_stream_ibrt_auto_synchronize_trigger_start(btif_media_header_t * header, unsigned char *buf, unsigned int len)
{
    APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T *sync_trigger = &app_bt_stream_ibrt_auto_synchronize_trigger;
    app_bt_stream_ibrt_auto_synchronize_stop();
    TRACE(6,"[auto_trigger_start] trigger:%d Seq:%d timestamp:%d SubSeq:%d/%d currSeq:%d",
                                                                                   sync_trigger->trigger_time, 
                                                                                   sync_trigger->audio_info.sequenceNumber, 
                                                                                   sync_trigger->audio_info.timestamp,
                                                                                   sync_trigger->audio_info.curSubSequenceNumber,
                                                                                   sync_trigger->audio_info.totalSubSequenceNumber,
                                                                                   header->sequenceNumber
                                                                                   );

    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    
    if (app_tws_ibrt_slave_ibrt_link_connected()){
        if (sync_trigger->trigger_time >= bt_syn_get_curr_ticks(p_ibrt_ctrl->ibrt_conhandle)){
            app_bt_stream_ibrt_set_trigger_time(sync_trigger);
        }else{
            TRACE(3,"[auto_trigger_start]failed trigger(%d)-->tg(%d) frame_samples:%d need resume", bt_syn_get_curr_ticks(p_ibrt_ctrl->ibrt_conhandle),
                                                                                                    sync_trigger->trigger_time,
                                                                                                    sync_trigger->audio_info.frame_samples);
            app_ibrt_if_force_audio_retrigger();
            //app_tws_ibrt_audio_sync_mismatch_resume_notify();
        }
    }else{
        TRACE(1,"[auto_trigger_start] ok but currRole:%d mismatch\n", p_ibrt_ctrl->current_role);
        app_ibrt_if_force_audio_retrigger();
    }

    return 0;
}

int app_bt_stream_ibrt_auto_synchronize_dataind_cb(btif_media_header_t * header, unsigned char *buf, unsigned int len)
{
    APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T *sync_trigger = &app_bt_stream_ibrt_auto_synchronize_trigger;
    bool synchronize_ok = false;
    int32_t timestamp_diff = 0;
    int32_t dma_buffer_samples = 0;
    int32_t frame_totle_samples = 0;

    frame_totle_samples = sync_trigger->audio_info.totalSubSequenceNumber * sync_trigger->audio_info.frame_samples;
    timestamp_diff = sync_trigger->audio_info.timestamp - header->timestamp;

    TRACE(5,"[auto_synchronize_dataind_cb] dataind_seq:%d/%d timestamp:%d/%d cnt:%d", header->sequenceNumber,
                                                                                    sync_trigger->audio_info.sequenceNumber,
                                                                                    header->timestamp,
                                                                                    sync_trigger->audio_info.timestamp,
                                                                                    app_bt_stream_ibrt_auto_synchronize_cnt);

    if (++app_bt_stream_ibrt_auto_synchronize_cnt > APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_CNT_LIMIT){
        app_bt_stream_ibrt_auto_synchronize_stop();
        TRACE(0,"[auto_synchronize_dataind_cb] SYNCHRONIZE_CNT_LIMIT, we need force retrigger");
        app_ibrt_if_force_audio_retrigger();
    }else if (app_tws_ibrt_mobile_link_connected()){
        app_bt_stream_ibrt_auto_synchronize_stop();
        TRACE(0,"[auto_synchronize_dataind_cb] find role to master, we need force retrigger");
        app_ibrt_if_force_audio_retrigger();
    }else if (sync_trigger->audio_info.sequenceNumber < header->sequenceNumber){        
        app_bt_stream_ibrt_auto_synchronize_stop();
        TRACE(2,"[auto_synchronize_dataind_cb]dataind_seq timestamp:%d/%d mismatch need resume", header->timestamp,
                                                                                               sync_trigger->audio_info.timestamp);
        app_ibrt_if_force_audio_retrigger();
    }else{
        if (header->sequenceNumber >= sync_trigger->audio_info.sequenceNumber && 
            !sync_trigger->audio_info.totalSubSequenceNumber){
            synchronize_ok = true;
        }else if (header->timestamp == sync_trigger->audio_info.timestamp){
            synchronize_ok = true;
        }

        dma_buffer_samples = app_bt_stream_get_dma_buffer_samples()/2;

        if (sync_trigger->audio_info.timestamp >= header->timestamp && sync_trigger->audio_info.totalSubSequenceNumber){
            if (timestamp_diff < dma_buffer_samples){                
                TRACE(0,"[auto_synchronize_dataind_cb] timestamp_diff < dma_buffer_samples synchronize ok");
                synchronize_ok = true;
            }else if (timestamp_diff < frame_totle_samples){
                TRACE(0,"[auto_synchronize_dataind_cb] timestamp_diff < frame_totle_samples synchronize ok");
                synchronize_ok = true;
            }
        }

        if (!synchronize_ok && header->sequenceNumber >= sync_trigger->audio_info.sequenceNumber){
            
            TRACE(2,"[auto_synchronize_dataind_cb] timestamp %d vs %d", sync_trigger->audio_info.timestamp - header->timestamp,
                                                                      frame_totle_samples);
            if ((sync_trigger->audio_info.timestamp - header->timestamp) <= (uint32_t)(frame_totle_samples*2)){
                sync_trigger->audio_info.sequenceNumber++;
                TRACE(1,"[auto_synchronize_dataind_cb] timestamp try sequenceNumber:%d", header->sequenceNumber);
            }
        }

        //flush all
        a2dp_audio_synchronize_dest_packet_mut(0);

        if (synchronize_ok){
            A2DP_AUDIO_LASTFRAME_INFO_T lastframe_info;
            if (a2dp_audio_lastframe_info_get(&lastframe_info) < 0){
                TRACE(0,"[auto_synchronize_dataind_cb]synchronize ok but lastframe error");
                goto exit;
            }

            TRACE(2,"[auto_synchronize_dataind_cb]synchronize ok timestamp_diff:%d frame_samples:%d", timestamp_diff,
                                                                                                    lastframe_info.frame_samples);
            sync_trigger->trigger_type = APP_TWS_IBRT_AUDIO_TRIGGER_TYPE_LOCAL;
            sync_trigger->audio_info.sequenceNumber = header->sequenceNumber;
            sync_trigger->audio_info.timestamp = header->timestamp;
            if (sync_trigger->audio_info.totalSubSequenceNumber){
                sync_trigger->audio_info.curSubSequenceNumber = timestamp_diff/lastframe_info.frame_samples;
                TRACE(3,"[auto_synchronize_dataind_cb]synchronize ok tstmp_diff:%d/%d SubSeq:%d", timestamp_diff, sync_trigger->audio_info.frame_samples,
                                                                                           sync_trigger->audio_info.curSubSequenceNumber);
            }else{
                sync_trigger->audio_info.curSubSequenceNumber = 0;
            }
            sync_trigger->audio_info.totalSubSequenceNumber = lastframe_info.totalSubSequenceNumber;
            sync_trigger->audio_info.frame_samples = lastframe_info.frame_samples;
            if (sync_trigger->audio_info.totalSubSequenceNumber &&
                sync_trigger->audio_info.curSubSequenceNumber >= sync_trigger->audio_info.totalSubSequenceNumber){
                TRACE(0,"[auto_synchronize_dataind_cb]synchronize ok but sbc & timestamp is ms so force trigger");
                app_bt_stream_ibrt_auto_synchronize_stop();
                app_ibrt_if_force_audio_retrigger();
            }else{
                a2dp_audio_detect_store_packet_callback_register(app_bt_stream_ibrt_auto_synchronize_trigger_start);
            }
        }else{
            a2dp_audio_detect_first_packet();
        }
    }
exit:
    return 0;
}

void app_bt_stream_ibrt_auto_synchronize_start(APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T *sync_trigger)
{
    TRACE(5,"[auto_synchronize_start] trigger_time:%d seq:%d timestamp:%d SubSeq:%d/%d", sync_trigger->trigger_time, 
                                                                                       sync_trigger->audio_info.sequenceNumber, 
                                                                                       sync_trigger->audio_info.timestamp,
                                                                                       sync_trigger->audio_info.curSubSequenceNumber,
                                                                                       sync_trigger->audio_info.totalSubSequenceNumber
                                                                                       );
    app_bt_stream_ibrt_auto_synchronize_status_set(APP_TWS_IBRT_AUDIO_SYNCHRONIZE_STATUS_ONPROCESS);
    app_bt_stream_ibrt_auto_synchronize_cnt = 0;
    app_bt_stream_ibrt_auto_synchronize_trigger = *sync_trigger;
    a2dp_audio_detect_next_packet_callback_register(app_bt_stream_ibrt_auto_synchronize_dataind_cb);
    a2dp_audio_detect_first_packet();
}


void app_bt_stream_ibrt_auto_synchronize_hungup(void)
{
    a2dp_audio_detect_next_packet_callback_register(NULL);
    a2dp_audio_detect_store_packet_callback_register(NULL);
}

void app_bt_stream_ibrt_auto_synchronize_stop(void)
{   
    app_bt_stream_ibrt_auto_synchronize_hungup();
    app_bt_stream_ibrt_auto_synchronize_cnt = 0;
    app_bt_stream_ibrt_auto_synchronize_status_set(APP_TWS_IBRT_AUDIO_SYNCHRONIZE_STATUS_IDLE);
}

bool app_bt_stream_ibrt_auto_synchronize_on_porcess(void)
{
    bool nRet = true;
    APP_TWS_IBRT_AUDIO_SYNCHRONIZE_STATUS_TYPE synchronize_status = app_bt_stream_ibrt_auto_synchronize_status_get();
    if (synchronize_status == APP_TWS_IBRT_AUDIO_SYNCHRONIZE_STATUS_IDLE){
        nRet = false;
    }
    return nRet;
}

void app_bt_stream_ibrt_start_sbc_player_callback(uint32_t status, uint32_t param)
{
    if (app_bt_stream_isrun(APP_BT_STREAM_A2DP_SBC)){        
        TRACE(3,"[%s] trigger(%d)-->tg(%d)", __func__, param, ((APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T *)param)->trigger_time);
        app_bt_stream_ibrt_set_trigger_time((APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T *)param);
    }else{
        TRACE(1,"[%s] try again", __func__);
        app_audio_manager_sendrequest_need_callback(
                                                    APP_BT_STREAM_MANAGER_START,BT_STREAM_SBC, 
                                                    BT_DEVICE_ID_1, 
                                                    MAX_RECORD_NUM, 
                                                    (uint32_t)app_bt_stream_ibrt_start_sbc_player_callback,
                                                    param);
    }
}

int app_bt_stream_ibrt_start_sbc_player(APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T *sync_trigger)
{
    TRACE(2,"[%s] tg(%d)", __func__, sync_trigger->trigger_time);
    app_audio_manager_sendrequest_need_callback(
                                                APP_BT_STREAM_MANAGER_START,BT_STREAM_SBC, 
                                                BT_DEVICE_ID_1, 
                                                MAX_RECORD_NUM, 
                                                (uint32_t)app_bt_stream_ibrt_start_sbc_player_callback,
                                                (uint32_t)sync_trigger);
    return 0;
}

uint16_t app_bt_stream_ibrt_trigger_seq_diff_calc(int32_t dma_samples, int32_t frame_samples, int32_t total_subseq, int32_t interval)
{
    float seq_factor = 1.0f;
    if (total_subseq){
        seq_factor = (float)(dma_samples/frame_samples)/(float)total_subseq;
    }else{
        seq_factor = (float)(dma_samples/frame_samples);
    }
    return (uint16_t)(seq_factor * (float)interval);
}

#define MOBILE_LINK_PLAYBACK_INFO_TRIG_DUMMY_DMA_CNT (5)
#define SYNCHRONIZE_DATAIND_CNT_LIMIT (25)

static int synchronize_need_discards_dma_cnt = 0;
static int synchronize_dataind_cnt = 0;

int app_bt_stream_ibrt_auto_synchronize_initsync_dataind_cb_v2(btif_media_header_t * header, unsigned char *buf, unsigned int len)
{
    bool synchronize_ok = false;
    bool discards_samples_finished = false;
    int dest_discards_samples = 0;
    uint32_t list_samples = 0;
    uint32_t curr_ticks = 0;
    A2DP_AUDIO_HEADFRAME_INFO_T headframe_info;
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T *sync_trigger = &app_bt_stream_ibrt_auto_synchronize_trigger;

    synchronize_dataind_cnt++;
    if (app_tws_ibrt_mobile_link_connected()){
        TRACE(0,"[auto_synchronize_initsync_cb] mobile_link is connect retrigger because role switch");
        app_ibrt_if_force_audio_retrigger();
        return 0;
    }

    if (synchronize_dataind_cnt >= SYNCHRONIZE_DATAIND_CNT_LIMIT){
        TRACE(0,"[auto_synchronize_initsync_cb] mobile_link is connect retrigger because CNT_LIMIT");
        app_ibrt_if_force_audio_retrigger();
        return 0;
    }

    is_need_discards_samples = false;
    dest_discards_samples = app_bt_stream_get_dma_buffer_samples()/2*synchronize_need_discards_dma_cnt;
    a2dp_audio_convert_list_to_samples(&list_samples);
    if ((int)list_samples > dest_discards_samples){
        discards_samples_finished = true;
        a2dp_audio_discards_samples(dest_discards_samples);
    }
    a2dp_audio_decoder_headframe_info_get(&headframe_info);
    TRACE(5,"[auto_synchronize_initsync_cb] sample:%d->%d seq:%d sub_seq:%d/%d",
                                                      list_samples,dest_discards_samples,
                                                      headframe_info.sequenceNumber,
                                                      headframe_info.curSubSequenceNumber,
                                                      headframe_info.totalSubSequenceNumber);

    curr_ticks = bt_syn_get_curr_ticks(p_ibrt_ctrl->ibrt_conhandle);
    TRACE(2,"[auto_synchronize_initsync_cb] trig:%x/%x", curr_ticks, sync_trigger->trigger_time);

    if (discards_samples_finished){
        if (sync_trigger->trigger_time > curr_ticks){
            synchronize_ok = true;
        }else{
            TRACE(0,"[auto_synchronize_initsync_cb] synchronize_failed");
            app_ibrt_if_force_audio_retrigger();
            return 0;
        }
    }
    if (synchronize_ok){
        TRACE(0,"[auto_synchronize_initsync_cb] synchronize_ok");
        tg_acl_trigger_time = sync_trigger->trigger_time;
        btdrv_syn_trigger_codec_en(1);
        btdrv_syn_clr_trigger();
        btdrv_enable_playback_triggler(ACL_TRIGGLE_MODE);
        bt_syn_set_tg_ticks(sync_trigger->trigger_time, p_ibrt_ctrl->ibrt_conhandle, BT_TRIG_SLAVE_ROLE);
        app_tws_ibrt_audio_analysis_start(sync_trigger->handler_cnt, AUDIO_ANALYSIS_CHECKER_INTERVEL_INVALID);
        app_tws_ibrt_audio_sync_start();
        app_tws_ibrt_audio_sync_new_reference(sync_trigger->factor_reference);
        TRACE(2,"[auto_synchronize_initsync_cb] trigger curr(%d)-->tg(%d)",
                                                            bt_syn_get_curr_ticks(p_ibrt_ctrl->ibrt_conhandle),
                                                            sync_trigger->trigger_time);
        synchronize_need_discards_dma_cnt = 0;
        synchronize_dataind_cnt = 0;
        a2dp_audio_detect_first_packet_clear();
        a2dp_audio_detect_next_packet_callback_register(NULL);
        a2dp_audio_detect_store_packet_callback_register(NULL);
    }else{
        a2dp_audio_detect_first_packet();
    }

    return 0;
}

void app_bt_stream_ibrt_mobile_link_playback_info_receive(APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T *sync_trigger)
{
    uint32_t tg_tick = 0;
    uint32_t next_dma_cnt = 0;
    A2DP_AUDIO_LASTFRAME_INFO_T lastframe_info;
    A2DP_AUDIO_SYNCFRAME_INFO_T sync_info;
    A2DP_AUDIO_HEADFRAME_INFO_T headframe_info;
    APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T *sync_trigger_loc = &app_bt_stream_ibrt_auto_synchronize_trigger;
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();

    TRACE(7,"[playback_info_receive] session:%d hdl:%d clk:%d cnt:%d seq:%d/%d/%d", sync_trigger->a2dp_session,
                                                                                    sync_trigger->handler_cnt,
                                                                                    sync_trigger->trigger_bt_clk,
                                                                                    sync_trigger->trigger_bt_cnt,
                                                                                    sync_trigger->audio_info.sequenceNumber,
                                                                                    sync_trigger->audio_info.curSubSequenceNumber,
                                                                                    sync_trigger->audio_info.totalSubSequenceNumber);

    if (app_bt_stream_ibrt_auto_synchronize_on_porcess()){
        TRACE(0,"[playback_info_receive] auto_synchronize_on_porcess skip it");
        return;
    }

    if (!app_bt_is_a2dp_streaming(BTIF_DEVICE_ID_1)){
        TRACE(0,"[playback_info_receive] streaming not ready skip it");
        app_bt_stream_ibrt_audio_mismatch_stopaudio();
        return;
    }

    if (a2dp_ibrt_session_get() != sync_trigger->a2dp_session){
        TRACE(2,"[playback_info_receive] session mismatch skip it loc:%d rmt:%d", a2dp_ibrt_session_get(), sync_trigger->a2dp_session);
        TRACE(0,"[playback_info_receive] session froce resume and try retrigger");
        a2dp_ibrt_session_set(sync_trigger->a2dp_session);
        app_bt_stream_ibrt_audio_mismatch_stopaudio();
        return;
    }

    if (a2dp_audio_lastframe_info_get(&lastframe_info) < 0){
        TRACE(0,"[playback_info_receive] lastframe not ready mismatch_stopaudio");
        app_bt_stream_ibrt_audio_mismatch_stopaudio();
        return;
    }

    *sync_trigger_loc = *sync_trigger;

    sync_info.sequenceNumber = sync_trigger->audio_info.sequenceNumber;
    sync_info.timestamp = sync_trigger->audio_info.timestamp;
    sync_info.curSubSequenceNumber = sync_trigger->audio_info.curSubSequenceNumber;
    sync_info.totalSubSequenceNumber = sync_trigger->audio_info.totalSubSequenceNumber;
    sync_info.frame_samples = sync_trigger->audio_info.frame_samples;
    if (a2dp_audio_synchronize_packet(&sync_info, A2DP_AUDIO_SYNCFRAME_MASK_ALL)){
        TRACE(0,"[playback_info_receive] synchronize_packe mismatch");
        app_ibrt_if_force_audio_retrigger();
        return;
    }

    a2dp_audio_decoder_headframe_info_get(&headframe_info);
    TRACE(3,"[playback_info_receive]sync with master packet step1 seq:%d sub_seq:%d/%d",
                                                                  headframe_info.sequenceNumber,
                                                                  headframe_info.curSubSequenceNumber,
                                                                  headframe_info.totalSubSequenceNumber);
    is_need_discards_samples = false;
    a2dp_audio_discards_samples(lastframe_info.list_samples);
    a2dp_audio_decoder_headframe_info_get(&headframe_info);
    TRACE(3,"[playback_info_receive]sync with master packet step2 seq:%d sub_seq:%d/%d",
                                                                  headframe_info.sequenceNumber,
                                                                  headframe_info.curSubSequenceNumber,
                                                                  headframe_info.totalSubSequenceNumber);

    uint32_t btclk;
    uint16_t btcnt;
    uint32_t mobile_master_clk = 0;
    uint16_t mobile_master_cnt = 0;
    int64_t mobile_master_us = 0;
    btclk = btdrv_syn_get_curr_ticks()/2;
    btcnt = 0;
    app_tws_ibrt_audio_mobile_clkcnt_get(btclk, btcnt,
                                         &mobile_master_clk, &mobile_master_cnt);
    mobile_master_us = (int64_t)mobile_master_clk * (int64_t)SLOT_SIZE + (int64_t)mobile_master_cnt;

    uint32_t rmt_mobile_master_clk = sync_trigger->trigger_bt_clk;
    uint16_t rmt_mobile_master_cnt = sync_trigger->trigger_bt_cnt;
    int64_t rmt_mobile_master_us = 0;
    int64_t tmp_mobile_master_us = 0;
    rmt_mobile_master_us = (int64_t)rmt_mobile_master_clk * (int64_t)SLOT_SIZE + (int64_t)rmt_mobile_master_cnt;

    uint32_t dma_buffer_us = 0;
    dma_buffer_us = app_bt_stream_get_dma_buffer_delay_us()/2;

    tmp_mobile_master_us = rmt_mobile_master_us;
    do{
        if (tmp_mobile_master_us - mobile_master_us >= 0){
            break;
        }
        tmp_mobile_master_us += dma_buffer_us;
        next_dma_cnt++;
    }while(1);
    next_dma_cnt += MOBILE_LINK_PLAYBACK_INFO_TRIG_DUMMY_DMA_CNT ;
    tmp_mobile_master_us += dma_buffer_us * (MOBILE_LINK_PLAYBACK_INFO_TRIG_DUMMY_DMA_CNT-1);
    synchronize_need_discards_dma_cnt = next_dma_cnt + a2dp_audio_frame_delay_get() - 1;
    synchronize_dataind_cnt = 0;
    TRACE(6,"[playback_info_receive] loc:%08x%08x rmt:%08x%08x tg:%08x%08x", (uint32_t)(mobile_master_us>>32), (uint32_t)(mobile_master_us&0xffffffff),
                                                                             (uint32_t)(rmt_mobile_master_us>>32), (uint32_t)(rmt_mobile_master_us&0xffffffff),
                                                                             (uint32_t)(tmp_mobile_master_us>>32), (uint32_t)(tmp_mobile_master_us&0xffffffff));

    tmp_mobile_master_us = tmp_mobile_master_us/SLOT_SIZE;

    tg_tick = tmp_mobile_master_us * 2;
    tg_tick &= 0x0fffffff;

    sync_trigger_loc->trigger_type = APP_TWS_IBRT_AUDIO_TRIGGER_TYPE_LOCAL;
    sync_trigger_loc->trigger_time = tg_tick;
    sync_trigger_loc->handler_cnt += next_dma_cnt;
    a2dp_audio_detect_next_packet_callback_register(app_bt_stream_ibrt_auto_synchronize_initsync_dataind_cb_v2);
    a2dp_audio_detect_first_packet();


    TRACE(3,"[playback_info_receive] mobile clk:%x/%x tg:%x", bt_syn_get_curr_ticks(p_ibrt_ctrl->ibrt_conhandle)/2,
                                                              mobile_master_clk,
                                                              tg_tick/2);

    TRACE(5,"[playback_info_receive] master_us:%x/%x/%x dma_cnt:%d/%d", (int32_t)mobile_master_us, (int32_t)rmt_mobile_master_us, (int32_t)tmp_mobile_master_us,
                                                                         next_dma_cnt, sync_trigger_loc->handler_cnt);
}

void app_bt_stream_ibrt_set_trigger_time(APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T *sync_trigger)
{
    uint32_t curr_ticks = 0;
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    uint32_t tg_tick = sync_trigger->trigger_time;
    A2DP_AUDIO_SYNCFRAME_INFO_T sync_info;
    int synchronize_ret;
    A2DP_AUDIO_LASTFRAME_INFO_T lastframe_info;
    A2DP_AUDIO_HEADFRAME_INFO_T headframe_info;

    if (app_bt_stream_ibrt_auto_synchronize_on_porcess()){
        TRACE(1,"[%s] auto_synchronize_on_porcess skip it", __func__);
        goto exit;
    }

    if (!app_bt_is_a2dp_streaming(BTIF_DEVICE_ID_1)){
        TRACE(1,"[%s] streaming not ready skip it", __func__);
        app_bt_stream_ibrt_audio_mismatch_stopaudio();
        goto exit;
    }

    if (a2dp_ibrt_session_get() != sync_trigger->a2dp_session){
        TRACE(3,"[%s] session mismatch skip it loc:%d rmt:%d", __func__, a2dp_ibrt_session_get(), sync_trigger->a2dp_session);
        TRACE(1,"[%s] session froce resume and try retrigger", __func__);
        a2dp_ibrt_session_set(sync_trigger->a2dp_session);
        app_bt_stream_ibrt_audio_mismatch_stopaudio();
        goto exit;
    }

    if (a2dp_audio_lastframe_info_get(&lastframe_info) < 0){
        TRACE(1,"[%s] lastframe not ready mismatch_stopaudio", __func__);
        app_bt_stream_ibrt_audio_mismatch_stopaudio();
        goto exit;
    }

    if (a2dp_audio_decoder_headframe_info_get(&headframe_info) < 0){
        TRACE(1,"[%s] lastframe not ready mismatch_stopaudio", __func__);
        app_bt_stream_ibrt_audio_mismatch_stopaudio();
        goto exit;
    }
    TRACE(3,"[%s] info base_seq:%d/%d", __func__, headframe_info.sequenceNumber, sync_trigger->sequenceNumberStart);

    a2dp_audio_detect_next_packet_callback_register(NULL);
    a2dp_audio_detect_store_packet_callback_register(NULL);

    if (headframe_info.sequenceNumber && sync_trigger->sequenceNumberStart)
    {
        sync_info.sequenceNumber = sync_trigger->sequenceNumberStart;
        synchronize_ret = a2dp_audio_synchronize_packet(&sync_info, A2DP_AUDIO_SYNCFRAME_MASK_SEQ);
        if (synchronize_ret){
            TRACE(1,"[%s] synchronize_packet failed", __func__);
            app_bt_stream_ibrt_audio_mismatch_stopaudio();
            goto exit;
        }
    }

    curr_ticks = bt_syn_get_curr_ticks(p_ibrt_ctrl->ibrt_conhandle);
    if (tg_tick < curr_ticks){
        TRACE(3,"[%s] synchronize tick failed:%x->%x", __func__, curr_ticks, tg_tick);
        app_bt_stream_ibrt_audio_mismatch_stopaudio();
        goto exit;
    }
    sync_info.sequenceNumber = sync_trigger->audio_info.sequenceNumber;
    sync_info.timestamp = sync_trigger->audio_info.timestamp;
    sync_info.curSubSequenceNumber = sync_trigger->audio_info.curSubSequenceNumber;
    sync_info.totalSubSequenceNumber = sync_trigger->audio_info.totalSubSequenceNumber;
    sync_info.frame_samples = sync_trigger->audio_info.frame_samples;

    if (app_tws_ibrt_slave_ibrt_link_connected()){
        if (app_bt_stream_isrun(APP_BT_STREAM_A2DP_SBC)){
              if (sync_trigger->trigger_type == APP_TWS_IBRT_AUDIO_TRIGGER_TYPE_INIT_SYNC){
                TRACE(2,"[%s] TRIGGER_TYPE_INIT_SYNC needskip:%d", __func__, sync_trigger->trigger_skip_frame);
                //limter to water line upper
                uint32_t list_samples = 0;
                uint32_t dest_samples = 0;
#if defined(USE_LOWLATENCY_LIB)
                if(app_get_ally_flag())
                    dest_samples = 0;
                else
#endif
                    dest_samples = a2dp_audio_dest_packet_mut_get() * lastframe_info.list_samples;
                a2dp_audio_convert_list_to_samples(&list_samples);
                if(lastframe_info.totalSubSequenceNumber == 0)
                    dest_samples += (A2DP_PLAYER_PLAYBACK_WATER_LINE * lastframe_info.list_samples);
                else
                    dest_samples += (A2DP_PLAYER_PLAYBACK_WATER_LINE * lastframe_info.list_samples * lastframe_info.totalSubSequenceNumber);
#if defined(USE_LOWLATENCY_LIB)
                if (sync_trigger->audio_info.sequenceNumber <= lastframe_info.sequenceNumber){
                    uint32_t list_samples_skip_frame = 0;
                    if (!a2dp_audio_convert_list_to_samples_by_start_seq(&list_samples_skip_frame, sync_trigger->audio_info.sequenceNumber)) {
                        dest_samples += list_samples_skip_frame;
                    }
                }
#endif
                TRACE(3,"[%s] synchronize:%d/%d", __func__, list_samples, dest_samples );
                if (list_samples > dest_samples){
                    is_need_discards_samples = true;
                }
                app_bt_stream_ibrt_auto_synchronize_initsync_start(sync_trigger);
                app_bt_stream_ibrt_auto_synchronize_status_set(APP_TWS_IBRT_AUDIO_SYNCHRONIZE_STATUS_SYNCOK);
            }
#if defined(USE_LOWLATENCY_LIB)
            else if (sync_trigger->trigger_type == APP_TWS_IBRT_AUDIO_TRIGGER_TYPE_RESUME){
                if(app_get_ally_flag()){
                    int analysis_interval = app_tws_ibrt_audio_analysis_interval_get();
                    int32_t dma_buffer_samples = app_bt_stream_get_dma_buffer_samples()/2;;
                    if (lastframe_info.sequenceNumber >= sync_trigger->audio_info.sequenceNumber){
                        sync_trigger->trigger_time += US_TO_BTCLKS(analysis_interval *
                                                                   (app_bt_stream_get_dma_buffer_delay_us()/2)/
                                                                   (double)sync_trigger->factor_reference);
                        sync_trigger->audio_info.sequenceNumber +=
                                            app_bt_stream_ibrt_trigger_seq_diff_calc(dma_buffer_samples, sync_trigger->audio_info.frame_samples,
                                            sync_trigger->audio_info.totalSubSequenceNumber, analysis_interval);
                        sync_trigger->audio_info.timestamp += analysis_interval*(app_bt_stream_get_dma_buffer_samples()/2);
                        sync_trigger->handler_cnt += analysis_interval;
                    }
                    TRACE(4,"[%s] TYPE_INIT_RESUME reconfig seq:%d last:%d tg_tick(%d)",__func__,
                                                                                  sync_trigger->audio_info.sequenceNumber,
                                                                                  lastframe_info.sequenceNumber,
                                                                                  sync_trigger->trigger_time);
                    app_bt_stream_ibrt_auto_synchronize_initsync_start(sync_trigger);
                    app_bt_stream_ibrt_auto_synchronize_status_set(APP_TWS_IBRT_AUDIO_SYNCHRONIZE_STATUS_SYNCOK);
                }
            }
#endif
        }else if (!app_bt_stream_isrun(APP_BT_STREAM_HFP_PCM)){            
            TRACE(1,"[%s] sbc player not active, so try to start it", __func__);
            app_bt_stream_ibrt_auto_synchronize_trigger = *sync_trigger;
            app_bt_stream_ibrt_audio_mismatch_stopaudio();
        }
    }else{
        TRACE(1,"[%s] Not Connected", __func__);
    }
exit:
    return;
}

void app_bt_stream_ibrt_audio_mismatch_resume(void)
{
    ibrt_a2dp_status_t a2dp_status;

    a2dp_ibrt_sync_get_status(&a2dp_status);

    TRACE(2,"[%s] state:%d", __func__, a2dp_status.state);

    if (a2dp_status.state == BTIF_AVDTP_STRM_STATE_STREAMING){
        if (app_tws_ibrt_mobile_link_connected()){
            TRACE(1,"[%s] find role switch so force retrigger", __func__);
            app_ibrt_if_force_audio_retrigger();
        }else{
            app_tws_ibrt_audio_sync_mismatch_resume_notify();
        }
    }
}

void app_bt_stream_ibrt_audio_mismatch_stopaudio_cb(uint32_t status, uint32_t param)
{
    TRACE(1,"[%s]", __func__);

    if (app_bt_stream_isrun(APP_BT_STREAM_A2DP_SBC)){
        TRACE(1,"[%s] try again", __func__);
        app_audio_manager_sendrequest_need_callback(
                                                    APP_BT_STREAM_MANAGER_STOP,BT_STREAM_SBC, 
                                                    BT_DEVICE_ID_1, 
                                                    MAX_RECORD_NUM, 
                                                    (uint32_t)app_bt_stream_ibrt_audio_mismatch_stopaudio_cb,
                                                    (uint32_t)NULL);
    }else{
        app_bt_stream_ibrt_audio_mismatch_resume();
    }    
}

int app_bt_stream_ibrt_audio_mismatch_stopaudio(void)
{
    ibrt_a2dp_status_t a2dp_status;

    a2dp_ibrt_sync_get_status(&a2dp_status);

    TRACE(5,"[%s] state:%d sco:%d sbc:%d media:%d", __func__, a2dp_status.state,
                                                 app_audio_manager_hfp_is_active(BT_DEVICE_ID_1),
                                                 app_audio_manager_a2dp_is_active(BT_DEVICE_ID_1),
                                                 app_bt_stream_isrun(APP_PLAY_BACK_AUDIO));

    if (a2dp_status.state == BTIF_AVDTP_STRM_STATE_STREAMING){
        if (app_audio_manager_a2dp_is_active(BT_DEVICE_ID_1)){            
            TRACE(1,"[%s] resume", __func__);
            app_audio_sendrequest_param(APP_BT_STREAM_A2DP_SBC, (uint8_t)APP_BT_SETTING_RESTART, 0, APP_SYSFREQ_52M);
            app_bt_stream_ibrt_audio_mismatch_resume();
        }else{
            if (app_ibrt_ui_is_profile_exchanged()){
                if (!bt_media_is_sbc_media_active()){
                    TRACE(1,"[%s] sbc not active resume it & force retrigger", __func__);
                    app_audio_manager_sendrequest(APP_BT_STREAM_MANAGER_START,BT_STREAM_SBC, BT_DEVICE_ID_1,MAX_RECORD_NUM);
                    app_ibrt_if_force_audio_retrigger();
                }else{
#ifdef MEDIA_PLAYER_SUPPORT
                    if (app_play_audio_get_aud_id() == AUDIO_ID_BT_MUTE &&
                        app_bt_stream_isrun(APP_PLAY_BACK_AUDIO)){
                        TRACE(1,"[%s] resum on process skip it", __func__);
                    }else
#endif
					{
                        TRACE(1,"[%s] cancel_media and force retrigger", __func__);
                        app_ibrt_if_force_audio_retrigger();
                    }
                }
            }else{
                TRACE(1,"[%s] profile not exchanged skip it", __func__);
            }
        }
    }

    return 0;
}
#endif

void app_bt_stream_set_trigger_time(uint32_t trigger_time_us)
{
    struct AF_STREAM_CONFIG_T *stream_cfg = NULL;
    uint32_t curr_ticks = 0;
    uint32_t dma_buffer_delay_us = 0;
    uint32_t tg_acl_trigger_offset_time = 0;


    if (trigger_time_us){
#if defined(IBRT)
        ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
        uint16_t conhandle = INVALID_HANDLE;
        if (app_tws_ibrt_mobile_link_connected()){
            conhandle = p_ibrt_ctrl->mobile_conhandle;
            curr_ticks = bt_syn_get_curr_ticks(conhandle);
        }else if (app_tws_ibrt_slave_ibrt_link_connected()){
            conhandle = p_ibrt_ctrl->ibrt_conhandle;
            curr_ticks = bt_syn_get_curr_ticks(conhandle);
        }else{
            return;
        }
#else
        curr_ticks = btdrv_syn_get_curr_ticks();
#endif
        af_stream_get_cfg(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg, false);
        btdrv_syn_trigger_codec_en(0);
        btdrv_syn_clr_trigger();

        btdrv_enable_playback_triggler(ACL_TRIGGLE_MODE);

        dma_buffer_delay_us = app_bt_stream_get_dma_buffer_delay_us();
        dma_buffer_delay_us /= 2;
        TRACE(6,"[%s] ch:%d sample_rate:%d bit:%d dma_size:%d delay:%d", __func__, stream_cfg->channel_num, stream_cfg->sample_rate, stream_cfg->bits, stream_cfg->data_size, dma_buffer_delay_us);

        tg_acl_trigger_offset_time = US_TO_BTCLKS(trigger_time_us-dma_buffer_delay_us);
        tg_acl_trigger_time = curr_ticks + tg_acl_trigger_offset_time;
        tg_acl_trigger_start_time = curr_ticks;
#if defined(IBRT)
        bt_syn_set_tg_ticks(tg_acl_trigger_time, conhandle, BT_TRIG_SLAVE_ROLE);
        TRACE(5,"[%s] ibrt trigger curr(%d)-->tg(%d) trig_delay:%d audio_delay:%dus", __func__, 
                                            curr_ticks, tg_acl_trigger_time, trigger_time_us-dma_buffer_delay_us, trigger_time_us+dma_buffer_delay_us);
#else
        bt_syn_set_tg_ticks(tg_acl_trigger_time, 0, BT_TRIG_NONE_ROLE);
        TRACE(5,"[%s] trigger curr(%d)-->tg(%d) trig_delay:%d audio_delay:%dus", __func__, 
                                    curr_ticks, tg_acl_trigger_time, trigger_time_us-dma_buffer_delay_us, trigger_time_us+dma_buffer_delay_us);
#endif

        btdrv_syn_trigger_codec_en(1);
        app_bt_stream_trigger_stauts_set(BT_STREAM_TRIGGER_STATUS_WAIT);
    }else{
        tg_acl_trigger_time = 0;
        tg_acl_trigger_start_time = 0;
        btdrv_syn_trigger_codec_en(0);
        btdrv_syn_clr_trigger();
        bt_syn_cancel_tg_ticks();
        app_bt_stream_trigger_stauts_set(BT_STREAM_TRIGGER_STATUS_NULL);
        TRACE(1,"[%s] trigger clear", __func__);
    }
}

void app_bt_stream_trigger_result(void)
{
    uint32_t curr_ticks = 0; 

    if(tg_acl_trigger_time){
#if defined(IBRT)
        ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
        if (app_tws_ibrt_mobile_link_connected()){
            curr_ticks = bt_syn_get_curr_ticks(p_ibrt_ctrl->mobile_conhandle);
            bt_syn_trig_checker(p_ibrt_ctrl->mobile_conhandle);
        }else if (app_tws_ibrt_slave_ibrt_link_connected()){
            curr_ticks = bt_syn_get_curr_ticks(p_ibrt_ctrl->ibrt_conhandle);
            bt_syn_trig_checker(p_ibrt_ctrl->ibrt_conhandle);
        }else{
            TRACE(5,"%s mobile_link:%d %04x ibrt_link:%d %04x", __func__, app_tws_ibrt_mobile_link_connected(), p_ibrt_ctrl->mobile_conhandle,
                                                                     app_tws_ibrt_slave_ibrt_link_connected(),   p_ibrt_ctrl->ibrt_conhandle);
        }
#else
        curr_ticks = btdrv_syn_get_curr_ticks();
#endif
        TRACE(4,"[trigger_result] trig(%d)-->curr(%d)-->tg(%d) start:%d", (curr_ticks - (uint32_t)US_TO_BTCLKS(app_bt_stream_get_dma_buffer_delay_us()/2)),
                                                                         curr_ticks, tg_acl_trigger_time, tg_acl_trigger_start_time);
        TRACE(2,"tg_trig_diff:%d trig_diff:%d", (uint32_t)BTCLKS_TO_US(curr_ticks-tg_acl_trigger_time), 
                                              (uint32_t)BTCLKS_TO_US(curr_ticks-tg_acl_trigger_start_time));                                                               
        app_bt_stream_set_trigger_time(0);        
        app_bt_stream_trigger_stauts_set(BT_STREAM_TRIGGER_STATUS_OK);
        A2DP_AUDIO_HEADFRAME_INFO_T headframe_info;
        a2dp_audio_decoder_headframe_info_get(&headframe_info);
        TRACE(1,"[trigger_result] synchronize_ok :%d", headframe_info.sequenceNumber);
#if defined(USE_LOWLATENCY_LIB)
        if (app_get_ally_flag()) {
            a2dp_audio_detect_first_packet_clear();
            a2dp_audio_detect_next_packet_callback_register(NULL);
            a2dp_audio_detect_store_packet_callback_register(NULL);
            TRACE(0,"[trigger_result] synchronize_failed");
        }
#endif
    }
}

void app_bt_stream_playback_irq_notification(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream)
{
    if (id != AUD_STREAM_ID_0 || stream != AUD_STREAM_PLAYBACK) {
        return;
    }
    app_bt_stream_trigger_result();
#if defined(IBRT)
    app_tws_ibrt_audio_analysis_interrupt_tick();
#endif
}
extern void a2dp_audio_set_mtu_limit(uint8_t mut);
extern float a2dp_audio_latency_factor_get(void);
extern uint8_t a2dp_lhdc_config_llc_get(void);

void app_bt_stream_trigger_init(void)
{
#if defined(IBRT)
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    if (app_tws_ibrt_mobile_link_connected()){
        tg_acl_trigger_init_time = bt_syn_get_curr_ticks(p_ibrt_ctrl->mobile_conhandle);
    }else if (app_tws_ibrt_slave_ibrt_link_connected()){
        tg_acl_trigger_init_time = bt_syn_get_curr_ticks(p_ibrt_ctrl->ibrt_conhandle);
    }else{
        TRACE(5,"%S mobile_link:%d %04x ibrt_link:%d %04x", __func__, app_tws_ibrt_mobile_link_connected(), p_ibrt_ctrl->mobile_conhandle,
                                                                 app_tws_ibrt_slave_ibrt_link_connected(),   p_ibrt_ctrl->ibrt_conhandle);
   }
#else
    tg_acl_trigger_init_time = btdrv_syn_get_curr_ticks();
#endif
    app_bt_stream_set_trigger_time(0);
    af_codec_sync_config(AUD_STREAM_PLAYBACK, AF_CODEC_SYNC_TYPE_BT, false);
    af_codec_sync_config(AUD_STREAM_PLAYBACK, AF_CODEC_SYNC_TYPE_BT, true);
    app_bt_stream_trigger_stauts_set(BT_STREAM_TRIGGER_STATUS_INIT);
}

void app_bt_stream_trigger_deinit(void)
{
    app_bt_stream_set_trigger_time(0);
}

void app_bt_stream_trigger_start(uint8_t offset)
{
    float tg_trigger_time;
    uint32_t curr_ticks;
    struct AF_STREAM_CONFIG_T *stream_cfg = NULL;
    af_stream_get_cfg(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg, false);

#if defined(IBRT)
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    if (app_tws_ibrt_mobile_link_connected()){
        curr_ticks = bt_syn_get_curr_ticks(p_ibrt_ctrl->mobile_conhandle);
    }else if (app_tws_ibrt_slave_ibrt_link_connected()){
        curr_ticks = bt_syn_get_curr_ticks(p_ibrt_ctrl->ibrt_conhandle);
    }else{
        return;
    }
#else
    curr_ticks = btdrv_syn_get_curr_ticks();    
#endif

    TRACE(4,"[%s] init(%d)-->set_trig(%d) %dus", __func__, tg_acl_trigger_init_time, curr_ticks, (uint32_t)BTCLKS_TO_US(curr_ticks-tg_acl_trigger_init_time));
#if defined(A2DP_SCALABLE_ON)
    if(bt_sbc_player_get_codec_type() == BTIF_AVDTP_CODEC_TYPE_NON_A2DP){
        if(stream_cfg->sample_rate > AUD_SAMPRATE_48000){
            tg_trigger_time = A2DP_PLAYER_PLAYBACK_DELAY_SCALABLE_HIRES_US;
            tg_trigger_time += offset*A2DP_PLAYER_PLAYBACK_DELAY_SCALABLE_HIRES_BASE;
        }else{
            tg_trigger_time = A2DP_PLAYER_PLAYBACK_DELAY_SCALABLE_BASERES_US;
            tg_trigger_time += offset*A2DP_PLAYER_PLAYBACK_DELAY_SCALABLE_BASERES_BASE;
        }
    }else
#endif
#if defined(A2DP_AAC_ON)
    if(bt_sbc_player_get_codec_type() == BTIF_AVDTP_CODEC_TYPE_MPEG2_4_AAC){
        tg_trigger_time = a2dp_stream_get_playcak_delay_us(BTIF_AVDTP_CODEC_TYPE_MPEG2_4_AAC);
        tg_trigger_time += offset*A2DP_PLAYER_PLAYBACK_DELAY_AAC_BASE;
    }else
#endif
#if defined(A2DP_LHDC_ON)
    if(bt_sbc_player_get_codec_type() == BTIF_AVDTP_CODEC_TYPE_NON_A2DP){
        if (a2dp_lhdc_config_llc_get()){
            tg_trigger_time = A2DP_PLAYER_PLAYBACK_DELAY_LHDC_LLC_US;
            tg_trigger_time += offset*A2DP_PLAYER_PLAYBACK_DELAY_LHDC_LLC_BASE;
        }else if(stream_cfg->sample_rate > AUD_SAMPRATE_48000){
            tg_trigger_time = A2DP_PLAYER_PLAYBACK_DELAY_LHDC_HIRES_US;
            tg_trigger_time += offset*A2DP_PLAYER_PLAYBACK_DELAY_LHDC_HIRES_BASE;
        }else{
            tg_trigger_time = A2DP_PLAYER_PLAYBACK_DELAY_LHDC_BASERES_US;
            tg_trigger_time += offset*A2DP_PLAYER_PLAYBACK_DELAY_LHDC_BASERES_BASE;
        }
    }else
#endif
    {
        uint32_t frame_mtu = A2DP_PLAYER_PLAYBACK_DELAY_SBC_FRAME_MTU;
#if defined(USE_LOWLATENCY_LIB)
        if(app_get_ally_flag()){
            frame_mtu = app_ally_get_frame_num();
        }else
#endif
        {
            A2DP_AUDIO_LASTFRAME_INFO_T lastframe_info;
            if (!a2dp_audio_lastframe_info_get(&lastframe_info)){
                frame_mtu = lastframe_info.totalSubSequenceNumber;
            }
        }
        tg_trigger_time = a2dp_stream_get_playcak_delay_us(BTIF_AVDTP_CODEC_TYPE_SBC);
        tg_trigger_time += offset*A2DP_PLAYER_PLAYBACK_DELAY_SBC_BASE*frame_mtu;
    }
#if defined(USE_LOWLATENCY_LIB) && !defined(A2DP_PLAYER_PLAYBACK_WATER_LINE)
    if(app_get_ally_flag())
        tg_trigger_time += app_adbuf_get_adbuf_delay();
#endif
    app_bt_stream_set_trigger_time((uint32_t)tg_trigger_time);
}

bool app_bt_stream_trigger_onprocess(void)
{
    if (app_bt_stream_trigger_stauts_get() == BT_STREAM_TRIGGER_STATUS_INIT){
        return true;
    }else{
        return false;
    }
}

#if defined(IBRT)
uint16_t app_bt_stream_get_skip_frame()
{
    uint16_t skip_frame;
#if defined(USE_LOWLATENCY_LIB)
    if(app_get_ally_flag()){
#if defined(A2DP_LHDC_ON)
        if( BTIF_AVDTP_CODEC_TYPE_LHDC == bt_sbc_player_get_codec_type()) {
            skip_frame = APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_INITSYNC_LHDC_SKIP_FRAME;
        } else
#endif
        {
            skip_frame = APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_INITSYNC_SKIP_FRAME;
        }
        if (retrigger_skip_frame) {
            skip_frame += APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_INITSYNC_SKIP_FRAME_RETRIGGER;
            skip_frame += (uint16_t)a2dp_audio_latency_factor_get();
        }
        if (skip_frame > APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_INITSYNC_SKIP_FRAME_MAX) {
            skip_frame = APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_INITSYNC_SKIP_FRAME_MAX;
        }
    }else
#endif
    {
       skip_frame = 4;
    }
    return skip_frame;
}
#ifdef A2DP_CP_ACCEL
#if defined(USE_LOWLATENCY_LIB)
#define APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_INITSYNC_SKIP_FRAME (6)
#define APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_INITSYNC_LHDC_SKIP_FRAME        (10)
#define APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_INITSYNC_SKIP_FRAME_RETRIGGER   (4)
#define APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_INITSYNC_SKIP_FRAME_MAX   (20)
static bool retrigger_skip_frame = false;
#endif
#if defined(USE_LOWLATENCY_LIB)
void  app_bt_stream_set_retrigger_skip_frame()
{
     retrigger_skip_frame = true;
}

void app_bt_stream_reset_skip_frame()
{
     retrigger_skip_frame = false;
}
#endif
#else
#define APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_INITSYNC_SKIP_FRAME (5)
#endif

APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T app_bt_stream_ibrt_auto_synchronize_initsync_trigger;

int app_bt_stream_ibrt_auto_synchronize_initsync_dataind_cb(btif_media_header_t * header, unsigned char *buf, unsigned int len)
{
    APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T *sync_trigger = &app_bt_stream_ibrt_auto_synchronize_initsync_trigger;
    bool synchronize_ok = false;
    A2DP_AUDIO_LASTFRAME_INFO_T lastframe_info;

    TRACE(4,"[auto_synchronize_initsync_cb] dataind_seq:%d/%d timestamp:%d/%d", header->sequenceNumber,
                                                                             sync_trigger->audio_info.sequenceNumber,
                                                                             header->timestamp,
                                                                             sync_trigger->audio_info.timestamp);
    
    if (a2dp_audio_lastframe_info_get(&lastframe_info)<0){
        TRACE(0,"[auto_synchronize_initsync_cb] force retrigger");
        app_ibrt_if_force_audio_retrigger();
        return 0;
    }

    if (app_bt_stream_trigger_stauts_get() != BT_STREAM_TRIGGER_STATUS_WAIT){
        TRACE(0,"[auto_synchronize_initsync_cb] already end");
        a2dp_audio_detect_next_packet_callback_register(NULL);
        a2dp_audio_detect_store_packet_callback_register(NULL);
        return 0;
    }

#if defined(USE_LOWLATENCY_LIB)
    uint32_t list_samples_skip_frame = 0;
    uint32_t limter_water_line_samples = 0;
#endif
    if (sync_trigger->audio_info.sequenceNumber < header->sequenceNumber){
#if defined(USE_LOWLATENCY_LIB)
        if(app_get_ally_flag()){
            uint32_t list_samples = 0;
            if(lastframe_info.totalSubSequenceNumber == 0)
            {
                limter_water_line_samples += (A2DP_PLAYER_PLAYBACK_WATER_LINE * lastframe_info.list_samples);
            }
            else
            {
                limter_water_line_samples += (A2DP_PLAYER_PLAYBACK_WATER_LINE * lastframe_info.list_samples * lastframe_info.totalSubSequenceNumber);
            }
            if (!a2dp_audio_convert_list_to_samples_by_start_seq(&list_samples_skip_frame, sync_trigger->audio_info.sequenceNumber)) {
                limter_water_line_samples += list_samples_skip_frame;
                a2dp_audio_convert_list_to_samples(&list_samples);
                TRACE(0,"water_line_samples %d/%d", limter_water_line_samples, list_samples);
                if (limter_water_line_samples > list_samples) {
                    TRACE(0,"[auto_synchronize_initsync_cb] force retrigger");
                    app_ibrt_if_force_audio_retrigger();
                    return 0;
                }
            } else {
                TRACE(0,"[auto_synchronize_initsync_cb] force retrigger");
                app_ibrt_if_force_audio_retrigger();
                return 0;
            }
        }else
#endif
        {
            TRACE(0,"[auto_synchronize_initsync_cb] force retrigger");
            app_ibrt_if_force_audio_retrigger();
        }
    }else{
        if (header->sequenceNumber >= sync_trigger->audio_info.sequenceNumber){
            synchronize_ok = true;
        }

        if (synchronize_ok && app_bt_stream_trigger_stauts_get() != BT_STREAM_TRIGGER_STATUS_WAIT){
            TRACE(0,"[auto_synchronize_initsync_cb] synchronize_failed");
            app_ibrt_if_force_audio_retrigger();
            return 0;
        }
#ifdef A2DP_PLAYER_PLAYBACK_WATER_LINE
        //limter to water line upper
        uint32_t list_samples = 0;
#if defined(USE_LOWLATENCY_LIB)
        if(app_get_ally_flag()){
            a2dp_audio_convert_list_to_samples(&list_samples);

            if (sync_trigger->trigger_type == APP_TWS_IBRT_AUDIO_TRIGGER_TYPE_RESUME) {
                int32_t dma_buffer_samples = app_bt_stream_get_dma_buffer_samples()/2;
                int32_t packet_samples;
                if (sync_trigger->audio_info.totalSubSequenceNumber) {
                    packet_samples = lastframe_info.list_samples * sync_trigger->audio_info.totalSubSequenceNumber;
                } else {
                    packet_samples = lastframe_info.list_samples;
                }

                TRACE(4,"[auto_synchronize_initsync_cb] dma_buffer_samples: %d/%d %d/%d", dma_buffer_samples, packet_samples, sync_trigger->audio_info.totalSubSequenceNumber,
                lastframe_info.list_samples);
                if (dma_buffer_samples > packet_samples) {
                    limter_water_line_samples = dma_buffer_samples - packet_samples;
                } else {
                    limter_water_line_samples = 0;
                }
            }
            limter_water_line_samples += list_samples_skip_frame;
            TRACE(2,"[auto_synchronize_initsync_cb] synchronize:%d/%d",list_samples,limter_water_line_samples);

            if (list_samples > limter_water_line_samples){
                a2dp_audio_discards_samples( list_samples - limter_water_line_samples);
            }
        }else
#endif
        {
            uint32_t dest_samples = a2dp_audio_dest_packet_mut_get() * lastframe_info.list_samples;
            a2dp_audio_convert_list_to_samples(&list_samples);
            if(lastframe_info.totalSubSequenceNumber == 0)
                list_samples -= (A2DP_PLAYER_PLAYBACK_WATER_LINE * lastframe_info.list_samples);
            else
                list_samples -= (A2DP_PLAYER_PLAYBACK_WATER_LINE * lastframe_info.list_samples * lastframe_info.totalSubSequenceNumber);
            TRACE(2,"[auto_synchronize_initsync_cb] synchronize: %d/%d",list_samples,dest_samples);
            if (list_samples >= dest_samples){
                is_need_discards_samples = true;
            }
        }
#else
        //flush all
        a2dp_audio_synchronize_dest_packet_mut(0);
#endif
        if (synchronize_ok){
            A2DP_AUDIO_HEADFRAME_INFO_T headframe_info;
            a2dp_audio_decoder_headframe_info_get(&headframe_info);
            TRACE(1,"[auto_synchronize_initsync_cb] synchronize_ok :%d", headframe_info.sequenceNumber);
            a2dp_audio_detect_next_packet_callback_register(NULL);
            a2dp_audio_detect_store_packet_callback_register(NULL);
        }else{
            a2dp_audio_detect_first_packet();
        }
    }
    return 0;
}

void app_bt_stream_ibrt_auto_synchronize_initsync_start(APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T *sync_trigger)
{
    TRACE(5,"[auto_synchronize_initsync_start] trigger_time:%d seq:%d timestamp:%d SubSeq:%d/%d", sync_trigger->trigger_time,
                                                                                       sync_trigger->audio_info.sequenceNumber,
                                                                                       sync_trigger->audio_info.timestamp,
                                                                                       sync_trigger->audio_info.curSubSequenceNumber,
                                                                                       sync_trigger->audio_info.totalSubSequenceNumber
                                                                                       );
    app_bt_stream_ibrt_auto_synchronize_initsync_trigger = *sync_trigger;
    a2dp_audio_detect_next_packet_callback_register(app_bt_stream_ibrt_auto_synchronize_initsync_dataind_cb);
    a2dp_audio_detect_first_packet();
    if (app_tws_ibrt_slave_ibrt_link_connected()){        
        ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
        if (sync_trigger->trigger_time > bt_syn_get_curr_ticks(p_ibrt_ctrl->ibrt_conhandle)){
            uint32_t tg_tick = sync_trigger->trigger_time;
            btdrv_syn_trigger_codec_en(1);
            btdrv_syn_clr_trigger();
            btdrv_enable_playback_triggler(ACL_TRIGGLE_MODE);
            bt_syn_set_tg_ticks(tg_tick, p_ibrt_ctrl->ibrt_conhandle, BT_TRIG_SLAVE_ROLE);
            tg_acl_trigger_time = tg_tick;
            app_bt_stream_trigger_stauts_set(BT_STREAM_TRIGGER_STATUS_WAIT);
            TRACE(3,"[%s] slave trigger curr(%d)-->tg(%d)", __func__, bt_syn_get_curr_ticks(p_ibrt_ctrl->ibrt_conhandle), tg_tick);
            app_tws_ibrt_audio_analysis_start(sync_trigger->handler_cnt, AUDIO_ANALYSIS_CHECKER_INTERVEL_INVALID);
            app_tws_ibrt_audio_sync_start();
            app_tws_ibrt_audio_sync_new_reference(sync_trigger->factor_reference);
        }else{
            TRACE(2,"[auto_synchronize_initsync_start] slave failed trigger(%d)-->tg(%d) need resume", 
                                                                                bt_syn_get_curr_ticks(p_ibrt_ctrl->ibrt_conhandle),
                                                                                sync_trigger->trigger_time);        
            app_tws_ibrt_audio_sync_mismatch_resume_notify();
        }
    }
}

int app_bt_stream_ibrt_audio_master_detect_next_packet_cb(btif_media_header_t * header, unsigned char *buf, unsigned int len)
{
    A2DP_AUDIO_HEADFRAME_INFO_T headframe_info;

    if(app_bt_stream_trigger_stauts_get() == BT_STREAM_TRIGGER_STATUS_INIT){
        ibrt_ctrl_t  *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
        int32_t dma_buffer_samples = app_bt_stream_get_dma_buffer_samples()/2;

        if (app_tws_ibrt_slave_ibrt_link_connected()){
            TRACE(1,"cache ok but currRole:%d mismatch\n", p_ibrt_ctrl->current_role);
            app_ibrt_if_force_audio_retrigger();
        }else if (!app_ibrt_ui_is_profile_exchanged() &&
                  !app_ibrt_if_start_ibrt_onporcess() &&
                  !app_ibrt_sync_a2dp_status_onporcess()){
            if (p_ibrt_ctrl->mobile_mode == IBRT_SNIFF_MODE){
                //flush all
                a2dp_audio_synchronize_dest_packet_mut(0);
                a2dp_audio_detect_first_packet();
                TRACE(0,"cache skip delay dma trigger1\n");
                return 0;
            }
            TRACE(0,"cache ok use dma trigger1\n");
            a2dp_audio_detect_next_packet_callback_register(NULL);
            a2dp_audio_detect_store_packet_callback_register(NULL);
#ifdef A2DP_PLAYER_PLAYBACK_WATER_LINE
            app_bt_stream_trigger_start(A2DP_PLAYER_PLAYBACK_WATER_LINE);
#else
            app_bt_stream_trigger_start(0);
#endif
        }else if (app_ibrt_if_start_ibrt_onporcess() ||
                  app_ibrt_sync_a2dp_status_onporcess() ||
                  app_ibrt_waiting_cmd_rsp()){
            //flush all
            a2dp_audio_synchronize_dest_packet_mut(0);
            a2dp_audio_detect_first_packet();
            TRACE(0,"cache skip profile_exchanged sync_a2dp_status_onporcess\n");
            return 0;
        }else{
            if (p_ibrt_ctrl->tws_mode == IBRT_SNIFF_MODE    ||
                p_ibrt_ctrl->mobile_mode == IBRT_SNIFF_MODE){
                //flush all
                a2dp_audio_synchronize_dest_packet_mut(0);
                a2dp_audio_detect_first_packet();
                TRACE(0,"cache skip delay dma trigger2\n");
                return 0;
            }

            uint32_t dest_waterline_samples = 0;
            uint32_t list_samples = 0;
            A2DP_AUDIO_SYNCFRAME_INFO_T sync_info;
            dest_waterline_samples = app_bt_stream_get_dma_buffer_samples()/2*A2DP_PLAYER_PLAYBACK_WATER_LINE;
            a2dp_audio_convert_list_to_samples(&list_samples);
            if (list_samples < dest_waterline_samples){
                a2dp_audio_detect_first_packet();
                TRACE(1,"cache skip fill data sample:%d\n", list_samples);
                return 0;
            }
            a2dp_audio_decoder_headframe_info_get(&headframe_info);
            sync_info.sequenceNumber = headframe_info.sequenceNumber;
            a2dp_audio_synchronize_packet(&sync_info, A2DP_AUDIO_SYNCFRAME_MASK_SEQ);
            a2dp_audio_detect_next_packet_callback_register(NULL);
            TRACE(0,"cache ok use dma trigger2\n");

#if defined(USE_LOWLATENCY_LIB)
            if(app_get_ally_flag()){
                app_bt_stream_trigger_start(APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_LLY_INITSYNC_SKIP_FRAME - a2dp_audio_frame_delay_get() );
                TRACE(1,"initsync_skip_frame %d\n", APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_LLY_INITSYNC_SKIP_FRAME);
                APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T sync_trigger;
                A2DP_AUDIO_HEADFRAME_INFO_T headframe_info;
                A2DP_AUDIO_LASTFRAME_INFO_T lastframe_info;
                sync_trigger.trigger_time = tg_acl_trigger_time;
                sync_trigger.trigger_skip_frame = APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_LLY_INITSYNC_SKIP_FRAME - a2dp_audio_frame_delay_get();
                sync_trigger.trigger_type = APP_TWS_IBRT_AUDIO_TRIGGER_TYPE_INIT_SYNC;
                if (a2dp_audio_lastframe_info_get(&lastframe_info)<0){
                    goto exit;
                }
                a2dp_audio_decoder_headframe_info_get(&headframe_info);
                sync_trigger.sequenceNumberStart = headframe_info.sequenceNumber;
                sync_trigger.audio_info.sequenceNumber = lastframe_info.sequenceNumber + APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_LLY_INITSYNC_SKIP_FRAME;
                if (lastframe_info.totalSubSequenceNumber){
                    sync_trigger.audio_info.timestamp = lastframe_info.timestamp +
                                                    (lastframe_info.totalSubSequenceNumber * lastframe_info.frame_samples) *
                                                    APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_LLY_INITSYNC_SKIP_FRAME;
                }else{
                    sync_trigger.audio_info.timestamp = lastframe_info.timestamp + dma_buffer_samples * APP_BT_STREAM_IBRT_AUTO_SYNCHRONIZE_LLY_INITSYNC_SKIP_FRAME;
                }
                sync_trigger.audio_info.curSubSequenceNumber = lastframe_info.curSubSequenceNumber;
                sync_trigger.audio_info.totalSubSequenceNumber = lastframe_info.totalSubSequenceNumber;
                sync_trigger.audio_info.frame_samples = lastframe_info.frame_samples;
                sync_trigger.factor_reference = a2dp_audio_get_output_config()->factor_reference;
                sync_trigger.a2dp_session = a2dp_ibrt_session_get();
                sync_trigger.handler_cnt = 0;

                app_bt_stream_ibrt_auto_synchronize_initsync_start(&sync_trigger);
                if (app_tws_ibrt_mobile_link_connected() &&
                    app_ibrt_ui_ibrt_connected() &&
                    app_ibrt_ui_is_profile_exchanged()){
                    tws_ctrl_send_cmd(APP_TWS_CMD_SET_TRIGGER_TIME, (uint8_t*)&sync_trigger, sizeof(APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T));
                }
            }else
#endif
            {
                uint16_t skip_frame = app_bt_stream_get_skip_frame();
                TRACE(1,"skip_frame %d", skip_frame);
                app_bt_stream_trigger_start(skip_frame - a2dp_audio_frame_delay_get());
                APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T sync_trigger;
                A2DP_AUDIO_HEADFRAME_INFO_T headframe_info;
                A2DP_AUDIO_LASTFRAME_INFO_T lastframe_info;
                sync_trigger.trigger_time = tg_acl_trigger_time;
                sync_trigger.trigger_skip_frame = skip_frame - a2dp_audio_frame_delay_get();
                sync_trigger.trigger_type = APP_TWS_IBRT_AUDIO_TRIGGER_TYPE_INIT_SYNC;
                if (a2dp_audio_lastframe_info_get(&lastframe_info)<0){
                    goto exit;
                }
                a2dp_audio_decoder_headframe_info_get(&headframe_info);
                sync_trigger.sequenceNumberStart = headframe_info.sequenceNumber;
#if defined(USE_LOWLATENCY_LIB)
                if(app_get_ally_flag()){
                    app_bt_stream_set_retrigger_skip_frame();
                    sync_trigger.audio_info.sequenceNumber = lastframe_info.sequenceNumber + skip_frame;
                    if (lastframe_info.totalSubSequenceNumber){
                        sync_trigger.audio_info.timestamp = lastframe_info.timestamp +
                                                            (lastframe_info.totalSubSequenceNumber * lastframe_info.frame_samples) * skip_frame;
                    }else{
                        sync_trigger.audio_info.timestamp = lastframe_info.timestamp + dma_buffer_samples * skip_frame;
                    }
                }else
#endif
                {
                    uint32_t dest_packet = a2dp_audio_dest_packet_mut_get();
                    if(lastframe_info.totalSubSequenceNumber)
                        dest_packet /= lastframe_info.totalSubSequenceNumber;
                    sync_trigger.audio_info.sequenceNumber = lastframe_info.sequenceNumber + skip_frame - a2dp_audio_frame_delay_get() + dest_packet;
                    if (lastframe_info.totalSubSequenceNumber){
                        sync_trigger.audio_info.timestamp = lastframe_info.timestamp + 
                                                            (lastframe_info.totalSubSequenceNumber * lastframe_info.frame_samples) *
                                                            (skip_frame - a2dp_audio_frame_delay_get() + dest_packet);
                    }else{
                        sync_trigger.audio_info.timestamp = lastframe_info.timestamp + dma_buffer_samples * (skip_frame - a2dp_audio_frame_delay_get() + dest_packet);
                    }
                }
                sync_trigger.audio_info.curSubSequenceNumber = lastframe_info.curSubSequenceNumber;
                sync_trigger.audio_info.totalSubSequenceNumber = lastframe_info.totalSubSequenceNumber;
                sync_trigger.audio_info.frame_samples = lastframe_info.frame_samples;
                sync_trigger.factor_reference = a2dp_audio_get_output_config()->factor_reference;
                sync_trigger.a2dp_session = a2dp_ibrt_session_get();
                sync_trigger.handler_cnt = 0;

                app_bt_stream_ibrt_auto_synchronize_initsync_start(&sync_trigger);

                if (app_tws_ibrt_mobile_link_connected() &&
                    app_ibrt_ui_is_profile_exchanged()){
                    tws_ctrl_send_cmd_high_priority(APP_TWS_CMD_SET_TRIGGER_TIME, (uint8_t*)&sync_trigger, sizeof(APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T));
                }
            }
        }
    }else{
        if(app_bt_stream_trigger_stauts_get() == BT_STREAM_TRIGGER_STATUS_NULL){
            TRACE(1,"%s audio not ready skip it" ,__func__);
        }else{
            TRACE(2,"%s unhandle status:%d" ,__func__, app_bt_stream_trigger_stauts_get());
            app_ibrt_if_force_audio_retrigger();
        }
    }
exit:
    return 0;
}


int app_bt_stream_ibrt_audio_master_detect_next_packet_start(void)
{
    TRACE(1,"%s" ,__func__);
    a2dp_audio_detect_next_packet_callback_register(app_bt_stream_ibrt_audio_master_detect_next_packet_cb);
    return 0;
}

#define SLAVE_DETECT_NEXT_PACKET_TO_RETRIGGER_THRESHOLD (120)
static uint32_t slave_detect_next_packet_cnt = 0;
int app_bt_stream_ibrt_audio_slave_detect_next_packet_waitforever_cb(btif_media_header_t * header, unsigned char *buf, unsigned int len)
{
    if (app_tws_ibrt_mobile_link_connected() || ++slave_detect_next_packet_cnt > SLAVE_DETECT_NEXT_PACKET_TO_RETRIGGER_THRESHOLD){
        ibrt_ctrl_t  *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
        TRACE(2,"detect_next_packet ok but currRole:%d mismatch packet_cnt:%d\n", p_ibrt_ctrl->current_role,
                                                                                slave_detect_next_packet_cnt);
        slave_detect_next_packet_cnt = 0;
        app_ibrt_if_force_audio_retrigger();
    }else{        
        TRACE(1,"detect_next_packet slave cnt:%d\n", slave_detect_next_packet_cnt);
        a2dp_audio_detect_first_packet();
    }
    return 0;
}

int app_bt_stream_ibrt_audio_slave_detect_next_packet_cb(btif_media_header_t * header, unsigned char *buf, unsigned int len)
{
    if(app_bt_stream_trigger_onprocess()){
        ibrt_ctrl_t  *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
        int32_t dma_buffer_samples = app_bt_stream_get_dma_buffer_samples()/2;

        if (app_tws_ibrt_mobile_link_connected()){
            TRACE(1,"cache ok but currRole:%d mismatch\n", p_ibrt_ctrl->current_role);
            app_ibrt_if_force_audio_retrigger();
        }else{
            if (app_tws_ibrt_slave_ibrt_link_connected() &&
                (p_ibrt_ctrl->tws_mode == IBRT_SNIFF_MODE    ||
                 p_ibrt_ctrl->mobile_mode == IBRT_SNIFF_MODE ||
                !app_ibrt_ui_is_profile_exchanged())){
                //flush all
                a2dp_audio_synchronize_dest_packet_mut(0);
                a2dp_audio_detect_first_packet();
                TRACE(0,"cache skip delay dma trigger2\n");
                return 0;
            }
            TRACE(0,"cache ok use dma trigger2\n");
            //flush all
            a2dp_audio_synchronize_dest_packet_mut(0);
            uint16_t skip_frame = app_bt_stream_get_skip_frame();
            TRACE(1,"skip_frame %d", skip_frame);
            app_bt_stream_trigger_start(skip_frame - a2dp_audio_frame_delay_get());
            APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T sync_trigger;
            A2DP_AUDIO_LASTFRAME_INFO_T lastframe_info;
            sync_trigger.trigger_time = tg_acl_trigger_time;
            sync_trigger.trigger_skip_frame = skip_frame - a2dp_audio_frame_delay_get();
            sync_trigger.trigger_type = APP_TWS_IBRT_AUDIO_TRIGGER_TYPE_INIT_SYNC;
            if (a2dp_audio_lastframe_info_get(&lastframe_info)<0){
                goto exit;
            }
            uint32_t dest_packet = a2dp_audio_dest_packet_mut_get();
            if(lastframe_info.totalSubSequenceNumber)
                dest_packet /= lastframe_info.totalSubSequenceNumber;
            sync_trigger.audio_info.sequenceNumber = lastframe_info.sequenceNumber + skip_frame - a2dp_audio_frame_delay_get() + dest_packet;
            if (lastframe_info.totalSubSequenceNumber){
                sync_trigger.audio_info.timestamp = lastframe_info.timestamp + 
                                                    (lastframe_info.totalSubSequenceNumber * lastframe_info.frame_samples) *
                                                    (skip_frame - a2dp_audio_frame_delay_get() + dest_packet);
            }else{
                sync_trigger.audio_info.timestamp = lastframe_info.timestamp + dma_buffer_samples * (skip_frame - a2dp_audio_frame_delay_get() + dest_packet);
            }
            sync_trigger.audio_info.curSubSequenceNumber = lastframe_info.curSubSequenceNumber;
            sync_trigger.audio_info.totalSubSequenceNumber = lastframe_info.totalSubSequenceNumber;
            sync_trigger.audio_info.frame_samples = lastframe_info.frame_samples;
            sync_trigger.factor_reference = a2dp_audio_get_output_config()->factor_reference;
            sync_trigger.a2dp_session = a2dp_ibrt_session_get();
            sync_trigger.handler_cnt = 0;

            app_bt_stream_ibrt_auto_synchronize_initsync_start(&sync_trigger);
        }
    }
exit:
    return 0;
}

int app_bt_stream_ibrt_audio_slave_detect_next_packet_start(int need_autotrigger)
{
    TRACE(1,"%s" ,__func__);
    slave_detect_next_packet_cnt = 0;
    if (need_autotrigger){
        a2dp_audio_detect_next_packet_callback_register(app_bt_stream_ibrt_audio_slave_detect_next_packet_waitforever_cb);
    }else{
        app_tws_ibrt_audio_analysis_start(0, AUDIO_ANALYSIS_CHECKER_INTERVEL_INVALID);
        app_tws_ibrt_audio_sync_start();
        a2dp_audio_detect_next_packet_callback_register(app_bt_stream_ibrt_audio_slave_detect_next_packet_cb);
    }
    return 0;
}

#else
int app_bt_stream_detect_next_packet_cb(btif_media_header_t * header, unsigned char *buf, unsigned int len)
{
    if(app_bt_stream_trigger_onprocess()){
        TRACE(0,"cache ok use dma trigger\n");
        app_bt_stream_trigger_start(0);
    }
    return 0;
}
#endif

void app_audio_buffer_check(void)
{
    TRACE(3,"audio buf size[%d] capture buf size[%d] total available space[%d]",
        APP_AUDIO_BUFFER_SIZE, APP_CAPTURE_AUDIO_BUFFER_SIZE, syspool_original_size());
        
    ASSERT((APP_AUDIO_BUFFER_SIZE + APP_CAPTURE_AUDIO_BUFFER_SIZE) <= syspool_original_size(),
        "Audio buffer[%d]+Capture buffer[%d] exceeds the maximum ram sapce[%d]",
        APP_AUDIO_BUFFER_SIZE, APP_CAPTURE_AUDIO_BUFFER_SIZE, syspool_original_size());
}

#if defined(USE_LOWLATENCY_LIB)
extern void a2dp_audio_set_dest_packet_mut(uint16_t dest_packet_mut);
static bool app_a2dp_adbuf_get_rssi(adbuf_rssi *rssi)
{
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    if (p_ibrt_ctrl && rssi) {
       rssi->mobile_rssi = p_ibrt_ctrl->raw_rssi.rssi0;
       rssi->tws_rssi    = p_ibrt_ctrl->raw_rssi.rssi1;
       return true;
    }else{
       return false;
    }
}
#endif
static bool isRun =  false;

#if defined(USE_LOWLATENCY_LIB)
A2DP_PLC_CH_HD *a2dp_plc_ctl = NULL;
#define A2DP_PLAYER_PLAYBACK_DELAY_GAME_MODE_SBC_MTU                (28)
#define A2DP_PLAYER_PLAYBACK_DELAY_GAME_MODE_AAC_MTU                (3)
#define A2DP_PLAYER_PLAYBACK_DELAY_GAME_MODE_LHDC_BASERES_MTU       (16)
#define A2DP_PLAYER_PLAYBACK_DELAY_GAME_MODE_LHDC_HIRES_MTU         (32)
#endif

uint32_t a2dp_stream_get_playcak_delay_ms(uint8_t codec_type,uint16_t mtu)
{
    uint32_t delay_ms = 0;
    switch (codec_type)
    {
        case BTIF_AVDTP_CODEC_TYPE_MPEG2_4_AAC:
            delay_ms = (mtu * A2DP_PLAYER_PLAYBACK_DELAY_AAC_BASE);
            break;
        case BTIF_AVDTP_CODEC_TYPE_NON_A2DP:
#if defined(A2DP_LHDC_ON)
            if (a2dp_lhdc_config_llc_get()){
                delay_ms = (mtu * A2DP_PLAYER_PLAYBACK_DELAY_LHDC_LLC_BASE);
            }else if (a2dp_sample_rate > AUD_SAMPRATE_48000){
                delay_ms = (mtu * A2DP_PLAYER_PLAYBACK_DELAY_LHDC_HIRES_BASE);
            }else{
                delay_ms = (mtu * A2DP_PLAYER_PLAYBACK_DELAY_LHDC_BASERES_BASE);
            }
#endif
            break;

        case BTIF_AVDTP_CODEC_TYPE_SBC:
        default:
            delay_ms = (mtu * A2DP_PLAYER_PLAYBACK_DELAY_SBC_BASE);
            break;
     }
     return delay_ms/1000;
}

uint32_t a2dp_stream_get_playcak_delay_us(uint8_t codec_type)
{
    switch (codec_type)
    {
        case BTIF_AVDTP_CODEC_TYPE_MPEG2_4_AAC:
#if defined(USE_LOWLATENCY_LIB)
            if (app_get_low_latency_game_mode_flag()) {
                return (A2DP_PLAYER_PLAYBACK_DELAY_AAC_BASE * (A2DP_PLAYER_PLAYBACK_DELAY_GAME_MODE_AAC_MTU + A2DP_PLAYER_PLAYBACK_DMA_AAC_MTU));
            } else
#endif
            {
                return (A2DP_PLAYER_PLAYBACK_DELAY_AAC_BASE * (A2DP_PLAYER_PLAYBACK_DELAY_AAC_MTU + A2DP_PLAYER_PLAYBACK_DMA_AAC_MTU));
            }
        case BTIF_AVDTP_CODEC_TYPE_NON_A2DP:
#if defined(A2DP_LHDC_ON)
            if (a2dp_lhdc_config_llc_get()){
                return (A2DP_PLAYER_PLAYBACK_DELAY_LHDC_LLC_MTU * (A2DP_PLAYER_PLAYBACK_DELAY_LHDC_LLC_BASE + A2DP_PLAYER_PLAYBACK_DMA_LHDC_LLC_MTU));
            }else if (a2dp_sample_rate > AUD_SAMPRATE_48000){
#if defined(USE_LOWLATENCY_LIB)
                if (app_get_low_latency_game_mode_flag()) {
                    return (A2DP_PLAYER_PLAYBACK_DELAY_GAME_MODE_LHDC_HIRES_MTU * (A2DP_PLAYER_PLAYBACK_DELAY_LHDC_HIRES_BASE + A2DP_PLAYER_PLAYBACK_DMA_LHDC_HIRES_MTU));
                } else
#endif
                {
                    return (A2DP_PLAYER_PLAYBACK_DELAY_LHDC_HIRES_MTU * (A2DP_PLAYER_PLAYBACK_DELAY_LHDC_HIRES_BASE + A2DP_PLAYER_PLAYBACK_DMA_LHDC_BASERES_MTU));
                }
            }else{
#if defined(USE_LOWLATENCY_LIB)
                if (app_get_low_latency_game_mode_flag()) {
                    return (A2DP_PLAYER_PLAYBACK_DELAY_GAME_MODE_LHDC_BASERES_MTU * (A2DP_PLAYER_PLAYBACK_DELAY_LHDC_BASERES_BASE + A2DP_PLAYER_PLAYBACK_DMA_LHDC_BASERES_MTU));
                } else
#endif
                {
                    return (A2DP_PLAYER_PLAYBACK_DELAY_LHDC_BASERES_MTU * (A2DP_PLAYER_PLAYBACK_DELAY_LHDC_BASERES_BASE  + A2DP_PLAYER_PLAYBACK_DMA_LHDC_BASERES_MTU));
                }
            }
#endif
            break;

            case BTIF_AVDTP_CODEC_TYPE_SBC:
            default:
#if defined(USE_LOWLATENCY_LIB)
            if(app_get_ally_flag()) {
                return (app_ally_get_cur_mtu() * A2DP_PLAYER_PLAYBACK_DELAY_SBC_BASE);
            } else
            if (app_get_low_latency_game_mode_flag()) {
                return (A2DP_PLAYER_PLAYBACK_DELAY_GAME_MODE_SBC_MTU * (A2DP_PLAYER_PLAYBACK_DELAY_SBC_BASE + A2DP_PLAYER_PLAYBACK_DMA_SBC_MTU));
            } else
#endif
            {
                return (A2DP_PLAYER_PLAYBACK_DELAY_SBC_BASE * (A2DP_PLAYER_PLAYBACK_DELAY_SBC_MTU + A2DP_PLAYER_PLAYBACK_DMA_SBC_MTU));
            }
            break;
     }
     return 0;
}

int a2dp_stream_get_dmabuf_mtu()
{
    uint16_t dma_frame_num = 1;
    switch (app_get_current_overlay())
    {
        case APP_OVERLAY_A2DP:
#if defined(USE_LOWLATENCY_LIB)
            if(app_get_ally_flag()){
                dma_frame_num = app_ally_get_cur_mtu();
            }else
#endif
                dma_frame_num = A2DP_PLAYER_PLAYBACK_DMA_SBC_MTU;
            break;
        case APP_OVERLAY_A2DP_AAC:
            dma_frame_num = A2DP_PLAYER_PLAYBACK_DMA_AAC_MTU;
            break;
#if defined(A2DP_SCALABLE_ON)
        case APP_OVERLAY_A2DP_SCALABLE:
            dma_frame_num = A2DP_PLAYER_PLAYBACK_DMA_SCALABLE_MTU;
            break;
#endif
#if defined(A2DP_LHDC_ON)
        case APP_OVERLAY_A2DP_LHDC:
            dma_frame_num = A2DP_PLAYER_PLAYBACK_DMA_LHDC_MTU;
            break;
#endif
#if defined(A2DP_LDAC_ON)
        case APP_OVERLAY_A2DP_LDAC:
            break;
#endif
        default:
            break;
    }
    return dma_frame_num;
}

#if defined(USE_LOWLATENCY_LIB)
uint32_t app_adbuf_get_adbuf_delay()
{
    uint8_t codec_type = bt_sbc_player_get_codec_type();
    uint32_t delay_time = 0;
    //A2DP_AUDIO_LASTFRAME_INFO_T lastframe_info;

    //if (!a2dp_audio_lastframe_info_get(&lastframe_info)){
    //    cur_frame_mtu = lastframe_info.totalSubSequenceNumber;
    //}

#if defined(A2DP_AAC_ON)
    if (codec_type == BTIF_AVDTP_CODEC_TYPE_MPEG2_4_AAC){
        delay_time = app_a2dp_adbf_get_level() * A2DP_PLAYER_PLAYBACK_DELAY_AAC_BASE;
    }else
#endif

    {
        delay_time = app_a2dp_adbf_get_level() * A2DP_PLAYER_PLAYBACK_DELAY_SBC_FRAME_MTU * A2DP_PLAYER_PLAYBACK_DELAY_SBC_BASE;
    }

    return delay_time;
}

#if defined(USE_LOWLATENCY_LIB)
void low_latency_status_change(uint8_t status)
{
    if (app_tws_ibrt_tws_link_connected() && app_tws_ibrt_mobile_link_connected()) {
        app_ally_sync_status_send();
    }
}
#endif
void bt_a2dp_stream_start(float *dest_packet_mut_ptr)
{
    uint8_t codec_type = bt_sbc_player_get_codec_type();

    TRACE(0,"bt_a2dp_stream_start");
    codec_type = codec_type;

#if defined(__TWS_SWITCH_ACCORDING_RSSI_LINK_QUALITY__)
    app_ibrt_ui_start_check_rssi_link_quality(ROLE_SWITCH_A2DP_CH);
    set_tws_switch_enable (ROLE_SWITCH_ENABLE_FLAG_A2DP, true);
#endif

#if defined(USE_LOWLATENCY_LIB)
    uint8_t *bt_audio_buff = NULL;
    int dma_sample = 0;
    uint8_t bits_depth = AUD_BITS_16;

#ifdef A2DP_EQ_24BIT
    bits_depth = AUD_BITS_24;
#elif defined(A2DP_LHDC_ON)
    if (codec_type == BTIF_AVDTP_CODEC_TYPE_NON_A2DP) {
        bits_depth = AUD_BITS_24;
    }
#endif
    app_audio_mempool_get_buff(&bt_audio_buff, a2dp_data_buf_size/2);
    if (a2dp_plc_ctl == NULL) {
        app_a2dp_apc_init(&a2dp_plc_ctl, (void *)bt_audio_buff, a2dp_data_buf_size/2, bits_depth);
    } else {
        TRACE(0,"a2dp plc no deinit!!!");
        app_a2dp_apc_deinit(&a2dp_plc_ctl);
        app_a2dp_apc_init(&a2dp_plc_ctl, (void *)bt_audio_buff, a2dp_data_buf_size/2, bits_depth);
    }
#if defined(USE_LOWLATENCY_LIB)
    if(app_get_ally_flag()) {
        app_a2dp_apc_check_bad_config(a2dp_plc_ctl, 20, 5, 10);
    } else
#endif
    {
        app_a2dp_apc_check_bad_config(a2dp_plc_ctl, 15, 3, 5);
    }
    if (bits_depth == AUD_BITS_24) {
        dma_sample = a2dp_data_buf_size/2/AUD_CHANNEL_NUM_2/sizeof(uint32_t);
    } else {
        dma_sample = a2dp_data_buf_size/2/AUD_CHANNEL_NUM_2/sizeof(uint16_t);
    }
    switch (codec_type)
    {
        case BTIF_AVDTP_CODEC_TYPE_SBC:
            app_a2dp_packet_sync_init(dma_sample, a2dp_audio_sbc_add_frame, a2dp_audio_sbc_remove_frame, a2dp_get_cur_buf_mtu);
            break;
        case BTIF_AVDTP_CODEC_TYPE_MPEG2_4_AAC:
            app_a2dp_packet_sync_init(dma_sample, a2dp_audio_aac_add_frame, a2dp_audio_aac_remove_frame, a2dp_get_cur_buf_mtu);
            break;
        case BTIF_AVDTP_CODEC_TYPE_NON_A2DP:
#if defined(A2DP_LHDC_ON)
            app_a2dp_packet_sync_init(dma_sample, a2dp_audio_lhdc_add_frame, a2dp_audio_lhdc_remove_frame, a2dp_get_cur_buf_mtu);
#endif
            break;
        default:
            break;
     }
#endif

#if defined(USE_LOWLATENCY_LIB)
    app_set_ally_mtu(0, A2DP_PLAYER_PLAYBACK_DELAY_SBC_MTU);
    app_ally_set_frame_num(0, A2DP_PLAYER_PLAYBACK_DELAY_SBC_FRAME_MTU);
    if(app_get_ally_flag()){
        app_ally_mode_start();
        if (app_tws_ibrt_tws_link_connected() == false) {
            //app_ibrt_if_a2dp_lowlatency_scan(A2DP_LOW_LATENCY_PAGE_SCAN_INTERVAL, A2DP_LOW_LATENCY_PAGE_SCAN_WINDOW, A2DP_LOW_LATENCY_PAGE_SCAN_TYPE);
        }
    }
#endif

#if defined(USE_LOWLATENCY_LIB)
    adbuf_cfg adbuf_conf;
    float adbuf_packet_mut = 0;
    float dest_packet_mut = 0;
    uint32_t playback_delay_us = a2dp_stream_get_playcak_delay_us(codec_type);
    if (dest_packet_mut_ptr) {
        dest_packet_mut = *dest_packet_mut_ptr;
    }

    memset(&adbuf_conf, 0x0, sizeof(adbuf_cfg));
    adbuf_conf.get_rssi = app_a2dp_adbuf_get_rssi;
    adbuf_conf.set_mtu_cb = a2dp_audio_set_dest_packet_mut;
    adbuf_conf.check_flag = ADBUF_CHECK_INTERVAL|ADBUF_CHECK_NO_CAHCE;

    switch (codec_type)
    {
        case BTIF_AVDTP_CODEC_TYPE_SBC:
            adbuf_conf.delay_base = A2DP_PLAYER_PLAYBACK_DELAY_SBC_BASE;
#if defined(USE_LOWLATENCY_LIB)
            if (app_get_ally_flag()) {
                adbuf_conf.leve_max = 3;
                adbuf_conf.check_flag = ADBUF_CHECK_INTERVAL;
                adbuf_conf.frame_mut = app_ally_get_frame_num();
                app_a2dp_adbf_set_interval_adjust_coeff(0.9, 0.7);
            } else
#endif

            if (app_get_low_latency_game_mode_flag()) {
                dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_GAME_MODE_SBC_MTU;
                adbuf_conf.leve_max = app_a2dp_adbf_calc_levl(GAME_MODE_MAX_DELAY - playback_delay_us, A2DP_PLAYER_PLAYBACK_DELAY_SBC_BASE, A2DP_PLAYER_PLAYBACK_DELAY_SBC_FRAME_MTU);
                adbuf_conf.check_flag = ADBUF_CHECK_INTERVAL;
                adbuf_conf.frame_mut = A2DP_PLAYER_PLAYBACK_DELAY_SBC_FRAME_MTU;
                app_a2dp_adbf_set_interval_adjust_coeff(0.9, 0.7);
            } else{
                adbuf_conf.frame_mut = A2DP_PLAYER_PLAYBACK_DELAY_SBC_FRAME_MTU;
                adbuf_conf.leve_max  = app_a2dp_adbf_calc_levl(ADBUF_MAX_BUF_DELAY - playback_delay_us, A2DP_PLAYER_PLAYBACK_DELAY_SBC_BASE, A2DP_PLAYER_PLAYBACK_DELAY_SBC_FRAME_MTU);;
            }
            adbuf_packet_mut += app_a2dp_adbf_get_level() * A2DP_PLAYER_PLAYBACK_DELAY_SBC_FRAME_MTU;
            break;

        case BTIF_AVDTP_CODEC_TYPE_MPEG2_4_AAC:
            adbuf_conf.delay_base = A2DP_PLAYER_PLAYBACK_DELAY_AAC_BASE;
            adbuf_conf.frame_mut = 1;
            if (app_get_low_latency_game_mode_flag()) {
                dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_GAME_MODE_AAC_MTU;
                adbuf_conf.leve_max = app_a2dp_adbf_calc_levl(GAME_MODE_MAX_DELAY - playback_delay_us, A2DP_PLAYER_PLAYBACK_DELAY_AAC_BASE, 1);
                adbuf_conf.check_flag = ADBUF_CHECK_INTERVAL;
                app_a2dp_adbf_set_interval_adjust_coeff(0.9, 0.7);
            }else{
                adbuf_conf.leve_max  = app_a2dp_adbf_calc_levl(ADBUF_MAX_BUF_DELAY - playback_delay_us, A2DP_PLAYER_PLAYBACK_DELAY_AAC_BASE, 1);
            }
            adbuf_packet_mut += app_a2dp_adbf_get_level();
            break;

        case BTIF_AVDTP_CODEC_TYPE_NON_A2DP:
#if defined(A2DP_LHDC_ON)
            if (a2dp_lhdc_config_llc_get()){
                adbuf_conf.delay_base = A2DP_PLAYER_PLAYBACK_DELAY_LHDC_LLC_BASE;
                adbuf_conf.frame_mut = A2DP_PLAYER_PLAYBACK_DELAY_LHDC_LLC_FRAME_MTU;
                adbuf_conf.leve_max  = app_a2dp_adbf_calc_levl(ADBUF_MAX_BUF_DELAY - playback_delay_us, A2DP_PLAYER_PLAYBACK_DELAY_LHDC_LLC_FRAME_MTU, A2DP_PLAYER_PLAYBACK_DELAY_LHDC_BASERES_FRAME_MTU);
                adbuf_packet_mut += app_a2dp_adbf_get_level() * A2DP_PLAYER_PLAYBACK_DELAY_LHDC_LLC_FRAME_MTU;
            }else if (a2dp_sample_rate > AUD_SAMPRATE_48000){
                if (app_get_low_latency_game_mode_flag()) {
                    dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_GAME_MODE_LHDC_HIRES_MTU;
                    adbuf_conf.leve_max = app_a2dp_adbf_calc_levl(GAME_MODE_MAX_DELAY - playback_delay_us, A2DP_PLAYER_PLAYBACK_DELAY_LHDC_HIRES_BASE, A2DP_PLAYER_PLAYBACK_DELAY_LHDC_HIRES_FRAME_MTU);
                    adbuf_conf.check_flag = ADBUF_CHECK_INTERVAL;
                    app_a2dp_adbf_set_interval_adjust_coeff(0.9, 0.7);
                } else{
                    adbuf_conf.leve_max  = app_a2dp_adbf_calc_levl(ADBUF_MAX_BUF_DELAY - playback_delay_us, A2DP_PLAYER_PLAYBACK_DELAY_LHDC_HIRES_FRAME_MTU, A2DP_PLAYER_PLAYBACK_DELAY_LHDC_BASERES_FRAME_MTU);
                }
                adbuf_conf.delay_base = A2DP_PLAYER_PLAYBACK_DELAY_LHDC_HIRES_BASE;
                adbuf_conf.frame_mut = A2DP_PLAYER_PLAYBACK_DELAY_LHDC_HIRES_FRAME_MTU;
                adbuf_packet_mut += app_a2dp_adbf_get_level() * A2DP_PLAYER_PLAYBACK_DELAY_LHDC_HIRES_FRAME_MTU;
            }else{
                if (app_get_low_latency_game_mode_flag()) {
                    dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_GAME_MODE_LHDC_BASERES_MTU;
                    adbuf_conf.leve_max = app_a2dp_adbf_calc_levl(GAME_MODE_MAX_DELAY - playback_delay_us, A2DP_PLAYER_PLAYBACK_DELAY_LHDC_BASERES_BASE, A2DP_PLAYER_PLAYBACK_DELAY_LHDC_BASERES_FRAME_MTU);
                    adbuf_conf.check_flag = ADBUF_CHECK_INTERVAL;
                    app_a2dp_adbf_set_interval_adjust_coeff(0.9, 0.7);
                } else{
                    adbuf_conf.leve_max  = app_a2dp_adbf_calc_levl(ADBUF_MAX_BUF_DELAY - playback_delay_us, A2DP_PLAYER_PLAYBACK_DELAY_LHDC_BASERES_BASE, A2DP_PLAYER_PLAYBACK_DELAY_LHDC_BASERES_FRAME_MTU);
                }
                adbuf_conf.delay_base = A2DP_PLAYER_PLAYBACK_DELAY_LHDC_BASERES_BASE;
                adbuf_conf.frame_mut = A2DP_PLAYER_PLAYBACK_DELAY_LHDC_BASERES_FRAME_MTU;
                adbuf_packet_mut += app_a2dp_adbf_get_level() * A2DP_PLAYER_PLAYBACK_DELAY_LHDC_BASERES_FRAME_MTU;
            }
#endif
            break;
        default:
            break;
    }
    adbuf_conf.base_mtu = (int16_t)dest_packet_mut;
    app_a2dp_adbf_set_config(&adbuf_conf);

    if (dest_packet_mut_ptr) {
       *dest_packet_mut_ptr =  dest_packet_mut;
#ifndef A2DP_PLAYER_PLAYBACK_WATER_LINE
        *dest_packet_mut_ptr += adbuf_packet_mut;
#endif
    }
#endif
}

void bt_a2dp_stream_stop()
{
    TRACE(0,"bt_a2dp_stream_stop");
#if defined(__TWS_SWITCH_ACCORDING_RSSI_LINK_QUALITY__)
    app_ibrt_ui_stop_check_rssi_link_quality();
#endif

#if defined(USE_LOWLATENCY_LIB)
    app_ally_mode_stop();
    //app_ibrt_if_a2dp_restore_scan();
#endif

#if defined(USE_LOWLATENCY_LIB)
    app_a2dp_apc_deinit(&a2dp_plc_ctl);
    app_a2dp_packet_sync_deinit();
#endif
}
#endif

int bt_sbc_player(enum PLAYER_OPER_T on, enum APP_SYSFREQ_FREQ_T freq)
{
    struct AF_STREAM_CONFIG_T stream_cfg;
    enum AUD_SAMPRATE_T sample_rate;
    const char *g_log_player_oper_str[] =
    {
        "PLAYER_OPER_START",
        "PLAYER_OPER_STOP",
        "PLAYER_OPER_RESTART",
    };

    uint8_t* bt_audio_buff = NULL;

    uint8_t POSSIBLY_UNUSED *bt_eq_buff = NULL;
    uint32_t POSSIBLY_UNUSED eq_buff_size = 0;
    uint8_t POSSIBLY_UNUSED play_samp_size;
    TRACE(4,"bt_sbc_player work:%d op:%s freq:%d :sample:%d \n", isRun, g_log_player_oper_str[on], freq,a2dp_sample_rate);

    bt_set_playback_triggered(false);

    if ((isRun && on == PLAYER_OPER_START) || (!isRun && on == PLAYER_OPER_STOP))
    {
        TRACE(3,"%s,fail,isRun=%x,on=%x",__func__,isRun,on);
        return 0;
    }
    //app_bt_stream_trigger_checker_stop();

#if defined(A2DP_LHDC_ON) ||defined(A2DP_AAC_ON) || defined(A2DP_SCALABLE_ON) || defined(A2DP_LDAC_ON)
    uint8_t codec_type POSSIBLY_UNUSED = bt_sbc_player_get_codec_type();
#endif

    audio_moredata_cnt = 0;

    if (on == PLAYER_OPER_STOP || on == PLAYER_OPER_RESTART)
    {
#ifdef __THIRDPARTY
        start_by_sbc = false;
#endif

#if defined(IBRT)
        app_bt_stream_ibrt_auto_synchronize_stop();
        app_tws_ibrt_audio_analysis_stop();
        app_tws_ibrt_audio_sync_stop();
#endif
        a2dp_audio_stop();

        af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
#if defined(AUDIO_ANC_FB_MC) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
        af_stream_stop(AUD_STREAM_ID_2, AUD_STREAM_PLAYBACK);
#endif

#if defined(MUSIC_DELAY_CONTROL) && (defined(CHIP_BEST2300) || defined(CHIP_BEST2300P) || defined(CHIP_BEST2300A))
        calib_reset = 1;
        af_stream_dma_tc_irq_disable(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
#endif
#if defined(IBRT)
        af_stream_dma_tc_irq_disable(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
#endif

#if defined(__AUDIO_SPECTRUM__)
        audio_spectrum_close();
#endif

        audio_process_close();

        TRACE(3,"[%s] syspool free size: %d/%d", __FUNCTION__, syspool_free_size(), syspool_total_size());

#if !(defined(__AUDIO_RESAMPLE__) && defined(SW_PLAYBACK_RESAMPLE))
        af_codec_tune(AUD_STREAM_PLAYBACK, 0);
#endif

        af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
#if defined(AUDIO_ANC_FB_MC) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
        af_stream_close(AUD_STREAM_ID_2, AUD_STREAM_PLAYBACK);
#endif

        af_codec_set_playback_post_handler(NULL);

#if defined(__THIRDPARTY)
        bt_sbc_mode = 0;
#endif
#ifdef VOICE_DATAPATH
        app_voicepath_set_stream_state(AUDIO_OUTPUT_STREAMING, false);
        app_voicepath_set_pending_started_stream(AUDIO_OUTPUT_STREAMING, false);
#endif

#if defined(ANC_ASSIST_ENABLED)
        app_anc_assist_set_playback_info(AUD_SAMPRATE_NULL);
        app_anc_assist_set_mode(ANC_ASSIST_MODE_STANDALONE);
#endif

        af_set_irq_notification(NULL);
        if (on == PLAYER_OPER_STOP)
        {
            app_bt_stream_trigger_checker_stop();
#ifdef __A2DP_PLAYER_USE_BT_TRIGGER__
            app_bt_stream_trigger_deinit();
#endif
#ifndef FPGA
#ifdef BT_XTAL_SYNC
            bt_term_xtal_sync(false);
#ifndef BT_XTAL_SYNC_NO_RESET
            bt_term_xtal_sync_default();
#endif
#endif
#endif
            a2dp_audio_deinit();

            app_overlay_unloadall();
#ifdef __THIRDPARTY
            app_thirdparty_specific_lib_event_handle(THIRDPARTY_ID_NO1,THIRDPARTY_STOP2MIC);
            app_thirdparty_specific_lib_event_handle(THIRDPARTY_ID_NO1,THIRDPARTY_START);
#endif
            app_sysfreq_req(APP_SYSFREQ_USER_BT_A2DP, APP_SYSFREQ_32K);
            af_set_priority(osPriorityAboveNormal);
            app_ibrt_if_exec_sleep_hook_blocker_clr(APP_IBRT_IF_SLEEP_HOOK_BLOCKER_A2DP_STREAMING);
        }
#if defined(USE_LOWLATENCY_LIB)
        if(app_get_ally_flag())
            bt_a2dp_stream_stop();
#endif
#if defined(IBRT)
        app_ibrt_ui_rssi_reset();
#endif
    }

    if (on == PLAYER_OPER_START || on == PLAYER_OPER_RESTART)
    {
#if defined(IBRT)
        app_ibrt_ui_rssi_reset();
#endif
        app_ibrt_if_exec_sleep_hook_blocker_set(APP_IBRT_IF_SLEEP_HOOK_BLOCKER_A2DP_STREAMING);
#ifdef __THIRDPARTY
        app_thirdparty_specific_lib_event_handle(THIRDPARTY_ID_NO1,THIRDPARTY_STOP);
#endif
        af_set_priority(osPriorityHigh);
        bt_media_volume_ptr_update_by_mediatype(BT_STREAM_SBC);
        stream_local_volume = btdevice_volume_p->a2dp_vol;

#ifdef __BT_ONE_BRING_TWO__
        if (btif_me_get_activeCons()>1)
        {
            if (freq < APP_SYSFREQ_104M) {
                freq = APP_SYSFREQ_104M;
            }
        }
#endif
#ifdef __PC_CMD_UART__
        if (freq < APP_SYSFREQ_104M) {
            freq = APP_SYSFREQ_104M;
        }
#endif
#if defined(__SW_IIR_EQ_PROCESS__)&&defined(__HW_FIR_EQ_PROCESS__)&&defined(CHIP_BEST1000)
        if (audio_eq_hw_fir_cfg_list[bt_audio_get_eq_index(AUDIO_EQ_TYPE_HW_FIR,0)]->len>128)
        {
            if (freq < APP_SYSFREQ_104M)
            {
                freq = APP_SYSFREQ_104M;
            }
        }
#endif
#if defined(APP_MUSIC_26M) && !defined(__SW_IIR_EQ_PROCESS__)&& !defined(__HW_IIR_EQ_PROCESS__)&& !defined(__HW_FIR_EQ_PROCESS__)
        if (freq < APP_SYSFREQ_26M) {
            freq = APP_SYSFREQ_26M;
        }
#else
        if (freq < APP_SYSFREQ_52M) {
            freq = APP_SYSFREQ_52M;
        }
#endif

#if defined(AUDIO_ANC_FB_MC) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
        if (freq < APP_SYSFREQ_104M) {
            freq = APP_SYSFREQ_104M;
        }
#endif

#if defined(A2DP_AAC_ON)
        if (codec_type == BTIF_AVDTP_CODEC_TYPE_MPEG2_4_AAC) {
            if(freq < APP_SYSFREQ_52M) {
                freq = APP_SYSFREQ_52M;
            }
        }
#endif
#if defined(A2DP_SCALABLE_ON)
        if (codec_type == BTIF_AVDTP_CODEC_TYPE_NON_A2DP)
        {
            if(a2dp_sample_rate==44100)
            {
                if(freq < APP_SYSFREQ_78M) {
                    freq = APP_SYSFREQ_78M;
                }
            }
            else if(a2dp_sample_rate==96000)
            {
                if (freq < APP_SYSFREQ_208M) {
                    freq = APP_SYSFREQ_208M;
                }
            }
        }
        TRACE(1,"a2dp_sample_rate=%d",a2dp_sample_rate);
#endif
#if defined(A2DP_LHDC_ON)
        if (codec_type == BTIF_AVDTP_CODEC_TYPE_NON_A2DP){
            freq = APP_SYSFREQ_52M;
            if (a2dp_sample_rate==96000)
            {
                freq = APP_SYSFREQ_104M;
            }
        }
#endif
#if defined(A2DP_LDAC_ON)
        if (codec_type == BTIF_AVDTP_CODEC_TYPE_NON_A2DP){
            if (freq < APP_SYSFREQ_104M) {
                freq = APP_SYSFREQ_104M;
            }
        }
#endif

#if defined(__AUDIO_DRC__) || defined(__AUDIO_DRC2__)
        freq = (freq < APP_SYSFREQ_208M)?APP_SYSFREQ_208M:freq;
#endif

#ifdef AUDIO_OUTPUT_SW_GAIN
        freq = (freq < APP_SYSFREQ_104M)?APP_SYSFREQ_52M:freq;
#endif

#ifdef PLAYBACK_FORCE_48K
        freq = (freq < APP_SYSFREQ_104M)?APP_SYSFREQ_104M:freq;
#endif

#ifdef A2DP_CP_ACCEL
        // Default freq for SBC
        freq = APP_SYSFREQ_26M;
#if defined(A2DP_AAC_ON)
        if (codec_type == BTIF_AVDTP_CODEC_TYPE_MPEG2_4_AAC) {
            freq = APP_SYSFREQ_26M;
        }
#endif
#if defined(A2DP_SCALABLE_ON) || defined(A2DP_LDAC_ON)
        if (codec_type == BTIF_AVDTP_CODEC_TYPE_NON_A2DP) {
            freq = APP_SYSFREQ_26M;
        }
#endif
#if defined(A2DP_LHDC_ON)
        if (codec_type == BTIF_AVDTP_CODEC_TYPE_NON_A2DP){
            freq = APP_SYSFREQ_52M;
        }
        if (a2dp_sample_rate==96000)
        {
            freq = APP_SYSFREQ_104M;
        }
#endif
#if defined(AUDIO_ANC_FB_MC) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
        if (freq < APP_SYSFREQ_52M) {
            freq = APP_SYSFREQ_52M;
        }
#endif
#if defined(A2DP_LDAC_ON)
        if (codec_type == BTIF_AVDTP_CODEC_TYPE_NON_A2DP){
            if (freq < APP_SYSFREQ_104M) {
                freq = APP_SYSFREQ_104M;
            }
        }
#endif
#endif

#if defined(USE_LOWLATENCY_LIB)
        if(app_get_ally_flag() && (codec_type == BTIF_AVDTP_CODEC_TYPE_SBC)){
            if (freq < APP_SYSFREQ_52M) {
                freq = APP_SYSFREQ_52M;
            }
        }
#endif
        app_sysfreq_req(APP_SYSFREQ_USER_BT_A2DP, freq);
        TRACE(1,"bt_sbc_player: app_sysfreq_req %d", freq);
        TRACE(1,"sys freq calc : %d\n", hal_sys_timer_calc_cpu_freq(5, 0));

        if (on == PLAYER_OPER_START)
        {
            af_set_irq_notification(app_bt_stream_playback_irq_notification);
            ASSERT(!app_ring_merge_isrun(), "Ring playback will be abnormal, please check.");
            if (0)
            {
            }
#if defined(A2DP_AAC_ON)
            else if (codec_type == BTIF_AVDTP_CODEC_TYPE_MPEG2_4_AAC)
            {
                app_overlay_select(APP_OVERLAY_A2DP_AAC);
            }
#endif
#if defined(A2DP_LHDC_ON)
            else if (codec_type == BTIF_AVDTP_CODEC_TYPE_NON_A2DP)
            {
                app_overlay_select(APP_OVERLAY_A2DP_LHDC);
            }
#endif
#if defined(A2DP_SCALABLE_ON)
            else if (codec_type == BTIF_AVDTP_CODEC_TYPE_NON_A2DP)
            {
                app_overlay_select(APP_OVERLAY_A2DP_SCALABLE);
            }
#endif
#if defined(A2DP_LDAC_ON)
            else if (codec_type == BTIF_AVDTP_CODEC_TYPE_NON_A2DP)
            {
                TRACE(0,"bt_sbc_player ldac overlay select \n"); //toto
                app_overlay_select(APP_OVERLAY_A2DP_LDAC);
            }
#endif
            else
            {
                app_overlay_select(APP_OVERLAY_A2DP);
            }

#ifdef BT_XTAL_SYNC

#ifdef __TWS__
            if(app_tws_mode_is_only_mobile())
            {
                btdrv_rf_bit_offset_track_enable(false);
            }
            else
#endif
            {
                btdrv_rf_bit_offset_track_enable(true);
            }
            bt_init_xtal_sync(BT_XTAL_SYNC_MODE_MUSIC, BT_INIT_XTAL_SYNC_MIN, BT_INIT_XTAL_SYNC_MAX, BT_INIT_XTAL_SYNC_FCAP_RANGE);
#endif // BT_XTAL_SYNC
#ifdef __THIRDPARTY     
            app_thirdparty_specific_lib_event_handle(THIRDPARTY_ID_NO1,THIRDPARTY_START2MIC);
#endif
#if defined(__THIRDPARTY)
            bt_sbc_mode = 1;
#endif
        }
        app_audio_mempool_init_with_specific_size(APP_AUDIO_BUFFER_SIZE);

#ifdef __THIRDPARTY
        //app_thirdparty_specific_lib_event_handle(THIRDPARTY_ID_NO1,THIRDPARTY_STOP);
        start_by_sbc = true;
        app_thirdparty_specific_lib_event_handle(THIRDPARTY_ID_NO1,THIRDPARTY_START);
#endif
#if defined(__AUDIO_RESAMPLE__) && defined(SW_PLAYBACK_RESAMPLE)
        sample_rate = AUD_SAMPRATE_50781;
#else
        sample_rate = a2dp_sample_rate;
#endif

        memset(&stream_cfg, 0, sizeof(stream_cfg));

        stream_cfg.channel_num = AUD_CHANNEL_NUM_2;
#ifdef PLAYBACK_FORCE_48K
        stream_cfg.sample_rate = AUD_SAMPRATE_48000;
#else
        stream_cfg.sample_rate = sample_rate;
#endif

#ifdef FPGA
        stream_cfg.device = AUD_STREAM_USE_EXT_CODEC;
#else
        stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
#endif
        stream_cfg.io_path = AUD_OUTPUT_PATH_SPEAKER;
        stream_cfg.vol = stream_local_volume;
        stream_cfg.handler = bt_sbc_player_more_data;

#if defined(A2DP_SCALABLE_ON)
        if (codec_type == BTIF_AVDTP_CODEC_TYPE_NON_A2DP){
            stream_cfg.data_size = SCALABLE_FRAME_SIZE*8;
        }else
#endif
#if defined(A2DP_LHDC_ON)
        if (codec_type == BTIF_AVDTP_CODEC_TYPE_NON_A2DP){
#if defined(A2DP_LHDC_V3)
            if (bt_get_sbc_sample_rate() == AUD_SAMPRATE_96000) {
                stream_cfg.data_size = (256*A2DP_PLAYER_PLAYBACK_DMA_LHDC_HIRES_MTU*2*2*2);
            } else {
                stream_cfg.data_size = (256*A2DP_PLAYER_PLAYBACK_DMA_LHDC_BASERES_MTU*2*2*2);
            }
            if (a2dp_lhdc_config_llc_get()){
                TRACE(0,"USE LHDC_LLC");
                stream_cfg.data_size = (256*A2DP_PLAYER_PLAYBACK_DMA_LHDC_LLC_MTU*2*2*2);
            }
#else
            stream_cfg.data_size = (512*A2DP_PLAYER_PLAYBACK_DMA_LHDC_BASERES_MTU*2*2*2);
#endif
        }else
#endif
#if defined(A2DP_LDAC_ON)
        if(codec_type == BTIF_AVDTP_CODEC_TYPE_NON_A2DP)
            stream_cfg.data_size = (1024*2*2*2);
        else
#endif
#if defined(A2DP_AAC_ON)
        if (codec_type == BTIF_AVDTP_CODEC_TYPE_MPEG2_4_AAC)
            stream_cfg.data_size = (1024*A2DP_PLAYER_PLAYBACK_DMA_AAC_MTU*2*2*2);
        else
#endif
        {
#if defined(USE_LOWLATENCY_LIB)
            if(app_get_ally_flag()){
                stream_cfg.data_size = app_ally_get_data_size();
            }else
#endif
            {
                stream_cfg.data_size = (128*A2DP_PLAYER_PLAYBACK_DMA_SBC_MTU*2*2*2);
            }
        }
        stream_cfg.bits = AUD_BITS_16;

#ifdef A2DP_EQ_24BIT
        stream_cfg.data_size *= 2;
        stream_cfg.bits = AUD_BITS_24;
#elif defined(A2DP_SCALABLE_ON) || defined(A2DP_LHDC_ON)
        if (codec_type == BTIF_AVDTP_CODEC_TYPE_NON_A2DP) {
            stream_cfg.data_size *= 2;
            stream_cfg.bits = AUD_BITS_24;
        }
#endif

        a2dp_data_buf_size = stream_cfg.data_size;

#ifdef VOICE_DATAPATH
        app_voicepath_set_audio_output_sample_rate(stream_cfg.sample_rate);
        app_voicepath_set_audio_output_data_buf_size(stream_cfg.data_size);
#endif

        app_audio_mempool_get_buff(&bt_audio_buff, stream_cfg.data_size);
        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(bt_audio_buff);
#if defined(MUSIC_DELAY_CONTROL) && (defined(CHIP_BEST2300) || defined(CHIP_BEST2300P) || defined(CHIP_BEST2300A))
        lowdelay_sample_size_play_bt=stream_cfg.bits;
        lowdelay_sample_rate_play_bt=stream_cfg.sample_rate;
        lowdelay_data_size_play_bt=stream_cfg.data_size;
        lowdelay_playback_ch_num_bt=stream_cfg.channel_num;
#endif


#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
        uint8_t* promptTmpSourcePcmDataBuf;
        uint8_t* promptTmpTargetPcmDataBuf;
        uint8_t* promptPcmDataBuf;

        app_audio_mempool_get_buff(&promptTmpSourcePcmDataBuf, AUDIO_PROMPT_SOURCE_PCM_BUFFER_SIZE);
        app_audio_mempool_get_buff(&promptTmpTargetPcmDataBuf, AUDIO_PROMPT_TARGET_PCM_BUFFER_SIZE);
        app_audio_mempool_get_buff(&promptPcmDataBuf, AUDIO_PROMPT_PCM_BUFFER_SIZE);
        audio_prompt_buffer_config(MIX_WITH_A2DP_STREAMING, stream_cfg.channel_num,
            stream_cfg.bits, promptTmpSourcePcmDataBuf, promptTmpTargetPcmDataBuf,
            promptPcmDataBuf, AUDIO_PROMPT_PCM_BUFFER_SIZE);
#endif

#if defined(AUDIO_ANC_FB_MC) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
        sample_size_play_bt=stream_cfg.bits;
        sample_rate_play_bt=stream_cfg.sample_rate;
        data_size_play_bt=stream_cfg.data_size;
        playback_buf_bt=stream_cfg.data_ptr;
        playback_size_bt=stream_cfg.data_size;
        if(sample_rate_play_bt==AUD_SAMPRATE_96000)
        {
            playback_samplerate_ratio_bt=4;
        }
        else
        {
            playback_samplerate_ratio_bt=8;
        }
        playback_ch_num_bt=stream_cfg.channel_num;
        mid_p_8_old_l=0;
        mid_p_8_old_r=0;
#endif

#ifdef PLAYBACK_FORCE_48K
        force48k_resample= app_force48k_resample_any_open( stream_cfg.channel_num,
                            app_force48k_resample_iter, stream_cfg.data_size / stream_cfg.channel_num,
                            (float)sample_rate / AUD_SAMPRATE_48000);
#endif

#if defined(ANC_ASSIST_ENABLED)
        app_anc_assist_set_playback_info(stream_cfg.sample_rate);
#if 1
        app_anc_assist_set_mode(ANC_ASSIST_MODE_MUSIC);
#else
        if (codec_type == BTIF_AVDTP_CODEC_TYPE_MPEG2_4_AAC) {
            app_anc_assist_set_mode(ANC_ASSIST_MODE_MUSIC_AAC);
        } else if (codec_type == BTIF_AVDTP_CODEC_TYPE_SBC) {
            app_anc_assist_set_mode(ANC_ASSIST_MODE_MUSIC_SBC);
        } else if (codec_type == BTIF_AVDTP_CODEC_TYPE_NON_A2DP) {
            // TODO: Be careful audioflinger callback frame length.
            app_anc_assist_set_mode(ANC_ASSIST_MODE_MUSIC_AAC);
        } else {
            ASSERT(0, "[%s] codec_type(%d) is invalid!!!", __func__, codec_type);
        }
#endif
#if defined(AUDIO_ADAPTIVE_EQ)
        app_voice_assist_adaptive_eq_open();
#endif
#endif
        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg);

#if defined(AUDIO_ANC_FB_MC) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
        stream_cfg.bits = sample_size_play_bt;
        stream_cfg.channel_num = playback_ch_num_bt;
        stream_cfg.sample_rate = sample_rate_play_bt;
        stream_cfg.device = AUD_STREAM_USE_MC;
        stream_cfg.vol = 0;
        stream_cfg.handler = audio_mc_data_playback_a2dp;
        stream_cfg.io_path = AUD_OUTPUT_PATH_SPEAKER;

        app_audio_mempool_get_buff(&bt_audio_buff, data_size_play_bt*playback_samplerate_ratio_bt);
        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(bt_audio_buff);
        stream_cfg.data_size = data_size_play_bt*playback_samplerate_ratio_bt;

        playback_buf_mc=stream_cfg.data_ptr;
        playback_size_mc=stream_cfg.data_size;

        anc_mc_run_init(hal_codec_anc_convert_rate(sample_rate_play_bt));

        memset(delay_buf_bt,0,sizeof(delay_buf_bt));

        af_stream_open(AUD_STREAM_ID_2, AUD_STREAM_PLAYBACK, &stream_cfg);
        //ASSERT(ret == 0, "af_stream_open playback failed: %d", ret);
#endif

#ifdef __HEAR_THRU_PEAK_DET__
        PEAK_DETECTOR_CFG_T peak_detector_cfg;
        peak_detector_cfg.fs = stream_cfg.sample_rate;
        peak_detector_cfg.bits = stream_cfg.bits;
        peak_detector_cfg.factor_up = 0.6;
        peak_detector_cfg.factor_down = 2.0;
        peak_detector_cfg.reduce_dB = -30;
        peak_detector_init();
        peak_detector_setup(&peak_detector_cfg);
#endif

#if defined(__AUDIO_SPECTRUM__)
        audio_spectrum_open(stream_cfg.sample_rate, stream_cfg.bits);
#endif

#if defined(__HW_FIR_EQ_PROCESS__) && defined(__HW_IIR_EQ_PROCESS__)
        eq_buff_size = stream_cfg.data_size*2;
#elif defined(__HW_FIR_EQ_PROCESS__) && !defined(__HW_IIR_EQ_PROCESS__)

        play_samp_size = (stream_cfg.bits <= AUD_BITS_16) ? 2 : 4;
#if defined(CHIP_BEST2000)
        eq_buff_size = stream_cfg.data_size * sizeof(int32_t) / play_samp_size;
#elif defined(CHIP_BEST1000)
        eq_buff_size = stream_cfg.data_size * sizeof(int16_t) / play_samp_size;
#elif defined(CHIP_BEST2300) || defined(CHIP_BEST2300P) || defined(CHIP_BEST2300A)
        eq_buff_size = stream_cfg.data_size;
#endif
#elif !defined(__HW_FIR_EQ_PROCESS__) && defined(__HW_IIR_EQ_PROCESS__)
        eq_buff_size = stream_cfg.data_size;
#else
        eq_buff_size = 0;
        bt_eq_buff = NULL;
#endif

        if(eq_buff_size > 0)
        {
            app_audio_mempool_get_buff(&bt_eq_buff, eq_buff_size);
        }

        audio_process_init();
#if defined(IBRT)
        enum AUD_CHANNEL_NUM_T sw_ch_num = AUD_CHANNEL_NUM_1;
#else
        enum AUD_CHANNEL_NUM_T sw_ch_num = stream_cfg.channel_num;
#endif
        audio_process_open(stream_cfg.sample_rate, stream_cfg.bits, sw_ch_num, stream_cfg.channel_num, stream_cfg.data_size/(stream_cfg.bits <= AUD_BITS_16 ? 2 : 4)/2, bt_eq_buff, eq_buff_size);

#ifdef __SW_IIR_EQ_PROCESS__
        bt_audio_set_eq(AUDIO_EQ_TYPE_SW_IIR,bt_audio_get_eq_index(AUDIO_EQ_TYPE_SW_IIR,0));
#endif

#ifdef __HW_FIR_EQ_PROCESS__
        bt_audio_set_eq(AUDIO_EQ_TYPE_HW_FIR,bt_audio_get_eq_index(AUDIO_EQ_TYPE_HW_FIR,0));
#endif

#ifdef __HW_DAC_IIR_EQ_PROCESS__
        bt_audio_set_eq(AUDIO_EQ_TYPE_HW_DAC_IIR,bt_audio_get_eq_index(AUDIO_EQ_TYPE_HW_DAC_IIR,0));
#endif

#ifdef __HW_IIR_EQ_PROCESS__
        bt_audio_set_eq(AUDIO_EQ_TYPE_HW_IIR,bt_audio_get_eq_index(AUDIO_EQ_TYPE_HW_IIR,0));
#endif

#ifdef ANC_APP
        anc_status_record = 0xff;
#endif

#if defined(IBRT)
        APP_TWS_IBRT_AUDIO_SYNC_CFG_T sync_config;
        sync_config.factor_reference  = TWS_IBRT_AUDIO_SYNC_FACTOR_REFERENCE;
        sync_config.factor_fast_limit = TWS_IBRT_AUDIO_SYNC_FACTOR_FAST_LIMIT;
        sync_config.factor_slow_limit = TWS_IBRT_AUDIO_SYNC_FACTOR_SLOW_LIMIT;;
        sync_config.dead_zone_us      = TWS_IBRT_AUDIO_SYNC_DEAD_ZONE_US;
        app_tws_ibrt_audio_sync_reconfig(&sync_config);
#else
#if !(defined(__AUDIO_RESAMPLE__) && defined(SW_PLAYBACK_RESAMPLE))
        af_codec_tune(AUD_STREAM_PLAYBACK, 0);
#endif
#endif
        if (on == PLAYER_OPER_START)
        {
            // This might use all of the rest buffer in the mempool,
            // so it must be the last configuration before starting stream.
            A2DP_AUDIO_OUTPUT_CONFIG_T output_config;
            A2DP_AUDIO_CODEC_TYPE a2dp_audio_codec_type = A2DP_AUDIO_CODEC_TYPE_SBC;
            output_config.sample_rate = sample_rate;    
            output_config.num_channels = 2;
            #ifdef A2DP_EQ_24BIT
            output_config.bits_depth = 24;
            #else
            output_config.bits_depth = 16;
            #endif
            output_config.frame_samples = app_bt_stream_get_dma_buffer_samples()/2;
            output_config.factor_reference = 1.0f;
#if defined(IBRT)
            ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
            uint8_t codec_type = bt_sbc_player_get_codec_type();
            float dest_packet_mut = 0;
            int need_autotrigger = a2dp_ibrt_stream_need_autotrigger_getandclean_flag();
            switch (codec_type)
            {
                case BTIF_AVDTP_CODEC_TYPE_SBC:
                    a2dp_audio_codec_type = A2DP_AUDIO_CODEC_TYPE_SBC;
#if defined(USE_LOWLATENCY_LIB)
                    if(app_get_ally_flag()){
                        dest_packet_mut = app_ally_get_cur_mtu();
                    }else
#endif
                        dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_SBC_MTU;
                    break;
                case BTIF_AVDTP_CODEC_TYPE_MPEG2_4_AAC:
                    a2dp_audio_codec_type = A2DP_AUDIO_CODEC_TYPE_MPEG2_4_AAC;
                    dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_AAC_MTU;
                    break;
                case BTIF_AVDTP_CODEC_TYPE_NON_A2DP:
#if defined(A2DP_LHDC_ON)
                    a2dp_audio_codec_type = A2DP_AUDIO_CODEC_TYPE_LHDC;
                    if (a2dp_lhdc_config_llc_get()){
                        dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_LHDC_LLC_MTU;
                    }else if (sample_rate > AUD_SAMPRATE_48000){
                        dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_LHDC_HIRES_MTU;
                    }else{
                        dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_LHDC_BASERES_MTU;
                    }
                    if (codec_type == BTIF_AVDTP_CODEC_TYPE_NON_A2DP)
                    {
                        if(bt_sbc_player_get_sample_bit() == AUD_BITS_16)
                        {
                            output_config.curr_bits = AUD_BITS_16;
                        }
                        else
                        {
                            output_config.curr_bits = AUD_BITS_24;
                        }
                    }
#endif

#if defined(A2DP_SCALABLE_ON)
                    a2dp_audio_codec_type = A2DP_AUDIO_CODEC_TYPE_SCALABL;
                    if (sample_rate > AUD_SAMPRATE_48000){
                        dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_SCALABLE_HIRES_MTU;
                    }else{
                        dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_SCALABLE_BASERES_MTU;
                    }
#endif

                    break;
                default:
                    break;
            }            
            output_config.factor_reference = TWS_IBRT_AUDIO_SYNC_FACTOR_REFERENCE;

#if defined(USE_LOWLATENCY_LIB)
            if(app_get_ally_flag())
                bt_a2dp_stream_start(&dest_packet_mut);
#endif
            A2DP_AUDIO_CHANNEL_SELECT_E a2dp_audio_channel_sel = A2DP_AUDIO_CHANNEL_SELECT_STEREO;
            switch ((AUDIO_CHANNEL_SELECT_E)p_ibrt_ctrl->audio_chnl_sel)
            {
                case AUDIO_CHANNEL_SELECT_STEREO:
                    a2dp_audio_channel_sel = A2DP_AUDIO_CHANNEL_SELECT_STEREO;
                    break;
                case AUDIO_CHANNEL_SELECT_LRMERGE:
                    a2dp_audio_channel_sel = A2DP_AUDIO_CHANNEL_SELECT_LRMERGE;
                    break;
                case AUDIO_CHANNEL_SELECT_LCHNL:
                    a2dp_audio_channel_sel = A2DP_AUDIO_CHANNEL_SELECT_LCHNL;
                    break;
                case AUDIO_CHANNEL_SELECT_RCHNL:
                    a2dp_audio_channel_sel = A2DP_AUDIO_CHANNEL_SELECT_RCHNL;
                    break;
                default:
                    break;
            }

            a2dp_audio_init(freq, a2dp_audio_codec_type, &output_config, a2dp_audio_channel_sel,(uint16_t)dest_packet_mut);

            app_tws_ibrt_audio_analysis_interval_set(sample_rate > AUD_SAMPRATE_48000 ? AUDIO_ANALYSIS_INTERVAL*2 : AUDIO_ANALYSIS_INTERVAL);
            if (app_tws_ibrt_mobile_link_connected()){
                app_tws_ibrt_audio_analysis_start(0, AUDIO_ANALYSIS_CHECKER_INTERVEL_INVALID);
                app_tws_ibrt_audio_sync_start();
                app_bt_stream_ibrt_audio_master_detect_next_packet_start();
            }else if (app_tws_ibrt_slave_ibrt_link_connected()){
                app_bt_stream_ibrt_audio_slave_detect_next_packet_start(need_autotrigger);
            }else{
                TRACE(5,"%S mobile_link:%d %04x ibrt_link:%d %04x", __func__, app_tws_ibrt_mobile_link_connected(), p_ibrt_ctrl->mobile_conhandle,
                                                                         app_tws_ibrt_slave_ibrt_link_connected(),   p_ibrt_ctrl->ibrt_conhandle);
            }
#else
            uint8_t codec_type = bt_sbc_player_get_codec_type();
            uint16_t dest_packet_mut = 0;

            switch (codec_type)
            {
                case BTIF_AVDTP_CODEC_TYPE_SBC:
                    a2dp_audio_codec_type = A2DP_AUDIO_CODEC_TYPE_SBC;
#if defined(USE_LOWLATENCY_LIB)
                    if(app_get_ally_flag()){
                        dest_packet_mut = app_ally_get_cur_mtu();
                    }else
#endif
                        dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_SBC_MTU;
                    break;
                case BTIF_AVDTP_CODEC_TYPE_MPEG2_4_AAC:
                    a2dp_audio_codec_type = A2DP_AUDIO_CODEC_TYPE_MPEG2_4_AAC;
                    dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_AAC_MTU;
                    break;
                case BTIF_AVDTP_CODEC_TYPE_NON_A2DP:
#if defined(A2DP_LHDC_ON)
                    a2dp_audio_codec_type = A2DP_AUDIO_CODEC_TYPE_LHDC;
                    if (a2dp_lhdc_config_llc_get()){
                        dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_LHDC_LLC_MTU;
                    }else if (sample_rate > AUD_SAMPRATE_48000){
                        dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_LHDC_HIRES_MTU;
                    }else{
                        dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_LHDC_BASERES_MTU;
                    }

                    if (codec_type == BTIF_AVDTP_CODEC_TYPE_NON_A2DP)
                    {
                        if(bt_sbc_player_get_sample_bit() == AUD_BITS_16)
                        {
                            output_config.curr_bits = AUD_BITS_16;
                        }
                        else
                        {
                            output_config.curr_bits = AUD_BITS_24;
                        }
                    }
#endif
                    break;
                default:
                    break;
            }
#if defined(USE_LOWLATENCY_LIB)
            if(app_get_ally_flag())
                bt_a2dp_stream_start(&dest_packet_mut);
#endif
            a2dp_audio_init(freq,a2dp_audio_codec_type, &output_config, A2DP_AUDIO_CHANNEL_SELECT_STEREO, dest_packet_mut);
            a2dp_audio_detect_next_packet_callback_register(app_bt_stream_detect_next_packet_cb);
#endif
            a2dp_audio_start();
        }


#if defined(MUSIC_DELAY_CONTROL) && (defined(CHIP_BEST2300) || defined(CHIP_BEST2300P) || defined(CHIP_BEST2300A))
        calib_reset = 1;
#endif
        af_stream_dma_tc_irq_enable(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
#if defined(AUDIO_OUTPUT_SW_GAIN) && defined(AUDIO_OUTPUT_SW_GAIN_BEFORE_DRC)
        af_codec_dac1_sw_gain_enable(false);
#endif
        af_codec_set_playback_post_handler(bt_sbc_player_playback_post_handler);
#ifdef __A2DP_PLAYER_USE_BT_TRIGGER__
        app_bt_stream_trigger_init();
#endif
#ifdef VOICE_DATAPATH
        if (app_voicepath_get_stream_state(VOICEPATH_STREAMING))
        {
            app_voicepath_set_pending_started_stream(AUDIO_OUTPUT_STREAMING, true);
        }
        else
        {
            app_voicepath_set_stream_state(AUDIO_OUTPUT_STREAMING, true);
            af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        }
#else
        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
#endif

#if defined(AUDIO_ANC_FB_MC) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
        af_stream_start(AUD_STREAM_ID_2, AUD_STREAM_PLAYBACK);
#endif

#ifdef __THIRDPARTY
        //app_thirdparty_specific_lib_event_handle(THIRDPARTY_ID_NO1,THIRDPARTY_OTHER_EVENT);
#endif
        app_bt_stream_trigger_checker_start();
    }

    isRun = (on != PLAYER_OPER_STOP);
    return 0;
}

#if defined(SCO_DMA_SNAPSHOT)
static uint32_t sco_trigger_wait_codecpcm = 0;
static uint32_t sco_trigger_wait_btpcm = 0;
void app_bt_stream_sco_trigger_set_codecpcm_triggle(uint8_t triggle_en)
{
    sco_trigger_wait_codecpcm = triggle_en;
}

uint32_t app_bt_stream_sco_trigger_wait_codecpcm_triggle(void)
{
    return sco_trigger_wait_codecpcm;
}

void app_bt_stream_sco_trigger_set_btpcm_triggle(uint32_t triggle_en)
{
    sco_trigger_wait_btpcm = triggle_en;
}

uint32_t app_bt_stream_sco_trigger_wait_btpcm_triggle(void)
{
    return sco_trigger_wait_btpcm;
}

int app_bt_stream_sco_trigger_codecpcm_tick(void)
{
    if(app_bt_stream_sco_trigger_wait_codecpcm_triggle()){
        app_bt_stream_sco_trigger_set_codecpcm_triggle(0);
#if defined(IBRT)
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    if (app_tws_ibrt_mobile_link_connected()){
        TRACE(3,"[%s]tick:%x/%x", __func__, btdrv_syn_get_curr_ticks(), bt_syn_get_curr_ticks(p_ibrt_ctrl->mobile_conhandle));
    }else if (app_tws_ibrt_slave_ibrt_link_connected()){
        TRACE(3,"[%s]tick:%x/%x", __func__, btdrv_syn_get_curr_ticks(), bt_syn_get_curr_ticks(p_ibrt_ctrl->ibrt_conhandle));
    }else{
        TRACE(5,"%s mobile_link:%d %04x ibrt_link:%d %04x", __func__, app_tws_ibrt_mobile_link_connected(), p_ibrt_ctrl->mobile_conhandle,
                                                                 app_tws_ibrt_slave_ibrt_link_connected(),   p_ibrt_ctrl->ibrt_conhandle);
    }
#else
    uint16_t conhdl = 0xFFFF;
    int curr_sco;

    curr_sco = app_audio_manager_get_active_sco_num();
    if (curr_sco != BT_DEVICE_NUM)
    {
        conhdl = btif_hf_get_remote_hci_handle(*app_audio_manager_get_active_sco_chnl());
    }
    if (conhdl != 0xFFFF){
        TRACE(2,"[%s] tick:%x", __func__, bt_syn_get_curr_ticks(conhdl));
    }
#endif
        btdrv_syn_clr_trigger();
        bt_syn_cancel_tg_ticks();
        return 1;
    }
    return 0;
}

int app_bt_stream_sco_trigger_btpcm_tick(void)
{
    if(app_bt_stream_sco_trigger_wait_btpcm_triggle()){
        app_bt_stream_sco_trigger_set_btpcm_triggle(0);
        btdrv_set_bt_pcm_triggler_en(0);
#if defined(IBRT)
     ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
     if (app_tws_ibrt_mobile_link_connected()){
         TRACE(3,"[%s]tick:%x/%x", __func__, btdrv_syn_get_curr_ticks(), bt_syn_get_curr_ticks(p_ibrt_ctrl->mobile_conhandle));
     }else if (app_tws_ibrt_slave_ibrt_link_connected()){
         TRACE(3,"[%s]tick:%x/%x", __func__, btdrv_syn_get_curr_ticks(), bt_syn_get_curr_ticks(p_ibrt_ctrl->ibrt_conhandle));
     }else{
         TRACE(5,"%S mobile_link:%d %04x ibrt_link:%d %04x", __func__, app_tws_ibrt_mobile_link_connected(), p_ibrt_ctrl->mobile_conhandle,
                                                                  app_tws_ibrt_slave_ibrt_link_connected(),   p_ibrt_ctrl->ibrt_conhandle);
     }
#else
    uint16_t conhdl = 0xFFFF;
    int curr_sco;

    curr_sco = app_audio_manager_get_active_sco_num();
    if (curr_sco != BT_DEVICE_NUM)
    {
        conhdl = btif_hf_get_remote_hci_handle(*app_audio_manager_get_active_sco_chnl());
    }
    TRACE(2,"[%s] tick:%x", __func__, bt_syn_get_curr_ticks(conhdl));
#endif
        btdrv_syn_clr_trigger();
        bt_syn_cancel_tg_ticks();
        return 1;
    }
    return 0;
}


void app_bt_stream_sco_trigger_btpcm_start(void )
{
    uint32_t curr_ticks = 0;
    uint32_t tg_acl_trigger_offset_time = 0;
    uint16_t conhdl = 0xFFFF;

    uint32_t lock;

#if defined(IBRT)
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    if (app_tws_ibrt_mobile_link_connected()){
        conhdl = p_ibrt_ctrl->mobile_conhandle;
    }else if (app_tws_ibrt_slave_ibrt_link_connected()){
        conhdl = p_ibrt_ctrl->ibrt_conhandle;
    }else{
        TRACE(5,"%s mobile_link:%d %04x ibrt_link:%d %04x", __func__, app_tws_ibrt_mobile_link_connected(), p_ibrt_ctrl->mobile_conhandle,
                                                                 app_tws_ibrt_slave_ibrt_link_connected(),   p_ibrt_ctrl->ibrt_conhandle);
    }
#else
    int curr_sco;
    curr_sco = app_audio_manager_get_active_sco_num();
    if (curr_sco != BT_DEVICE_NUM)
    {
        conhdl = btif_hf_get_remote_hci_handle(*app_audio_manager_get_active_sco_chnl());
    }
#endif

    lock = int_lock();
    curr_ticks = bt_syn_get_curr_ticks(conhdl);

    tg_acl_trigger_offset_time = (curr_ticks+0x180) - ((curr_ticks+0x180)%192);

    btdrv_set_bt_pcm_triggler_en(0);
    btdrv_set_bt_pcm_en(0);
    btdrv_syn_clr_trigger();
    btdrv_enable_playback_triggler(SCO_TRIGGLE_MODE);

#if defined(IBRT)
    if (app_tws_ibrt_mobile_link_connected()){
        bt_syn_set_tg_ticks(tg_acl_trigger_offset_time, p_ibrt_ctrl->mobile_conhandle, BT_TRIG_SLAVE_ROLE);
        TRACE(1,"app_bt_stream_sco_trigger_btpcm_start set ticks:%d,",tg_acl_trigger_offset_time);
    }else if (app_tws_ibrt_slave_ibrt_link_connected()){
        bt_syn_set_tg_ticks(tg_acl_trigger_offset_time, p_ibrt_ctrl->ibrt_conhandle, BT_TRIG_SLAVE_ROLE);
        TRACE(1,"app_bt_stream_sco_trigger_btpcm_start set ticks:%d,",tg_acl_trigger_offset_time);
    }else{
        TRACE(5,"%S mobile_link:%d %04x ibrt_link:%d %04x", __func__, app_tws_ibrt_mobile_link_connected(), p_ibrt_ctrl->mobile_conhandle,
                                                                 app_tws_ibrt_slave_ibrt_link_connected(),   p_ibrt_ctrl->ibrt_conhandle);
    }
    TRACE(1,"app_bt_stream_sco_trigger_btpcm_start get ticks:%d,",curr_ticks);

#else
    bt_syn_set_tg_ticks(tg_acl_trigger_offset_time, conhdl, BT_TRIG_SLAVE_ROLE);
#endif
    btdrv_set_bt_pcm_triggler_en(1);
    btdrv_set_bt_pcm_en(1); 
    app_bt_stream_sco_trigger_set_btpcm_triggle(1);
    TRACE(4,"[%s]enable pcm_trigger curr clk=%x, triggle_clk=%x, bt_clk=%x", __func__,
                                                                       btdrv_syn_get_curr_ticks(), 
                                                                       tg_acl_trigger_offset_time,
                                                                       bt_syn_get_curr_ticks(conhdl));
    int_unlock(lock);
}

void app_bt_stream_sco_trigger_btpcm_stop(void)
{
    return;
}


#define TIRG_DELAY_THRESHOLD_325US (15) //total:TIRG_DELAY_THRESHOLD_325US*325us

#define TIRG_DELAY_MAX  (20) //total:20*TIRG_DELAY_325US*325us

#define TIRG_DELAY_325US (96) //total:TIRG_DELAY_325US*325us It' up to the codec and bt pcm pingpang buffer.

void app_bt_stream_sco_trigger_codecpcm_start(uint32_t btclk, uint16_t btcnt)
{
    uint32_t curr_ticks = 0;    
    uint32_t tg_acl_trigger_offset_time = 0;
    uint32_t lock;
    uint16_t conhdl = 0xFFFF;
    //must lock the interrupts when set trig ticks.

#if defined(IBRT)
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    if (app_tws_ibrt_mobile_link_connected()){
        conhdl = p_ibrt_ctrl->mobile_conhandle;
    }else if (app_tws_ibrt_slave_ibrt_link_connected()){
        conhdl = p_ibrt_ctrl->ibrt_conhandle;
    }else{
        TRACE(4,"[sco_trigger] mobile_link:%d %04x ibrt_link:%d %04x", app_tws_ibrt_mobile_link_connected(), p_ibrt_ctrl->mobile_conhandle,
                                                                     app_tws_ibrt_slave_ibrt_link_connected(),   p_ibrt_ctrl->ibrt_conhandle);
    }
#else
    int curr_sco;
    curr_sco = app_audio_manager_get_active_sco_num();
    if (curr_sco != BT_DEVICE_NUM)
    {
        conhdl = btif_hf_get_remote_hci_handle(*app_audio_manager_get_active_sco_chnl());
    }
#endif

    lock = int_lock();
    curr_ticks = bt_syn_get_curr_ticks(conhdl);
    TRACE(3,"[sco_trigger] get 1 curr_ticks:%d,btclk:%d,btcnt:%d,",curr_ticks,btclk,btcnt);
#ifdef LOW_DELAY_SCO
    tg_acl_trigger_offset_time=btclk+12+MASTER_MOBILE_BTCLK_OFFSET;
#else
    tg_acl_trigger_offset_time=btclk+24+MASTER_MOBILE_BTCLK_OFFSET;
#endif

    tg_acl_trigger_offset_time = tg_acl_trigger_offset_time * 2;
    if(tg_acl_trigger_offset_time<curr_ticks+TIRG_DELAY_THRESHOLD_325US)
    {
        int tirg_delay=0;
        tirg_delay=((curr_ticks+TIRG_DELAY_THRESHOLD_325US)-tg_acl_trigger_offset_time)/TIRG_DELAY_325US;
        tirg_delay=tirg_delay+1;
        if(tirg_delay>TIRG_DELAY_MAX)
        {
            tirg_delay=TIRG_DELAY_MAX;
            TRACE(0,"[sco_trigger] bt clk convolution!");
        }
        tg_acl_trigger_offset_time=tg_acl_trigger_offset_time+tirg_delay*TIRG_DELAY_325US;
        TRACE(3,"[sco_trigger] Need more tirg_delay:%d,tg_acl_trigger_offset_time:%d,curr_ticks:%d,",tirg_delay,tg_acl_trigger_offset_time,curr_ticks);
    }
    tg_acl_trigger_offset_time &= 0x0fffffff;

    btdrv_syn_trigger_codec_en(0);
//    af_codec_sync_config(AUD_STREAM_PLAYBACK, AF_CODEC_SYNC_TYPE_BT, false);
//    af_codec_sync_config(AUD_STREAM_CAPTURE, AF_CODEC_SYNC_TYPE_BT, false);
    btdrv_syn_clr_trigger();


#if defined(IBRT)
    if (app_tws_ibrt_mobile_link_connected()){
        bt_syn_set_tg_ticks(tg_acl_trigger_offset_time, p_ibrt_ctrl->mobile_conhandle, BT_TRIG_SLAVE_ROLE);
        TRACE(1,"[sco_trigger] set 2 tg_acl_trigger_offset_time:%d",tg_acl_trigger_offset_time);
    }else if (app_tws_ibrt_slave_ibrt_link_connected()){
        bt_syn_set_tg_ticks(tg_acl_trigger_offset_time, p_ibrt_ctrl->ibrt_conhandle, BT_TRIG_SLAVE_ROLE);
        TRACE(1,"[sco_trigger] set 2 tg_acl_trigger_offset_time:%d",tg_acl_trigger_offset_time);
    }else{
        TRACE(4,"[sco_trigger] mobile_link:%d %04x ibrt_link:%d %04x", app_tws_ibrt_mobile_link_connected(), p_ibrt_ctrl->mobile_conhandle,
                                                          app_tws_ibrt_slave_ibrt_link_connected(),   p_ibrt_ctrl->ibrt_conhandle);
    }
#else
    bt_syn_set_tg_ticks(tg_acl_trigger_offset_time, conhdl, BT_TRIG_SLAVE_ROLE);
#endif
    btdrv_syn_trigger_codec_en(1);
    app_bt_stream_sco_trigger_set_codecpcm_triggle(1);
	
    btdrv_enable_playback_triggler(ACL_TRIGGLE_MODE);
	
    int_unlock(lock);
	
    TRACE(4,"[sco_trigger] enable pcm_trigger curr clk=%x trig_offset=%x curr=%x/%x", btdrv_syn_get_curr_ticks(), 
                                                                                     tg_acl_trigger_offset_time,
                                                                                     curr_ticks,
                                                                                     bt_syn_get_curr_ticks(conhdl));
	


//    af_codec_sync_config(AUD_STREAM_PLAYBACK, AF_CODEC_SYNC_TYPE_BT, true);
//    af_codec_sync_config(AUD_STREAM_CAPTURE, AF_CODEC_SYNC_TYPE_BT, true);


}

void app_bt_stream_sco_trigger_codecpcm_stop(void)
{
    af_codec_sync_config(AUD_STREAM_PLAYBACK, AF_CODEC_SYNC_TYPE_BT, false);
    af_codec_sync_config(AUD_STREAM_CAPTURE, AF_CODEC_SYNC_TYPE_BT, false);    
}
#endif

void speech_tx_aec_set_frame_len(int len);
int voicebtpcm_pcm_echo_buf_queue_init(uint32_t size);
void voicebtpcm_pcm_echo_buf_queue_reset(void);
void voicebtpcm_pcm_echo_buf_queue_deinit(void);
#if defined(SCO_DMA_SNAPSHOT)
int voicebtpcm_pcm_audio_init(int sco_sample_rate, int tx_vqe_sample_rate, int rx_vqe_sample_rate, int tx_codec_sample_rate, int rx_codec_sample_rate, int capture_channel_num);
#else
int voicebtpcm_pcm_audio_init(int sco_sample_rate, int codec_sample_rate);
#endif
int voicebtpcm_pcm_audio_deinit(void);
uint32_t voicebtpcm_pcm_audio_data_come(uint8_t *buf, uint32_t len);
uint32_t voicebtpcm_pcm_audio_more_data(uint8_t *buf, uint32_t len);
int store_voicebtpcm_m2p_buffer(unsigned char *buf, unsigned int len);
int get_voicebtpcm_p2m_frame(unsigned char *buf, unsigned int len);
static uint32_t mic_force_mute = 0;
static uint32_t spk_force_mute = 0;
static uint32_t bt_sco_player_code_type = 0;

static enum AUD_CHANNEL_NUM_T sco_play_chan_num;
static enum AUD_CHANNEL_NUM_T sco_cap_chan_num;

static hfp_sco_codec_t g_bt_sco_codec_type = BTIF_HF_SCO_CODEC_NONE;

static void bt_sco_codec_store(void)
{
    g_bt_sco_codec_type = app_audio_manager_get_scocodecid();

    if (g_bt_sco_codec_type == BTIF_HF_SCO_CODEC_NONE) {
        TRACE(2, "[%s] WARNING:%d is invalid sco codec type, use default codec type!", __func__, g_bt_sco_codec_type);
        g_bt_sco_codec_type = BTIF_HF_SCO_CODEC_CVSD;
    }
    TRACE(0, "[%s] Codec Type: %d", __func__, g_bt_sco_codec_type);
}

hfp_sco_codec_t bt_sco_codec_get_type(void)
{
    return g_bt_sco_codec_type;
}

extern "C" bool bt_sco_codec_is_msbc(void)
{
    if (g_bt_sco_codec_type == BTIF_HF_SCO_CODEC_MSBC) {
        return true;
    } else {
        return false;
    }
}

extern "C" bool bt_sco_codec_is_cvsd(void)
{
    if (g_bt_sco_codec_type == BTIF_HF_SCO_CODEC_CVSD) {
        return true;
    } else {
        return false;
    }
}

void bt_sco_mobile_clkcnt_get(uint32_t btclk, uint16_t btcnt,
                                     uint32_t *mobile_master_clk, uint16_t *mobile_master_cnt)
{
#if defined(IBRT)
    app_tws_ibrt_audio_mobile_clkcnt_get(btclk, btcnt, mobile_master_clk, mobile_master_cnt);
#else
    uint16_t conhdl = 0xFFFF;
    int32_t clock_offset;
    uint16_t bit_offset;
    int curr_sco;
    
    curr_sco = app_audio_manager_get_active_sco_num();
    if (curr_sco != BT_DEVICE_NUM){
        conhdl = btif_hf_get_remote_hci_handle(*app_audio_manager_get_active_sco_chnl());
    }

    if (conhdl != 0xFFFF){
        bt_drv_reg_op_piconet_clk_offset_get(conhdl, &clock_offset, &bit_offset);
        //TRACE(4,"mobile piconet clk:%d bit:%d loc clk:%d cnt:%d", clock_offset, bit_offset, btclk, btcnt);
        btdrv_slave2master_clkcnt_convert(btclk, btcnt,
                                          clock_offset, bit_offset,
                                          mobile_master_clk, mobile_master_cnt);
    }else{
        TRACE(3,"%s warning conhdl NULL role:%d conhdl:%x", __func__, conhdl);
        *mobile_master_clk = 0;
        *mobile_master_cnt = 0;
    }
#endif
}


#if defined(__AUDIO_RESAMPLE__) && defined(SW_SCO_RESAMPLE)

#ifdef CHIP_BEST1000
#error "Unsupport SW_SCO_RESAMPLE on best1000 by now"
#endif
#ifdef NO_SCO_RESAMPLE
#error "Conflicted config: NO_SCO_RESAMPLE and SW_SCO_RESAMPLE"
#endif

// The decoded playback data in the first irq is output to DAC after the second irq (PING-PONG buffer)
#define SCO_PLAY_RESAMPLE_ALIGN_CNT     2

static uint8_t sco_play_irq_cnt;
static bool sco_dma_buf_err;
static struct APP_RESAMPLE_T *sco_capture_resample;
static struct APP_RESAMPLE_T *sco_playback_resample;

static int bt_sco_capture_resample_iter(uint8_t *buf, uint32_t len)
{
    voicebtpcm_pcm_audio_data_come(buf, len);
    return 0;
}

static int bt_sco_playback_resample_iter(uint8_t *buf, uint32_t len)
{
    voicebtpcm_pcm_audio_more_data(buf, len);
    return 0;
}

#endif
#if defined(SCO_DMA_SNAPSHOT)

extern int process_downlink_bt_voice_frames(uint8_t *in_buf, uint32_t in_len, uint8_t *out_buf, uint32_t out_len);
extern int process_uplink_bt_voice_frames(uint8_t *in_buf, uint32_t in_len, uint8_t *ref_buf, uint32_t ref_len, uint8_t *out_buf, uint32_t out_len);
#define MSBC_FRAME_LEN (60)
#define PCM_LEN_PER_FRAME (240)
#define CAL_FRAME_NUM (22)
static void bt_sco_codec_tuning(void)
{
    uint32_t btclk;
    uint16_t btcnt;

    uint32_t mobile_master_clk;
    uint16_t mobile_master_cnt;

    uint32_t mobile_master_clk_offset;
    int32_t mobile_master_cnt_offset;    

    static float fre_offset=0.0f;
    static int32_t mobile_master_cnt_offset_init;
    static int32_t mobile_master_cnt_offset_old;
    static uint32_t first_proc_flag=0;
#if defined( __AUDIO_RESAMPLE__) && !defined(AUDIO_RESAMPLE_ANTI_DITHER)
    static uint32_t frame_counter=0;
    static int32_t mobile_master_cnt_offset_max=0;
    static int32_t mobile_master_cnt_offset_min=0;

    static int32_t mobile_master_cnt_offset_resample=0;

     int32_t offset_max=0;
     int32_t offset_min=0;
#endif

    bt_drv_reg_op_dma_tc_clkcnt_get(&btclk, &btcnt);
    bt_sco_mobile_clkcnt_get(btclk, btcnt,
                                             &mobile_master_clk, &mobile_master_cnt);

#if defined(SCO_DMA_SNAPSHOT_DEBUG)
    TRACE(2,"bt_sco_codec_capture_playback_data:btclk:%d,btcnt:%d,",mobile_master_clk,mobile_master_cnt);
#endif

    if((mobile_master_clk < last_mobile_master_clk)&&(mobile_master_clk < BUF_BTCLK_NUM*2))
    {
        //clock wrapped, 0x555556*24=0x8000010, so after clock wrap, offset init need add 16
        mobile_master_clk_offset_init = (mobile_master_clk_offset_init+WRAPED_CLK_OFFSET)%BUF_BTCLK_NUM;
    }
    last_mobile_master_clk = mobile_master_clk;

    mobile_master_clk_offset=(mobile_master_clk-mobile_master_clk_offset_init+BUF_BTCLK_NUM)%BUF_BTCLK_NUM;
    mobile_master_cnt_offset=(int32_t)(mobile_master_clk_offset*BTCLK_CNT_UNIT+(BTCLK_CNT_UNIT-mobile_master_cnt));
    mobile_master_cnt_offset=(int32_t)(mobile_master_cnt_offset-(MASTER_MOBILE_BTCNT_OFFSET+mobile_master_cnt_offset_init));

    if(app_bt_stream_sco_trigger_codecpcm_tick())
    {
        fre_offset=0.0f;
        mobile_master_cnt_offset_old=0;
        first_proc_flag=0;

       if(playback_samplerate_codecpcm==AUD_SAMPRATE_16000)
       {
#ifdef  ANC_APP

#if defined( __AUDIO_RESAMPLE__)
        if (hal_cmu_get_audio_resample_status())
        {
#if defined(AUDIO_RESAMPLE_ANTI_DITHER)
             mobile_master_cnt_offset_init=171;
#else
             mobile_master_cnt_offset_init=107;
#endif
        }
        else
#endif
        {
            mobile_master_cnt_offset_init=90;
        }

#else

#if defined( __AUDIO_RESAMPLE__)
        if (hal_cmu_get_audio_resample_status())
        {
#if defined(AUDIO_RESAMPLE_ANTI_DITHER)
             mobile_master_cnt_offset_init=171;
#else
             mobile_master_cnt_offset_init=146;
#endif
        }
        else
#endif
        {
            mobile_master_cnt_offset_init=113;
        }

#endif
       }
       else if(playback_samplerate_codecpcm==AUD_SAMPRATE_8000)
       {
#ifdef  ANC_APP
#if defined( __AUDIO_RESAMPLE__)
        if (hal_cmu_get_audio_resample_status())
        {
#if defined(AUDIO_RESAMPLE_ANTI_DITHER)
             mobile_master_cnt_offset_init=-270;
#else
             mobile_master_cnt_offset_init=-468;
#endif
        }
        else
#endif
        {
            mobile_master_cnt_offset_init=-445;
        }
#else
#if defined( __AUDIO_RESAMPLE__)
        if (hal_cmu_get_audio_resample_status())
        {
#if defined(AUDIO_RESAMPLE_ANTI_DITHER)
             mobile_master_cnt_offset_init=-270;
#else
             mobile_master_cnt_offset_init=-327;
#endif
        }
        else
#endif
        {
            mobile_master_cnt_offset_init=-386;
        }
#endif
       }
        mobile_master_cnt_offset=(int32_t)(mobile_master_clk_offset*BTCLK_CNT_UNIT+(BTCLK_CNT_UNIT-mobile_master_cnt));
        mobile_master_cnt_offset=(int32_t)(mobile_master_cnt_offset-(MASTER_MOBILE_BTCNT_OFFSET+mobile_master_cnt_offset_init));
        TRACE(1,"****************mobile_master_cnt_offset:%d,",mobile_master_cnt_offset);


#if defined( __AUDIO_RESAMPLE__) && !defined(AUDIO_RESAMPLE_ANTI_DITHER)
#ifdef LOW_DELAY_SCO
        fre_offset=(float)mobile_master_cnt_offset/(CAL_FRAME_NUM*7.5f*1000.0f);
#else
        fre_offset=(float)mobile_master_cnt_offset/(CAL_FRAME_NUM*15.0f*1000.0f);
#endif
#endif

#if defined( __AUDIO_RESAMPLE__) && !defined(AUDIO_RESAMPLE_ANTI_DITHER)
      if (hal_cmu_get_audio_resample_status())
      {
            frame_counter=0;
            mobile_master_cnt_offset_max=0;
            mobile_master_cnt_offset_min=0;
            mobile_master_cnt_offset_resample=0;
      }
#endif
    }


#if defined(  __AUDIO_RESAMPLE__) &&!defined(SW_PLAYBACK_RESAMPLE)&& !defined(AUDIO_RESAMPLE_ANTI_DITHER)
      if (hal_cmu_get_audio_resample_status())
      {
           if(playback_samplerate_codecpcm==AUD_SAMPRATE_16000)
           {
            offset_max=28;
            offset_min=-33;
           }
           else if(playback_samplerate_codecpcm==AUD_SAMPRATE_8000)
           {
            offset_max=12;
            offset_min=-112;
           }

           if(mobile_master_cnt_offset>mobile_master_cnt_offset_max)
           {
            mobile_master_cnt_offset_max=mobile_master_cnt_offset;
           }

           if(mobile_master_cnt_offset<mobile_master_cnt_offset_min)
           {
            mobile_master_cnt_offset_min=mobile_master_cnt_offset;
           }

            frame_counter++;

            if(frame_counter>=CAL_FRAME_NUM)
            {
               if(mobile_master_cnt_offset_min<offset_min)
               {
                    mobile_master_cnt_offset_resample=mobile_master_cnt_offset_min-offset_min;
               }
               else if(mobile_master_cnt_offset_max>offset_max)
               {
                   mobile_master_cnt_offset_resample=mobile_master_cnt_offset_max-offset_max;
               }
               else
               {
                   mobile_master_cnt_offset_resample=0;
               }
              TRACE(2,"mobile_master_cnt_offset_min:%d,mobile_master_cnt_offset_max:%d",mobile_master_cnt_offset_min,mobile_master_cnt_offset_max);
               mobile_master_cnt_offset=mobile_master_cnt_offset_resample;

            if(first_proc_flag==0)
            {
                  fre_offset=((int32_t)(mobile_master_cnt_offset*0.5f))*0.0000001f
                    +(mobile_master_cnt_offset-mobile_master_cnt_offset_old)*0.0000001f;
                  first_proc_flag=1;
            }
            else
            {
                  fre_offset=fre_offset+((int32_t)(mobile_master_cnt_offset*0.5f))*0.0000001f
                    +(mobile_master_cnt_offset-mobile_master_cnt_offset_old)*0.0000001f;
                  first_proc_flag=1;
            }

            mobile_master_cnt_offset_old=mobile_master_cnt_offset;
#if defined(SCO_DMA_SNAPSHOT_DEBUG)
               TRACE(1,"mobile_master_cnt_offset:%d",mobile_master_cnt_offset);
#endif
               mobile_master_cnt_offset_max=0;
               mobile_master_cnt_offset_min=0;
               frame_counter=0;
            }
        }
        else
#endif
        {
            if(mobile_master_cnt_offset>5&&first_proc_flag==0)
            {
                fre_offset=0.0001f;
            }
            else if(mobile_master_cnt_offset<-5&&first_proc_flag==0)
            {
                fre_offset=-0.0001f;
            }
            else
            {
                if(first_proc_flag==0)
                {
                    fre_offset=0;
                }
                fre_offset=fre_offset+((int32_t)(mobile_master_cnt_offset*0.5f))*0.00000001f
                    +(mobile_master_cnt_offset-mobile_master_cnt_offset_old)*0.00000001f;

                mobile_master_cnt_offset_old=mobile_master_cnt_offset;
                first_proc_flag=1;
            }
        }

#if defined(SCO_DMA_SNAPSHOT_DEBUG)
    TRACE(1,"mobile_master_cnt_offset:%d",mobile_master_cnt_offset);
    TRACE(1,"fre_offset:%d",(int)(fre_offset*10000000.0f));
#endif
        if(first_proc_flag==1)
       {
            if(fre_offset>0.0001f)fre_offset=0.0001f;
            if(fre_offset<-0.0001f)fre_offset=-0.0001f;
       }
#if defined(__AUDIO_RESAMPLE__) && defined(SW_PLAYBACK_RESAMPLE)
    app_resample_tune(playback_samplerate_codecpcm, fre_offset);
#else
    af_codec_tune(AUD_STREAM_NUM, fre_offset);
#endif

    return;
}
#endif
extern CQueue* get_tx_esco_queue_ptr();

#if defined(ANC_NOISE_TRACKER)
static int16_t *anc_buf = NULL;
#endif

#if defined(SCO_DMA_SNAPSHOT)
// This is reletive with chip.
#ifdef TX_RX_PCM_MASK
#define BTPCM_TX_OFFSET_BYTES   (0)
#else
#if defined (PCM_FAST_MODE)
#define BTPCM_TX_OFFSET_BYTES   (1)
#else
#define BTPCM_TX_OFFSET_BYTES   (2)
#endif
#endif

#define SCO_MSBC_PACKET_SIZE    (60)

#ifndef PCM_PRIVATE_DATA_FLAG
static uint8_t g_btpcm_tx_buf[BTPCM_TX_OFFSET_BYTES];
#endif
POSSIBLY_UNUSED static void adjust_btpcm_msbc_tx(uint8_t *buf, uint32_t len)
{
    uint16_t *buf_u16 = (uint16_t *)buf;
#ifndef PCM_PRIVATE_DATA_FLAG
    uint8_t *buf_ptr = (uint8_t *)buf;
    uint32_t loop_cnt = len / SCO_MSBC_PACKET_SIZE;

    ASSERT(len % SCO_MSBC_PACKET_SIZE == 0, "[%s] len(%d) is invalid!", __func__, len);

    // Shift
    for (uint32_t cnt=0; cnt <loop_cnt; cnt++) {
        for (uint32_t i=0; i<BTPCM_TX_OFFSET_BYTES; i++) {
            g_btpcm_tx_buf[i] = buf_ptr[i];
        }

        for (uint32_t i=0; i<SCO_MSBC_PACKET_SIZE - BTPCM_TX_OFFSET_BYTES; i++) {
            buf_ptr[i] = buf_ptr[i + BTPCM_TX_OFFSET_BYTES];
        }

        for (uint32_t i=0; i<BTPCM_TX_OFFSET_BYTES; i++) {
            buf_ptr[i + SCO_MSBC_PACKET_SIZE - BTPCM_TX_OFFSET_BYTES] = g_btpcm_tx_buf[i];
        }

        buf_ptr += SCO_MSBC_PACKET_SIZE;
    }
#endif
    // BTCPM trans data with 16bits format and valid data is in high 8 bits.
    for (int32_t i = len-1; i >= 0; i--) {
        buf_u16[i] = ((int16_t)buf[i]) << 8;
    }

#if 0
    TRACE(0, "[%s] %x, %x, %x, %x, %x, %x | %x, %x, %x, %x, %x, %x,", __func__,
                                                        buf_u16[0],
                                                        buf_u16[1],
                                                        buf_u16[2],
                                                        buf_u16[3],
                                                        buf_u16[4],
                                                        buf_u16[5],
                                                        buf_u16[54],
                                                        buf_u16[55],
                                                        buf_u16[56],
                                                        buf_u16[57],
                                                        buf_u16[58],
                                                        buf_u16[59]);
    TRACE(0, "[%s] %x, %x, %x, %x, %x, %x | %x, %x, %x, %x, %x, %x,", __func__,
                                                        buf_u16[60],
                                                        buf_u16[61],
                                                        buf_u16[62],
                                                        buf_u16[63],
                                                        buf_u16[64],
                                                        buf_u16[65],
                                                        buf_u16[114],
                                                        buf_u16[115],
                                                        buf_u16[116],
                                                        buf_u16[117],
                                                        buf_u16[118],
                                                        buf_u16[119]);
#endif
}
#endif

//#define BT_SCO_HANDLER_PROFILE

//( codec:mic-->btpcm:tx
// codec:mic
bool app_bt_sco_chain_bypass_flag = false;
static uint32_t bt_sco_codec_capture_data(uint8_t *buf, uint32_t len)
{
    bool bt_sco_chain_bypass_flag;

    app_bt_stream_trigger_checker_handler(TRIGGER_CHECKER_HFP_AUDPCM_CAPTURE);

    if(app_tws_ibrt_mobile_link_connected()){
        bt_sco_chain_bypass_flag = false;
    }
    if(app_tws_ibrt_slave_ibrt_link_connected()){
        bt_sco_chain_bypass_flag = true;
    }

    if(app_bt_sco_chain_bypass_flag != bt_sco_chain_bypass_flag){
        if(!bt_sco_chain_bypass_flag){
            app_sysfreq_req(APP_SYSFREQ_USER_BT_SCO, APP_SYSFREQ_104M);
            codec_int_mic_active(1);
        }else{
            app_sysfreq_req(APP_SYSFREQ_USER_BT_SCO, APP_SYSFREQ_52M);
            codec_int_mic_active(0);
        }
    }
    app_bt_sco_chain_bypass_flag = bt_sco_chain_bypass_flag;

#if defined(ANC_NOISE_TRACKER)
    int16_t *pcm_buf = (int16_t *)buf;
    uint32_t pcm_len = len / sizeof(short);
    uint32_t ch_num = SPEECH_CODEC_CAPTURE_CHANNEL_NUM + ANC_NOISE_TRACKER_CHANNEL_NUM;
    uint32_t remain_ch_num = SPEECH_CODEC_CAPTURE_CHANNEL_NUM;
    

#if defined(SPEECH_TX_AEC_CODEC_REF)
    ch_num += 1;
    remain_ch_num += 1;
#endif

    ASSERT(pcm_len % ch_num == 0, "[%s] input data length error", __FUNCTION__);

    // assume anc mic in ch0
    for (uint32_t i = 0, j = 0; i < pcm_len; i += ch_num, j += ANC_NOISE_TRACKER_CHANNEL_NUM)
    {
        for (uint32_t ch = 0; ch < ANC_NOISE_TRACKER_CHANNEL_NUM; ch++)
            anc_buf[j + ch] = pcm_buf[i + ch];
    }

    noise_tracker_process(anc_buf, pcm_len / ch_num * ANC_NOISE_TRACKER_CHANNEL_NUM);

    for (uint32_t i = 0, j = 0; i < pcm_len; i += ch_num, j += remain_ch_num)
    {
        for (uint32_t chi = ANC_NOISE_TRACKER_CHANNEL_NUM, cho = 0; chi < ch_num; chi++, cho++)
            pcm_buf[j + cho] = pcm_buf[i + chi];
    }

    len = len / ch_num * remain_ch_num;
#endif

#if defined(AF_ADC_I2S_SYNC)
    //TRACE(2,"[%s] cnt = %d", __func__, codec_capture_cnt++);
#endif

#if defined(ANC_ASSIST_ENABLED)
        if(app_anc_assist_is_runing()) {
#if defined(ASSIST_LOW_RAM_MOD)
            //resample  16k->8k
            anc_assist_resample_process((int32_t *)buf, anc_assist_resample_buf, len / sizeof(_PCM_T));
            // TODO: Use capture buf
            app_anc_assist_process(anc_assist_resample_buf, len / 2);
#else
            app_anc_assist_process(buf, len);
#endif
        }
        app_anc_assist_parser_app_mic_buf(buf, &len);
#endif

#if defined(SCO_DMA_SNAPSHOT)

#if defined(BT_SCO_HANDLER_PROFILE)
    uint32_t start_ticks = hal_fast_sys_timer_get();
#endif

    int pingpang;

    //processing  ping pang flag
    if(buf==capture_buf_codecpcm)
    {
     pingpang=0;
    }
    else
    {
     pingpang=1;
    }

    uint16_t *playback_dst=(uint16_t *)(playback_buf_btpcm+(pingpang)*playback_size_btpcm/2);
    uint16_t *playback_src=(uint16_t *)playback_buf_btpcm_cache;

    for(uint32_t  i =0; i<playback_size_btpcm/4; i++)
    {
        playback_dst[i]=playback_src[i];
    }
#ifdef TX_RX_PCM_MASK
    //processing btpcm.(It must be from CPU's copy )
   if(btdrv_is_pcm_mask_enable()==1&&bt_sco_codec_is_msbc())
   {
	uint32_t lock;
	uint32_t i;
	//must lock the interrupts when exchanging data.
	lock = int_lock();
	uint16_t *playback_src=(uint16_t *)(playback_buf_btpcm+(pingpang)*playback_size_btpcm/2);
	for( i =0; i<playback_size_btpcm_copy; i++)
	{
		playback_buf_btpcm_copy[i]=(uint8_t)(playback_src[i]>>8);
	}
	int_unlock(lock);
   }
#endif

    
    //TRACE(1,"pcm length:%d",len);

#ifdef SPEECH_SIDETONE
#ifdef HW_SIDETONE_IIR_PROCESS
    if(sidetone_opened==0)
    {
        sidetone_set_gain_f32(1.0f,1.0f);
        sidetone_opened=1;
    }
#endif
#endif

    //processing clock
    bt_sco_codec_tuning();


    //processing mic
    POSSIBLY_UNUSED uint8_t *capture_pcm_frame_p = capture_buf_codecpcm + pingpang * capture_size_codecpcm / 2;
    POSSIBLY_UNUSED uint8_t *ref_pcm_frame_p = playback_buf_codecpcm + (pingpang^1) * playback_size_codecpcm / 2;

    POSSIBLY_UNUSED uint8_t *dst = playback_buf_btpcm_cache;
    POSSIBLY_UNUSED uint32_t packet_len = playback_size_btpcm/2;

    if (mic_force_mute||btapp_hfp_mic_need_skip_frame())
    {
        memset(capture_buf_codecpcm, 0, capture_size_codecpcm);
    }
#if defined(HFP_1_6_ENABLE)
    if (bt_sco_codec_is_cvsd()) {
        if (!bt_sco_chain_bypass_flag) {
            process_uplink_bt_voice_frames(capture_pcm_frame_p, len, ref_pcm_frame_p, playback_size_codecpcm / 2, dst, packet_len);
        }
    } else {
        if (!bt_sco_chain_bypass_flag) {
            process_uplink_bt_voice_frames(capture_pcm_frame_p, len, ref_pcm_frame_p, playback_size_codecpcm / 2, dst, packet_len/2);
        }
        adjust_btpcm_msbc_tx(dst, packet_len/2);
    }
#endif

#if defined(BT_SCO_HANDLER_PROFILE)
    uint32_t end_ticks = hal_fast_sys_timer_get();
    TRACE(2,"[%s] takes %d us", __FUNCTION__,
        FAST_TICKS_TO_US(end_ticks - start_ticks));
#endif

    return len;

#else

#if defined(BT_SCO_HANDLER_PROFILE)
    uint32_t start_ticks = hal_fast_sys_timer_get();
#endif

#if defined(BONE_SENSOR_TDM)
    int16_t *pcm_buf = (int16_t *)buf;
    uint32_t frame_len = len / sizeof(short) / SPEECH_CODEC_CAPTURE_CHANNEL_NUM;
    uint32_t ch_num = SPEECH_CODEC_CAPTURE_CHANNEL_NUM + 1;

    ASSERT(SPEECH_CODEC_CAPTURE_CHANNEL_NUM < MIC_TDM_MAX_CH, "[%s] SPEECH_CODEC_CAPTURE_CHANNEL_NUM(%d) is invalid", __func__, SPEECH_CODEC_CAPTURE_CHANNEL_NUM);
    // TODO: Check len, depend on sample rate

    for (uint32_t ch = 0; ch < SPEECH_CODEC_CAPTURE_CHANNEL_NUM; ch++)
    {
        for (uint32_t i = 0; i < frame_len; i++)
        {
            mic_tdm_buf[ch_num * i + ch] = pcm_buf[SPEECH_CODEC_CAPTURE_CHANNEL_NUM * i + ch];
        }
    }

    return len;
#endif

#if defined(__AUDIO_RESAMPLE__) && defined(SW_SCO_RESAMPLE)
    if(hal_cmu_get_audio_resample_status())
    {
        if (af_stream_buffer_error(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE)) {
            sco_dma_buf_err = true;
        }
        // The decoded playback data in the first irq is output to DAC after the second irq (PING-PONG buffer),
        // so it is aligned with the capture data after 2 playback irqs.
        if (sco_play_irq_cnt < SCO_PLAY_RESAMPLE_ALIGN_CNT) {
            // Skip processing
            return len;
        }
        app_capture_resample_run(sco_capture_resample, buf, len);
    }
    else
#endif
    {
        voicebtpcm_pcm_audio_data_come(buf, len);
    }

#if defined(BT_SCO_HANDLER_PROFILE)
    uint32_t end_ticks = hal_fast_sys_timer_get();
    TRACE(2,"[%s] takes %d us", __FUNCTION__,
        FAST_TICKS_TO_US(end_ticks - start_ticks));
#endif

    return len;
#endif
}

#ifdef _SCO_BTPCM_CHANNEL_
// btpcm:tx
static uint32_t bt_sco_btpcm_playback_data(uint8_t *buf, uint32_t len)
{
    app_bt_stream_trigger_checker_handler(TRIGGER_CHECKER_HFP_BTPCM_PLAYERBLACK);

#if defined(SCO_DMA_SNAPSHOT)
    return len;
#else

#if defined(BT_SCO_HANDLER_PROFILE)
    uint32_t start_ticks = hal_fast_sys_timer_get();
#endif

    get_voicebtpcm_p2m_frame(buf, len);

#if defined(BT_SCO_HANDLER_PROFILE)
    uint32_t end_ticks = hal_fast_sys_timer_get();
    TRACE(2,"[%s] takes %d us", __FUNCTION__,
        FAST_TICKS_TO_US(end_ticks - start_ticks));
#endif

    return len;

#endif
}
//)

#if defined(AF_ADC_I2S_SYNC)
void codec_capture_i2s_enable(void)
{
    uint32_t lock;

    TRACE(1,"[%s] Start...", __func__);

    lock = int_lock();
    hal_codec_capture_enable();
    hal_i2s_enable();
    int_unlock(lock);

}
#endif

extern CQueue* get_rx_esco_queue_ptr();

static volatile bool is_codec_stream_started = false;
#ifdef PCM_PRIVATE_DATA_FLAG

void bt_sco_btpcm_get_pcm_priv_data(struct PCM_DATA_FLAG_T *pcm_data, uint8_t *buf, uint32_t len)
{
    uint8_t frame_num = len/120;
    for(uint8_t i=0; i<frame_num; i++)
    {
        uint8_t head_pos = 120*i;
        pcm_data[i].undef = buf[head_pos];
        pcm_data[i].bitcnt = (buf[head_pos+2]|(buf[head_pos+4]<<8))&0x3ff;
        pcm_data[i].softbit_flag = (buf[head_pos+4]>>5)&3;
        pcm_data[i].btclk = buf[head_pos+6]|(buf[head_pos+8]<<8)|(buf[head_pos+10]<<16)|(buf[head_pos+12]<<24);
        pcm_data[i].reserved = buf[head_pos+14]|(buf[head_pos+16]<<8)|(buf[head_pos+18]<<16)|(buf[head_pos+20]<<24);
        //clear private msg in buffer
        for(uint8_t j=0; j<PCM_PRIVATE_DATA_LENGTH; j++)
            buf[head_pos+2*j] = 0;
    }
}
#endif
//( btpcm:rx-->codec:spk
// btpcm:rx
extern bool a2dp_is_music_ongoing(void);
extern void bt_drv_reg_op_set_music_ongong_flag();
extern void bt_drv_reg_op_clear_music_ongong_flag();
static uint32_t bt_sco_btpcm_capture_data(uint8_t *buf, uint32_t len)
{
    app_bt_stream_trigger_checker_handler(TRIGGER_CHECKER_HFP_BTPCM_CAPTURE);

#ifdef __BT_ONE_BRING_TWO__
    if(a2dp_is_music_ongoing())
      bt_drv_reg_op_set_music_ongong_flag();
    else
      bt_drv_reg_op_clear_music_ongong_flag();
#endif
#if defined(PCM_PRIVATE_DATA_FLAG) && defined(PCM_FAST_MODE)
    bt_sco_btpcm_get_pcm_priv_data(pcm_data_param, buf, len);

#endif
#if defined(SCO_DMA_SNAPSHOT)
    uint32_t btclk;
    uint16_t btcnt;

    uint32_t mobile_master_clk;
    uint16_t mobile_master_cnt;
    
    bool  codec_stream_trig = false;

#if defined(BT_SCO_HANDLER_PROFILE)
    uint32_t start_ticks = hal_fast_sys_timer_get();
#endif

    if((is_codec_stream_started == false)&&(buf==capture_buf_btpcm))
    {
	 if(!af_stream_buffer_error(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE))
	 {
	        bt_drv_reg_op_dma_tc_clkcnt_get(&btclk, &btcnt);
	        bt_sco_mobile_clkcnt_get(btclk, btcnt,&mobile_master_clk, &mobile_master_cnt);
		 hal_sys_timer_delay_us(1);
		 if(!af_stream_buffer_error(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE))
		 {
		 	codec_stream_trig = true;
		 }
	 }
    }
/*
    uint32_t curr_ticks;

    ibbt_ctrl_t *p_ibbt_ctrl = app_tws_ibbt_get_bt_ctrl_ctx();
    if (app_tws_ibbt_tws_link_connected()){
        curr_ticks = bt_syn_get_curr_ticks(IBBT_MASTER == p_ibbt_ctrl->current_role ? p_ibbt_ctrl->mobile_conhandle : p_ibbt_ctrl->ibbt_conhandle);
         TRACE(1,"bt_sco_btpcm_capture_data +++++++++++++++++++++++++++++++++curr_ticks:%d,",curr_ticks);
    }else{
        curr_ticks = btdrv_syn_get_curr_ticks();
     TRACE(0,"--------------------------------------");
    }
*/
    sco_btpcm_mute_flag=0;

    if (codec_stream_trig) {
        if (app_bt_stream_sco_trigger_btpcm_tick()) {
            af_stream_dma_tc_irq_enable(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
            af_stream_dma_tc_irq_disable(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);

#if defined(SCO_DMA_SNAPSHOT_DEBUG)
            af_stream_dma_tc_irq_enable(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);
#endif
            TRACE(2,"buf:%p,capture_buf_btpcm:%p",buf,capture_buf_btpcm);

            // uint16_t *source=(uint16_t *)buf;
            // DUMP16("%02x,", source, MSBC_FRAME_LEN);
        
#if defined(BONE_SENSOR_TDM)
#if defined(AF_ADC_I2S_SYNC)
            hal_i2s_enable_delay(HAL_I2S_ID_0);
            // hal_codec_capture_enable_delay();
#endif
            tdm_stream_start();
#endif
            mobile_master_clk_offset_init=mobile_master_clk%BUF_BTCLK_NUM;
            last_mobile_master_clk = mobile_master_clk;
            app_bt_stream_sco_trigger_codecpcm_start(mobile_master_clk,mobile_master_cnt);
            is_codec_stream_started = true;

#if defined(SCO_DMA_SNAPSHOT_DEBUG)
            TRACE(2,"bt_sco_btpcm_capture_data:btclk:%d,btcnt:%d,",mobile_master_clk,mobile_master_cnt);
#endif
        }
    } else {
#if defined(SCO_DMA_SNAPSHOT_DEBUG)
        bt_drv_reg_op_dma_tc_clkcnt_get(&btclk, &btcnt);
        bt_sco_mobile_clkcnt_get(btclk, btcnt,&mobile_master_clk, &mobile_master_cnt);
        TRACE(2,"bt_sco_btpcm_capture_data:btclk:%d,btcnt:%d,",mobile_master_clk,mobile_master_cnt);
#endif
    }

#if defined(BT_SCO_HANDLER_PROFILE)
    uint32_t end_ticks = hal_fast_sys_timer_get();
    TRACE(2,"[%s] takes %d us", __FUNCTION__,
        FAST_TICKS_TO_US(end_ticks - start_ticks));
#endif

    return len;
    
#else
#if defined(BT_SCO_HANDLER_PROFILE)
    uint32_t start_ticks = hal_fast_sys_timer_get();
#endif

    if(!is_sco_mode()){
        TRACE(0,"sco player exit!");
        memset(buf,0x0,len);
        return len;
    }

#if defined(TX_RX_PCM_MASK)
    TRACE(0,"TX_RX_PCM_MASK");
    CQueue* Rx_esco_queue_temp = NULL;
    Rx_esco_queue_temp = get_rx_esco_queue_ptr();
    if(bt_sco_codec_is_msbc() && btdrv_is_pcm_mask_enable() ==1 )
    {
        memset(buf,0,len);
        int status = 0;
        len /= 2;
        uint8_t rx_data[len];
        status = DeCQueue(Rx_esco_queue_temp,rx_data,len);
        for(uint32_t i = 0; i<len; i++)
        {
            buf[2*i+1] = rx_data[i];
        }
        len*=2;
        if(status)
        {
            TRACE(0,"Rx Dec Fail");
         }
    }
#endif

    if (is_codec_stream_started == false) {
        if (bt_sco_codec_is_cvsd())
            hal_sys_timer_delay_us(3000);

        TRACE(2,"[%s] start codec %d", __FUNCTION__, FAST_TICKS_TO_US(hal_fast_sys_timer_get()));
#if defined(AUDIO_ANC_FB_MC) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
        af_stream_start(AUD_STREAM_ID_2, AUD_STREAM_PLAYBACK);
#endif

#if defined(BONE_SENSOR_TDM)
#if defined(AF_ADC_I2S_SYNC)
        hal_i2s_enable_delay();
        hal_codec_capture_enable_delay();
#endif
       tdm_stream_start();
#endif

        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);

#if defined(AF_ADC_I2S_SYNC)
        codec_capture_i2s_enable();
#endif
        is_codec_stream_started = true;

        return len;
    }
    store_voicebtpcm_m2p_buffer(buf, len);

#if defined(BT_SCO_HANDLER_PROFILE)
    uint32_t end_ticks = hal_fast_sys_timer_get();
    TRACE(2,"[%s] takes %d us", __FUNCTION__,
        FAST_TICKS_TO_US(end_ticks - start_ticks));
#endif

    return len;
#endif
}
#endif
static void bt_sco_codec_playback_data_post_handler(uint8_t *buf, uint32_t len, void *cfg)
{
    POSSIBLY_UNUSED struct AF_STREAM_CONFIG_T *config = (struct AF_STREAM_CONFIG_T *)cfg;

#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
    if (audio_prompt_is_playing_ongoing())
    {
        audio_prompt_processing_handler(len, buf);
    }
#else
    app_ring_merge_more_data(buf, len);
#endif
}

static uint32_t bt_sco_codec_playback_data(uint8_t *buf, uint32_t len)
{
    app_bt_stream_trigger_checker_handler(TRIGGER_CHECKER_HFP_AUDPCM_PLAYERBLACK);

    bt_set_playback_triggered(true);
#if defined(IBRT)
    app_ibrt_ui_rssi_process();
#endif
#ifdef BT_XTAL_SYNC
#ifdef BT_XTAL_SYNC_NEW_METHOD
#ifdef IBRT
    bool valid = false;
    uint32_t bitoffset = 0;
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();

    if (app_tws_ibrt_mobile_link_connected())
    {
        valid = true;
        bitoffset = btdrv_rf_bitoffset_get(p_ibrt_ctrl->mobile_conhandle -0x80);
    }
    else if (app_tws_ibrt_slave_ibrt_link_connected())
    {
        valid = true;
        bitoffset = btdrv_rf_bitoffset_get(p_ibrt_ctrl->ibrt_conhandle -0x80);
    }

    if(valid)
    {
        if(app_if_need_fix_target_rxbit() == false)
        {
            if(bitoffset<XTAL_OFFSET)
                bitoffset = XTAL_OFFSET;
            else if(bitoffset>SLOT_SIZE-XTAL_OFFSET)
                bitoffset = SLOT_SIZE-XTAL_OFFSET;
        }
#ifdef BT_XTAL_SYNC_SLOW        
        bt_xtal_sync_new_new(bitoffset,app_if_need_fix_target_rxbit(),BT_XTAL_SYNC_MODE_WITH_MOBILE);
#else        
        bt_xtal_sync_new(bitoffset,app_if_need_fix_target_rxbit(),BT_XTAL_SYNC_MODE_WITH_MOBILE);
#endif
    }
#endif
#else
    bt_xtal_sync(BT_XTAL_SYNC_MODE_VOICE);
#endif
#endif

#if defined(AF_ADC_I2S_SYNC)
    //TRACE(2,"[%s] cnt = %d", __func__, codec_playback_cnt++);
#endif

#if defined(SCO_DMA_SNAPSHOT)
    //processing  ping pang flag
    int pingpang;

    if(buf==playback_buf_codecpcm)
    {
     pingpang=0;
    }
    else
    {
     pingpang=1;
    }
#ifdef TX_RX_PCM_MASK
    //processing btpcm.(It must be from CPU's copy )
   if(btdrv_is_pcm_mask_enable()==1&&bt_sco_codec_is_msbc())
   {
    uint32_t lock;
    uint32_t i;
    //must lock the interrupts when exchanging data.
	lock = int_lock();
	uint16_t *capture_dst=(uint16_t *)(capture_buf_btpcm + pingpang * capture_size_btpcm / 2);

	for( i =0; i<capture_size_btpcm/4; i++)
	{
		capture_dst[i]=(uint16_t)capture_buf_btpcm_copy[i]<<8;
	}
	int_unlock(lock);
   }
#endif

    //processing spk
    uint8_t *playbakce_pcm_frame_p = playback_buf_codecpcm + pingpang * playback_size_codecpcm / 2;
    uint8_t *source = capture_buf_btpcm + pingpang * capture_size_btpcm / 2;
    uint32_t source_len=playback_size_btpcm / 2;

    if(sco_btpcm_mute_flag == 1 || sco_disconnect_mute_flag == 1) {
        for(uint32_t i =0; i < source_len; i++) {
            source[i]=MUTE_PATTERN;
        }
        TRACE(0,"mute....................");
    } else {
        sco_btpcm_mute_flag=1;
    }

    if (bt_sco_codec_is_cvsd() == false) {
        uint16_t *source_u16 = (uint16_t *)source;
        for (uint32_t i = 0; i < source_len/2; i++) {
            source[i] = (source_u16[i] >> 8);
        }
        source_len >>= 1;
    }
    process_downlink_bt_voice_frames(source, source_len, playbakce_pcm_frame_p, playback_size_codecpcm / sco_play_chan_num / 2);

    if (spk_force_mute)
    {
        memset(playback_buf_codecpcm, 0, playback_size_codecpcm);
    }

    if (sco_play_chan_num == AUD_CHANNEL_NUM_2) {
        // Convert mono data to stereo data
#if defined(SPEECH_RX_24BIT)
        app_bt_stream_copy_track_one_to_two_24bits((int32_t *)playbakce_pcm_frame_p, (int32_t *)playbakce_pcm_frame_p, playback_size_codecpcm / 2 / sco_play_chan_num / sizeof(int32_t));
#else
        app_bt_stream_copy_track_one_to_two_16bits((int16_t *)playbakce_pcm_frame_p, (int16_t *)playbakce_pcm_frame_p, playback_size_codecpcm / 2 / sco_play_chan_num / sizeof(int16_t));
#endif
    }
    return len;
#else
#if defined(BT_SCO_HANDLER_PROFILE)
    uint32_t start_ticks = hal_fast_sys_timer_get();
#endif

    uint8_t *dec_buf;
    uint32_t mono_len;

#if defined(SPEECH_RX_24BIT)
    len /= 2;
#endif

    mono_len = len / sco_play_chan_num;
    dec_buf = buf;

#if defined(__AUDIO_RESAMPLE__) && defined(SW_SCO_RESAMPLE)
    if(hal_cmu_get_audio_resample_status())
    {
        if (sco_play_irq_cnt < SCO_PLAY_RESAMPLE_ALIGN_CNT) {
            sco_play_irq_cnt++;
        }
        if (sco_dma_buf_err || af_stream_buffer_error(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK)) {
            sco_dma_buf_err = false;
            sco_play_irq_cnt = 0;
            app_resample_reset(sco_playback_resample);
            app_resample_reset(sco_capture_resample);
            voicebtpcm_pcm_echo_buf_queue_reset();
            TRACE(1,"%s: DMA buffer error: reset resample", __func__);
        }
        app_playback_resample_run(sco_playback_resample, dec_buf, mono_len);
    }
    else
#endif
    {
        if (sco_play_chan_num == AUD_CHANNEL_NUM_2) {
            // Convert stereo data to mono data (to save into echo_buf)
            app_bt_stream_copy_track_two_to_one_16bits((int16_t *)dec_buf, (int16_t *)buf, mono_len / 2);
        }
        voicebtpcm_pcm_audio_more_data(dec_buf, mono_len);
    }

#if defined(SPEECH_RX_24BIT)
    len <<= 1;
#endif

    if (sco_play_chan_num == AUD_CHANNEL_NUM_2) {
        // Convert mono data to stereo data
#if defined(SPEECH_RX_24BIT)
        app_bt_stream_copy_track_one_to_two_24bits((int32_t *)buf, (int32_t *)buf, len / 2 / sizeof(int32_t));
#else
        app_bt_stream_copy_track_one_to_two_16bits((int16_t *)buf, (int16_t *)buf, len / 2 / sizeof(int16_t));
#endif
    }

#if defined(BT_SCO_HANDLER_PROFILE)
    uint32_t end_ticks = hal_fast_sys_timer_get();
    TRACE(2,"[%s] takes %d us", __FUNCTION__,
        FAST_TICKS_TO_US(end_ticks - start_ticks));
#endif

    return len;
#endif
}

int bt_sco_player_forcemute(bool mic_mute, bool spk_mute)
{
    TRACE(1,"[%s]",__func__);
    mic_force_mute = mic_mute;
    spk_force_mute = spk_mute;
    return 0;
}

int bt_sco_player_get_codetype(void)
{
    if (gStreamplayer & APP_BT_STREAM_HFP_PCM)
    {
        return bt_sco_player_code_type;
    }
    else
    {
        return 0;
    }
}

#if defined(AUDIO_ANC_FB_MC_SCO) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
static uint32_t audio_mc_data_playback_sco(uint8_t *buf, uint32_t mc_len_bytes)
{
  // uint32_t begin_time;
   //uint32_t end_time;
  // begin_time = hal_sys_timer_get();
  // TRACE(1,"phone cancel: %d",begin_time);

   float left_gain;
   float right_gain;
   int32_t playback_len_bytes,mc_len_bytes_8;
   int32_t i,j,k;
   int delay_sample;

   mc_len_bytes_8=mc_len_bytes/8;

   hal_codec_get_dac_gain(&left_gain,&right_gain);

   TRACE(3,"playback_samplerate_ratio:  %d,ch:%d,sample_size:%d.",playback_samplerate_ratio_bt,playback_ch_num_bt,sample_size_play_bt);
   TRACE(1,"mc_len_bytes:  %d",mc_len_bytes);

  // TRACE(1,"left_gain:  %d",(int)(left_gain*(1<<12)));
  // TRACE(1,"right_gain: %d",(int)(right_gain*(1<<12)));

   playback_len_bytes=mc_len_bytes/playback_samplerate_ratio_bt;

    if (sample_size_play_bt == AUD_BITS_16)
    {
        int16_t *sour_p=(int16_t *)(playback_buf_bt+playback_size_bt/2);
        int16_t *mid_p=(int16_t *)(buf);
        int16_t *mid_p_8=(int16_t *)(buf+mc_len_bytes-mc_len_bytes_8);
        int16_t *dest_p=(int16_t *)buf;

        if(buf == playback_buf_mc)
        {
            sour_p=(int16_t *)playback_buf_bt;
        }

        if(playback_ch_num_bt==AUD_CHANNEL_NUM_2)
        {
            delay_sample=DELAY_SAMPLE_MC;

            for(i=0,j=0;i<delay_sample;i=i+2)
            {
                mid_p[j++]=delay_buf_bt[i];
                mid_p[j++]=delay_buf_bt[i+1];
            }

            for(i=0;i<playback_len_bytes/2-delay_sample;i=i+2)
            {
                mid_p[j++]=sour_p[i];
                mid_p[j++]=sour_p[i+1];
            }

            for(j=0;i<playback_len_bytes/2;i=i+2)
            {
                delay_buf_bt[j++]=sour_p[i];
                delay_buf_bt[j++]=sour_p[i+1];
            }

            if(playback_samplerate_ratio_bt<=8)
            {
                for(i=0,j=0;i<playback_len_bytes/2;i=i+2*(8/playback_samplerate_ratio_bt))
                {
                    mid_p_8[j++]=mid_p[i];
                    mid_p_8[j++]=mid_p[i+1];
                }
            }
            else
            {
                for(i=0,j=0;i<playback_len_bytes/2;i=i+2)
                {
                    for(k=0;k<playback_samplerate_ratio_bt/8;k++)
                    {
                        mid_p_8[j++]=mid_p[i];
                        mid_p_8[j++]=mid_p[i+1];
                    }
                }
            }

            anc_mc_run_stereo((uint8_t *)mid_p_8,mc_len_bytes_8,left_gain,right_gain,AUD_BITS_16);

            for(i=0,j=0;i<(mc_len_bytes_8)/2;i=i+2)
            {
                for(k=0;k<8;k++)
                {
                    dest_p[j++]=mid_p_8[i];
                    dest_p[j++]=mid_p_8[i+1];
                }
            }

        }
        else if(playback_ch_num_bt==AUD_CHANNEL_NUM_1)
        {
            delay_sample=DELAY_SAMPLE_MC/2;

            for(i=0,j=0;i<delay_sample;i=i+1)
            {
                mid_p[j++]=delay_buf_bt[i];
            }

            for(i=0;i<playback_len_bytes/2-delay_sample;i=i+1)
            {
                mid_p[j++]=sour_p[i];
            }

            for(j=0;i<playback_len_bytes/2;i=i+1)
            {
                delay_buf_bt[j++]=sour_p[i];
            }

            if(playback_samplerate_ratio_bt<=8)
            {
                for(i=0,j=0;i<playback_len_bytes/2;i=i+1*(8/playback_samplerate_ratio_bt))
                {
                    mid_p_8[j++]=mid_p[i];
                }
            }
            else
            {
                for(i=0,j=0;i<playback_len_bytes/2;i=i+1)
                {
                    for(k=0;k<playback_samplerate_ratio_bt/8;k++)
                    {
                        mid_p_8[j++]=mid_p[i];
                    }
                }
            }

            anc_mc_run_mono((uint8_t *)mid_p_8,mc_len_bytes_8,left_gain,AUD_BITS_16);

            for(i=0,j=0;i<(mc_len_bytes_8)/2;i=i+1)
            {
                for(k=0;k<8;k++)
                {
                    dest_p[j++]=mid_p_8[i];
                }
            }
        }

    }
    else if (sample_size_play_bt == AUD_BITS_24)
    {
        int32_t *sour_p=(int32_t *)(playback_buf_bt+playback_size_bt/2);
        int32_t *mid_p=(int32_t *)(buf);
        int32_t *mid_p_8=(int32_t *)(buf+mc_len_bytes-mc_len_bytes_8);
        int32_t *dest_p=(int32_t *)buf;

        if(buf == playback_buf_mc)
        {
            sour_p=(int32_t *)playback_buf_bt;
        }

        if(playback_ch_num_bt==AUD_CHANNEL_NUM_2)
        {
            delay_sample=DELAY_SAMPLE_MC;

            for(i=0,j=0;i<delay_sample;i=i+2)
            {
                mid_p[j++]=delay_buf_bt[i];
                mid_p[j++]=delay_buf_bt[i+1];
            }

            for(i=0;i<playback_len_bytes/4-delay_sample;i=i+2)
            {
                mid_p[j++]=sour_p[i];
                mid_p[j++]=sour_p[i+1];
            }

            for(j=0;i<playback_len_bytes/4;i=i+2)
            {
                delay_buf_bt[j++]=sour_p[i];
                delay_buf_bt[j++]=sour_p[i+1];
            }

            if(playback_samplerate_ratio_bt<=8)
            {
                for(i=0,j=0;i<playback_len_bytes/4;i=i+2*(8/playback_samplerate_ratio_bt))
                {
                    mid_p_8[j++]=mid_p[i];
                    mid_p_8[j++]=mid_p[i+1];
                }
            }
            else
            {
                for(i=0,j=0;i<playback_len_bytes/4;i=i+2)
                {
                    for(k=0;k<playback_samplerate_ratio_bt/8;k++)
                    {
                        mid_p_8[j++]=mid_p[i];
                        mid_p_8[j++]=mid_p[i+1];
                    }
                }
            }

            anc_mc_run_stereo((uint8_t *)mid_p_8,mc_len_bytes_8,left_gain,right_gain,AUD_BITS_24);

            for(i=0,j=0;i<(mc_len_bytes_8)/4;i=i+2)
            {
                for(k=0;k<8;k++)
                {
                    dest_p[j++]=mid_p_8[i];
                    dest_p[j++]=mid_p_8[i+1];
                }
            }

        }
        else if(playback_ch_num_bt==AUD_CHANNEL_NUM_1)
        {
            delay_sample=DELAY_SAMPLE_MC/2;

            for(i=0,j=0;i<delay_sample;i=i+1)
            {
                mid_p[j++]=delay_buf_bt[i];
            }

            for(i=0;i<playback_len_bytes/4-delay_sample;i=i+1)
            {
                mid_p[j++]=sour_p[i];
            }

            for(j=0;i<playback_len_bytes/4;i=i+1)
            {
                delay_buf_bt[j++]=sour_p[i];
            }

            if(playback_samplerate_ratio_bt<=8)
            {
                for(i=0,j=0;i<playback_len_bytes/4;i=i+1*(8/playback_samplerate_ratio_bt))
                {
                    mid_p_8[j++]=mid_p[i];
                }
            }
            else
            {
                for(i=0,j=0;i<playback_len_bytes/4;i=i+1)
                {
                    for(k=0;k<playback_samplerate_ratio_bt/8;k++)
                    {
                        mid_p_8[j++]=mid_p[i];
                    }
                }
            }

            anc_mc_run_mono((uint8_t *)mid_p_8,mc_len_bytes_8,left_gain,AUD_BITS_24);

            for(i=0,j=0;i<(mc_len_bytes_8)/4;i=i+1)
            {
                for(k=0;k<8;k++)
                {
                    dest_p[j++]=mid_p_8[i];
                }
            }
        }

    }

  //  end_time = hal_sys_timer_get();

 //   TRACE(2,"%s:run time: %d", __FUNCTION__, end_time-begin_time);

    return 0;
}
#endif

#if defined(LOW_DELAY_SCO)
int speech_get_frame_size(int fs, int ch, int ms)
{
    return (fs / 1000 * ch * ms)/2;
}
#else
int speech_get_frame_size(int fs, int ch, int ms)
{
    return (fs / 1000 * ch * ms);
}
#endif


int speech_get_af_buffer_size(int fs, int ch, int ms)
{
    return speech_get_frame_size(fs, ch, ms) * 2 * 2;
}

enum AUD_SAMPRATE_T speech_sco_get_sample_rate(void)
{
    enum AUD_SAMPRATE_T sample_rate;

#if defined(HFP_1_6_ENABLE)
    if (bt_sco_codec_is_msbc())
    {
        sample_rate = AUD_SAMPRATE_16000;
    }
    else
#endif
    {
        sample_rate = AUD_SAMPRATE_8000;
    }

    return sample_rate;
}

enum AUD_SAMPRATE_T speech_vqe_get_sample_rate(void)
{
    enum AUD_SAMPRATE_T sample_rate;

#if defined(SPEECH_VQE_FIXED_SAMPLE_RATE)
    sample_rate = (enum AUD_SAMPRATE_T)SPEECH_VQE_FIXED_SAMPLE_RATE;
#else
#if defined(HFP_1_6_ENABLE)
    if (bt_sco_codec_is_msbc())
    {
        sample_rate = AUD_SAMPRATE_16000;
    }
    else
#endif
    {
        sample_rate = AUD_SAMPRATE_8000;
    }
#endif

    return sample_rate;
}

enum AUD_SAMPRATE_T speech_codec_get_sample_rate(void)
{
    enum AUD_SAMPRATE_T sample_rate;

#if defined(SPEECH_CODEC_FIXED_SAMPLE_RATE)
    sample_rate = (enum AUD_SAMPRATE_T)SPEECH_CODEC_FIXED_SAMPLE_RATE;
#else
#if defined(HFP_1_6_ENABLE)
    if (bt_sco_codec_is_msbc())
    {
        sample_rate = AUD_SAMPRATE_16000;
    }
    else
#endif
    {
        sample_rate = AUD_SAMPRATE_8000;
    }
#endif

    return sample_rate;
}

int app_bt_stream_volumeset(int8_t vol);

enum AUD_SAMPRATE_T sco_sample_rate;

#if defined(AF_ADC_I2S_SYNC)
void bt_sco_bt_trigger_callback(void)
{
    TRACE(1,"[%s] Start...", __func__);

    hal_i2s_enable();
}
#endif

extern void bt_drv_pcm_set(uint8_t en);
extern uint8_t bt_drv_pcm_get();
int bt_sco_player(bool on, enum APP_SYSFREQ_FREQ_T freq)
{
    struct AF_STREAM_CONFIG_T stream_cfg;
    static bool isRun =  false;
    uint8_t * bt_audio_buff = NULL;
    enum AUD_SAMPRATE_T sample_rate;

    TRACE(3,"bt_sco_player work:%d op:%d freq:%d", isRun, on, freq);

#ifdef CHIP_BEST2000
    btdrv_enable_one_packet_more_head(0);
#endif

    if (isRun==on)
        return 0;

    if (on)
    {
        bt_sco_codec_store();
#if defined(ANC_APP) && defined(ADAPTIVE_ANC)
#ifndef __BT_ANC_KEY__
        app_anc_adaptive_detect_close();
#endif
#endif
#if defined(IBRT)
        app_ibrt_ui_rssi_reset();
#endif
        app_ibrt_if_exec_sleep_hook_blocker_set(APP_IBRT_IF_SLEEP_HOOK_BLOCKER_HFP_SCO);
    #ifdef __IAG_BLE_INCLUDE__
        app_ble_force_switch_adv(BLE_SWITCH_USER_SCO, false);
    #endif
#ifdef TX_RX_PCM_MASK
    if (btdrv_is_pcm_mask_enable() ==1 && bt_sco_codec_is_msbc())
    {
        bt_drv_reg_op_pcm_set(1);
        TRACE(0,"PCM MASK");
    }
#endif

#if defined(PCM_FAST_MODE)
        btdrv_open_pcm_fast_mode_enable();
#ifdef PCM_PRIVATE_DATA_FLAG
        bt_drv_reg_op_set_pcm_flag();
#endif
#endif

#ifdef __THIRDPARTY
        app_thirdparty_specific_lib_event_handle(THIRDPARTY_ID_NO1,THIRDPARTY_STOP);
        app_thirdparty_specific_lib_event_handle(THIRDPARTY_ID_NO2,THIRDPARTY_MIC_OPEN);
        app_thirdparty_specific_lib_event_handle(THIRDPARTY_ID_NO3,THIRDPARTY_STOP);
#endif
        //bt_syncerr set to max(0x0a)
//        BTDIGITAL_REG_SET_FIELD(REG_BTCORE_BASE_ADDR, 0x0f, 0, 0x0f);
//        af_set_priority(osPriorityRealtime);
        af_set_priority(osPriorityHigh);
        bt_media_volume_ptr_update_by_mediatype(BT_STREAM_VOICE);
        stream_local_volume = btdevice_volume_p->hfp_vol;
        app_audio_manager_sco_status_checker();

#if defined(HFP_1_6_ENABLE)
        bt_sco_player_code_type = app_audio_manager_get_scocodecid();
#endif

        if (freq < APP_SYSFREQ_104M) {
            freq = APP_SYSFREQ_104M;
        }

#if defined(AUDIO_ANC_FB_MC_SCO) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
        if (freq < APP_SYSFREQ_208M) {
            freq = APP_SYSFREQ_208M;
        }
#endif

#if defined(SCO_CP_ACCEL)
        freq = APP_SYSFREQ_104M;
#endif
freq = APP_SYSFREQ_208M;//YWW
        app_sysfreq_req(APP_SYSFREQ_USER_BT_SCO, freq);
        TRACE(1,"bt_sco_player: app_sysfreq_req %d", freq);
        TRACE(1,"sys freq calc : %d\n", hal_sys_timer_calc_cpu_freq(5, 0));

#ifndef FPGA
        app_overlay_select(APP_OVERLAY_HFP);
#ifdef BT_XTAL_SYNC
        bt_init_xtal_sync(BT_XTAL_SYNC_MODE_VOICE, BT_INIT_XTAL_SYNC_MIN, BT_INIT_XTAL_SYNC_MAX, BT_INIT_XTAL_SYNC_FCAP_RANGE);
#endif
#endif
        btdrv_rf_bit_offset_track_enable(true);

#if !defined(SCO_DMA_SNAPSHOT)      
        int aec_frame_len = speech_get_frame_size(speech_codec_get_sample_rate(), 1, SPEECH_SCO_FRAME_MS);
        speech_tx_aec_set_frame_len(aec_frame_len);
#endif

        bt_sco_player_forcemute(false, false);

        bt_sco_mode = 1;

#if defined(ANC_ASSIST_ENABLED)
        app_anc_assist_set_playback_info(speech_codec_get_sample_rate());
        app_anc_assist_set_mode(ANC_ASSIST_MODE_PHONE_CALL);
#endif
        app_audio_mempool_init();

#ifndef _SCO_BTPCM_CHANNEL_
        memset(&hf_sendbuff_ctrl, 0, sizeof(hf_sendbuff_ctrl));
#endif

        sample_rate = speech_codec_get_sample_rate();

        sco_cap_chan_num = (enum AUD_CHANNEL_NUM_T)SPEECH_CODEC_CAPTURE_CHANNEL_NUM;

#if defined(SPEECH_TX_AEC_CODEC_REF)
        sco_cap_chan_num = (enum AUD_CHANNEL_NUM_T)(sco_cap_chan_num + 1);
#endif

#if defined(ANC_NOISE_TRACKER)
        sco_cap_chan_num = (enum AUD_CHANNEL_NUM_T)(sco_cap_chan_num + ANC_NOISE_TRACKER_CHANNEL_NUM);
#endif

        memset(&stream_cfg, 0, sizeof(stream_cfg));

        // codec:mic
#if defined(ANC_ASSIST_ENABLED)
        stream_cfg.channel_map  = (enum AUD_CHANNEL_MAP_T)app_anc_assist_get_mic_ch_map(AUD_INPUT_PATH_MAINMIC);
        sco_cap_chan_num        = (enum AUD_CHANNEL_NUM_T)app_anc_assist_get_mic_ch_num(AUD_INPUT_PATH_MAINMIC);
#if defined(ASSIST_LOW_RAM_MOD) && !defined(ANC_ASSIST_UNUSED_ON_PHONE_CALL)
        anc_assist_resample_init(sample_rate, 120 , pool_allocator(), sco_cap_chan_num);
        app_audio_mempool_get_buff((uint8_t **)&anc_assist_resample_buf, speech_get_af_buffer_size(sample_rate / 2, sco_cap_chan_num, SPEECH_SCO_FRAME_MS));
        memset(anc_assist_resample_buf, 0, speech_get_af_buffer_size(sample_rate / 2, sco_cap_chan_num, SPEECH_SCO_FRAME_MS));
#endif
#endif
        stream_cfg.io_path = AUD_INPUT_PATH_MAINMIC;
        stream_cfg.channel_num = sco_cap_chan_num;
        stream_cfg.data_size = speech_get_af_buffer_size(sample_rate, sco_cap_chan_num, SPEECH_SCO_FRAME_MS);

#if defined(__AUDIO_RESAMPLE__) && defined(NO_SCO_RESAMPLE)
        // When __AUDIO_RESAMPLE__ is defined,
        // resample is off by default on best1000, and on by default on other platforms
#ifndef CHIP_BEST1000
        hal_cmu_audio_resample_disable();
#endif
#endif

#if defined(__AUDIO_RESAMPLE__) && defined(SW_SCO_RESAMPLE)
        if (sample_rate == AUD_SAMPRATE_8000)
        {
            stream_cfg.sample_rate = AUD_SAMPRATE_8463;
        }
        else if (sample_rate == AUD_SAMPRATE_16000)
        {
            stream_cfg.sample_rate = AUD_SAMPRATE_16927;
        }
#ifdef RESAMPLE_ANY_SAMPLE_RATE
        sco_capture_resample = app_capture_resample_any_open( stream_cfg.channel_num,
                            bt_sco_capture_resample_iter, stream_cfg.data_size / 2,
                            (float)CODEC_FREQ_26M / CODEC_FREQ_24P576M);
#else
        sco_capture_resample = app_capture_resample_open(sample_rate, stream_cfg.channel_num,
                            bt_sco_capture_resample_iter, stream_cfg.data_size / 2);
#endif
        uint32_t mono_cap_samp_cnt = stream_cfg.data_size / 2 / 2 / stream_cfg.channel_num;
        uint32_t cap_irq_cnt_per_frm = ((mono_cap_samp_cnt * stream_cfg.sample_rate + (sample_rate - 1)) / sample_rate +
            (aec_frame_len - 1)) / aec_frame_len;
        if (cap_irq_cnt_per_frm == 0) {
            cap_irq_cnt_per_frm = 1;
        }
#else
        stream_cfg.sample_rate = sample_rate;
#endif

#if defined(SPEECH_TX_24BIT)
        stream_cfg.bits = AUD_BITS_24;
        stream_cfg.data_size *= 2;
#else
        stream_cfg.bits = AUD_BITS_16;
#endif
        stream_cfg.vol = stream_local_volume;

#ifdef FPGA
        stream_cfg.device = AUD_STREAM_USE_EXT_CODEC;
#else
        stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
#endif
        stream_cfg.handler = bt_sco_codec_capture_data;
        app_audio_mempool_get_buff(&bt_audio_buff, stream_cfg.data_size);
        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(bt_audio_buff);

#if defined(SCO_DMA_SNAPSHOT)
    capture_buf_codecpcm=stream_cfg.data_ptr;
    capture_size_codecpcm=stream_cfg.data_size;
#endif
        TRACE(2,"capture sample_rate:%d, data_size:%d",stream_cfg.sample_rate,stream_cfg.data_size);

        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE, &stream_cfg);

#if defined(HW_DC_FILTER_WITH_IIR)
        hw_filter_codec_iir_st = hw_filter_codec_iir_create(stream_cfg.sample_rate, stream_cfg.channel_num, stream_cfg.bits, &adc_iir_cfg);
#endif

#if defined(CHIP_BEST2300)
        btdrv_set_bt_pcm_triggler_delay(60);
#elif defined(CHIP_BEST1400) || defined(CHIP_BEST1402)

#if defined(SCO_DMA_SNAPSHOT)
        btdrv_set_bt_pcm_triggler_delay(2);
#else
        btdrv_set_bt_pcm_triggler_delay(60);
#endif

#elif defined(CHIP_BEST2300P) || defined(CHIP_BEST2300A)

#if defined(SCO_DMA_SNAPSHOT)
        btdrv_set_bt_pcm_triggler_delay(2);
#else
        btdrv_set_bt_pcm_triggler_delay(59);
#endif

#else
        btdrv_set_bt_pcm_triggler_delay(55);
#endif
        // codec:spk
        stream_cfg.channel_map = (enum AUD_CHANNEL_MAP_T)0;
        sample_rate = speech_sco_get_sample_rate();
#if defined(CHIP_BEST1000)
        sco_play_chan_num = AUD_CHANNEL_NUM_2;
#else
     sco_play_chan_num = AUD_CHANNEL_NUM_1;
#endif

#if defined(BONE_SENSOR_TDM)
        codec_capture_cnt = 0;
        codec_playback_cnt = 0;
        lis25ba_init();
        tdm_stream_register(tdm_callback_func);
#endif

        stream_cfg.channel_num = sco_play_chan_num;
        // stream_cfg.data_size = BT_AUDIO_SCO_BUFF_SIZE * stream_cfg.channel_num;
        stream_cfg.data_size = speech_get_af_buffer_size(sample_rate, sco_play_chan_num, SPEECH_SCO_FRAME_MS);
#if defined(__AUDIO_RESAMPLE__) && defined(SW_SCO_RESAMPLE)
        if (sample_rate == AUD_SAMPRATE_8000)
        {
            stream_cfg.sample_rate = AUD_SAMPRATE_8463;
        }
        else if (sample_rate == AUD_SAMPRATE_16000)
        {
            stream_cfg.sample_rate = AUD_SAMPRATE_16927;
        }
#ifdef RESAMPLE_ANY_SAMPLE_RATE
        sco_playback_resample = app_playback_resample_any_open( AUD_CHANNEL_NUM_1,
                            bt_sco_playback_resample_iter, stream_cfg.data_size / stream_cfg.channel_num / 2,
                            (float)CODEC_FREQ_24P576M / CODEC_FREQ_26M);
#else
        sco_playback_resample = app_playback_resample_open(sample_rate, AUD_CHANNEL_NUM_1,
                            bt_sco_playback_resample_iter, stream_cfg.data_size / stream_cfg.channel_num / 2);
#endif
        sco_play_irq_cnt = 0;
        sco_dma_buf_err = false;

        uint32_t mono_play_samp_cnt = stream_cfg.data_size / 2 / 2 / stream_cfg.channel_num;
        uint32_t play_irq_cnt_per_frm = ((mono_play_samp_cnt * stream_cfg.sample_rate + (sample_rate - 1)) / sample_rate +
            (aec_frame_len - 1)) / aec_frame_len;
        if (play_irq_cnt_per_frm == 0) {
            play_irq_cnt_per_frm = 1;
        }
        uint32_t play_samp_cnt_per_frm = mono_play_samp_cnt * play_irq_cnt_per_frm;
        uint32_t cap_samp_cnt_per_frm = mono_cap_samp_cnt * cap_irq_cnt_per_frm;
        uint32_t max_samp_cnt_per_frm = (play_samp_cnt_per_frm >= cap_samp_cnt_per_frm) ? play_samp_cnt_per_frm : cap_samp_cnt_per_frm;
        uint32_t echo_q_samp_cnt = (((max_samp_cnt_per_frm + mono_play_samp_cnt * SCO_PLAY_RESAMPLE_ALIGN_CNT) *
            // convert to 8K/16K sample cnt
             sample_rate +(stream_cfg.sample_rate - 1)) / stream_cfg.sample_rate +
            // aligned with aec_frame_len
            (aec_frame_len - 1)) / aec_frame_len * aec_frame_len;
        if (echo_q_samp_cnt == 0) {
            echo_q_samp_cnt = aec_frame_len;
        }
        voicebtpcm_pcm_echo_buf_queue_init(echo_q_samp_cnt * 2);
#else
        stream_cfg.sample_rate = sample_rate;
#endif

        stream_cfg.bits = AUD_BITS_16;
        stream_cfg.io_path = AUD_OUTPUT_PATH_SPEAKER;
        stream_cfg.handler = bt_sco_codec_playback_data;

#if defined(SPEECH_RX_24BIT)
        stream_cfg.bits = AUD_BITS_24;
        stream_cfg.data_size *= 2;
#endif

#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
        uint8_t* promptTmpSourcePcmDataBuf;
        uint8_t* promptTmpTargetPcmDataBuf;
        uint8_t* promptPcmDataBuf;

        sco_sample_rate = stream_cfg.sample_rate;
        app_audio_mempool_get_buff(&promptTmpSourcePcmDataBuf, AUDIO_PROMPT_SOURCE_PCM_BUFFER_SIZE);
        app_audio_mempool_get_buff(&promptTmpTargetPcmDataBuf, AUDIO_PROMPT_TARGET_PCM_BUFFER_SIZE);
        app_audio_mempool_get_buff(&promptPcmDataBuf, AUDIO_PROMPT_PCM_BUFFER_SIZE);
        audio_prompt_buffer_config(MIX_WITH_SCO_STREAMING, stream_cfg.channel_num,
            stream_cfg.bits, promptTmpSourcePcmDataBuf, promptTmpTargetPcmDataBuf,
            promptPcmDataBuf, AUDIO_PROMPT_PCM_BUFFER_SIZE);
#endif

        app_audio_mempool_get_buff(&bt_audio_buff, stream_cfg.data_size);
        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(bt_audio_buff);

        TRACE(2,"playback sample_rate:%d, data_size:%d",stream_cfg.sample_rate,stream_cfg.data_size);

#if defined(AUDIO_ANC_FB_MC_SCO) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
        sample_size_play_bt=stream_cfg.bits;
        sample_rate_play_bt=stream_cfg.sample_rate;
        data_size_play_bt=stream_cfg.data_size;
        playback_buf_bt=stream_cfg.data_ptr;
        playback_size_bt=stream_cfg.data_size;

        if (sample_rate_play_bt == AUD_SAMPRATE_8000)
        {
            playback_samplerate_ratio_bt=8*3*2;
        }
        else if (sample_rate_play_bt == AUD_SAMPRATE_16000)
        {
            playback_samplerate_ratio_bt=8*3;
        }

        playback_ch_num_bt=stream_cfg.channel_num;
#endif

#if defined(SCO_DMA_SNAPSHOT)
        playback_buf_codecpcm=stream_cfg.data_ptr;
        playback_size_codecpcm=stream_cfg.data_size;
        playback_samplerate_codecpcm=stream_cfg.sample_rate;
#endif
        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg);

#if defined(SCO_DMA_SNAPSHOT)
        af_stream_dma_tc_irq_enable(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
#endif
        af_codec_set_playback_post_handler(bt_sco_codec_playback_data_post_handler);

#if defined(AUDIO_ANC_FB_MC_SCO) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
        stream_cfg.bits = sample_size_play_bt;
        stream_cfg.channel_num = playback_ch_num_bt;
        stream_cfg.sample_rate = sample_rate_play_bt;
        stream_cfg.device = AUD_STREAM_USE_MC;
        stream_cfg.vol = 0;
        stream_cfg.handler = audio_mc_data_playback_sco;
        stream_cfg.io_path = AUD_OUTPUT_PATH_SPEAKER;

        app_audio_mempool_get_buff(&bt_audio_buff, data_size_play_bt*playback_samplerate_ratio_bt);
        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(bt_audio_buff);
        stream_cfg.data_size = data_size_play_bt*playback_samplerate_ratio_bt;

        playback_buf_mc=stream_cfg.data_ptr;
        playback_size_mc=stream_cfg.data_size;

        anc_mc_run_init(hal_codec_anc_convert_rate(sample_rate_play_bt));

        memset(delay_buf_bt,0,sizeof(delay_buf_bt));

        af_stream_open(AUD_STREAM_ID_2, AUD_STREAM_PLAYBACK, &stream_cfg);
#endif

#if defined(SCO_DMA_SNAPSHOT)
        btdrv_disable_playback_triggler();

        af_codec_sync_config(AUD_STREAM_PLAYBACK, AF_CODEC_SYNC_TYPE_BT, true);
        af_codec_sync_config(AUD_STREAM_CAPTURE, AF_CODEC_SYNC_TYPE_BT, true);

        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
#endif

#if defined(BONE_SENSOR_TDM)
        tdm_stream_open();

//#if defined(AF_ADC_I2S_SYNC)
//        af_codec_bt_trigger_config(true, bt_sco_bt_trigger_callback);
//#endif
#endif

#if defined(ANC_NOISE_TRACKER)
        app_audio_mempool_get_buff(
            (uint8_t **)&anc_buf,
            speech_get_frame_size(
                speech_codec_get_sample_rate(),
                ANC_NOISE_TRACKER_CHANNEL_NUM,
                SPEECH_SCO_FRAME_MS) * sizeof(int16_t));
        noise_tracker_init(nt_demo_words_cb, ANC_NOISE_TRACKER_CHANNEL_NUM, -20);
#endif

        // Must call this function before af_stream_start
        // Get all free app audio buffer except SCO_BTPCM used(2k)
#if defined(SCO_DMA_SNAPSHOT)
        voicebtpcm_pcm_audio_init(speech_sco_get_sample_rate(), speech_vqe_get_sample_rate(), speech_sco_get_sample_rate(), speech_codec_get_sample_rate(), speech_sco_get_sample_rate(), sco_cap_chan_num);
#else
        voicebtpcm_pcm_audio_init(speech_sco_get_sample_rate(), speech_codec_get_sample_rate());
#endif

        /*
        TRACE(2,"[%s] start codec %d", __FUNCTION__, FAST_TICKS_TO_US(hal_fast_sys_timer_get()));
#if defined(AUDIO_ANC_FB_MC) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
        af_stream_start(AUD_STREAM_ID_2, AUD_STREAM_PLAYBACK);
#endif
        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        */

#ifdef SPEECH_SIDETONE
#ifdef HW_SIDETONE_IIR_PROCESS
        {
            HW_CODEC_IIR_CFG_T *hw_iir_cfg_dac=NULL;
            sidetone_open();
            hal_codec_sidetone_enable();
            hw_iir_cfg_dac = hw_codec_iir_get_cfg(speech_sco_get_sample_rate(), audio_sidetone_iir_cfg_list[0]);
            sidetone_set_cfg(hw_iir_cfg_dac);
        }
#else
        hal_codec_sidetone_enable();
#endif
#endif


#ifdef _SCO_BTPCM_CHANNEL_
        stream_cfg.bits = AUD_BITS_16;
        stream_cfg.sample_rate = speech_sco_get_sample_rate();
        stream_cfg.channel_num = AUD_CHANNEL_NUM_1;
        // stream_cfg.data_size = BT_AUDIO_SCO_BUFF_SIZE * stream_cfg.channel_num;

        if (bt_sco_codec_is_msbc())
        {
            stream_cfg.data_size = speech_get_af_buffer_size(stream_cfg.sample_rate, stream_cfg.channel_num, SPEECH_SCO_FRAME_MS)/2;
        }
        else
        {
            stream_cfg.data_size = speech_get_af_buffer_size(stream_cfg.sample_rate, stream_cfg.channel_num, SPEECH_SCO_FRAME_MS);
        }

        // btpcm:rx
        stream_cfg.device = AUD_STREAM_USE_BT_PCM;
        stream_cfg.handler = bt_sco_btpcm_capture_data;
        app_audio_mempool_get_buff(&bt_audio_buff, stream_cfg.data_size);
        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(bt_audio_buff);

        TRACE(2,"sco btpcm sample_rate:%d, data_size:%d",stream_cfg.sample_rate,stream_cfg.data_size);

#if defined(SCO_DMA_SNAPSHOT)
        sco_btpcm_mute_flag=0;
        sco_disconnect_mute_flag=0;

        capture_buf_btpcm=stream_cfg.data_ptr;
        capture_size_btpcm=stream_cfg.data_size;
#ifdef TX_RX_PCM_MASK
     capture_size_btpcm_copy=stream_cfg.data_size/4;//only need ping or pang;
        app_audio_mempool_get_buff(&capture_buf_btpcm_copy, capture_size_btpcm_copy);
#endif
#endif
        af_stream_open(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE, &stream_cfg);

        // btpcm:tx
        stream_cfg.device = AUD_STREAM_USE_BT_PCM;
        stream_cfg.handler = bt_sco_btpcm_playback_data;
        app_audio_mempool_get_buff(&bt_audio_buff, stream_cfg.data_size);
        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(bt_audio_buff);

        if (bt_sco_codec_is_msbc()) {
            memset(stream_cfg.data_ptr, MSBC_MUTE_PATTERN, stream_cfg.data_size);
        } else {
            memset(stream_cfg.data_ptr, SCO_TX_MUTE_PATTERN, stream_cfg.data_size);
        }

#if defined(SCO_DMA_SNAPSHOT)
        playback_buf_btpcm=stream_cfg.data_ptr;
        playback_size_btpcm=stream_cfg.data_size;
#ifdef TX_RX_PCM_MASK
     playback_size_btpcm_copy=stream_cfg.data_size/4; //only need ping or pang;
        app_audio_mempool_get_buff(&playback_buf_btpcm_copy, playback_size_btpcm_copy);
#endif
         //only need ping or pang;
        app_audio_mempool_get_buff(&playback_buf_btpcm_cache, stream_cfg.data_size/2);
        if (bt_sco_codec_is_msbc()) {
            memset(playback_buf_btpcm_cache, MSBC_MUTE_PATTERN, stream_cfg.data_size/2);
        } else {
            memset(playback_buf_btpcm_cache, SCO_TX_MUTE_PATTERN, stream_cfg.data_size/2);
        }
#endif

        af_stream_open(AUD_STREAM_ID_1, AUD_STREAM_PLAYBACK, &stream_cfg);

#if defined(SCO_DMA_SNAPSHOT)
        app_bt_stream_sco_trigger_btpcm_start();
        af_stream_dma_tc_irq_enable(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);
#endif

#if !(defined(__AUDIO_RESAMPLE__) && defined(SW_PLAYBACK_RESAMPLE))
        af_codec_tune(AUD_STREAM_NUM, 0);
#endif

        TRACE(2,"[%s] start btpcm %d", __FUNCTION__, FAST_TICKS_TO_US(hal_fast_sys_timer_get()));
        af_stream_start(AUD_STREAM_ID_1, AUD_STREAM_PLAYBACK);
        af_stream_start(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);

        is_codec_stream_started = false;

        btdrv_set_btpcm_mute_pattern(MSBC_MUTE_PATTERN | (MSBC_MUTE_PATTERN << 8));

#if defined(CVSD_BYPASS)
        btdrv_cvsd_bypass_enable();
#endif
#if defined(CHIP_BEST1305)
        osDelay(10);
        bt_drv_reg_op_set_btpcm_trig_flag(true);
#endif
#if defined(CHIP_BEST2300) || defined(CHIP_BEST2300P) || defined(CHIP_BEST2300A) || defined(CHIP_BEST1400) || defined(CHIP_BEST1402) || defined(CHIP_BEST2001)
#if !defined(SCO_DMA_SNAPSHOT)
        btdrv_pcm_enable();
#endif
#endif

#endif

#ifdef FPGA
        app_bt_stream_volumeset(stream_local_volume);
        //btdrv_set_bt_pcm_en(1);
#endif
        app_bt_stream_trigger_checker_start();
        TRACE(0,"bt_sco_player on");
    }
    else
    {
        app_bt_stream_trigger_checker_stop();
#if defined(SCO_DMA_SNAPSHOT)
#ifdef TX_RX_PCM_MASK
     playback_buf_btpcm_copy=NULL;
     capture_buf_btpcm_copy=NULL;
     playback_size_btpcm_copy=0;
        capture_size_btpcm_copy=0;
#endif
#endif

#ifdef __THIRDPARTY
        app_thirdparty_specific_lib_event_handle(THIRDPARTY_ID_NO2,THIRDPARTY_MIC_CLOSE);
#endif
#if defined(SCO_DMA_SNAPSHOT)
        app_bt_stream_sco_trigger_codecpcm_stop();
#endif
        af_stream_dma_tc_irq_disable(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        af_codec_set_playback_post_handler(NULL);

        af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
#if defined(AUDIO_ANC_FB_MC_SCO) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
        af_stream_stop(AUD_STREAM_ID_2, AUD_STREAM_PLAYBACK);
#endif
        is_codec_stream_started = false;

#if defined(BONE_SENSOR_TDM)
        tdm_stream_stop();
#endif

#if !(defined(__AUDIO_RESAMPLE__) && defined(SW_PLAYBACK_RESAMPLE))
        af_codec_tune(AUD_STREAM_NUM, 0);
#endif

#ifdef _SCO_BTPCM_CHANNEL_
        af_stream_stop(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);
        af_stream_stop(AUD_STREAM_ID_1, AUD_STREAM_PLAYBACK);

        af_stream_close(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);
        af_stream_close(AUD_STREAM_ID_1, AUD_STREAM_PLAYBACK);
#if defined(CHIP_BEST1305)
        bt_drv_reg_op_set_btpcm_trig_flag(false);
#endif
#endif
#ifdef TX_RX_PCM_MASK
        if (btdrv_is_pcm_mask_enable()==1 && bt_drv_reg_op_pcm_get())
        {
            bt_drv_reg_op_pcm_set(0);
            TRACE(0,"PCM UNMASK");
        }
#endif
#if defined(PCM_FAST_MODE)
        btdrv_open_pcm_fast_mode_disable();
#endif

	    bt_drv_reg_op_sco_txfifo_reset(bt_sco_codec_get_type());
#if defined(HW_DC_FILTER_WITH_IIR)
        hw_filter_codec_iir_destroy(hw_filter_codec_iir_st);
#endif

        af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
#if defined(AUDIO_ANC_FB_MC_SCO) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
        af_stream_close(AUD_STREAM_ID_2, AUD_STREAM_PLAYBACK);
#endif

#if defined(BONE_SENSOR_TDM)
        tdm_stream_close();
#endif

#ifdef SPEECH_SIDETONE
        hal_codec_sidetone_disable();
#ifdef HW_SIDETONE_IIR_PROCESS
        sidetone_close();
        sidetone_opened=0;
#endif
#endif

#if defined(__AUDIO_RESAMPLE__) && defined(SW_SCO_RESAMPLE)
        app_capture_resample_close(sco_capture_resample);
        sco_capture_resample = NULL;
        app_capture_resample_close(sco_playback_resample);
        sco_playback_resample = NULL;
#endif

#if defined(__AUDIO_RESAMPLE__) && defined(NO_SCO_RESAMPLE)
#ifndef CHIP_BEST1000
        // When __AUDIO_RESAMPLE__ is defined,
        // resample is off by default on best1000, and on by default on other platforms
        hal_cmu_audio_resample_enable();
#endif
#endif

        bt_sco_mode = 0;

        voicebtpcm_pcm_audio_deinit();

#if defined(BONE_SENSOR_TDM)
        lis25ba_deinit();
#endif

#ifndef FPGA
#ifdef BT_XTAL_SYNC
        bt_term_xtal_sync(false);
#ifndef BT_XTAL_SYNC_NO_RESET
        bt_term_xtal_sync_default();
#endif
#endif
#endif
#if defined(HFP_1_6_ENABLE)
        TRACE(1,"clear sco tx fifo codec:%d",bt_sco_player_code_type);
        bt_drv_reg_op_sco_txfifo_reset(bt_sco_player_code_type);
        bt_sco_player_code_type = BTIF_HF_SCO_CODEC_NONE;
#else
        bt_drv_reg_op_sco_txfifo_reset(1);
#endif

#if defined(ANC_ASSIST_ENABLED)
#if defined(AUDIO_ADAPTIVE_EQ)
        app_voice_assist_adaptive_eq_close();
#endif
        app_anc_assist_set_playback_info(AUD_SAMPRATE_NULL);
        app_anc_assist_set_mode(ANC_ASSIST_MODE_STANDALONE);
#endif

        TRACE(0,"bt_sco_player off");
        app_overlay_unloadall();
        app_sysfreq_req(APP_SYSFREQ_USER_BT_SCO, APP_SYSFREQ_32K);
        af_set_priority(osPriorityAboveNormal);

        //bt_syncerr set to default(0x07)
//       BTDIGITAL_REG_SET_FIELD(REG_BTCORE_BASE_ADDR, 0x0f, 0, 0x07);
#ifdef __THIRDPARTY
        //app_thirdparty_specific_lib_event_handle(THIRDPARTY_ID_NO1,THIRDPARTY_START);
        app_thirdparty_specific_lib_event_handle(THIRDPARTY_ID_NO3,THIRDPARTY_START);
#endif
#if defined(IBRT)
        app_ibrt_ui_rssi_reset();
#endif
        app_ibrt_if_exec_sleep_hook_blocker_clr(APP_IBRT_IF_SLEEP_HOOK_BLOCKER_HFP_SCO);
#if defined(ANC_APP) && defined(ADAPTIVE_ANC)
#ifndef __BT_ANC_KEY__
        app_anc_adaptive_detect_open();
#endif
#endif
    }
    app_bt_sco_chain_bypass_flag = false;
    isRun=on;
    return 0;
}

#ifdef AUDIO_LINEIN
#include "app_status_ind.h"
//player channel should <= capture channel number
//player must be 2 channel
#define LINEIN_PLAYER_CHANNEL (2)
#ifdef __AUDIO_OUTPUT_MONO_MODE__
#define LINEIN_CAPTURE_CHANNEL (1)
#else
#define LINEIN_CAPTURE_CHANNEL (2)
#endif

#if (LINEIN_CAPTURE_CHANNEL == 1)
#define LINEIN_PLAYER_BUFFER_SIZE (1024*LINEIN_PLAYER_CHANNEL)
#define LINEIN_CAPTURE_BUFFER_SIZE (LINEIN_PLAYER_BUFFER_SIZE/2)
#elif (LINEIN_CAPTURE_CHANNEL == 2)
#define LINEIN_PLAYER_BUFFER_SIZE (1024*LINEIN_PLAYER_CHANNEL)
#define LINEIN_CAPTURE_BUFFER_SIZE (LINEIN_PLAYER_BUFFER_SIZE)
#endif

static int16_t *app_linein_play_cache = NULL;

int8_t app_linein_buffer_is_empty(void)
{
    if (app_audio_pcmbuff_length()){
        return 0;
    }else{
        return 1;
    }
}

uint32_t app_linein_pcm_come(uint8_t * pcm_buf, uint32_t len)
{
    app_audio_pcmbuff_put(pcm_buf, len);

    return len;
}

uint32_t app_linein_need_pcm_data(uint8_t* pcm_buf, uint32_t len)
{
#if (LINEIN_CAPTURE_CHANNEL == 1)
    app_audio_pcmbuff_get((uint8_t *)app_linein_play_cache, len/2);
    app_play_audio_lineinmode_more_data((uint8_t *)app_linein_play_cache,len/2);
    app_bt_stream_copy_track_one_to_two_16bits((int16_t *)pcm_buf, app_linein_play_cache, len/2/2);
#elif (LINEIN_CAPTURE_CHANNEL == 2)
    app_audio_pcmbuff_get((uint8_t *)pcm_buf, len);
    app_play_audio_lineinmode_more_data((uint8_t *)pcm_buf, len);
#endif

#if defined(__AUDIO_OUTPUT_MONO_MODE__)
    merge_stereo_to_mono_16bits((int16_t *)buf, (int16_t *)pcm_buf, len/2);
#endif

#ifdef ANC_APP
    bt_audio_updata_eq_for_anc(app_anc_work_status());
#else
    bt_audio_updata_eq(app_audio_get_eq());
#endif

    audio_process_run(pcm_buf, len);

    return len;
}

int app_play_linein_onoff(bool onoff)
{
    static bool isRun =  false;
    uint8_t *linein_audio_cap_buff = 0;
    uint8_t *linein_audio_play_buff = 0;
    uint8_t *linein_audio_loop_buf = NULL;
    struct AF_STREAM_CONFIG_T stream_cfg;

    uint8_t POSSIBLY_UNUSED *bt_eq_buff = NULL;
    uint32_t POSSIBLY_UNUSED eq_buff_size;
    uint8_t POSSIBLY_UNUSED play_samp_size;

    TRACE(2,"app_play_linein_onoff work:%d op:%d", isRun, onoff);

    if (isRun == onoff)
        return 0;

     if (onoff){
        app_sysfreq_req(APP_SYSFREQ_USER_APP_0, APP_SYSFREQ_104M);
        app_overlay_select(APP_OVERLAY_A2DP);
        app_audio_mempool_init();
        app_audio_mempool_get_buff(&linein_audio_cap_buff, LINEIN_CAPTURE_BUFFER_SIZE);
        app_audio_mempool_get_buff(&linein_audio_play_buff, LINEIN_PLAYER_BUFFER_SIZE);
        app_audio_mempool_get_buff(&linein_audio_loop_buf, LINEIN_PLAYER_BUFFER_SIZE<<2);
        app_audio_pcmbuff_init(linein_audio_loop_buf, LINEIN_PLAYER_BUFFER_SIZE<<2);

#if (LINEIN_CAPTURE_CHANNEL == 1)
        app_audio_mempool_get_buff((uint8_t **)&app_linein_play_cache, LINEIN_PLAYER_BUFFER_SIZE/2/2);
        app_play_audio_lineinmode_init(LINEIN_CAPTURE_CHANNEL, LINEIN_PLAYER_BUFFER_SIZE/2/2);
#elif (LINEIN_CAPTURE_CHANNEL == 2)
        app_play_audio_lineinmode_init(LINEIN_CAPTURE_CHANNEL, LINEIN_PLAYER_BUFFER_SIZE/2);
#endif

        memset(&stream_cfg, 0, sizeof(stream_cfg));

        stream_cfg.bits = AUD_BITS_16;
        stream_cfg.channel_num = (enum AUD_CHANNEL_NUM_T)LINEIN_PLAYER_CHANNEL;
#if defined(__AUDIO_RESAMPLE__)
        stream_cfg.sample_rate = AUD_SAMPRATE_50781;
#else
        stream_cfg.sample_rate = AUD_SAMPRATE_44100;
#endif
#if FPGA==0
        stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
#else
        stream_cfg.device = AUD_STREAM_USE_EXT_CODEC;
#endif
        stream_cfg.vol = stream_linein_volume;
        TRACE(1,"vol = %d",stream_linein_volume);
        stream_cfg.io_path = AUD_OUTPUT_PATH_SPEAKER;
        stream_cfg.handler = app_linein_need_pcm_data;
        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(linein_audio_play_buff);
        stream_cfg.data_size = LINEIN_PLAYER_BUFFER_SIZE;

#if defined(AUDIO_ANC_FB_MC) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
        sample_size_play_bt=stream_cfg.bits;
        sample_rate_play_bt=stream_cfg.sample_rate;
        data_size_play_bt=stream_cfg.data_size;
        playback_buf_bt=stream_cfg.data_ptr;
        playback_size_bt=stream_cfg.data_size;
        if(sample_rate_play_bt==AUD_SAMPRATE_96000)
        {
            playback_samplerate_ratio_bt=4;
        }
        else
        {
            playback_samplerate_ratio_bt=8;
        }
        playback_ch_num_bt=stream_cfg.channel_num;
        mid_p_8_old_l=0;
        mid_p_8_old_r=0;
#endif

        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg);
        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);

        stream_cfg.io_path = AUD_INPUT_PATH_LINEIN;
        stream_cfg.channel_num = (enum AUD_CHANNEL_NUM_T)LINEIN_CAPTURE_CHANNEL;
        stream_cfg.channel_map = (enum AUD_CHANNEL_MAP_T)hal_codec_get_input_path_cfg(stream_cfg.io_path);
        stream_cfg.handler = app_linein_pcm_come;
        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(linein_audio_cap_buff);
        stream_cfg.data_size = LINEIN_CAPTURE_BUFFER_SIZE;

        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE, &stream_cfg);

#if defined(__HW_FIR_EQ_PROCESS__) && defined(__HW_IIR_EQ_PROCESS__)
        eq_buff_size = stream_cfg.data_size*2;
#elif defined(__HW_FIR_EQ_PROCESS__) && !defined(__HW_IIR_EQ_PROCESS__)

        play_samp_size = (stream_cfg.bits <= AUD_BITS_16) ? 2 : 4;
#if defined(CHIP_BEST2000)
        eq_buff_size = stream_cfg.data_size * sizeof(int32_t) / play_samp_size;
#elif  defined(CHIP_BEST1000)
        eq_buff_size = stream_cfg.data_size * sizeof(int16_t) / play_samp_size;
#elif defined(CHIP_BEST2300) || defined(CHIP_BEST2300P) || defined(CHIP_BEST2300A)
        eq_buff_size = stream_cfg.data_size;
#endif

#elif !defined(__HW_FIR_EQ_PROCESS__) && defined(__HW_IIR_EQ_PROCESS__)
        eq_buff_size = stream_cfg.data_size;
#else
        eq_buff_size = 0;
        bt_eq_buff = NULL;
#endif

        if(eq_buff_size>0)
        {
            app_audio_mempool_get_buff(&bt_eq_buff, eq_buff_size);
        }

        audio_process_init();
#if defined(IBRT)
        enum AUD_CHANNEL_NUM_T sw_ch_num = AUD_CHANNEL_NUM_1;
#else
        enum AUD_CHANNEL_NUM_T sw_ch_num = stream_cfg.channel_num;
#endif
        audio_process_open(stream_cfg.sample_rate, stream_cfg.bits,sw_ch_num, stream_cfg.channel_num, bt_eq_buff, eq_buff_size);

#ifdef __SW_IIR_EQ_PROCESS__
        bt_audio_set_eq(AUDIO_EQ_TYPE_SW_IIR,bt_audio_get_eq_index(AUDIO_EQ_TYPE_SW_IIR,0));
#endif

#ifdef __HW_FIR_EQ_PROCESS__
        bt_audio_set_eq(AUDIO_EQ_TYPE_HW_FIR,bt_audio_get_eq_index(AUDIO_EQ_TYPE_HW_FIR,0));
#endif

#ifdef __HW_DAC_IIR_EQ_PROCESS__
        bt_audio_set_eq(AUDIO_EQ_TYPE_HW_DAC_IIR,bt_audio_get_eq_index(AUDIO_EQ_TYPE_HW_DAC_IIR,0));
#endif

#ifdef __HW_IIR_EQ_PROCESS__
        bt_audio_set_eq(AUDIO_EQ_TYPE_HW_IIR,bt_audio_get_eq_index(AUDIO_EQ_TYPE_HW_IIR,0));
#endif

#ifdef ANC_APP
        anc_status_record = 0xff;
#endif

#if defined(AUDIO_ANC_FB_MC) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
        uint8_t* bt_audio_buff = NULL;
        stream_cfg.bits = sample_size_play_bt;
        stream_cfg.channel_num = playback_ch_num_bt;
        stream_cfg.sample_rate = sample_rate_play_bt;
        stream_cfg.device = AUD_STREAM_USE_MC;
        stream_cfg.vol = 0;
        stream_cfg.handler = audio_mc_data_playback_a2dp;
        stream_cfg.io_path = AUD_OUTPUT_PATH_SPEAKER;

        app_audio_mempool_get_buff(&bt_audio_buff, data_size_play_bt*playback_samplerate_ratio_bt);
        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(bt_audio_buff);
        stream_cfg.data_size = data_size_play_bt*playback_samplerate_ratio_bt;

        playback_buf_mc=stream_cfg.data_ptr;
        playback_size_mc=stream_cfg.data_size;

        anc_mc_run_init(hal_codec_anc_convert_rate(sample_rate_play_bt));

        memset(delay_buf_bt,0,sizeof(delay_buf_bt));

        af_stream_open(AUD_STREAM_ID_2, AUD_STREAM_PLAYBACK, &stream_cfg);
        //ASSERT(ret == 0, "af_stream_open playback failed: %d", ret);
#endif


        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);

#if defined(AUDIO_ANC_FB_MC) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
        af_stream_start(AUD_STREAM_ID_2, AUD_STREAM_PLAYBACK);
#endif

        memset(&stream_cfg, 0, sizeof(stream_cfg));

        stream_cfg.bits = AUD_BITS_16;
#if defined(__AUDIO_RESAMPLE__)
        stream_cfg.sample_rate = AUD_SAMPRATE_50781;
#else
        stream_cfg.sample_rate = AUD_SAMPRATE_44100;
#endif
#if FPGA==0
        stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
#else
        stream_cfg.device = AUD_STREAM_USE_EXT_CODEC;
#endif
        stream_cfg.io_path = AUD_INPUT_PATH_LINEIN;
        stream_cfg.channel_num = (enum AUD_CHANNEL_NUM_T)LINEIN_CAPTURE_CHANNEL;
        stream_cfg.channel_map = (enum AUD_CHANNEL_MAP_T)hal_codec_get_input_path_cfg(stream_cfg.io_path);
        stream_cfg.handler = app_linein_pcm_come;
        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(linein_audio_cap_buff);
        stream_cfg.data_size = LINEIN_CAPTURE_BUFFER_SIZE;

        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE, &stream_cfg);
        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
     }else     {
        af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);

        aaudio_process_close();

        af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);

        app_overlay_unloadall();
        app_sysfreq_req(APP_SYSFREQ_USER_APP_0, APP_SYSFREQ_32K);
     }

    isRun = onoff;
    TRACE(1,"%s end!\n", __func__);
    return 0;
}
#endif

int app_bt_stream_open(APP_AUDIO_STATUS* status)
{
    int nRet = -1;
    uint16_t player = status->id;
    APP_AUDIO_STATUS next_status;
    enum APP_SYSFREQ_FREQ_T freq = (enum APP_SYSFREQ_FREQ_T)status->freq;

    app_bt_stream_trigger_checker_init();

    TRACE(3,"app_bt_stream_open prev:%d%s freq:%d", gStreamplayer, player2str(gStreamplayer), freq);
    TRACE(3,"app_bt_stream_open cur:%d%s freq:%d", player, player2str(player), freq);
    APP_AUDIO_STATUS streamToClose;
    if (gStreamplayer != APP_BT_STREAM_INVALID)
    {
    #if !ISOLATED_AUDIO_STREAM_ENABLED
        TRACE(0,"Close prev bt stream before opening");
        nRet = app_bt_stream_close(gStreamplayer);
        if (nRet)
        {
            return -1;
        }
        else
        {
            streamToClose.id = gStreamplayer;
            app_audio_list_rmv_callback(&streamToClose, &next_status, APP_BT_SETTING_Q_POS_TAIL);
        }
    #else
        if (gStreamplayer & player)
        {
            TRACE(3,"%s,fail,gStreamplayer=%x,player=%x",__func__,gStreamplayer,player);
            return -1;
        }

        if (player >= APP_BT_STREAM_BORDER_INDEX)
        {
            if (APP_BT_INPUT_STREAM_INDEX(gStreamplayer) > 0)
            {
                TRACE(0,"Close prev input bt stream before opening");
                nRet = app_bt_stream_close(APP_BT_INPUT_STREAM_INDEX(gStreamplayer));
                if (nRet)
                {
                    return -1;
                }
                else
                {
                    streamToClose.id = APP_BT_INPUT_STREAM_INDEX(gStreamplayer);
                    app_audio_list_rmv_callback(&streamToClose, &next_status, APP_BT_SETTING_Q_POS_TAIL);
                }
            }
        }
        else
        {
            if (APP_BT_OUTPUT_STREAM_INDEX(gStreamplayer) > 0)
            {
                TRACE(0,"Close prev output bt stream before opening");
                nRet = app_bt_stream_close(APP_BT_OUTPUT_STREAM_INDEX(gStreamplayer));
                if (nRet)
                {
                    return -1;
                }
                else
                {
                    streamToClose.id = APP_BT_OUTPUT_STREAM_INDEX(gStreamplayer);
                    app_audio_list_rmv_callback(&streamToClose, &next_status, APP_BT_SETTING_Q_POS_TAIL);
                }
            }
        }
    #endif
    }

    switch (player)
    {
        case APP_BT_STREAM_HFP_PCM:
        case APP_BT_STREAM_HFP_CVSD:
        case APP_BT_STREAM_HFP_VENDOR:
            nRet = bt_sco_player(true, freq);
            break;
        case APP_BT_STREAM_A2DP_SBC:
        case APP_BT_STREAM_A2DP_AAC:
        case APP_BT_STREAM_A2DP_VENDOR:
            nRet = bt_sbc_player(PLAYER_OPER_START, freq);
            break;
#ifdef __FACTORY_MODE_SUPPORT__
        case APP_FACTORYMODE_AUDIO_LOOP:
            nRet = app_factorymode_audioloop(true, freq);
            break;
#endif
#ifdef MEDIA_PLAYER_SUPPORT
        case APP_PLAY_BACK_AUDIO:
            nRet = app_play_audio_onoff(true, status);
            break;
#endif

#ifdef RB_CODEC
        case APP_BT_STREAM_RBCODEC:
            nRet = app_rbplay_audio_onoff(true, 0);
            break;
#endif

#ifdef VOICE_DATAPATH
        case APP_BT_STREAM_VOICEPATH:
            nRet = app_voicepath_start_audio_stream();
            break;
#endif

#ifdef AUDIO_LINEIN
        case APP_PLAY_LINEIN_AUDIO:
            nRet = app_play_linein_onoff(true);
            break;
#endif

#if defined(APP_LINEIN_A2DP_SOURCE)
        case APP_A2DP_SOURCE_LINEIN_AUDIO:
            nRet = app_a2dp_source_linein_on(true);
            break;
#endif
#if defined(APP_I2S_A2DP_SOURCE)
        case APP_A2DP_SOURCE_I2S_AUDIO:
            nRet = app_a2dp_source_I2S_onoff(true);
            break;
#endif
        default:
            nRet = -1;
            break;
    }

    if (!nRet)
    {
        gStreamplayer |= player;
        TRACE(1,"gStreamplayer is updated to 0x%x", gStreamplayer);
    }
    return nRet;
}

int app_bt_stream_close(uint16_t player)
{
    int nRet = -1;
    TRACE(3,"%s gStreamplayer: 0x%x%s", __FUNCTION__, gStreamplayer, player2str(gStreamplayer));
    TRACE(3,"%s player:0x%x%s", __FUNCTION__, player, player2str(player));

    if ((gStreamplayer & player) != player)
    {
        return -1;
    }

    switch (player)
    {
        case APP_BT_STREAM_HFP_PCM:
        case APP_BT_STREAM_HFP_CVSD:
        case APP_BT_STREAM_HFP_VENDOR:
            nRet = bt_sco_player(false, APP_SYSFREQ_32K);
            break;
        case APP_BT_STREAM_A2DP_SBC:
        case APP_BT_STREAM_A2DP_AAC:
        case APP_BT_STREAM_A2DP_VENDOR:
            nRet = bt_sbc_player(PLAYER_OPER_STOP, APP_SYSFREQ_32K);
            break;
#ifdef __FACTORY_MODE_SUPPORT__
        case APP_FACTORYMODE_AUDIO_LOOP:
            nRet = app_factorymode_audioloop(false, APP_SYSFREQ_32K);
            break;
#endif
#ifdef MEDIA_PLAYER_SUPPORT
        case APP_PLAY_BACK_AUDIO:
            nRet = app_play_audio_onoff(false, NULL);
            break;
#endif
#ifdef RB_CODEC
        case APP_BT_STREAM_RBCODEC:
            nRet = app_rbplay_audio_onoff(false, 0);
            break;
#endif
#ifdef VOICE_DATAPATH
        case APP_BT_STREAM_VOICEPATH:
            nRet = app_voicepath_stop_audio_stream();
            break;
#endif

#ifdef AUDIO_LINEIN
        case APP_PLAY_LINEIN_AUDIO:
            nRet = app_play_linein_onoff(false);
            break;
#endif

#if defined(APP_LINEIN_A2DP_SOURCE)
        case APP_A2DP_SOURCE_LINEIN_AUDIO:
            nRet = app_a2dp_source_linein_on(false);
            break;
#endif
#if defined(APP_I2S_A2DP_SOURCE)
        case APP_A2DP_SOURCE_I2S_AUDIO:
            nRet = app_a2dp_source_I2S_onoff(false);
            break;
#endif
        default:
            nRet = -1;
            break;
    }
    if (!nRet)
    {
        gStreamplayer &= (~player);
        TRACE(2,"gStreamplayer is updated to 0x%x%s", gStreamplayer, player2str(gStreamplayer));
    }
    return nRet;
}

int app_bt_stream_setup(uint16_t player, uint8_t status)
{
    int nRet = -1;

    TRACE(3,"app_bt_stream_setup prev:%d%s sample:%d", gStreamplayer, player2str(gStreamplayer), status);
    TRACE(3,"app_bt_stream_setup cur:%d%s sample:%d", player, player2str(player), status);

    switch (player)
    {
        case APP_BT_STREAM_HFP_PCM:
        case APP_BT_STREAM_HFP_CVSD:
        case APP_BT_STREAM_HFP_VENDOR:
            break;
        case APP_BT_STREAM_A2DP_SBC:
        case APP_BT_STREAM_A2DP_AAC:
        case APP_BT_STREAM_A2DP_VENDOR:
            bt_sbc_player_setup(status);
            break;
        default:
            nRet = -1;
            break;
    }

    return nRet;
}

int app_bt_stream_restart(APP_AUDIO_STATUS* status)
{
    int nRet = -1;
    uint16_t player = status->id;
    enum APP_SYSFREQ_FREQ_T freq = (enum APP_SYSFREQ_FREQ_T)status->freq;

    TRACE(3,"app_bt_stream_restart prev:%d%s freq:%d", gStreamplayer, player2str(gStreamplayer), freq);
    TRACE(3,"app_bt_stream_restart cur:%d%s freq:%d", player, player2str(player), freq);

    if ((gStreamplayer & player) != player)
    {
        return -1;
    }

    switch (player)
    {
        case APP_BT_STREAM_HFP_PCM:
        case APP_BT_STREAM_HFP_CVSD:
        case APP_BT_STREAM_HFP_VENDOR:
            nRet = bt_sco_player(false, freq);
            nRet = bt_sco_player(true, freq);
            break;
        case APP_BT_STREAM_A2DP_SBC:
        case APP_BT_STREAM_A2DP_AAC:
        case APP_BT_STREAM_A2DP_VENDOR:
#if defined(IBRT)
            ibrt_a2dp_status_t a2dp_status;
            a2dp_ibrt_sync_get_status(&a2dp_status);
            TRACE(2,"[%s] state:%d", __func__, a2dp_status.state);
            if (a2dp_status.state == BTIF_AVDTP_STRM_STATE_STREAMING){
                if (app_audio_manager_a2dp_is_active(BT_DEVICE_ID_1)){            
                    TRACE(1,"[%s] resume", __func__);
                    nRet = bt_sbc_player(PLAYER_OPER_STOP, freq);
                    nRet = bt_sbc_player(PLAYER_OPER_START, freq);
                }else{
                    if (app_ibrt_ui_is_profile_exchanged()){
                        TRACE(1,"[%s] force_audio_retrigger", __func__);
                        app_audio_manager_sendrequest(APP_BT_STREAM_MANAGER_START,BT_STREAM_SBC, BT_DEVICE_ID_1,MAX_RECORD_NUM);
                        app_ibrt_if_force_audio_retrigger();
                    }
                }
            }
#elif defined(__BT_ONE_BRING_TWO__)
            if (btif_me_get_activeCons()>1)
            {
                enum APP_SYSFREQ_FREQ_T sysfreq;

#ifdef A2DP_CP_ACCEL
                sysfreq = APP_SYSFREQ_26M;
#else
                sysfreq = APP_SYSFREQ_104M;
#endif
                app_sysfreq_req(APP_SYSFREQ_USER_BT_A2DP, sysfreq);
                bt_media_volume_ptr_update_by_mediatype(BT_STREAM_SBC);
                app_bt_stream_volumeset(btdevice_volume_p->a2dp_vol);
            }
#endif
            break;
#ifdef __FACTORY_MODE_SUPPORT__
        case APP_FACTORYMODE_AUDIO_LOOP:
            break;
#endif
#ifdef MEDIA_PLAYER_SUPPORT
        case APP_PLAY_BACK_AUDIO:
            break;
#endif
        default:
            nRet = -1;
            break;
    }

    return nRet;
}

void app_bt_stream_volumeup(void)
{
#if defined AUDIO_LINEIN
    if(app_bt_stream_isrun(APP_PLAY_LINEIN_AUDIO))
    {
        stream_linein_volume ++;
        if (stream_linein_volume > TGT_VOLUME_LEVEL_MAX)
        stream_linein_volume = TGT_VOLUME_LEVEL_MAX;
        app_bt_stream_volumeset(stream_linein_volume);
        TRACE(1,"set linein volume %d\n", stream_linein_volume);
    }else
#endif
    {
        if (app_bt_stream_isrun(APP_BT_STREAM_HFP_PCM))
        {
            TRACE(1,"%s set hfp volume", __func__);
            int8_t updatedVol = btdevice_volume_p->hfp_vol;
            if (updatedVol >= TGT_VOLUME_LEVEL_MAX){
                updatedVol = TGT_VOLUME_LEVEL_MAX;
            }else{
                updatedVol++;
                uint32_t lock = nv_record_pre_write_operation();
                btdevice_volume_p->hfp_vol = updatedVol;
                nv_record_post_write_operation(lock);
                current_btdevice_volume.hfp_vol=updatedVol;
                app_bt_stream_volumeset(updatedVol);
            }
            if (updatedVol == TGT_VOLUME_LEVEL_MAX)
            {
#ifdef MEDIA_PLAYER_SUPPORT
                media_PlayAudio(AUD_ID_BT_WARNING,0);
#endif
            }
        }
        else if ((app_bt_stream_isrun(APP_BT_STREAM_A2DP_SBC)) ||
            (app_bt_stream_isrun(APP_BT_STREAM_INVALID)))
        {
            TRACE(1,"%s set audio volume", __func__);
            int8_t updatedVol = btdevice_volume_p->a2dp_vol;
            if (updatedVol >= TGT_VOLUME_LEVEL_MAX){
                updatedVol = TGT_VOLUME_LEVEL_MAX;
            }else{
                updatedVol++;
                uint32_t lock = nv_record_pre_write_operation();
                btdevice_volume_p->a2dp_vol = updatedVol;
                nv_record_post_write_operation(lock);
                current_btdevice_volume.a2dp_vol = updatedVol;
                app_bt_stream_volumeset(updatedVol);
            }
            if (updatedVol == TGT_VOLUME_LEVEL_MAX)
            {
#ifdef MEDIA_PLAYER_SUPPORT
                media_PlayAudio(AUD_ID_BT_WARNING,0);
#endif
            }
        }
        TRACE(2,"%s a2dp: %d", __func__, btdevice_volume_p->a2dp_vol);
        TRACE(2,"%s hfp: %d", __func__, btdevice_volume_p->hfp_vol);
    }
#ifndef FPGA
    nv_record_touch_cause_flush();
#endif
}

void app_bt_set_volume(uint16_t type,uint8_t level)
{
    if ((type&APP_BT_STREAM_HFP_PCM) && app_bt_stream_isrun(APP_BT_STREAM_HFP_PCM)) {
        TRACE(1,"%s set hfp volume", __func__);
        if (level >= TGT_VOLUME_LEVEL_MUTE && level <= TGT_VOLUME_LEVEL_MAX)
        {
            uint32_t lock = nv_record_pre_write_operation();
            btdevice_volume_p->hfp_vol = level;
            nv_record_post_write_operation(lock);
            app_bt_stream_volumeset(btdevice_volume_p->hfp_vol);
        }
        if (btdevice_volume_p->hfp_vol == TGT_VOLUME_LEVEL_MUTE)
        {
#ifdef MEDIA_PLAYER_SUPPORT
            media_PlayAudio(AUD_ID_BT_WARNING,0);
#endif
        }
    }
    if ((type&APP_BT_STREAM_A2DP_SBC) && ((app_bt_stream_isrun(APP_BT_STREAM_INVALID)) ||
        (app_bt_stream_isrun(APP_BT_STREAM_A2DP_SBC)))) {
        TRACE(1,"%s set a2dp volume", __func__);
        if (level >= TGT_VOLUME_LEVEL_MUTE && level <= TGT_VOLUME_LEVEL_MAX)
        {
            uint32_t lock = nv_record_pre_write_operation();
            btdevice_volume_p->a2dp_vol = level;
            nv_record_post_write_operation(lock);
            app_bt_stream_volumeset(btdevice_volume_p->a2dp_vol);
        }
        if (btdevice_volume_p->a2dp_vol == TGT_VOLUME_LEVEL_MUTE)
        {
#ifdef MEDIA_PLAYER_SUPPORT
            media_PlayAudio(AUD_ID_BT_WARNING,0);
#endif
        }
    }

    TRACE(2,"%s a2dp: %d", __func__, btdevice_volume_p->a2dp_vol);
    TRACE(2,"%s hfp: %d", __func__, btdevice_volume_p->hfp_vol);
#ifndef FPGA
    nv_record_touch_cause_flush();
#endif
}

void app_bt_stream_volumedown(void)
{
#if defined AUDIO_LINEIN
    if(app_bt_stream_isrun(APP_PLAY_LINEIN_AUDIO))
    {
        stream_linein_volume --;
        if (stream_linein_volume < TGT_VOLUME_LEVEL_MUTE)
            stream_linein_volume = TGT_VOLUME_LEVEL_MUTE;
        app_bt_stream_volumeset(stream_linein_volume);
        TRACE(1,"set linein volume %d\n", stream_linein_volume);
    }else
#endif
    {
        if (app_bt_stream_isrun(APP_BT_STREAM_HFP_PCM))
        {
            TRACE(1,"%s set hfp volume", __func__);
            int8_t updatedVol = btdevice_volume_p->hfp_vol;
            if (updatedVol <= TGT_VOLUME_LEVEL_MUTE){
                updatedVol = TGT_VOLUME_LEVEL_MUTE;
            }else{
                updatedVol--;
                uint32_t lock = nv_record_pre_write_operation();
                btdevice_volume_p->hfp_vol = updatedVol;
                nv_record_post_write_operation(lock);
                current_btdevice_volume.hfp_vol=updatedVol;
                app_bt_stream_volumeset(updatedVol);
            }

            if (updatedVol == TGT_VOLUME_LEVEL_MUTE)
            {
#ifdef MEDIA_PLAYER_SUPPORT
                media_PlayAudio(AUD_ID_BT_WARNING,0);
#endif
            }
        }else if ((app_bt_stream_isrun(APP_BT_STREAM_INVALID)) ||
            (app_bt_stream_isrun(APP_BT_STREAM_A2DP_SBC))) {
            TRACE(1,"%s set a2dp volume", __func__);
            int8_t updatedVol = btdevice_volume_p->a2dp_vol;
            if (updatedVol <= TGT_VOLUME_LEVEL_MUTE){
                updatedVol = TGT_VOLUME_LEVEL_MUTE;
            }else{
                updatedVol--;
                uint32_t lock = nv_record_pre_write_operation();
                btdevice_volume_p->a2dp_vol = updatedVol;
                nv_record_post_write_operation(lock);
                current_btdevice_volume.a2dp_vol=updatedVol;
                app_bt_stream_volumeset(updatedVol);
            }

            if (updatedVol == TGT_VOLUME_LEVEL_MUTE)
            {
#ifdef MEDIA_PLAYER_SUPPORT
                media_PlayAudio(AUD_ID_BT_WARNING,0);
#endif
            }
        }
        TRACE(2,"%s a2dp: %d", __func__, btdevice_volume_p->a2dp_vol);
        TRACE(2,"%s hfp: %d", __func__, btdevice_volume_p->hfp_vol);
    }
#ifndef FPGA
    nv_record_touch_cause_flush();
#endif
}

void app_bt_stream_volumeset_handler(int8_t vol)
{
    uint32_t ret;
    struct AF_STREAM_CONFIG_T *stream_cfg = NULL;
    ret = af_stream_get_cfg(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg, false);
    if (ret == 0) {
        stream_cfg->vol = vol;
        af_stream_setup(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, stream_cfg);
    }
#if (defined(AUDIO_ANC_FB_MC)||defined(AUDIO_ANC_FB_MC_SCO)) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
    ret = af_stream_get_cfg(AUD_STREAM_ID_2, AUD_STREAM_PLAYBACK, &stream_cfg, false);
    if (ret == 0) {
        stream_cfg->vol = vol;
        af_stream_setup(AUD_STREAM_ID_2, AUD_STREAM_PLAYBACK, stream_cfg);
    }
#endif
}

int app_bt_stream_volumeset(int8_t vol)
{
    TRACE(1,"app_bt_stream_volumeset vol=%d", vol);

    if (vol > TGT_VOLUME_LEVEL_MAX)
        vol = TGT_VOLUME_LEVEL_MAX;
    if (vol < TGT_VOLUME_LEVEL_MUTE)
        vol = TGT_VOLUME_LEVEL_MUTE;

    stream_local_volume = vol;
    if (!app_bt_stream_isrun(APP_PLAY_BACK_AUDIO))
    {
        app_bt_stream_volumeset_handler(vol);
    }
    return 0;
}

int app_bt_stream_local_volume_get(void)
{
    return stream_local_volume;
}

uint8_t app_bt_stream_a2dpvolume_get(void)
{
   // return btdevice_volume_p->a2dp_vol;
   return current_btdevice_volume.a2dp_vol;
}

uint8_t app_bt_stream_hfpvolume_get(void)
{
    //return btdevice_volume_p->hfp_vol;
    return current_btdevice_volume.hfp_vol;

}

void app_bt_stream_a2dpvolume_reset(void)
{
    btdevice_volume_p->a2dp_vol = NVRAM_ENV_STREAM_VOLUME_A2DP_VOL_DEFAULT ;
    current_btdevice_volume.a2dp_vol=NVRAM_ENV_STREAM_VOLUME_A2DP_VOL_DEFAULT;
}

void app_bt_stream_hfpvolume_reset(void)
{
    btdevice_volume_p->hfp_vol = NVRAM_ENV_STREAM_VOLUME_HFP_VOL_DEFAULT;
    current_btdevice_volume.hfp_vol=NVRAM_ENV_STREAM_VOLUME_HFP_VOL_DEFAULT;
}

void app_bt_stream_volume_ptr_update(uint8_t *bdAddr)
{
    static struct btdevice_volume stream_volume = {NVRAM_ENV_STREAM_VOLUME_A2DP_VOL_DEFAULT,NVRAM_ENV_STREAM_VOLUME_HFP_VOL_DEFAULT};

#ifndef FPGA
    nvrec_btdevicerecord *record = NULL;

    memset(&current_btdevice_volume, 0, sizeof(btdevice_volume));

    if (bdAddr && !nv_record_btdevicerecord_find((bt_bdaddr_t*)bdAddr,&record))
    {
        btdevice_volume_p = &(record->device_vol);
        DUMP8("0x%02x ", bdAddr, BTIF_BD_ADDR_SIZE);
        TRACE(4,"%s a2dp_vol:%d hfp_vol:%d ptr:0x%x", __func__, btdevice_volume_p->a2dp_vol, btdevice_volume_p->hfp_vol,btdevice_volume_p);
    }
    else
#endif
    {
        btdevice_volume_p = &stream_volume;
        TRACE(1,"%s default", __func__);
        if (bdAddr){
            DUMP8("0x%02x ", bdAddr, BTIF_BD_ADDR_SIZE);
        }
    }
    current_btdevice_volume.a2dp_vol=btdevice_volume_p->a2dp_vol;
    current_btdevice_volume.hfp_vol=btdevice_volume_p->hfp_vol;

}

struct btdevice_volume * app_bt_stream_volume_get_ptr(void)
{
    return btdevice_volume_p;
}

bool app_bt_stream_isrun(uint16_t player)
{
    if ((gStreamplayer & player) == player)
    {
        return true;
    }
    else
    {
        return false;
    }
}

int app_bt_stream_closeall()
{
    TRACE(0,"app_bt_stream_closeall");

    bt_sco_player(false, APP_SYSFREQ_32K);
    bt_sbc_player(PLAYER_OPER_STOP, APP_SYSFREQ_32K);

#ifdef MEDIA_PLAYER_SUPPORT
    app_play_audio_onoff(false, 0);
#endif
#ifdef RB_CODEC
    app_rbplay_audio_onoff(false, 0);
#endif

#ifdef VOICE_DATAPATH
    app_voicepath_stop_audio_stream();
#endif

    gStreamplayer = APP_BT_STREAM_INVALID;

    return 0;
}

void app_bt_stream_copy_track_one_to_two_24bits(int32_t *dst_buf, int32_t *src_buf, uint32_t src_len)
{
    // Copy from tail so that it works even if dst_buf == src_buf
    for (int i = (int)(src_len - 1); i >= 0; i--)
    {
        dst_buf[i*2 + 0] = dst_buf[i*2 + 1] = src_buf[i];
    }
}

void app_bt_stream_copy_track_one_to_two_16bits(int16_t *dst_buf, int16_t *src_buf, uint32_t src_len)
{
    // Copy from tail so that it works even if dst_buf == src_buf
    for (int i = (int)(src_len - 1); i >= 0; i--)
    {
        dst_buf[i*2 + 0] = dst_buf[i*2 + 1] = src_buf[i];
    }
}

void app_bt_stream_copy_track_two_to_one_16bits(int16_t *dst_buf, int16_t *src_buf, uint32_t dst_len)
{
    for (uint32_t i = 0; i < dst_len; i++)
    {
        dst_buf[i] = src_buf[i*2];
    }
}

void app_bt_stream_adaptive_frequency_adjusting(void)
{
#if 0//(A2DP_DECODER_VER == 2)
    if (app_bt_stream_isrun(APP_BT_STREAM_A2DP_SBC)){
        if (!a2dp_audio_sysfreq_boost_running()){
            a2dp_audio_sysfreq_boost_start(1);
        }
    }
#endif
}

#ifdef PLAYBACK_FORCE_48K
static int app_force48k_resample_iter(uint8_t *buf, uint32_t len)
{
    uint8_t codec_type = bt_sbc_player_get_codec_type();
    uint32_t overlay_id = 0;
    if(codec_type ==  BTIF_AVDTP_CODEC_TYPE_MPEG2_4_AAC)
    {
        overlay_id = APP_OVERLAY_A2DP_AAC;
    }

    a2dp_audio_more_data(overlay_id, buf, len);
    return 0;
}

struct APP_RESAMPLE_T *app_force48k_resample_any_open(enum AUD_CHANNEL_NUM_T chans,
        APP_RESAMPLE_ITER_CALLBACK cb, uint32_t iter_len,
        float ratio_step)
{
    return app_playback_resample_any_open(coef, chans, cb, iter_len, ratio_step);
}
#endif

// =======================================================
// APP RESAMPLE
// =======================================================

#ifndef MIX_MIC_DURING_MUSIC
#include "resample_coef.h"
#endif

static APP_RESAMPLE_BUF_ALLOC_CALLBACK resamp_buf_alloc = app_audio_mempool_get_buff;

static void memzero_int16(void *dst, uint32_t len)
{
    if (dst)
    {
        int16_t *dst16 = (int16_t *)dst;
        int16_t *dst16_end = dst16 + len / 2;

        while (dst16 < dst16_end)
        {
            *dst16++ = 0;
        }
    }
    else
    {
        TRACE(0,"WRN: receive null pointer");
    }
}

struct APP_RESAMPLE_T *app_resample_open_with_preallocated_buf(enum AUD_STREAM_T stream, const struct RESAMPLE_COEF_T *coef, enum AUD_CHANNEL_NUM_T chans,
        APP_RESAMPLE_ITER_CALLBACK cb, uint32_t iter_len,
        float ratio_step, uint8_t* buf, uint32_t bufSize)
{
    TRACE(0,"prompt mixer resample is opened.");
    struct APP_RESAMPLE_T *resamp;
    struct RESAMPLE_CFG_T cfg;
    enum RESAMPLE_STATUS_T status;
    uint32_t size, resamp_size;

    resamp_size = audio_resample_ex_get_buffer_size(chans, AUD_BITS_16, coef->phase_coef_num);

    size = sizeof(struct APP_RESAMPLE_T);
    size += ALIGN(iter_len, 4);
    size += resamp_size;

    ASSERT(size < bufSize, "Pre-allocated buffer size %d is smaller than the needed size %d",
        bufSize, size);

    resamp = (struct APP_RESAMPLE_T *)buf;
    buf += sizeof(*resamp);
    resamp->stream = stream;
    resamp->cb = cb;
    resamp->iter_buf = buf;
    buf += ALIGN(iter_len, 4);
    resamp->iter_len = iter_len;
    resamp->offset = iter_len;
    resamp->ratio_step = ratio_step;

    memset(&cfg, 0, sizeof(cfg));
    cfg.chans = chans;
    cfg.bits = AUD_BITS_16;
    cfg.ratio_step = ratio_step;
    cfg.coef = coef;
    cfg.buf = buf;
    cfg.size = resamp_size;

    status = audio_resample_ex_open(&cfg, (RESAMPLE_ID *)&resamp->id);
    ASSERT(status == RESAMPLE_STATUS_OK, "%s: Failed to open resample: %d", __func__, status);

#ifdef CHIP_BEST1000
    hal_cmu_audio_resample_enable();
#endif

    return resamp;
}


static struct APP_RESAMPLE_T *app_resample_open(enum AUD_STREAM_T stream, const struct RESAMPLE_COEF_T *coef, enum AUD_CHANNEL_NUM_T chans,
        APP_RESAMPLE_ITER_CALLBACK cb, uint32_t iter_len,
        float ratio_step)
{
    TRACE(1,"app resample ratio: %d/1000", uint32_t(ratio_step * 1000));
    struct APP_RESAMPLE_T *resamp;
    struct RESAMPLE_CFG_T cfg;
    enum RESAMPLE_STATUS_T status;
    uint32_t size, resamp_size;
    uint8_t *buf;

    resamp_size = audio_resample_ex_get_buffer_size(chans, AUD_BITS_16, coef->phase_coef_num);

    size = sizeof(struct APP_RESAMPLE_T);
    size += ALIGN(iter_len, 4);
    size += resamp_size;

    resamp_buf_alloc(&buf, size);

    resamp = (struct APP_RESAMPLE_T *)buf;
    buf += sizeof(*resamp);
    resamp->stream = stream;
    resamp->cb = cb;
    resamp->iter_buf = buf;
    buf += ALIGN(iter_len, 4);
    resamp->iter_len = iter_len;
    resamp->offset = iter_len;
    resamp->ratio_step = ratio_step;

    memset(&cfg, 0, sizeof(cfg));
    cfg.chans = chans;
    cfg.bits = AUD_BITS_16;
    cfg.ratio_step = ratio_step;
    cfg.coef = coef;
    cfg.buf = buf;
    cfg.size = resamp_size;

    status = audio_resample_ex_open(&cfg, (RESAMPLE_ID *)&resamp->id);
    ASSERT(status == RESAMPLE_STATUS_OK, "%s: Failed to open resample: %d", __func__, status);

#ifdef CHIP_BEST1000
    hal_cmu_audio_resample_enable();
#endif

    return resamp;
}

static int app_resample_close(struct APP_RESAMPLE_T *resamp)
{
#ifdef CHIP_BEST1000
    hal_cmu_audio_resample_disable();
#endif

    if (resamp)
    {
        audio_resample_ex_close((RESAMPLE_ID *)resamp->id);
    }

    return 0;
}

struct APP_RESAMPLE_T *app_playback_resample_open(enum AUD_SAMPRATE_T sample_rate, enum AUD_CHANNEL_NUM_T chans,
        APP_RESAMPLE_ITER_CALLBACK cb, uint32_t iter_len)
{
    const struct RESAMPLE_COEF_T *coef = NULL;

    if (sample_rate == AUD_SAMPRATE_8000)
    {
        coef = &resample_coef_8k_to_8p4k;
    }
    else if (sample_rate == AUD_SAMPRATE_16000)
    {
        coef = &resample_coef_8k_to_8p4k;
    }
    else if (sample_rate == AUD_SAMPRATE_32000)
    {
        coef = &resample_coef_32k_to_50p7k;
    }
    else if (sample_rate == AUD_SAMPRATE_44100)
    {
        coef = &resample_coef_44p1k_to_50p7k;
    }
    else if (sample_rate == AUD_SAMPRATE_48000)
    {
        coef = &resample_coef_48k_to_50p7k;
    }
    else
    {
        ASSERT(false, "%s: Bad sample rate: %u", __func__, sample_rate);
    }

    return app_resample_open(AUD_STREAM_PLAYBACK, coef, chans, cb, iter_len, 0);
}

#ifdef RESAMPLE_ANY_SAMPLE_RATE
struct APP_RESAMPLE_T *app_playback_resample_any_open(enum AUD_CHANNEL_NUM_T chans,
        APP_RESAMPLE_ITER_CALLBACK cb, uint32_t iter_len,
        float ratio_step)
{
    const struct RESAMPLE_COEF_T *coef = &resample_coef_any_up256;

    return app_resample_open(AUD_STREAM_PLAYBACK, coef, chans, cb, iter_len, ratio_step);
}

struct APP_RESAMPLE_T *app_playback_resample_any_open_with_pre_allocated_buffer(enum AUD_CHANNEL_NUM_T chans,
        APP_RESAMPLE_ITER_CALLBACK cb, uint32_t iter_len,
        float ratio_step, uint8_t* ptrBuf, uint32_t bufSize)
{
    const struct RESAMPLE_COEF_T *coef = &resample_coef_any_up256;

    return app_resample_open_with_preallocated_buf(
        AUD_STREAM_PLAYBACK, coef, chans, cb, iter_len, ratio_step, ptrBuf, bufSize);
}
#endif

int app_playback_resample_close(struct APP_RESAMPLE_T *resamp)
{
    return app_resample_close(resamp);
}

int app_playback_resample_run(struct APP_RESAMPLE_T *resamp, uint8_t *buf, uint32_t len)
{
    uint32_t in_size, out_size;
    struct RESAMPLE_IO_BUF_T io;
    enum RESAMPLE_STATUS_T status;
    int ret;
    //uint32_t lock;

    if (resamp == NULL)
    {
        goto _err_exit;
    }

    io.out_cyclic_start = NULL;
    io.out_cyclic_end = NULL;

    if (resamp->offset < resamp->iter_len)
    {
        io.in = resamp->iter_buf + resamp->offset;
        io.in_size = resamp->iter_len - resamp->offset;
        io.out = buf;
        io.out_size = len;

        //lock = int_lock();
        status = audio_resample_ex_run((RESAMPLE_ID *)resamp->id, &io, &in_size, &out_size);
        //int_unlock(lock);
        if (status != RESAMPLE_STATUS_OUT_FULL && status != RESAMPLE_STATUS_IN_EMPTY &&
            status != RESAMPLE_STATUS_DONE)
        {
            goto _err_exit;
        }

        buf += out_size;
        len -= out_size;
        resamp->offset += in_size;

        ASSERT(len == 0 || resamp->offset == resamp->iter_len,
            "%s: Bad resample offset: len=%d offset=%u iter_len=%u",
            __func__, len, resamp->offset, resamp->iter_len);
    }

    while (len)
    {
        ret = resamp->cb(resamp->iter_buf, resamp->iter_len);
        if (ret)
        {
            goto _err_exit;
        }

        io.in = resamp->iter_buf;
        io.in_size = resamp->iter_len;
        io.out = buf;
        io.out_size = len;

        //lock = int_lock();
        status = audio_resample_ex_run((RESAMPLE_ID *)resamp->id, &io, &in_size, &out_size);
        //int_unlock(lock);
        if (status != RESAMPLE_STATUS_OUT_FULL && status != RESAMPLE_STATUS_IN_EMPTY &&
            status != RESAMPLE_STATUS_DONE)
        {
            goto _err_exit;
        }

        ASSERT(out_size <= len, "%s: Bad resample out_size: out_size=%u len=%d", __func__, out_size, len);
        ASSERT(in_size <= resamp->iter_len, "%s: Bad resample in_size: in_size=%u iter_len=%u", __func__, in_size, resamp->iter_len);

        buf += out_size;
        len -= out_size;
        if (in_size != resamp->iter_len)
        {
            resamp->offset = in_size;

            ASSERT(len == 0, "%s: Bad resample len: len=%d out_size=%u", __func__, len, out_size);
        }
    }

    return 0;

_err_exit:
    if (resamp)
    {
        app_resample_reset(resamp);
    }

    memzero_int16(buf, len);

    return 1;
}

struct APP_RESAMPLE_T *app_capture_resample_open(enum AUD_SAMPRATE_T sample_rate, enum AUD_CHANNEL_NUM_T chans,
        APP_RESAMPLE_ITER_CALLBACK cb, uint32_t iter_len)
{
    const struct RESAMPLE_COEF_T *coef = NULL;

    if (sample_rate == AUD_SAMPRATE_8000)
    {
        coef = &resample_coef_8p4k_to_8k;
    }
    else if (sample_rate == AUD_SAMPRATE_16000)
    {
        // Same coef as 8K sample rate
        coef = &resample_coef_8p4k_to_8k;
    }
    else
    {
        ASSERT(false, "%s: Bad sample rate: %u", __func__, sample_rate);
    }

    return app_resample_open(AUD_STREAM_CAPTURE, coef, chans, cb, iter_len, 0);

}

#ifdef RESAMPLE_ANY_SAMPLE_RATE
struct APP_RESAMPLE_T *app_capture_resample_any_open(enum AUD_CHANNEL_NUM_T chans,
        APP_RESAMPLE_ITER_CALLBACK cb, uint32_t iter_len,
        float ratio_step)
{
    const struct RESAMPLE_COEF_T *coef = &resample_coef_any_up256;
    return app_resample_open(AUD_STREAM_CAPTURE, coef, chans, cb, iter_len, ratio_step);
}
#endif

int app_capture_resample_close(struct APP_RESAMPLE_T *resamp)
{
    return app_resample_close(resamp);
}

int app_capture_resample_run(struct APP_RESAMPLE_T *resamp, uint8_t *buf, uint32_t len)
{
    uint32_t in_size, out_size;
    struct RESAMPLE_IO_BUF_T io;
    enum RESAMPLE_STATUS_T status;
    int ret;

    if (resamp == NULL)
    {
        goto _err_exit;
    }

    io.out_cyclic_start = NULL;
    io.out_cyclic_end = NULL;

    if (resamp->offset < resamp->iter_len)
    {
        io.in = buf;
        io.in_size = len;
        io.out = resamp->iter_buf + resamp->offset;
        io.out_size = resamp->iter_len - resamp->offset;

        status = audio_resample_ex_run((RESAMPLE_ID *)resamp->id, &io, &in_size, &out_size);
        if (status != RESAMPLE_STATUS_OUT_FULL && status != RESAMPLE_STATUS_IN_EMPTY &&
            status != RESAMPLE_STATUS_DONE)
        {
            goto _err_exit;
        }

        buf += in_size;
        len -= in_size;
        resamp->offset += out_size;

        ASSERT(len == 0 || resamp->offset == resamp->iter_len,
            "%s: Bad resample offset: len=%d offset=%u iter_len=%u",
            __func__, len, resamp->offset, resamp->iter_len);

        if (resamp->offset == resamp->iter_len)
        {
            ret = resamp->cb(resamp->iter_buf, resamp->iter_len);
            if (ret)
            {
                goto _err_exit;
            }
        }
    }

    while (len)
    {
        io.in = buf;
        io.in_size = len;
        io.out = resamp->iter_buf;
        io.out_size = resamp->iter_len;

        status = audio_resample_ex_run((RESAMPLE_ID *)resamp->id, &io, &in_size, &out_size);
        if (status != RESAMPLE_STATUS_OUT_FULL && status != RESAMPLE_STATUS_IN_EMPTY &&
            status != RESAMPLE_STATUS_DONE)
        {
            goto _err_exit;
        }

        ASSERT(in_size <= len, "%s: Bad resample in_size: in_size=%u len=%u", __func__, in_size, len);
        ASSERT(out_size <= resamp->iter_len, "%s: Bad resample out_size: out_size=%u iter_len=%u", __func__, out_size, resamp->iter_len);

        buf += in_size;
        len -= in_size;
        if (out_size == resamp->iter_len)
        {
            ret = resamp->cb(resamp->iter_buf, resamp->iter_len);
            if (ret)
            {
                goto _err_exit;
            }
        }
        else
        {
            resamp->offset = out_size;

            ASSERT(len == 0, "%s: Bad resample len: len=%u in_size=%u", __func__, len, in_size);
        }
    }

    return 0;

_err_exit:
    if (resamp)
    {
        app_resample_reset(resamp);
    }

    memzero_int16(buf, len);

    return 1;
}

void app_resample_reset(struct APP_RESAMPLE_T *resamp)
{
    audio_resample_ex_flush((RESAMPLE_ID *)resamp->id);
    resamp->offset = resamp->iter_len;
}

void app_resample_tune(struct APP_RESAMPLE_T *resamp, float ratio)
{
    float new_step;

    if (resamp == NULL)
    {
        return;
    }

    TRACE(3,"%s: stream=%d ratio=%d", __FUNCTION__, resamp->stream, FLOAT_TO_PPB_INT(ratio));

    if (resamp->stream == AUD_STREAM_PLAYBACK) {
        new_step = resamp->ratio_step + resamp->ratio_step * ratio;
    } else {
        new_step = resamp->ratio_step - resamp->ratio_step * ratio;
    }
    audio_resample_ex_set_ratio_step(resamp->id, new_step);
}

APP_RESAMPLE_BUF_ALLOC_CALLBACK app_resample_set_buf_alloc_callback(APP_RESAMPLE_BUF_ALLOC_CALLBACK cb)
{
    APP_RESAMPLE_BUF_ALLOC_CALLBACK old_cb;

    old_cb = resamp_buf_alloc;
    resamp_buf_alloc = cb;

    return old_cb;
}

#ifdef TX_RX_PCM_MASK

#ifdef SCO_DMA_SNAPSHOT

#define MSBC_LEN  60

void store_encode_frame2buff()
{
    if(bt_sco_codec_is_msbc())
    {
    uint32_t len;
    //processing uplink msbc data.
        if(playback_buf_btpcm_copy!=NULL)
        {
            len=playback_size_btpcm_copy-MSBC_LEN;
        memcpy((uint8_t *)(*(volatile uint32_t *)(MIC_BUFF_ADRR_REG)),playback_buf_btpcm_copy,MSBC_LEN);
        memcpy(playback_buf_btpcm_copy,playback_buf_btpcm_copy+MSBC_LEN,len);
        }
    //processing downlink msbc data.    
        if(capture_buf_btpcm_copy!=NULL)
        {
            len=capture_size_btpcm_copy-MSBC_LEN;
        memcpy(capture_buf_btpcm_copy,capture_buf_btpcm_copy+MSBC_LEN,len);
        memcpy(capture_buf_btpcm_copy+len,(uint8_t *)(*(volatile uint32_t *)(RX_BUFF_ADRR)),MSBC_LEN);
        }
#if defined(CHIP_BEST2300A)
        uint8_t sco_toggle = *(volatile uint8_t *)(RX_BUFF_ADRR+8);
        pcm_data_param[sco_toggle].curr_time = *(volatile uint32_t *)(RX_BUFF_ADRR+4);
        pcm_data_param[sco_toggle].toggle = sco_toggle;
        pcm_data_param[sco_toggle].flag = *(volatile uint8_t *)(RX_BUFF_ADRR+9);
        pcm_data_param[sco_toggle].counter = *(volatile uint16_t *)(RX_BUFF_ADRR+10);
#endif
    }
    return;
}
#else
extern CQueue* get_tx_esco_queue_ptr();
extern CQueue* get_rx_esco_queue_ptr();
void store_encode_frame2buff()
{
    CQueue* Tx_esco_queue_temp = NULL;
    CQueue* Rx_esco_queue_temp = NULL;
    Tx_esco_queue_temp = get_tx_esco_queue_ptr();
    Rx_esco_queue_temp = get_rx_esco_queue_ptr();
    unsigned int len;
    len= 60;
    int status = 0;
    if(bt_sco_codec_is_msbc())
    {
        status = DeCQueue(Tx_esco_queue_temp,(uint8_t *)(*(volatile uint32_t *)(MIC_BUFF_ADRR_REG)),len); 
        if(status){
            //TRACE(0,"TX DeC Fail");
        }
        status =EnCQueue(Rx_esco_queue_temp, (uint8_t *)(*(volatile uint32_t *)(RX_BUFF_ADRR)), len);
        if(status){
            //TRACE(0,"RX EnC Fail");
        }
    }

}
#endif
#endif

int app_bt_stream_init(void)
{
    app_bt_stream_trigger_checker_init();
#if defined(USE_LOWLATENCY_LIB)
    app_ally_change_cb_register(app_ibrt_lly_cmd, low_latency_status_change);
#endif
    return 0;
}

