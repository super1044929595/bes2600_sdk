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
#include "anc_assist_mic.h"
#include "hal_codec.h"
#include "hal_trace.h"
#include "tgt_hardware.h"

#ifndef ANC_FF_MIC_CH_L
#define ANC_FF_MIC_CH_L			(0xFF)
#endif

#ifndef ANC_FF_MIC_CH_R
#define ANC_FF_MIC_CH_R			(0xFF)
#endif

#ifndef ANC_FB_MIC_CH_L
#define ANC_FB_MIC_CH_L			(0xFF)
#endif

#ifndef ANC_FB_MIC_CH_R
#define ANC_FB_MIC_CH_R			(0xFF)
#endif

#ifndef ANC_TALK_MIC_CH
#define ANC_TALK_MIC_CH			(0xFF)
#endif

#ifndef ANC_REF_MIC_CH
#define ANC_REF_MIC_CH			(0xFF)
#endif

#ifndef ANC_SPK_CH
#define ANC_SPK_CH				(AUD_CHANNEL_MAP_CH0)
#endif

static uint32_t anc_mic_cfg		= 0;
static uint32_t app_mic_cfg 	= 0;
static uint32_t all_mic_cfg 	= 0;
static uint32_t anc_mic_ch_num 	= 0;
static uint32_t app_mic_ch_num	= 0;
static uint32_t all_mic_ch_num 	= 0;

static uint32_t ff_l_mic_index	= MIC_INDEX_QTY;
static uint32_t ff_r_mic_index  = MIC_INDEX_QTY;
static uint32_t fb_l_mic_index	= MIC_INDEX_QTY;
static uint32_t fb_r_mic_index  = MIC_INDEX_QTY;
static uint32_t talk_mic_index	= MIC_INDEX_QTY;
static uint32_t ref_mic_index 	= MIC_INDEX_QTY;

static uint32_t codec_dev_get_mic_cfg(enum AUD_IO_PATH_T path)
{
    if (path == AUD_IO_PATH_NULL) {
        return 0;
    } else {
        return hal_codec_get_input_path_cfg(path) & AUD_CHANNEL_MAP_ALL;
    }
}

static uint32_t codec_dev_get_mic_path_ch_num(enum AUD_IO_PATH_T path)
{
    return hal_codec_get_input_path_chan_num(path);
}

static uint32_t codec_dev_get_mic_map_ch_num(uint32_t ch_map)
{
    return hal_codec_get_input_map_chan_num(ch_map);
}

int32_t anc_assist_mic_reset(void)
{
	STATIC_ASSERT(ANC_FF_MIC_CH_L 	!= 0xFF, "Need define ANC_FF_MIC_CH_L in tgt_hardware.h");
	STATIC_ASSERT(ANC_FF_MIC_CH_R 	!= 0xFF, "Need define ANC_FF_MIC_CH_L in tgt_hardware.h");
	STATIC_ASSERT(ANC_FB_MIC_CH_L 	!= 0xFF, "Need define ANC_FB_MIC_CH_L in tgt_hardware.h");
	STATIC_ASSERT(ANC_FB_MIC_CH_R 	!= 0xFF, "Need define ANC_FB_MIC_CH_L in tgt_hardware.h");
	STATIC_ASSERT(ANC_TALK_MIC_CH 	!= 0xFF, "Need define ANC_TALK_MIC_CH in tgt_hardware.h");
	STATIC_ASSERT(ANC_REF_MIC_CH 	!= 0xFF, "Need define ANC_REF_MIC_CH  in tgt_hardware.h");

	anc_mic_cfg		= codec_dev_get_mic_cfg(AUD_INPUT_PATH_ANC_ASSIST);
	app_mic_cfg 	= 0;

	anc_mic_ch_num 	= codec_dev_get_mic_path_ch_num(AUD_INPUT_PATH_ANC_ASSIST);
	app_mic_ch_num	= 0;

	all_mic_cfg 	= anc_mic_cfg;
	all_mic_ch_num 	= anc_mic_ch_num;

	TRACE(4, "[%s] CH MAP: 0x%x", __func__, anc_mic_cfg);
	TRACE(4, "[%s] CH NUM: %d", __func__, anc_mic_ch_num);

	// TODO: Check ANC VMIC includes other VMIC

	return 0;
}

int32_t anc_assist_mic_parser_app_buf(void *buf, uint32_t *len, void *tmp_buf, uint32_t tmp_buf_len)
{
	uint32_t all_cfg = all_mic_cfg;
	uint32_t app_cfg = app_mic_cfg;
	uint32_t all_ch	= 0;
	uint32_t app_ch = 0;
	_PCM_T *pcm_buf = (_PCM_T *)buf;
	uint32_t frame_len = *len / sizeof(_PCM_T) / all_mic_ch_num;
	_PCM_T *tmp_pcm_buf = (_PCM_T *)tmp_buf;
	uint32_t tmp_pcm_buf_len = tmp_buf_len / sizeof(_PCM_T);

	ASSERT(tmp_pcm_buf_len >= frame_len * app_mic_ch_num,
			"[%s] tmp_pcm_buf_len(%d) < frame_len(%d) * app_mic_ch_num(%d)",
			__func__, tmp_pcm_buf_len, frame_len, app_mic_ch_num);

	// TRACE(4, "%x,%x; %d,%d", all_mic_cfg, app_mic_cfg, all_mic_ch_num, app_mic_ch_num);

	while (app_cfg) {
		if (all_cfg & 0x1) {
			if (app_cfg & 0x1) {
				for (uint32_t i=0; i<frame_len; i++) {
					tmp_pcm_buf[frame_len * app_ch + i] = pcm_buf[i * all_mic_ch_num + all_ch];
				}
				app_ch++;
			}
			all_ch++;
		}

		all_cfg >>= 1;
		app_cfg >>= 1;
	}

	ASSERT(app_ch == app_mic_ch_num, "[%s] app_ch(%d) != app_mic_ch_num(%d)", __func__, app_ch, app_mic_ch_num);

	for (uint32_t ch=0; ch<app_ch; ch++) {	
		for (uint32_t i=0; i<frame_len; i++) {
			pcm_buf[i * app_ch + ch] = tmp_pcm_buf[frame_len * ch + i];
		}
	}

	*len = frame_len * sizeof(_PCM_T) * app_mic_ch_num;

	return 0;
}

int32_t anc_assist_mic_parser_anc_buf(enum AUD_IO_PATH_T path, float *anc_buf, uint32_t anc_frame_len, _PCM_T *pcm_buf, uint32_t pcm_len)
{
	uint32_t frame_len = 0;
	uint32_t ch_num = 0;
	float *anc_buf_ptr = NULL;

	if (path == AUD_INPUT_PATH_ANC_ASSIST) {
		ch_num = anc_mic_ch_num;
	} else {
		ch_num = all_mic_ch_num;
	}

	frame_len = pcm_len / ch_num;

	for (int32_t ch=0; ch<ch_num; ch++) {
		if (ch == ff_l_mic_index) {
			anc_buf_ptr = anc_buf + anc_frame_len * MIC_INDEX_FF_L;
		} else if (ch == ff_r_mic_index) {
			anc_buf_ptr = anc_buf + anc_frame_len * MIC_INDEX_FF_R;
		} else if (ch == fb_l_mic_index) {
			anc_buf_ptr = anc_buf + anc_frame_len * MIC_INDEX_FB_L;
		} else if (ch == fb_r_mic_index) {
			anc_buf_ptr = anc_buf + anc_frame_len * MIC_INDEX_FB_R;
		} else if (ch == talk_mic_index) {
			anc_buf_ptr = anc_buf + anc_frame_len * MIC_INDEX_TALK;
		} else if (ch == ref_mic_index) {
			anc_buf_ptr = anc_buf + anc_frame_len * MIC_INDEX_REF;
		} else {
			anc_buf = NULL;	
		}

		if (anc_buf) {
			for (int32_t i=0; i<frame_len; i++) {
				anc_buf_ptr[i] = (float)pcm_buf[i* ch_num + ch];
			}
		}		
	}

	return 0;
}

int32_t anc_assist_mic_parser_index(enum AUD_IO_PATH_T path)
{
	uint32_t pos 	= 0;
	uint32_t index 	= 0;
	uint32_t mic_cfg = 0;

	if (path == AUD_INPUT_PATH_ANC_ASSIST) {
		mic_cfg = anc_mic_cfg;
	} else {
		mic_cfg = all_mic_cfg;
	}

	ff_l_mic_index	= MIC_INDEX_QTY;
	ff_r_mic_index  = MIC_INDEX_QTY;
	fb_l_mic_index	= MIC_INDEX_QTY;
	fb_r_mic_index  = MIC_INDEX_QTY;
	talk_mic_index	= MIC_INDEX_QTY;
	ref_mic_index 	= MIC_INDEX_QTY;

	while (mic_cfg) {
		if (mic_cfg & 0x1) {
			if ((0x1 << pos) & ANC_FF_MIC_CH_L) {
				ff_l_mic_index = index;
			} else if ((0x1 << pos) & ANC_FF_MIC_CH_R) {
				ff_r_mic_index = index;
			} else if ((0x1 << pos) & ANC_FB_MIC_CH_L) {
				fb_l_mic_index = index;
			} else if ((0x1 << pos) & ANC_FB_MIC_CH_R) {
				fb_r_mic_index = index;
			} else if ((0x1 << pos) & ANC_TALK_MIC_CH) {
				talk_mic_index = index;
			} else if ((0x1 << pos) & ANC_REF_MIC_CH) {
				ref_mic_index = index;
			}

			index++;
		}

		pos++;
		mic_cfg >>= 1;
	}

	TRACE(7, "[%s] MIC INDEX: ff_l: %d, ff_r: %d, fb_l: %d, fb_r: %d, talk: %d, ref: %d",
		__func__, ff_l_mic_index, ff_r_mic_index, fb_l_mic_index, fb_r_mic_index, talk_mic_index, ref_mic_index);

	return 0;
}

int32_t anc_assist_mic_set_app_cfg(enum AUD_IO_PATH_T path)
{

    app_mic_cfg = codec_dev_get_mic_cfg(path);
    app_mic_ch_num = codec_dev_get_mic_map_ch_num(app_mic_cfg);

	// Update all mic cfg and channel number
	all_mic_cfg = anc_mic_cfg | app_mic_cfg;
	all_mic_ch_num = codec_dev_get_mic_map_ch_num(all_mic_cfg);

	TRACE(3, "[%s] CH NUM: app: %d, all: %d", __func__, app_mic_ch_num, all_mic_ch_num);
	TRACE(3, "[%s] CH MAP: app: 0x%x, all: 0x%x", __func__, app_mic_cfg, all_mic_cfg);

	return 0;
}

int32_t anc_assist_mic_set_anc_cfg(enum AUD_IO_PATH_T path)
{
	ASSERT(path == AUD_INPUT_PATH_ANC_ASSIST, "[%s] path: %d", __func__, path);

	anc_mic_cfg = codec_dev_get_mic_cfg(path);
	anc_mic_ch_num = codec_dev_get_mic_map_ch_num(anc_mic_cfg);

	// Update all mic cfg and channel number
	all_mic_cfg = anc_mic_cfg | app_mic_cfg;
	all_mic_ch_num = codec_dev_get_mic_map_ch_num(all_mic_cfg);

	TRACE(3, "[%s] CH NUM: anc: %d, all: %d", __func__, anc_mic_ch_num, all_mic_ch_num);
	TRACE(3, "[%s] CH MAP: anc: 0x%x, all: 0x%x", __func__, anc_mic_cfg, all_mic_cfg);

	return 0;
}

uint32_t anc_assist_mic_parser_anc_cfg(AncAssistConfig *cfg, ThirdpartyAssistConfig *thirdparty_cfg)
{
	uint32_t mic_cfg = 0;

	if (cfg->ff_howling_en) {
		mic_cfg |= ANC_FF_MIC_CH_L;
#if !defined(IBRT)
		mic_cfg |= ANC_FF_MIC_CH_R;
#endif
	}
	if (thirdparty_cfg->en) { 
        mic_cfg |= 0; 
	}

	if (cfg->fb_howling_en) {
		mic_cfg |= ANC_FB_MIC_CH_L;
#if !defined(IBRT)
		mic_cfg |= ANC_FB_MIC_CH_R;
#endif
	}

	if (cfg->noise_en) {
		mic_cfg |= ANC_FF_MIC_CH_L;
	}

	if (cfg->noise_classify_en) {
		mic_cfg |= ANC_FF_MIC_CH_L;
	}

	if (cfg->wind_en) {
		mic_cfg |= ANC_FF_MIC_CH_L | ANC_TALK_MIC_CH;
	}

	if (cfg->pilot_en || cfg->ultrasound_en) {
		mic_cfg |= ANC_FB_MIC_CH_L | ANC_REF_MIC_CH;
#if !defined(IBRT)
		mic_cfg |= ANC_FB_MIC_CH_R;
#endif
	}

	if (cfg->prompt_en) {
		mic_cfg |= ANC_FB_MIC_CH_L | ANC_FF_MIC_CH_L | ANC_REF_MIC_CH;
	}

	if (cfg->wsd_en) {
		mic_cfg |= ANC_FF_MIC_CH_L | ANC_FB_MIC_CH_L;
	}

	if (cfg->extern_kws_en) {
		mic_cfg |= ANC_TALK_MIC_CH; //NOTE: Add customer mic cfg
	}

	if (cfg->extern_adaptive_eq_en) {
		mic_cfg |= ANC_FB_MIC_CH_L | ANC_REF_MIC_CH; //NOTE: Add customer mic cfg
	}

	// Workaround for phone call and record
	// anc_mic_cfg |= ANC_FF_MIC_CH_L | ANC_FB_MIC_CH_L | ANC_TALK_MIC_CH | ANC_REF_MIC_CH;

	return mic_cfg;
}

uint32_t anc_assist_spk_parser_anc_cfg(AncAssistConfig *cfg, ThirdpartyAssistConfig *thirdparty_cfg)
{
	uint32_t spk_cfg = 0;

	if (cfg->pilot_en || cfg->ultrasound_en) {
		spk_cfg |= ANC_SPK_CH;
	}

	if (thirdparty_cfg->en) {
		spk_cfg |= 0;
	}
	return spk_cfg;
}

int32_t anc_assist_mic_update_anc_cfg(AncAssistConfig *cfg, ThirdpartyAssistConfig *thirdparty_cfg)
{
	anc_mic_cfg = anc_assist_mic_parser_anc_cfg(cfg, thirdparty_cfg);
	anc_mic_ch_num = codec_dev_get_mic_map_ch_num(anc_mic_cfg);

	// Update all mic cfg and channel number
	all_mic_cfg = anc_mic_cfg | app_mic_cfg;
	all_mic_ch_num = codec_dev_get_mic_map_ch_num(all_mic_cfg);

	TRACE(3, "[%s] CH NUM: anc: %d, all: %d", __func__, anc_mic_ch_num, all_mic_ch_num);
	TRACE(3, "[%s] CH MAP: anc: 0x%x, all: 0x%x", __func__, anc_mic_cfg, all_mic_cfg);

	return 0;
}

uint32_t anc_assist_mic_get_cfg(enum AUD_IO_PATH_T path)
{
	if (path == AUD_INPUT_PATH_ANC_ASSIST) {
		return anc_mic_cfg;
	} else {
		return all_mic_cfg;
	}
}

uint32_t anc_assist_mic_get_ch_num(enum AUD_IO_PATH_T path)
{
	if (path == AUD_INPUT_PATH_ANC_ASSIST) {
		return anc_mic_ch_num;
	} else {
		return all_mic_ch_num;
	}
}

uint32_t anc_assist_mic_anc_mic_is_enabled(anc_assist_mic_index_t index)
{
	if ((index == MIC_INDEX_FF_L) && (anc_mic_cfg & ANC_FF_MIC_CH_L)) {
		return true;
	} else if ((index == MIC_INDEX_FF_R) && (anc_mic_cfg & ANC_FF_MIC_CH_R)) {
		return true;
	} else if ((index == MIC_INDEX_FB_L) && (anc_mic_cfg & ANC_FB_MIC_CH_L)) {
		return true;
	} else if ((index == MIC_INDEX_FB_R) && (anc_mic_cfg & ANC_FB_MIC_CH_R)) {
		return true;
	} else if ((index == MIC_INDEX_TALK) && (anc_mic_cfg & ANC_TALK_MIC_CH)) {
		return true;
	} else if ((index == MIC_INDEX_REF) && (anc_mic_cfg & ANC_REF_MIC_CH)) {
		return true;
	} else {
		return false;
	}
}