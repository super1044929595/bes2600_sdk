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
#include "hal_trace.h"
#include "anc_assist_resample.h"
#include "anc_assist_mic.h"
#include "integer_resampling.h"

static IntegerResamplingState *resample_st = NULL;

int32_t anc_assist_resample_init(uint32_t sample_rate, uint32_t frame_len, custom_allocator *allocator, uint32_t channel_num)
{
	TRACE(0, "[%s] sample_rate: %d, frame_len: %d", __func__, sample_rate, frame_len);
	INTEGER_RESAMPLING_ITEM_T item = INTEGER_RESAMPLING_ITEM_32K_TO_16K;

	if (sample_rate == 32000) {
		item = INTEGER_RESAMPLING_ITEM_32K_TO_16K;
	} else if (sample_rate == 48000) {
		item = INTEGER_RESAMPLING_ITEM_48K_TO_16K;
	} else if (sample_rate == 96000) {
		item = INTEGER_RESAMPLING_ITEM_96K_TO_16K;
	} else {
		TRACE(0, "[%s] TODO: INTEGER_RESAMPLING_ITEM_16K_TO_8K", __func__);
	}

	resample_st = integer_resampling_create_with_custom_allocator(frame_len, channel_num, item, allocator);

	return 0;
}

int32_t anc_assist_resample_deinit(void)
{
	resample_st = NULL;
	return 0;
}


int32_t anc_assist_resample_process(int32_t *pcm_buf_in,int32_t *pcm_buf_out, uint32_t inbuf_frame_len)
{
	if (resample_st) {
		integer_resampling_process_q23(resample_st, pcm_buf_in, inbuf_frame_len, pcm_buf_out);
	}

	return 0;
}
