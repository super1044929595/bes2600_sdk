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
#ifndef __AUDIO_CFG_H__
#define __AUDIO_CFG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"


enum AUDIO_PROCESS_TYPE_T {
    AUDIO_PROCESS_TYPE_IIR_EQ = 0,
    AUDIO_PROCESS_TYPE_DRC,
    AUDIO_PROCESS_TYPE_LIMITER,
    AUDIO_PROCESS_TYPE_HFP_CFG,

    AUDIO_PROCESS_TYPE_NUM,
};

#define TOOL_SUPPORT_MAX_IIR_EQ_BAND_NUM    (10)

#define SCO_CODEC_NUM (2)
#define SCO_MODE_NUM (3) // off/anc/talk_through


struct AUDIO_CFG_T_;
typedef struct AUDIO_CFG_T_ AUDIO_CFG_T;

int sizeof_audio_cfg(void);
int sizeof_audio_section(void);
int store_audio_cfg_into_audio_section(AUDIO_CFG_T *cfg);
void *load_audio_cfg_from_audio_section(enum AUDIO_PROCESS_TYPE_T type, uint8_t index);
void audio_cfg_get_eq_section_info(uint32_t *startAddr, uint32_t *length, uint16_t *version);
void *audio_cfg_get_default_audio_section(void);

#ifdef __cplusplus
}
#endif

#endif
