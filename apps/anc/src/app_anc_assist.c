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
#include "string.h"
#include "app_utils.h"
#include "app_thread.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "anc_process.h"
#include "audioflinger.h"
#include "hwtimer_list.h"
#include "audio_dump.h"
#include "speech_cfg.h"
#include "anc_assist.h"
#include "app_anc_assist.h"
#include "assist/anc_assist_defs.h"
#include "assist/anc_assist_utils.h"
#include "assist/anc_assist_anc.h"
#include "assist/anc_assist_mic.h"
#include "assist/anc_assist_tws_sync.h"
#include "assist_ultrasound.h"
#include "app_anc_assist_trigger.h"
#include "app_voice_assist_ultrasound.h"
#include "app_anc_assist_thirdparty.h"

#include "app_audio.h"

// #define ANC_ASSIST_AUDIO_DUMP_96K

// #define ANC_ASSIST_AUDIO_DUMP

#define _LOOP_CNT_FIXED_MAX

#define ANC_ASSIST_UPDATE_SYSFREQ

#if defined(ASSIST_LOW_RAM_MOD)
#define ALGO_SAMPLE_RATE    (8000)
#else
#define ALGO_SAMPLE_RATE    (16000)
#endif
#if defined(IBRT)
#define ALGO_CHANNEL_NUM    (3)
#else
#define ALGO_CHANNEL_NUM    (5)
#endif
#if defined(ASSIST_LOW_RAM_MOD)
#define ALGO_FRAME_LEN      (60)
#else
#define ALGO_FRAME_LEN      (120)
#endif
#define ALGO_FRAME_MS       (1000.0 * ALGO_FRAME_LEN / ALGO_SAMPLE_RATE)

#ifdef DUAL_MIC_RECORDING
#define SAMPLE_RATE_MAX     (48000)
#elif defined (VOICE_ASSIST_WD_ENABLED)
#define SAMPLE_RATE_MAX     (96000)
#elif defined (ASSIST_LOW_RAM_MOD)
#define SAMPLE_RATE_MAX     (16000)
#else
#define SAMPLE_RATE_MAX     (ALGO_SAMPLE_RATE)
#endif

#define _SAMPLE_BITS        (24)
#define MAX_CHANNEL_NUM        (ALGO_CHANNEL_NUM + 1)
#if defined (ASSIST_LOW_RAM_MOD)
#define _FRAME_LEN          (ALGO_FRAME_LEN)
#else
#define _FRAME_LEN          (ALGO_FRAME_LEN * SAMPLE_RATE_MAX / ALGO_SAMPLE_RATE)
#endif
#ifdef VOICE_ASSIST_WD_ENABLED
#define _LOOP_CNT           (2)
#else
#define _LOOP_CNT           (3)
#endif

#ifdef VOICE_ASSIST_WD_ENABLED
#define _PLAY_SAMPLE_RATE   (96000)
#else
#define _PLAY_SAMPLE_RATE   (48000)
#endif
#define MAX_PLAY_CHANNEL_NUM   (1)
#define _PLAY_FRAME_LEN     (ALGO_FRAME_LEN * _PLAY_SAMPLE_RATE / ALGO_SAMPLE_RATE)

#if _SAMPLE_BITS != 24
#pragma message ("APP ANC ASSIST: Just support 24 bits!!!")
#endif

typedef enum {
    _ASSIST_MSG_OPEN_CLOSE = 0,
    _ASSIST_MSG_CLOSE_SPK,
} _ASSIST_MSG_T;

// Capture stream
#define AF_PINGPONG					(2)
#define STREAM_CAPTURE_ID			AUD_STREAM_ID_0
#define MAX_CODEC_CAPTURE_BUF_SIZE    	(_FRAME_LEN * sizeof(_PCM_T) * MAX_CHANNEL_NUM * AF_PINGPONG * _LOOP_CNT)
static uint8_t *codec_capture_buf = NULL;

#define CAPTURE_BUF_LEN 			(_FRAME_LEN * _LOOP_CNT)
static float capture_buf[MAX_CHANNEL_NUM][CAPTURE_BUF_LEN];

// Play stream
#if defined(ANC_ASSIST_USE_INT_CODEC) || !defined(ANC_ASSIST_PILOT_TONE_ALWAYS_ON)
#define STREAM_PLAY_ID				AUD_STREAM_ID_0
#define STREAM_PLAY_CODEC			AUD_STREAM_USE_INT_CODEC
#else
#define STREAM_PLAY_ID				AUD_STREAM_ID_3
#define STREAM_PLAY_CODEC			AUD_STREAM_USE_INT_CODEC2
#endif

#define MAX_CODEC_PLAY_BUF_SIZE         (_PLAY_FRAME_LEN * sizeof(_PCM_T) * MAX_PLAY_CHANNEL_NUM * AF_PINGPONG * _LOOP_CNT)
#define PLAY_BUF_LEN                    (_PLAY_FRAME_LEN * _LOOP_CNT)

#if defined(ANC_ASSIST_PILOT_TONE_ALWAYS_ON)
static  uint8_t __attribute__((aligned(4))) codec_play_buf[MAX_CODEC_PLAY_BUF_SIZE];
static  float play_buf[PLAY_BUF_LEN];
#elif defined(VOICE_ASSIST_CUSTOM_LEAK_DETECT)
static uint8_t *codec_play_buf = NULL;
static float *play_buf = NULL;
#endif


static int32_t g_sample_rate = ALGO_SAMPLE_RATE;
static int32_t g_sample_bits = _SAMPLE_BITS;
static int32_t g_chan_num = MAX_CHANNEL_NUM;
static int32_t g_frame_len = ALGO_FRAME_LEN;
static int32_t g_loop_cnt = _LOOP_CNT;
static int32_t g_capture_buf_size = MAX_CODEC_CAPTURE_BUF_SIZE;

static POSSIBLY_UNUSED int32_t g_play_sample_rate = _PLAY_SAMPLE_RATE;
static int32_t g_play_buf_size = MAX_CODEC_PLAY_BUF_SIZE;

static int32_t g_phone_call_sample_rate = ALGO_SAMPLE_RATE;

static enum APP_SYSFREQ_FREQ_T g_sys_freq = APP_SYSFREQ_32K;
bool g_opened_flag = false;
static anc_assist_mode_t g_anc_assist_mode = ANC_ASSIST_MODE_QTY;

static bool g_need_open_mic = true;
static bool g_need_open_spk = true;
static bool g_mic_open_flag = false;
static bool g_spk_open_flag = false;

static uint8_t g_ff_ch_num = MAX_FF_CHANNEL_NUM;
static float *g_ff_mic_buf[MAX_FF_CHANNEL_NUM] 		= {NULL};
static uint8_t g_fb_ch_num = MAX_FB_CHANNEL_NUM;
static float *g_fb_mic_buf[MAX_FB_CHANNEL_NUM] 		= {NULL};
static float *g_talk_mic_buf 	= NULL;
static float *g_ref_mic_buf 	= NULL;
static uint32_t mic_cfg_old = 0;
static uint32_t spk_cfg_old = 0;

static AncAssistState *anc_assist_st = NULL;
static AncAssistRes anc_assist_res;
static ThirdpartyAssistRes thirdparty_assist_res;
extern AncAssistConfig anc_assist_cfg;
extern ThirdpartyAssistConfig thirdparty_assist_cfg;

static uint32_t g_user_status = 0;
static anc_assist_user_callback_t g_user_callback[ANC_ASSIST_USER_QTY];

static void _open_mic(void);
static void _open_spk(void);
static void _close_mic(void);
static void _close_spk(void);

static int32_t _anc_assist_open_impl(AncAssistConfig *cfg, ThirdpartyAssistConfig *thirdparty_cfg);
static int32_t _anc_assist_close_impl(void);

#ifdef ANC_ASSIST_UPDATE_SYSFREQ
static void _anc_assist_update_sysfreq(void);
#endif

#ifdef ANC_ASSIST_AUDIO_DUMP
typedef short		_DUMP_PCM_T;
static _DUMP_PCM_T audio_dump_buf[ALGO_FRAME_LEN];
#endif

#ifdef ANC_ASSIST_AUDIO_DUMP_96K
typedef int16_t		_DUMP_PCM_T;
static _DUMP_PCM_T audio_dump_buf[_FRAME_LEN * _LOOP_CNT];
#endif

#include "cmsis_os.h"

bool ultrasound_process_flag = true;
osMutexId _anc_assist_mutex_id = NULL;
osMutexDef(_anc_assist_mutex);

#if defined(IBRT)
#if defined(ASSIST_LOW_RAM_MOD)
#define HEAP_BUFF_SIZE       (1024 * 21)
#else
#define HEAP_BUFF_SIZE       (1024 * 32)
#endif
#else
#define HEAP_BUFF_SIZE       (1024 * 48)
#endif

#if defined(ANC_ASSIST_UNUSED_ON_PHONE_CALL)
static bool need_open_anc_assist = false;
static uint8_t *anc_assist_heap_buff = NULL;
#else
static uint8_t __attribute__((aligned(4))) anc_assist_heap_buff[HEAP_BUFF_SIZE];
#endif

//dynamic allocation init
static multi_heap_handle_t stream_heap = NULL;

static uint32_t stream_heap_init(void)
{
    uint32_t mic_chan_num = app_anc_get_anc_assist_mic_chan_num();
    uint32_t anc_assist_buff_size = 0;
    uint8_t *app_anc_assist_buff_start = NULL;

    anc_assist_buff_size = MAX_CODEC_CAPTURE_BUF_SIZE / MAX_CHANNEL_NUM * mic_chan_num
#if defined(ANC_ASSIST_UNUSED_ON_PHONE_CALL)
    + HEAP_BUFF_SIZE
#endif
    + 80;//sizeof(struct multi_heap_info) = 44;
#if defined(VOICE_ASSIST_CUSTOM_LEAK_DETECT) && !defined(ANC_ASSIST_PILOT_TONE_ALWAYS_ON)
    anc_assist_buff_size += MAX_CODEC_PLAY_BUF_SIZE;
    anc_assist_buff_size += PLAY_BUF_LEN * sizeof(float);
#endif

    app_anc_assist_mempool_init(anc_assist_buff_size);
    app_anc_assist_mempool_get_buff(&app_anc_assist_buff_start, anc_assist_buff_size);
    stream_heap = heap_register(app_anc_assist_buff_start, anc_assist_buff_size);

    return 0;
}

static void *stream_heap_malloc(size_t size)
{
    // TRACE(0,"[%s] multi_heap_free_size = %d, size - %d", __FUNCTION__, heap_free_size(stream_heap), size);
    void *ptr = heap_malloc(stream_heap, size);
    ASSERT(ptr, "[%s] no memory", __FUNCTION__);

    return ptr;
}

static void stream_heap_free(void *ptr)
{
    ASSERT(ptr, "[%s] ptr is null", __FUNCTION__);
    heap_free(stream_heap, ptr);
    // TRACE(0,"[%s] multi_heap_free_size = %d", __func__, heap_free_size(stream_heap));
}

static void _anc_assist_create_lock(void)
{
    if (_anc_assist_mutex_id == NULL) {
        _anc_assist_mutex_id = osMutexCreate((osMutex(_anc_assist_mutex)));
    }
}

static void _anc_assist_lock(void)
{
    osMutexWait(_anc_assist_mutex_id, osWaitForever);
}

static void _anc_assist_unlock(void)
{
    osMutexRelease(_anc_assist_mutex_id);
}

static int32_t update_stream_cfg(AncAssistConfig *cfg, ThirdpartyAssistConfig *thirdparty_cfg)
{
	_anc_assist_lock();
	g_need_open_mic = true;
	g_need_open_spk = true;
	g_loop_cnt      = _LOOP_CNT;
#ifdef VOICE_ASSIST_WD_ENABLED
	g_sample_rate   = AUD_SAMPRATE_96000;
#else
	g_sample_rate   = ALGO_SAMPLE_RATE;
#endif
    enum AUD_IO_PATH_T app_path = AUD_IO_PATH_NULL;

	if (g_anc_assist_mode == ANC_ASSIST_MODE_STANDALONE) {
		TRACE(0, "[%s] ANC_ASSIST_MODE_STANDALONE", __func__);
	} else if (g_anc_assist_mode == ANC_ASSIST_MODE_PHONE_CALL) {
		TRACE(0, "[%s] ANC_ASSIST_MODE_PHONE_CALL", __func__);
		g_need_open_mic = false;
#ifdef ANC_ASSIST_USE_INT_CODEC
		g_need_open_spk = false;
#endif
		g_loop_cnt = 2;
		g_sample_rate = g_phone_call_sample_rate;
        app_path = AUD_INPUT_PATH_MAINMIC;
	} else if (g_anc_assist_mode == ANC_ASSIST_MODE_RECORD) {
		TRACE(0, "[%s] ANC_ASSIST_MODE_RECORD", __func__);
		g_need_open_mic = false;
		// g_need_open_spk = false;
		g_loop_cnt = 2;
		g_sample_rate = AUD_SAMPRATE_48000;
        app_path = AUD_INPUT_PATH_ASRMIC;
	} else if (g_anc_assist_mode == ANC_ASSIST_MODE_MUSIC) {
		TRACE(0, "[%s] ANC_ASSIST_MODE_MUSIC", __func__);
		g_loop_cnt = 3;
#ifdef ANC_ASSIST_USE_INT_CODEC
		g_need_open_spk = false;
#endif
	} else if (g_anc_assist_mode == ANC_ASSIST_MODE_MUSIC_AAC) {
		TRACE(0, "[%s] ANC_ASSIST_MODE_MUSIC_AAC", __func__);
#ifdef ANC_ASSIST_USE_INT_CODEC
		g_need_open_spk = false;
#endif
		g_loop_cnt = 3;
	} else if (g_anc_assist_mode == ANC_ASSIST_MODE_MUSIC_SBC) {
		TRACE(0, "[%s] ANC_ASSIST_MODE_MUSIC_SBC", __func__);
#ifdef ANC_ASSIST_USE_INT_CODEC
		g_need_open_spk = false;
#endif
		g_loop_cnt = 2;
	} else if (g_anc_assist_mode == ANC_ASSIST_MODE_NONE) {
		TRACE(0, "[%s] ANC_ASSIST_MODE_NONE", __func__);
		g_need_open_mic = false;
		g_need_open_spk = false;
	} else {
		ASSERT(0, "[%s] g_anc_assist_mode(%d) is invalid!", __func__, g_anc_assist_mode);
	}

    if (app_path == AUD_IO_PATH_NULL) {
        anc_assist_mic_update_anc_cfg(cfg, thirdparty_cfg);
        anc_assist_mic_set_app_cfg(AUD_IO_PATH_NULL);
        anc_assist_mic_parser_index(AUD_INPUT_PATH_ANC_ASSIST);
        g_chan_num = anc_assist_mic_get_ch_num(AUD_INPUT_PATH_ANC_ASSIST);
    } else {
        anc_assist_mic_set_anc_cfg(AUD_INPUT_PATH_ANC_ASSIST);
        anc_assist_mic_set_app_cfg(app_path);
        anc_assist_mic_parser_index(app_path);
        g_chan_num = anc_assist_mic_get_ch_num(app_path);
    }

	if (anc_assist_spk_parser_anc_cfg(cfg, thirdparty_cfg) == 0) {
		g_need_open_spk = false;
	}

#if defined(_LOOP_CNT_FIXED_MAX)
    g_loop_cnt = _LOOP_CNT;
#endif

	g_frame_len = ALGO_FRAME_LEN * (g_sample_rate / ALGO_SAMPLE_RATE);
	g_capture_buf_size = g_frame_len * sizeof(_PCM_T) * g_chan_num * AF_PINGPONG * g_loop_cnt;
	g_play_buf_size = (MAX_CODEC_PLAY_BUF_SIZE / _LOOP_CNT) * g_loop_cnt;

    TRACE(0, "[%s] Need to open MIC(%d), SPK(%d)", __func__, g_need_open_mic, g_need_open_spk);
    TRACE(0, "[%s] fs: %d, chan_num: %d, loop_cnt: %d", __func__, g_sample_rate, g_chan_num, g_loop_cnt);
    _anc_assist_unlock();

    return 0;
}

#ifdef VOICE_ASSIST_WD_ENABLED
static void update_trigger_status(void)
{
	app_anc_assist_trigger_init();

	if (g_mic_open_flag) {
		af_stream_start(STREAM_CAPTURE_ID, AUD_STREAM_CAPTURE);
	}

	if (g_spk_open_flag) {
		af_stream_start(STREAM_PLAY_ID, AUD_STREAM_PLAYBACK);
	}
}
#endif

static int32_t update_codec_status(void)
{
	// MIC
	uint32_t mic_cfg_new = anc_assist_mic_parser_anc_cfg(&anc_assist_cfg, &thirdparty_assist_cfg);

	// SPK
	uint32_t spk_cfg_new = anc_assist_spk_parser_anc_cfg(&anc_assist_cfg, &thirdparty_assist_cfg);

	if (spk_cfg_old != spk_cfg_new) {
		if (g_spk_open_flag) {
			_close_spk();
		}
	}

	if (mic_cfg_old != mic_cfg_new) {
		if (g_mic_open_flag) {
			_close_mic();
		}
	}

    update_stream_cfg(&anc_assist_cfg, &thirdparty_assist_cfg);
    anc_assist_set_cfg_sync(anc_assist_st, &anc_assist_cfg);
	thirdparty_anc_assist_set_cfg(&thirdparty_assist_cfg);

	if (mic_cfg_old != mic_cfg_new) {
		if (mic_cfg_new) {
			_open_mic();
		}
	}

	if (spk_cfg_old != spk_cfg_new) {
		if (spk_cfg_new) {
			_open_spk();
		}
	}
	mic_cfg_old = mic_cfg_new;
	spk_cfg_old = spk_cfg_new;

#ifdef VOICE_ASSIST_WD_ENABLED
	update_trigger_status();
#endif

	return 0;
}

static int32_t update_algo_cfg(uint32_t user_status, AncAssistConfig *cfg, ThirdpartyAssistConfig *thirdparty_cfg)
{
	cfg->ff_howling_en = false;
	cfg->fb_howling_en = false;
	cfg->noise_en = false;
	cfg->noise_classify_en = false;
	cfg->wind_en = false;
	cfg->pilot_en = false;
	cfg->pnc_en = false;
	cfg->wsd_en = false;
	cfg->extern_kws_en = false;
	cfg->ultrasound_en = false;
	cfg->extern_adaptive_eq_en = false;
	cfg->prompt_en = false;

	for (uint32_t i=0; i<ANC_ASSIST_USER_QTY; i++) {
		if (user_status & (0x1 << i)) {
			if (i ==  ANC_ASSIST_USER_ANC) {
				TRACE(0, "[%s] ANC_ASSIST_USER_ANC", __func__);
				cfg->ff_howling_en = false;
				cfg->fb_howling_en = false;
				cfg->wind_en = true;
				thirdparty_cfg->en = true;
			} else if (i ==  ANC_ASSIST_USER_PSAP) {
				TRACE(0, "[%s] ANC_ASSIST_USER_PSAP", __func__);
				cfg->ff_howling_en = true;
				cfg->fb_howling_en = true;
				cfg->wind_en = true;
			} else if (i ==  ANC_ASSIST_USER_KWS) {
				TRACE(0, "[%s] ANC_ASSIST_USER_KWS", __func__);
				// cfg->wsd_en = true;
				cfg->extern_kws_en = true;
			} else if (i ==  ANC_ASSIST_USER_ULTRASOUND) {
				TRACE(0, "[%s] ANC_ASSIST_USER_ULTRASOUND", __func__);
				cfg->ultrasound_en = true;
			} else if (i ==  ANC_ASSIST_USER_WD) {
				TRACE(0, "[%s] ANC_ASSIST_USER_WD", __func__);
				cfg->pilot_en = true;
				cfg->pilot_cfg.adaptive_anc_en = 0;
				cfg->pilot_cfg.wd_en = 1;
				cfg->pilot_cfg.custom_leak_detect_en = 0;
			} else if (i ==  ANC_ASSIST_USER_CUSTOM_LEAK_DETECT) {
				TRACE(0, "[%s] ANC_ASSIST_USER_CUSTOM_LEAK_DETECT", __func__);
				cfg->pilot_en = true;
				cfg->pilot_cfg.adaptive_anc_en = 0;
				cfg->pilot_cfg.wd_en = 0;
				cfg->pilot_cfg.custom_leak_detect_en = 1;
				// cfg->fb_howling_en = true;
			} else if(i == ANC_ASSIST_USER_ONESHOT_ADAPT_ANC){
				TRACE(0, "[%s] ANC_ASSIST_USER_ONESHOT_ADAPT_ANC", __func__);
				cfg->ff_howling_en = true;
				cfg->fb_howling_en = true;
				cfg->wind_en = true;
			} else if (i == ANC_ASSIST_USER_PILOT_ANC) {
				TRACE(0, "[%s] ANC_ASSIST_USER_PILOT_ANC", __func__);
				cfg->pilot_en = true;
				cfg->pilot_cfg.adaptive_anc_en = 1;
				cfg->pilot_cfg.playback_ms = 10000;
				cfg->pilot_cfg.wd_en = 0;
				cfg->pilot_cfg.custom_leak_detect_en = 0;
			} else if (i == ANC_ASSIST_USER_PROMPT_LEAK_DETECT) {
				TRACE(0, "[%s] ANC_ASSIST_USER_PROMPT_LEAK_DETECT", __func__);
				cfg->prompt_en = true;
			} else if (i ==  ANC_ASSIST_USER_ADAPTIVE_EQ) {
				TRACE(0, "[%s] ANC_ASSIST_USER_ADAPTIVE_EQ", __func__);
				cfg->extern_adaptive_eq_en = true;
			} else if (i == ANC_ASSIST_USER_NOISE_ADAPT_ANC) {
				TRACE(0, "[%s] ANC_ASSIST_USER_NOISE_ADAPT_ANC", __func__);
				cfg->noise_en = true;
			} else {
				ASSERT(0, "[%s] i(%d) is invalid!", __func__, i);
			}
		}
	}
#if defined(ASSIST_LOW_RAM_MOD)
    //If ASSIST LOW RAM MOD is ON, we should abandon some function.
    ASSERT((0==cfg->ff_howling_en && 0==cfg->fb_howling_en),
           "Do not support howling when ASSIST_LOW_RAM_MOD is ON!!!");
#endif
	return 0;
}

static int32_t _assist_open_close_msg_post(anc_assist_user_t user, bool open_opt)
{
    TRACE(0, "[%s] ", __func__);

    APP_MESSAGE_BLOCK msg;
    msg.mod_id = APP_MODUAL_VOICE_ASSIST;
    msg.msg_body.message_id = _ASSIST_MSG_OPEN_CLOSE;
    msg.msg_body.message_Param0 = user;
    msg.msg_body.message_Param1 = open_opt;
    app_mailbox_put(&msg);

    return 0;
}

static bool ultrasound_reset_needed = true;
static int app_anc_assist_thread_handler(APP_MESSAGE_BODY *msg_body)
{
    uint32_t id = msg_body->message_id;
	// anc_assist_user_t user = ANC_ASSIST_USER_QTY;

	mic_cfg_old = anc_assist_mic_parser_anc_cfg(&anc_assist_cfg, &thirdparty_assist_cfg);
	spk_cfg_old = anc_assist_spk_parser_anc_cfg(&anc_assist_cfg, &thirdparty_assist_cfg);

    TRACE(0, "[%s] id :%d", __func__, id);

    switch (id) {
		case _ASSIST_MSG_OPEN_CLOSE: {
			anc_assist_user_t user = (uint32_t)(msg_body->message_Param0);
			bool open_opt = msg_body->message_Param1;
			uint32_t user_status_old = g_user_status;
			uint32_t user_status_new = 0;

			if (open_opt == true) {
				user_status_new = user_status_old | (0x1 << user);
			} else {
				user_status_new = user_status_old & ~(0x1 << user);
			}

			TRACE(0, "[%s] opt: %d, old: 0x%x, new: 0x%x", __func__, open_opt, user_status_old, user_status_new);

			if (user_status_old != user_status_new) {
				update_algo_cfg(user_status_new, &anc_assist_cfg, &thirdparty_assist_cfg); 

				if (open_opt == true && user == ANC_ASSIST_USER_ULTRASOUND) {
					ultrasound_reset_needed = true;
				}

				if (open_opt == false && user == ANC_ASSIST_USER_WD) {
					anc_assist_pilot_set_play_fadeout(anc_assist_st);
				}

				if ((user_status_old == 0) && (user_status_new != 0)) {		// Open
					update_stream_cfg(&anc_assist_cfg, &thirdparty_assist_cfg);
#if defined(ANC_ASSIST_UNUSED_ON_PHONE_CALL)
					if (g_anc_assist_mode != ANC_ASSIST_MODE_PHONE_CALL) {
						_anc_assist_open_impl(&anc_assist_cfg, &thirdparty_assist_cfg);
					}else {
						need_open_anc_assist = true;
					}
#else
					_anc_assist_open_impl(&anc_assist_cfg, &thirdparty_assist_cfg);
#endif
				} else if ((user_status_old != 0) && (user_status_new == 0)) {	// Close
					_anc_assist_close_impl();
				} else {	// Update
					// update_stream_cfg(&anc_assist_cfg, &thirdparty_assist_cfg);
					update_codec_status();
				}

				// Save parameters
				g_user_status = user_status_new;
			}
			break;
		}
		default:
			ASSERT(0, "[%s] id(%d) is invalid", __func__, id);
			break;
    }

    return 0;
}

int32_t app_anc_assist_init(void)
{
	g_opened_flag = false;
	g_anc_assist_mode = ANC_ASSIST_MODE_STANDALONE;

	for (uint8_t i = 0; i < MAX_FF_CHANNEL_NUM; i++) {
		g_ff_mic_buf[i] = capture_buf[MIC_INDEX_FF + i];
	}
	for (uint8_t i = 0; i < MAX_FB_CHANNEL_NUM; i++) {
		g_fb_mic_buf[i] = capture_buf[MIC_INDEX_FB + i];
	}
	g_talk_mic_buf 	= capture_buf[MIC_INDEX_TALK];
	g_ref_mic_buf 	= capture_buf[MIC_INDEX_REF];

	g_user_status = 0;
	for (uint32_t i = 0; i<ANC_ASSIST_USER_QTY; i++) {
		g_user_callback[i] = NULL;
	}

	_anc_assist_create_lock();
	anc_assist_mic_reset();

	app_set_threadhandle(APP_MODUAL_VOICE_ASSIST, app_anc_assist_thread_handler);

	return 0;
}

int32_t app_anc_assist_deinit(void)
{
	return 0;
}

int32_t app_anc_assist_register(anc_assist_user_t user, anc_assist_user_callback_t func)
{
	TRACE(0, "[%s] user: %d", __func__, user);

	g_user_callback[user] = func;

	return 0;
}

// TODO: Currently, just used by sco. Can extend
uint32_t app_anc_assist_get_mic_ch_num(enum AUD_IO_PATH_T path)
{
	return anc_assist_mic_get_ch_num(path);
}

uint32_t app_anc_assist_get_mic_ch_map(enum AUD_IO_PATH_T path)
{
	return anc_assist_mic_get_cfg(path);
}

int32_t app_anc_assist_parser_app_mic_buf(void *buf, uint32_t *len)
{
	_anc_assist_lock();
	anc_assist_mic_parser_app_buf(buf, len, capture_buf, sizeof(capture_buf));
	_anc_assist_unlock();

	return 0;
}

static bool _need_switch_mode(anc_assist_mode_t old_mode, anc_assist_mode_t new_mode)
{
#ifndef ANC_ASSIST_USE_INT_CODEC
    if ((old_mode == ANC_ASSIST_MODE_STANDALONE)    &&
        (new_mode == ANC_ASSIST_MODE_MUSIC)) {
            return false;
    }

    if ((old_mode == ANC_ASSIST_MODE_MUSIC)         &&
        (new_mode == ANC_ASSIST_MODE_STANDALONE)) {
            return false;
    }

    if ((old_mode == ANC_ASSIST_MODE_STANDALONE)    &&
        (new_mode == ANC_ASSIST_MODE_MUSIC_AAC)     &&
        (g_loop_cnt == 3)) {
            return false;
    }

    if ((old_mode == ANC_ASSIST_MODE_STANDALONE)    &&
        (new_mode == ANC_ASSIST_MODE_MUSIC_SBC)     &&
        (g_loop_cnt == 2)) {
            return false;
    }

    if ((old_mode == ANC_ASSIST_MODE_MUSIC_AAC)     &&
        (new_mode == ANC_ASSIST_MODE_STANDALONE)) {
            return false;
    }

    if ((old_mode == ANC_ASSIST_MODE_MUSIC_SBC)     &&
        (new_mode == ANC_ASSIST_MODE_STANDALONE)) {
            return false;
    }
#endif

    return true;
}

int32_t app_anc_assist_set_mode(anc_assist_mode_t mode)
{
    TRACE(0, "[%s] %d --> %d", __func__, g_anc_assist_mode, mode);

    if (g_anc_assist_mode == mode) {
        TRACE(0, "[%s] WARNING: Same mode = %d", __func__, mode);
        return 1;
    }

    if (_need_switch_mode(g_anc_assist_mode, mode) == false) {
        g_anc_assist_mode = mode;
#ifdef ANC_ASSIST_UPDATE_SYSFREQ
        if (g_opened_flag) {
            _anc_assist_update_sysfreq();
        }
#endif
        return 0;
    }

    g_anc_assist_mode = mode;
    if (g_opened_flag) {
        TRACE(0, "[%s] ------ START ------", __func__);
        _anc_assist_close_impl();
        update_algo_cfg(g_user_status, &anc_assist_cfg, &thirdparty_assist_cfg); 
        update_stream_cfg(&anc_assist_cfg, &thirdparty_assist_cfg);
#if defined(ANC_ASSIST_UNUSED_ON_PHONE_CALL)
        if (g_anc_assist_mode != ANC_ASSIST_MODE_PHONE_CALL){
            _anc_assist_open_impl(&anc_assist_cfg, &thirdparty_assist_cfg);
        }else {
            need_open_anc_assist = true;
        }
#else
        _anc_assist_open_impl(&anc_assist_cfg, &thirdparty_assist_cfg);
#endif
        TRACE(0, "[%s] ------ END ------", __func__);
    } else {
        update_stream_cfg(&anc_assist_cfg, &thirdparty_assist_cfg);
#if defined(ANC_ASSIST_UNUSED_ON_PHONE_CALL)
        if (need_open_anc_assist) {
            _anc_assist_open_impl(&anc_assist_cfg, &thirdparty_assist_cfg);
            need_open_anc_assist = false;
        }
#endif
    }

    return 0;
}

int32_t app_anc_assist_set_playback_info(int32_t sample_rate)
{
#if defined(ANC_ASSIST_USE_INT_CODEC)
	g_play_sample_rate = (sample_rate == 0) ? _PLAY_SAMPLE_RATE : sample_rate;
#else
	TRACE(1, "[%s] No need to set playback info when use DAC2", __FUNCTION__);
#endif
    //phone call sample rate needs to be set if ASSIST use different sample rate with SCO
    g_phone_call_sample_rate = sample_rate;
    return 0;
}

int32_t app_anc_assist_is_runing(void)
{
	return g_opened_flag;
}

static void _anc_assist_callback(anc_assist_notify_t msg, void *data, uint32_t value)
{
    switch (msg) {
        case ANC_ASSIST_NOTIFY_FREQ:
#ifdef ANC_ASSIST_UPDATE_SYSFREQ
            _anc_assist_update_sysfreq();
#else
            TRACE(0, "[%s] TODO: Impl ANC_ASSIST_NOTIFY_FREQ", __func__);
#endif
            break;
        default:
            break;
    }
}

static int32_t _anc_assist_open_impl(AncAssistConfig *cfg, ThirdpartyAssistConfig *thirdparty_cfg)
{
    TRACE(0, "[%s] ...", __func__);

	if (g_opened_flag == true) {
		TRACE(0, "[%s] WARNING: g_opened_flag is true", __func__);
		return 1;
	}

	// Temporary
	app_sysfreq_req(APP_SYSFREQ_USER_VOICE_ASSIST, HAL_CMU_FREQ_104M);

    if (g_need_open_mic
#if defined(VOICE_ASSIST_CUSTOM_LEAK_DETECT) && !defined(ANC_ASSIST_PILOT_TONE_ALWAYS_ON)
        || g_need_open_spk
#endif
        ) {
        stream_heap_init();
    }

#if defined(ANC_ASSIST_UNUSED_ON_PHONE_CALL)
	//anc_assist get buff for syspool
	anc_assist_heap_buff = stream_heap_malloc(HEAP_BUFF_SIZE);
#endif

	_anc_assist_lock();
	anc_assist_st = anc_assist_create(ALGO_SAMPLE_RATE, g_sample_bits, ALGO_CHANNEL_NUM, ALGO_FRAME_LEN, cfg, _anc_assist_callback,anc_assist_heap_buff,HEAP_BUFF_SIZE);
	thirdparty_anc_assist_create(ALGO_SAMPLE_RATE, ALGO_CHANNEL_NUM, ALGO_FRAME_LEN, thirdparty_cfg); 

	TRACE(2, "[%s] playback stream sample rate = %d", __FUNCTION__, g_play_sample_rate);
	anc_assist_pilot_set_play_sample_rate(anc_assist_st, g_play_sample_rate);

#ifdef ANC_ASSIST_AUDIO_DUMP
	audio_dump_init(ALGO_FRAME_LEN, sizeof(_DUMP_PCM_T), 4);
#endif

#ifdef ANC_ASSIST_AUDIO_DUMP_96K
	// audio_dump_init(g_frame_len * g_loop_cnt, sizeof(_DUMP_PCM_T), 2);

	audio_dump_init(193, sizeof(_DUMP_PCM_T), 2);
#endif

	anc_assist_anc_init();
	anc_assist_tws_sync_init(ALGO_FRAME_MS);

	g_opened_flag = true;
	_anc_assist_unlock();

	_open_mic();
	_open_spk();

#ifdef VOICE_ASSIST_WD_ENABLED
	update_trigger_status();
#endif

#ifdef ANC_ASSIST_UPDATE_SYSFREQ
    _anc_assist_update_sysfreq();
#else
    g_sys_freq = APP_SYSFREQ_26M;
    app_sysfreq_req(APP_SYSFREQ_USER_VOICE_ASSIST, g_sys_freq);

#if defined(ENABLE_CALCU_CPU_FREQ_LOG)
    TRACE(0, "[%s] Sys freq[%d]: %d", __func__, g_sys_freq, hal_sys_timer_calc_cpu_freq(5, 0));
#endif
#endif

    return 0;
}

static int32_t _anc_assist_close_impl(void)
{
    TRACE(0, "[%s] ...", __func__);

	if (g_opened_flag == false) {
		TRACE(0, "[%s] WARNING: g_opened_flag is false", __func__);
		return 1;
	}

	// Temporary
	app_sysfreq_req(APP_SYSFREQ_USER_VOICE_ASSIST, HAL_CMU_FREQ_104M);

	_close_spk();
	_close_mic();

#ifdef VOICE_ASSIST_WD_ENABLED
	app_anc_assist_trigger_deinit();
#endif

	_anc_assist_lock();
	g_opened_flag = false;

	anc_assist_anc_deinit();

	anc_assist_destroy(anc_assist_st);
#if defined(ANC_ASSIST_UNUSED_ON_PHONE_CALL)
	stream_heap_free(anc_assist_heap_buff);
#endif
	thirdparty_anc_assist_destroy(); 
	_anc_assist_unlock();

	g_sys_freq = APP_SYSFREQ_32K;
    app_sysfreq_req(APP_SYSFREQ_USER_VOICE_ASSIST, g_sys_freq);

#if defined(ENABLE_CALCU_CPU_FREQ_LOG)
    TRACE(0, "[%s] Sys freq[%d]: %d", __func__, g_sys_freq, hal_sys_timer_calc_cpu_freq(5, 0));
#endif

    return 0;
}

int32_t app_anc_assist_open(anc_assist_user_t user)
{
	TRACE(0, "[%s] g_user: 0x%x, user: %d", __func__, g_user_status, user);

	if (g_user_status & (0x1 << user)) {
		TRACE(0, "[%s] skip open", __FUNCTION__);
		return 0;
	}

	_assist_open_close_msg_post(user, true);

	return 0;
}

int32_t app_anc_assist_close(anc_assist_user_t user)
{
	TRACE(0, "[%s] g_user: 0x%x, user: %d", __func__, g_user_status, user);

	if ((g_user_status & (0x1 << user)) == false) {
		TRACE(0, "[%s] skip close", __FUNCTION__);
		return 0;
	}

	_assist_open_close_msg_post(user, false);

	return 0;

}

bool ultrasound_changed = false;
static int32_t POSSIBLY_UNUSED _process_frame_ultrasound(float *fb_mic, float *ref, uint32_t frame_len)
{
	if (ultrasound_reset_needed) {
		assist_ultrasound_reset();
		ultrasound_reset_needed = false;
	}

	ultrasound_changed = assist_ultrasound_process(fb_mic, ref, frame_len);

	if ((g_user_callback[ANC_ASSIST_USER_ULTRASOUND] != NULL) &&
		(g_user_status & (0x1 << ANC_ASSIST_USER_ULTRASOUND))) {
		uint32_t res = ultrasound_changed;
		g_user_callback[ANC_ASSIST_USER_ULTRASOUND](&res, 1, NULL);
	}

	return 0;
}

static int32_t POSSIBLY_UNUSED _process_frame(float **ff_mic, uint8_t ff_ch_num, float **fb_mic, uint8_t fb_ch_num, float *talk_mic, float *ref, uint32_t frame_len)
{
    anc_assist_res = anc_assist_process(anc_assist_st, ff_mic, ff_ch_num, fb_mic, fb_ch_num, talk_mic, ref, frame_len);
    thirdparty_assist_res = thirdparty_anc_assist_process(ff_mic, ff_ch_num, fb_mic, fb_ch_num, talk_mic, ref, frame_len);

#if 0
	for (uint32_t i=0; i<ANC_ASSIST_USER_QTY; i++) {
		if ((g_user_callback[i] != NULL) && (g_user_status & (0x1 << i))) {
			g_user_callback[i](talk_mic, frame_len, NULL);
		}
	}
#else
	if ((g_user_callback[ANC_ASSIST_USER_WD] != NULL) &&
		(g_user_status & (0x1 << ANC_ASSIST_USER_WD))) {
		uint32_t res[2 * MAX_FB_CHANNEL_NUM];
		for (uint8_t i = 0, j = 0; i < 2 * MAX_FB_CHANNEL_NUM; i += 2, j++) {
			res[i + 0] = anc_assist_res.wd_changed[j];
			res[i + 1] = anc_assist_res.wd_status[j];
		}
		g_user_callback[ANC_ASSIST_USER_WD](res, 2 * MAX_FB_CHANNEL_NUM, NULL);
	}

	if ((g_user_callback[ANC_ASSIST_USER_KWS] != NULL) &&
		(g_user_status & (0x1 << ANC_ASSIST_USER_KWS))) {
		g_user_callback[ANC_ASSIST_USER_KWS](talk_mic, frame_len, NULL);
	}

	if ((g_user_callback[ANC_ASSIST_USER_CUSTOM_LEAK_DETECT] != NULL) &&
		(g_user_status & (0x1 << ANC_ASSIST_USER_CUSTOM_LEAK_DETECT))) {
		float res[2];
		res[0] = anc_assist_res.custom_leak_detect_result;
		res[1] = (float)anc_assist_res.custom_leak_detect_status;
		g_user_callback[ANC_ASSIST_USER_CUSTOM_LEAK_DETECT](res, 2, NULL);
	}

	if ((g_user_callback[ANC_ASSIST_USER_PROMPT_LEAK_DETECT] != NULL) &&
		(g_user_status & (0x1 << ANC_ASSIST_USER_PROMPT_LEAK_DETECT))) {
		float res[4];
		res[0] = anc_assist_res.fb_energy_band1;
		res[1] = anc_assist_res.fb_energy_band2;
		res[2] = anc_assist_res.fb_energy_band3;
		res[3] = (float)anc_assist_res.prompt_leak_detect_status;
		g_user_callback[ANC_ASSIST_USER_PROMPT_LEAK_DETECT](res, 4, NULL);
	}

	if ((g_user_callback[ANC_ASSIST_USER_ONESHOT_ADAPT_ANC] != NULL) &&
		(g_user_status & (0x1 << ANC_ASSIST_USER_ONESHOT_ADAPT_ANC))){
		float * input_data[2];
		input_data[0] = ff_mic[0];
		input_data[1] = fb_mic[0];
		g_user_callback[ANC_ASSIST_USER_ONESHOT_ADAPT_ANC](input_data, frame_len, &anc_assist_res);
	}

	if ((g_user_callback[ANC_ASSIST_USER_PILOT_ANC] != NULL) &&
		(g_user_status & (0x1 << ANC_ASSIST_USER_PILOT_ANC))){
		g_user_callback[ANC_ASSIST_USER_PILOT_ANC](NULL, frame_len, &anc_assist_res);
	}

	if ((g_user_callback[ANC_ASSIST_USER_ADAPTIVE_EQ] != NULL) &&
		(g_user_status & (0x1 << ANC_ASSIST_USER_ADAPTIVE_EQ))) {
		g_user_callback[ANC_ASSIST_USER_ADAPTIVE_EQ](fb_mic, frame_len, ref);
	}

	if ((g_user_callback[ANC_ASSIST_USER_NOISE_ADAPT_ANC] != NULL) &&
		(g_user_status & (0x1 << ANC_ASSIST_USER_NOISE_ADAPT_ANC))) {
		g_user_callback[ANC_ASSIST_USER_NOISE_ADAPT_ANC](NULL, 0, &anc_assist_res);
	}
#endif

#ifdef VOICE_ASSIST_NOISE_ADAPT_ANC_ENABLED
	// ignore ff/fb gain changed by noise detection module
	for (uint32_t i = 0; i < ff_ch_num; i++) {
		if (anc_assist_res.ff_gain_changed[i] && (anc_assist_res.ff_gain_id[i] == ANC_ASSIST_ALGO_ID_NOISE)) {
			anc_assist_res.ff_gain_changed[i] = false;
			anc_assist_res.ff_gain[i] = 1.f;
		}

		if (anc_assist_res.fb_gain_changed[i] && (anc_assist_res.fb_gain_id[i] == ANC_ASSIST_ALGO_ID_NOISE)) {
			anc_assist_res.fb_gain_changed[i] = false;
			anc_assist_res.fb_gain[i] = 1.f;
		}
	}
#endif

	if (any_of_u32(anc_assist_res.ff_gain_changed, ff_ch_num, ANC_ASSIST_ALGO_STATUS_CHANGED) ||
		any_of_u32(anc_assist_res.fb_gain_changed, fb_ch_num, ANC_ASSIST_ALGO_STATUS_CHANGED)) {
		anc_assist_anc_set_gain_coef(anc_assist_res.ff_gain_changed, anc_assist_res.ff_gain_id, anc_assist_res.ff_gain, ff_ch_num, anc_assist_res.fb_gain_changed, anc_assist_res.fb_gain_id, anc_assist_res.fb_gain, fb_ch_num);
	} else {
		;
	}

	// TODO: deal with stereo headphone
	if (anc_assist_res.curve_changed[0]) {
		anc_assist_anc_switch_curve(anc_assist_res.curve_id[0], anc_assist_res.curve_index[0]);
	} else {
		;
	}

	anc_assist_tws_sync_heartbeat();

	return 0;
}

extern bool infrasound_fadeout_flag;
int32_t app_anc_assist_process(void *buf, uint32_t len)
{
	_anc_assist_lock();
	uint32_t pcm_len = len / sizeof(_PCM_T);
	uint32_t frame_len = pcm_len / g_chan_num;
	uint32_t loop_cnt = frame_len / ALGO_FRAME_LEN;
	_PCM_T *pcm_buf = (_PCM_T *)buf;

	if (g_opened_flag == false) {
		TRACE(1, "[%s] WARNING: g_opened_flag is false", __func__);
		return -1;
	}

	// TRACE(3, "[%s] len = %d, loop_cnt = %d", __func__, len, g_loop_cnt);

	if (g_anc_assist_mode == ANC_ASSIST_MODE_PHONE_CALL) {
		anc_assist_mic_parser_anc_buf(AUD_INPUT_PATH_MAINMIC, (float *)capture_buf, CAPTURE_BUF_LEN, pcm_buf, pcm_len);
	} else {
		anc_assist_mic_parser_anc_buf(AUD_INPUT_PATH_ANC_ASSIST, (float *)capture_buf, CAPTURE_BUF_LEN, pcm_buf, pcm_len);
	}

#ifdef ANC_ASSIST_AUDIO_DUMP_96K
	uint32_t dump_ch = 0;
	audio_dump_clear_up();
	int offseti = 105;

	for (uint32_t i = offseti,j = 0; i < offseti + 193; i++,j++) {
		audio_dump_buf[j] = (_PCM_T)g_fb_mic_buf[0][i] >> 8;
	}
	audio_dump_add_channel_data(dump_ch++, audio_dump_buf, 193);

	for (uint32_t i=offseti, j=0; i<193+offseti; i++, j++) {
		audio_dump_buf[j] = (_PCM_T)g_ref_mic_buf[i] >> 8;
	}
	audio_dump_add_channel_data(dump_ch++, audio_dump_buf, 193);

	audio_dump_run();
#endif

#if SAMPLE_RATE_MAX != ALGO_SAMPLE_RATE
	if (g_sample_rate != ALGO_SAMPLE_RATE) {
		ASSERT(g_sample_rate == SAMPLE_RATE_MAX, "[%s] g_sample_rate(%d) is invalid!", __func__, g_sample_rate);

#ifdef VOICE_ASSIST_WD_ENABLED
		_process_frame_ultrasound(g_fb_mic_buf[0], g_ref_mic_buf, frame_len);
#endif
	}
#endif
	_anc_assist_unlock();

	for (uint32_t offset = 0, cnt=0; cnt<loop_cnt; cnt++) {
#ifdef ANC_ASSIST_AUDIO_DUMP
		uint32_t dump_ch = 0;
		audio_dump_clear_up();

		// TODO: Use capture buf
		for (uint32_t i=0; i<ALGO_FRAME_LEN; i++) {
			audio_dump_buf[i] = (_PCM_T)g_ff_mic_buf[0][offset + i] >> 8;
		}
		audio_dump_add_channel_data(dump_ch++, audio_dump_buf, ALGO_FRAME_LEN);

		for (uint32_t i=0; i<ALGO_FRAME_LEN; i++) {
			audio_dump_buf[i] = (_PCM_T)g_fb_mic_buf[0][offset + i] >> 8;
		}
		audio_dump_add_channel_data(dump_ch++, audio_dump_buf, ALGO_FRAME_LEN);

		for (uint32_t i=0; i<ALGO_FRAME_LEN; i++) {
			audio_dump_buf[i] = (_PCM_T)g_talk_mic_buf[offset + i] >> 8;
		}
		audio_dump_add_channel_data(dump_ch++, audio_dump_buf, ALGO_FRAME_LEN);

		for (uint32_t i=0; i<ALGO_FRAME_LEN; i++) {
			audio_dump_buf[i] = (_PCM_T)g_ref_mic_buf[offset + i] >> 8;
		}
		audio_dump_add_channel_data(dump_ch++, audio_dump_buf, ALGO_FRAME_LEN);

		audio_dump_run();
#endif

		float *ff_mic_buf[MAX_FF_CHANNEL_NUM];
		float *fb_mic_buf[MAX_FB_CHANNEL_NUM];

		for (uint8_t i = 0; i < g_ff_ch_num; i++) {
			ff_mic_buf[i] = g_ff_mic_buf[i] + offset;
		}

		for (uint8_t i = 0; i < g_fb_ch_num; i++) {
			fb_mic_buf[i] = g_fb_mic_buf[i] + offset;
		}

		_process_frame(ff_mic_buf, g_ff_ch_num,
					fb_mic_buf, g_fb_ch_num,
					g_talk_mic_buf 	+ offset,
					g_ref_mic_buf 	+ offset,
					ALGO_FRAME_LEN);
		offset += ALGO_FRAME_LEN;
	}

    return 0;
}

static uint32_t codec_capture_stream_callback(uint8_t *buf, uint32_t len)
{
	app_anc_assist_process(buf, len);

    return len;
}


static void _open_mic(void)
{
	if (g_mic_open_flag == true) {
		TRACE(0, "[%s] WARNING: MIC is opened", __func__);
		return;
	}

	if (g_need_open_mic) {
        codec_capture_buf = stream_heap_malloc(g_capture_buf_size);

		struct AF_STREAM_CONFIG_T stream_cfg;

		memset(&stream_cfg, 0, sizeof(stream_cfg));
		stream_cfg.channel_num 	= (enum AUD_CHANNEL_NUM_T)g_chan_num;
		stream_cfg.sample_rate 	= (enum AUD_SAMPRATE_T)g_sample_rate;
		stream_cfg.bits 		= (enum AUD_BITS_T)g_sample_bits;
		stream_cfg.vol 			= 12;
		stream_cfg.chan_sep_buf = false;
		stream_cfg.device       = AUD_STREAM_USE_INT_CODEC;
		stream_cfg.io_path      = AUD_INPUT_PATH_ANC_ASSIST;
		stream_cfg.channel_map	= anc_assist_mic_get_cfg(AUD_INPUT_PATH_ANC_ASSIST);
		stream_cfg.handler      = codec_capture_stream_callback;
		stream_cfg.data_size    = g_capture_buf_size;
		stream_cfg.data_ptr     = codec_capture_buf;

        TRACE(0, "[%s] sample_rate:%d, data_size:%d", __func__, stream_cfg.sample_rate, stream_cfg.data_size);

		af_stream_open(STREAM_CAPTURE_ID, AUD_STREAM_CAPTURE, &stream_cfg);
#ifndef VOICE_ASSIST_WD_ENABLED
		af_stream_start(STREAM_CAPTURE_ID, AUD_STREAM_CAPTURE);
#endif

		g_mic_open_flag = true;
	}
}

static void _close_mic(void)
{
	if (g_mic_open_flag == false) {
		TRACE(0, "[%s] WARNING: MIC is closed", __func__);
		return;
	}

	TRACE(0, "[%s] ...", __func__);
	af_stream_stop(STREAM_CAPTURE_ID, AUD_STREAM_CAPTURE);
	af_stream_close(STREAM_CAPTURE_ID, AUD_STREAM_CAPTURE);

    stream_heap_free(codec_capture_buf);

	g_mic_open_flag = false;
}

#if (defined VOICE_ASSIST_PILOT_ANC_ENABLED) || (defined VOICE_ASSIST_CUSTOM_LEAK_DETECT) || (defined VOICE_ASSIST_WD_ENABLED)
#ifdef VOICE_ASSIST_WD_ENABLED
static const int16_t local_96kHz_pcm_data[] = {
	#include "pilot_oed_pcm.h"
};
#endif

#define ULTRASOUND_VOL (DB2LIN(-40))

static uint32_t codec_play_stream_callback(uint8_t *buf, uint32_t len)
{
	uint32_t pcm_len = len / sizeof(_PCM_T);
	uint32_t frame_len = pcm_len;
	_PCM_T *pcm_buf = (_PCM_T *)buf;

#ifdef VOICE_ASSIST_WD_ENABLED
	app_anc_assist_trigger_checker();
#endif

	anc_assist_pilot_get_play_data(anc_assist_st, play_buf, frame_len);

	for (int32_t i=0; i<frame_len; i++) {
		pcm_buf[i] = (_PCM_T)play_buf[i];
	}

#ifdef VOICE_ASSIST_WD_ENABLED
	for (int32_t i=0; i<ARRAY_SIZE(local_96kHz_pcm_data); i++) {
		pcm_buf[i] = speech_ssat_int24(pcm_buf[i] + (int32_t)(ULTRASOUND_VOL * (local_96kHz_pcm_data[i] << 8)));
	}
#endif

#if 0//def ANC_ASSIST_AUDIO_DUMP_96K
	audio_dump_clear_up();
	for (uint32_t i=0; i<frame_len; i++) {
		audio_dump_buf[i] = (_PCM_T)pcm_buf[i] >> 8;
	}
	audio_dump_add_channel_data(0, audio_dump_buf, frame_len);
	audio_dump_run();
#endif

    return len;
}

static void _open_spk(void)
{
	if (g_spk_open_flag == true) {
		TRACE(0, "[%s] WARNING: SPK is opened", __func__);
		return;
	}

	if (g_need_open_spk) {
#if defined(VOICE_ASSIST_CUSTOM_LEAK_DETECT) && !defined(ANC_ASSIST_PILOT_TONE_ALWAYS_ON)
        codec_play_buf = stream_heap_malloc(g_play_buf_size);
        play_buf = stream_heap_malloc(PLAY_BUF_LEN * sizeof(float));
#endif

		struct AF_STREAM_CONFIG_T stream_cfg;

		memset(&stream_cfg, 0, sizeof(stream_cfg));
		stream_cfg.channel_num = AUD_CHANNEL_NUM_1;
		stream_cfg.channel_map = (enum AUD_CHANNEL_MAP_T)AUD_CHANNEL_MAP_CH0;
		stream_cfg.sample_rate = _PLAY_SAMPLE_RATE;
		stream_cfg.bits = AUD_BITS_24;
		stream_cfg.vol = TGT_VOLUME_LEVEL_MAX;
		stream_cfg.chan_sep_buf = false;
		stream_cfg.device       = STREAM_PLAY_CODEC;
		stream_cfg.handler      = codec_play_stream_callback;
		stream_cfg.data_size    = g_play_buf_size;
		stream_cfg.data_ptr     = codec_play_buf;

        TRACE(0, "[%s] sample_rate:%d, data_size:%d", __func__, stream_cfg.sample_rate, stream_cfg.data_size);

		af_stream_open(STREAM_PLAY_ID, AUD_STREAM_PLAYBACK, &stream_cfg);

		// add the pingpang
#ifdef VOICE_ASSIST_WD_ENABLED
		int32_t *pcm_buf = (int32_t *)codec_play_buf;
		for (int32_t i=0; i<ARRAY_SIZE(local_96kHz_pcm_data); i++) {
			pcm_buf[i] = (int32_t)(ULTRASOUND_VOL * (local_96kHz_pcm_data[i] << 8));
		}

		pcm_buf = (int32_t *)(codec_play_buf + g_play_buf_size / 2);
		for (int32_t i=0; i<ARRAY_SIZE(local_96kHz_pcm_data); i++) {
			pcm_buf[i] = (int32_t)(ULTRASOUND_VOL * (local_96kHz_pcm_data[i] << 8));
		}
#endif

#ifndef VOICE_ASSIST_WD_ENABLED
		af_stream_start(STREAM_PLAY_ID, AUD_STREAM_PLAYBACK);
#endif

		g_spk_open_flag = true;
	}
}

extern bool pilot_play_get_fadeout_state(void);
static void _close_spk(void)
{
	if (g_spk_open_flag == false) {
		TRACE(0, "[%s] WARNING: SPK is closed", __func__);
		return;
	}

	//anc_assist_pilot_set_play_fadeout(anc_assist_st);
	if (pilot_play_get_fadeout_state()) {
		osDelay(anc_assist_cfg.pilot_cfg.gain_smooth_ms + 300);     // 300: More time to fadeout
	} else {
		osDelay(10);
	}

	TRACE(0, "[%s] ...", __func__);
	af_stream_stop(STREAM_PLAY_ID, AUD_STREAM_PLAYBACK);
	af_stream_close(STREAM_PLAY_ID, AUD_STREAM_PLAYBACK);

#if defined(VOICE_ASSIST_CUSTOM_LEAK_DETECT) && !defined(ANC_ASSIST_PILOT_TONE_ALWAYS_ON)
    stream_heap_free(play_buf);
    stream_heap_free(codec_play_buf);
#endif

	g_spk_open_flag = false;
}
#else

static void _open_spk(void)
{
    ;
}

static void _close_spk(void)
{
    ;
}
#endif

#ifdef ANC_ASSIST_UPDATE_SYSFREQ

#define ANC_ASSIST_BASE_MIPS (3)

#define ULTRA_INFRASOUND_BASE_MIPS (20)     //need to be changed

static enum APP_SYSFREQ_FREQ_T _anc_assist_get_sysfreq(void)
{
    enum APP_SYSFREQ_FREQ_T freq = APP_SYSFREQ_32K;
#ifdef VOICE_ASSIST_WD_ENABLED
    int32_t required_mips = anc_assist_get_required_mips(anc_assist_st) + ANC_ASSIST_BASE_MIPS + ULTRA_INFRASOUND_BASE_MIPS;
#else
	int32_t required_mips = anc_assist_get_required_mips(anc_assist_st) + ANC_ASSIST_BASE_MIPS;
#endif
    TRACE(0, "[%s] Required mips: %dM", __func__,  required_mips);

    if (required_mips >= 96) {
        freq = APP_SYSFREQ_208M;
    } else if(required_mips >= 72) {
        freq = APP_SYSFREQ_104M;
    } else if (required_mips >= 48) {
        freq = APP_SYSFREQ_78M;
    } else if (required_mips >= 24) {
        freq = APP_SYSFREQ_52M;
    } else {
        freq = APP_SYSFREQ_26M;
    }

    // NOTE: Optimize power consumption for special project
    if (g_anc_assist_mode == ANC_ASSIST_MODE_PHONE_CALL) {
        // if (required_mips < 24) {
        //     freq = APP_SYSFREQ_32K;
        // }
    } else if (g_anc_assist_mode == ANC_ASSIST_MODE_RECORD) {
        // if (required_mips < 24) {
        //     freq = APP_SYSFREQ_32K;
        // }
    } else if (g_anc_assist_mode == ANC_ASSIST_MODE_MUSIC) {
        // if (required_mips < 24) {
        //     freq = APP_SYSFREQ_32K;
        // }
    } else if (g_anc_assist_mode == ANC_ASSIST_MODE_MUSIC_AAC) {
        // if (required_mips < 24) {
        //     freq = APP_SYSFREQ_32K;
        // }

    } else if (g_anc_assist_mode == ANC_ASSIST_MODE_MUSIC_SBC) {
        // if (required_mips < 24) {
        //     freq = APP_SYSFREQ_32K;
        // }

    }

    return freq;
}

static void _anc_assist_update_sysfreq(void)
{
    g_sys_freq = _anc_assist_get_sysfreq();
    app_sysfreq_req(APP_SYSFREQ_USER_VOICE_ASSIST, g_sys_freq);

#if defined(ENABLE_CALCU_CPU_FREQ_LOG)
    TRACE(0, "[%s] Sys freq[%d]: %d", __func__, g_sys_freq, hal_sys_timer_calc_cpu_freq(5, 0));
#endif
}
#endif

int32_t app_anc_assist_get_prompt_anc_index(int *anc_index,int32_t channel_idx)
{
	ASSERT(anc_assist_st != NULL,"[%s] st is null",__func__);
	TRACE(0,"[%s] st is not null, set new one",__func__);
	anc_assist_get_prompt_anc_index(anc_assist_st,anc_index,channel_idx);

	return 0;
}

uint32_t app_anc_assist_get_frame_len(void)
{
	return g_frame_len;
}
