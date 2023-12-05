/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#ifndef __ANALOG_BEST1305_H__
#define __ANALOG_BEST1305_H__

#ifdef __cplusplus
extern "C" {
#endif

#define ISPI_ANA_REG(r)                     (((r) & 0xFFF) | 0x1000)

#define MAX_ANA_MIC_CH_NUM                  3

typedef void (*ANALOG_ANC_BOOST_DELAY_FUNC)(uint32_t ms);

enum ANA_AUD_PLL_USER_T {
    ANA_AUD_PLL_USER_CODEC      = (1 << 0),
    ANA_AUD_PLL_USER_I2S        = (1 << 1),
    ANA_AUD_PLL_USER_SPDIF      = (1 << 2),
    ANA_AUD_PLL_USER_PCM        = (1 << 3),
    ANA_AUD_PLL_USER_IIR        = (1 << 4),
    ANA_AUD_PLL_USER_IIR_EQ     = (1 << 5),
    ANA_AUD_PLL_USER_RS_DAC     = (1 << 6),
    ANA_AUD_PLL_USER_RS_ADC     = (1 << 7),

    ANA_AUD_PLL_USER_END        = (1 << 8),
};
#define ANA_AUD_PLL_USER_T                  ANA_AUD_PLL_USER_T

void analog_aud_pll_set_dig_div(uint32_t div);

uint32_t analog_aud_get_max_dre_gain(void);

void analog_aud_codec_anc_boost(bool en, ANALOG_ANC_BOOST_DELAY_FUNC delay_func);

int analog_debug_config_vad_mic(bool enable);

void analog_aud_vad_enable(enum AUD_VAD_TYPE_T type, bool en);

void analog_aud_vad_adc_enable(bool en);

#ifdef __cplusplus
}
#endif

#endif

