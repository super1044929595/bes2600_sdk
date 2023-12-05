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
#include "analog.h"
#include CHIP_SPECIFIC_HDR(reg_analog)
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_chipid.h"
#include "hal_cmu.h"
#include "hal_codec.h"
#include "hal_sysfreq.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "pmu.h"
#include "tgt_hardware.h"


#define VCM_ON

// Not using 1uF
#define VCM_CAP_100NF

#define DAC_DC_CALIB_BIT_WIDTH              14

#define DAC_DC_ADJUST_STEP                  90

#define DEFAULT_ANC_FF_ADC_GAIN_DB          6
#define DEFAULT_ANC_FB_ADC_GAIN_DB          6
#define DEFAULT_ANC_TT_ADC_GAIN_DB          6
#define DEFAULT_VOICE_ADC_GAIN_DB           12

#ifndef ANALOG_ADC_A_GAIN_DB
#if defined(ANC_APP) && defined(ANC_FF_ENABLED) && ((ANC_FF_MIC_CH_L == AUD_CHANNEL_MAP_CH0) || (ANC_FF_MIC_CH_R == AUD_CHANNEL_MAP_CH0))
#define ANALOG_ADC_A_GAIN_DB                DEFAULT_ANC_FF_ADC_GAIN_DB
#elif defined(ANC_APP) && defined(ANC_FB_ENABLED) && ((ANC_FB_MIC_CH_L == AUD_CHANNEL_MAP_CH0) || (ANC_FB_MIC_CH_R == AUD_CHANNEL_MAP_CH0))
#define ANALOG_ADC_A_GAIN_DB                DEFAULT_ANC_FB_ADC_GAIN_DB
#elif defined(ANC_APP) && (defined(AUDIO_ANC_TT_HW)||defined(PSAP_APP)) && ((ANC_TT_MIC_CH_L == AUD_CHANNEL_MAP_CH0) || (ANC_TT_MIC_CH_R == AUD_CHANNEL_MAP_CH0))
#define ANALOG_ADC_A_GAIN_DB                DEFAULT_ANC_TT_ADC_GAIN_DB
#else
#define ANALOG_ADC_A_GAIN_DB                DEFAULT_VOICE_ADC_GAIN_DB
#endif
#endif

#ifndef ANALOG_ADC_B_GAIN_DB
#if defined(ANC_APP) && defined(ANC_FF_ENABLED) && ((ANC_FF_MIC_CH_L == AUD_CHANNEL_MAP_CH1) || (ANC_FF_MIC_CH_R == AUD_CHANNEL_MAP_CH1))
#define ANALOG_ADC_B_GAIN_DB                DEFAULT_ANC_FF_ADC_GAIN_DB
#elif defined(ANC_APP) && defined(ANC_FB_ENABLED) && ((ANC_FB_MIC_CH_L == AUD_CHANNEL_MAP_CH1) || (ANC_FB_MIC_CH_R == AUD_CHANNEL_MAP_CH1))
#define ANALOG_ADC_B_GAIN_DB                DEFAULT_ANC_FB_ADC_GAIN_DB
#elif defined(ANC_APP) && (defined(AUDIO_ANC_TT_HW)||defined(PSAP_APP)) && ((ANC_TT_MIC_CH_L == AUD_CHANNEL_MAP_CH1) || (ANC_TT_MIC_CH_R == AUD_CHANNEL_MAP_CH1))
#define ANALOG_ADC_B_GAIN_DB                DEFAULT_ANC_TT_ADC_GAIN_DB
#else
#define ANALOG_ADC_B_GAIN_DB                DEFAULT_VOICE_ADC_GAIN_DB
#endif
#endif

#ifndef ANALOG_ADC_C_GAIN_DB
#if defined(ANC_APP) && defined(ANC_FF_ENABLED) && ((ANC_FF_MIC_CH_L == AUD_CHANNEL_MAP_CH2) || (ANC_FF_MIC_CH_R == AUD_CHANNEL_MAP_CH2))
#define ANALOG_ADC_C_GAIN_DB                DEFAULT_ANC_FF_ADC_GAIN_DB
#elif defined(ANC_APP) && defined(ANC_FB_ENABLED) && ((ANC_FB_MIC_CH_L == AUD_CHANNEL_MAP_CH2) || (ANC_FB_MIC_CH_R == AUD_CHANNEL_MAP_CH2))
#define ANALOG_ADC_C_GAIN_DB                DEFAULT_ANC_FB_ADC_GAIN_DB
#elif defined(ANC_APP) && (defined(AUDIO_ANC_TT_HW)||defined(PSAP_APP))  && ((ANC_TT_MIC_CH_L == AUD_CHANNEL_MAP_CH2) || (ANC_TT_MIC_CH_R == AUD_CHANNEL_MAP_CH2))
#define ANALOG_ADC_C_GAIN_DB                DEFAULT_ANC_TT_ADC_GAIN_DB
#else
#define ANALOG_ADC_C_GAIN_DB                DEFAULT_VOICE_ADC_GAIN_DB
#endif
#endif

#ifndef LINEIN_ADC_GAIN_DB
#define LINEIN_ADC_GAIN_DB                  0
#endif

#ifndef CFG_HW_AUD_MICKEY_DEV
#define CFG_HW_AUD_MICKEY_DEV               (AUD_VMIC_MAP_VMIC1)
#endif

#ifndef ANC_VMIC_CFG
#define ANC_VMIC_CFG                        (AUD_VMIC_MAP_VMIC1)
#endif

#define CODEC_TX_EN_ANA_LDAC                RESERVED_ANA_15_0(1 << 2)

enum ANA_CODEC_USER_T {
    ANA_CODEC_USER_DAC          = (1 << 0),
    ANA_CODEC_USER_ADC          = (1 << 1),

    ANA_CODEC_USER_CODEC        = (1 << 2),
    ANA_CODEC_USER_MICKEY       = (1 << 3),

    ANA_CODEC_USER_ANC_FF       = (1 << 4),
    ANA_CODEC_USER_ANC_FB       = (1 << 5),
    ANA_CODEC_USER_ANC_TT       = (1 << 6),
};

struct ANALOG_PLL_CFG_T {
    uint32_t freq;
    uint8_t div;
    uint64_t val;
};

#ifdef ANC_PROD_TEST
#define OPT_TYPE
#else
#define OPT_TYPE                        const
#endif

static OPT_TYPE uint16_t vcodec_mv = (uint16_t)(VCODEC_VOLT * 1000);

static bool ana_spk_req;
static bool ana_spk_muted;
static bool ana_spk_enabled;

static bool anc_calib_mode;

static enum ANA_CODEC_USER_T adc_map[MAX_ANA_MIC_CH_NUM];
static enum ANA_CODEC_USER_T vmic_map[MAX_VMIC_CH_NUM];
static enum ANA_CODEC_USER_T codec_common_map;
static enum ANA_CODEC_USER_T adda_common_map;
static enum ANA_CODEC_USER_T vcodec_map;

static enum ANA_AUD_PLL_USER_T ana_aud_pll_map;

#ifdef ANC_APP
#ifndef DYN_ADC_GAIN
#define DYN_ADC_GAIN
#endif
#if defined(ANC_FF_MIC_CH_L)
#if defined(ANC_TT_MIC_CH_L)
static int8_t anc_tt_gain_offset_l;
#endif
#endif
#endif

#ifdef DYN_ADC_GAIN
static int8_t adc_gain_offset[MAX_ANA_MIC_CH_NUM];
#endif

#ifdef VOICE_DETECTOR_EN
static bool vad_started;
#endif

static const int8_t adc_db[] = { -9, -6, -3, 0, 3, 6, 9, 12, };

static const int8_t tgt_adc_db[MAX_ANA_MIC_CH_NUM] = {
    ANALOG_ADC_A_GAIN_DB, ANALOG_ADC_B_GAIN_DB,
    ANALOG_ADC_C_GAIN_DB,
};

// Max allowed total tune ratio (5000ppm)
#define MAX_TOTAL_TUNE_RATIO                0.005000

static struct ANALOG_PLL_CFG_T ana_pll_cfg[2];
static int pll_cfg_idx;

static void audpll_pll_update(uint64_t pll_cfg_val, bool calib)
{
    uint16_t low, high, high2;

    low = pll_cfg_val & 0xFFFF;
    high = (pll_cfg_val >> 16) & 0xFFFF;
    high2 = (pll_cfg_val >> 32) & 0xFFFF;

    pmu_pll_freq_reg_set(low, high, high2);
}

void analog_aud_freq_pll_config(uint32_t freq, uint32_t div)
{
    // CODEC_FREQ is likely 24.576M (48K series) or 22.5792M (44.1K series)
    // PLL_nominal = CODEC_FREQ * CODEC_DIV
    // PLL_cfg_val = ((CODEC_FREQ * CODEC_DIV) / OSC) * (1 << 28)

    int i, j;
    uint64_t PLL_cfg_val;
    uint32_t crystal;


    if (pll_cfg_idx < ARRAY_SIZE(ana_pll_cfg) &&
            ana_pll_cfg[pll_cfg_idx].freq == freq &&
            ana_pll_cfg[pll_cfg_idx].div == div) {
        pmu_pll_codec_clock_enable(true);
        return;
    }

    crystal = hal_cmu_get_crystal_freq();

    j = ARRAY_SIZE(ana_pll_cfg);
    for (i = 0; i < ARRAY_SIZE(ana_pll_cfg); i++) {
        if (ana_pll_cfg[i].freq == freq && ana_pll_cfg[i].div == div) {
            break;
        }
        if (j == ARRAY_SIZE(ana_pll_cfg) && ana_pll_cfg[i].freq == 0) {
            j = i;
        }
    }

    if (i < ARRAY_SIZE(ana_pll_cfg)) {
        pll_cfg_idx = i;
        PLL_cfg_val = ana_pll_cfg[pll_cfg_idx].val;
    } else {
        if (j < ARRAY_SIZE(ana_pll_cfg)) {
            pll_cfg_idx = j;
        } else {
            pll_cfg_idx = 0;
        }

        PLL_cfg_val = ((uint64_t)(1 << 28) * (freq * div / 2) + crystal / 2) / crystal;

        ana_pll_cfg[pll_cfg_idx].freq = freq;
        ana_pll_cfg[pll_cfg_idx].div = div;
        ana_pll_cfg[pll_cfg_idx].val = PLL_cfg_val;
    }
    pmu_pll_div_set(HAL_CMU_PLL_AUD, PMU_PLL_DIV_CODEC, div);
    audpll_pll_update(PLL_cfg_val, true);
    pmu_pll_codec_clock_enable(true);
}

void analog_aud_pll_tune(float ratio)
{
#ifdef __AUDIO_RESAMPLE__
    if (hal_cmu_get_audio_resample_status()) {
        return;
    }
#endif

    // CODEC_FREQ is likely 24.576M (48K series) or 22.5792M (44.1K series)
    // PLL_nominal = CODEC_FREQ * CODEC_DIV
    // PLL_cfg_val = ((CODEC_FREQ * CODEC_DIV) / 26M) * (1 << 28)
    // Delta = ((SampleDiff / Fs) / TimeDiff) * PLL_cfg_val

    int64_t delta, new_pll;

    if (pll_cfg_idx >= ARRAY_SIZE(ana_pll_cfg) ||
            ana_pll_cfg[pll_cfg_idx].freq == 0) {
        ANALOG_INFO_TRACE(1,"%s: WARNING: aud pll config cache invalid. Skip tuning", __FUNCTION__);
        return;
    }

    if (ABS(ratio) > MAX_TOTAL_TUNE_RATIO) {
        ANALOG_INFO_TRACE(1,"\n------\nWARNING: TUNE: ratio=%d is too large and will be cut\n------\n", FLOAT_TO_PPB_INT(ratio));
        if (ratio > 0) {
            ratio = MAX_TOTAL_TUNE_RATIO;
        } else {
            ratio = -MAX_TOTAL_TUNE_RATIO;
        }
    }

    ANALOG_INFO_TRACE(2,"%s: ratio=%d", __FUNCTION__, FLOAT_TO_PPB_INT(ratio));

    new_pll = (int64_t)ana_pll_cfg[pll_cfg_idx].val;
    delta = (int64_t)(new_pll * ratio);

    new_pll += delta;

    audpll_pll_update(new_pll, false);
}

void analog_aud_pll_set_dig_div(uint32_t div)
{
}

void analog_aud_osc_clk_enable(bool enable)
{
    uint16_t val;

    if (enable) {
        analog_read(ANA_REG_16D, &val);
        val |= REG_PU_OSC | REG_CRYSTAL_SEL_LV;
        analog_write(ANA_REG_16D, val);
#if 0
        analog_read(ANA_REG_176, &val);
        val |= CFG_TX_CLK_INV;
        analog_write(ANA_REG_176, val);
#endif
    } else {
#if 0
        analog_read(ANA_REG_176, &val);
        val &= ~CFG_TX_CLK_INV;
        analog_write(ANA_REG_176, val);
#endif
        analog_read(ANA_REG_16D, &val);
        val &= ~(REG_PU_OSC | REG_CRYSTAL_SEL_LV);
        analog_write(ANA_REG_16D, val);
    }
}

void analog_aud_codec_select_pll_clock(bool pll)
{
    uint16_t val;

    analog_read(ANA_REG_AD, &val);
    if (pll) {
        val |= (PU_OSC_PLL24MDAC|PU_OSC_PLL24MADC);
    } else {
        val &= ~(PU_OSC_PLL24MDAC|PU_OSC_PLL24MADC);
    }
    analog_write(ANA_REG_AD, val);
}

void analog_aud_pll_open(enum ANA_AUD_PLL_USER_T user)
{
    if (user >= ANA_AUD_PLL_USER_END) {
        return;
    }

    if (user == ANA_AUD_PLL_USER_CODEC) {
#ifdef __AUDIO_RESAMPLE__
        if (hal_cmu_get_audio_resample_status()) {
            analog_aud_osc_clk_enable(true);
            return;
        } else
#endif
        {
            analog_aud_codec_select_pll_clock(true);
        }
    }

    if (ana_aud_pll_map == 0) {
        hal_cmu_pll_enable(HAL_CMU_PLL_AUD, HAL_CMU_PLL_USER_AUD);
    }
    ana_aud_pll_map |= user;
}

void analog_aud_pll_close(enum ANA_AUD_PLL_USER_T user)
{
    if (user >= ANA_AUD_PLL_USER_END) {
        return;
    }

    if (user == ANA_AUD_PLL_USER_CODEC) {
#ifdef __AUDIO_RESAMPLE__
        if (hal_cmu_get_audio_resample_status()) {
            analog_aud_osc_clk_enable(false);
            return;
        } else
#endif
        {
            analog_aud_codec_select_pll_clock(false);
        }
    }

    if (ana_aud_pll_map) {
        ana_aud_pll_map &= ~user;
        if (ana_aud_pll_map == 0) {
            hal_cmu_pll_disable(HAL_CMU_PLL_AUD, HAL_CMU_PLL_USER_AUD);
            pmu_pll_codec_clock_enable(false);
        }
    }
}

static void analog_aud_enable_dac_with_classab(uint32_t dac, bool switch_pa)
{
    uint16_t val_16e;
    uint16_t val_172;
    uint16_t val_175;
    uint16_t val_176;

    analog_read(ANA_REG_16E, &val_16e);
    analog_read(ANA_REG_172, &val_172);
    analog_read(ANA_REG_175, &val_175);
    analog_read(ANA_REG_176, &val_176);

    if (dac & (AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1)) {
        val_16e |= REG_CODEC_TX_EAR_DR_EN | REG_CODEC_TX_EAR_ENBIAS | REG_CODEC_TX_EAR_LPBIAS;
        analog_write(ANA_REG_16E, val_16e);
        osDelay(1);
        val_176 |= CFG_TX_TREE_EN;
        analog_write(ANA_REG_176, val_176);
        osDelay(1);

        if (dac & AUD_CHANNEL_MAP_CH0) {
            val_175 |= REG_CODEC_TX_EN_EARPA_L | REG_CODEC_TX_EN_LCLK;
            if (!switch_pa) {
                val_175 |= REG_CODEC_TX_EN_LDAC;
            }
        }
        analog_write(ANA_REG_175, val_175);
        val_172 |= REG_CODEC_TX_EN_DACLDO;
        analog_write(ANA_REG_172, val_172);
        val_176 |= REG_CODEC_TX_EN_LPPA;
        analog_write(ANA_REG_176, val_176);
        osDelay(1);
        val_175 |= REG_CODEC_TX_EN_S1PA;
        val_175 |= REG_CODEC_TX_EN_S2PA;
        analog_write(ANA_REG_175, val_175);
        // Ensure 1ms delay before enabling dac_pa
        osDelay(1);
    } else {
        // Ensure 1ms delay after disabling dac_pa
        osDelay(1);
        val_175 &= ~REG_CODEC_TX_EN_S1PA;
        analog_write(ANA_REG_175, val_175);
        osDelay(1);
        val_175 &= ~(REG_CODEC_TX_EN_EARPA_L | REG_CODEC_TX_EN_LCLK);
        if (!switch_pa) {
            val_175 &= ~REG_CODEC_TX_EN_LDAC;
        }
        analog_write(ANA_REG_175, val_175);
        val_172 &= ~REG_CODEC_TX_EN_DACLDO;
        analog_write(ANA_REG_172, val_172);
        val_176 &= ~REG_CODEC_TX_EN_LPPA;
        analog_write(ANA_REG_176, val_176);
        osDelay(1);

        val_176 &= ~CFG_TX_TREE_EN;
        analog_write(ANA_REG_176, val_176);
        osDelay(1);

        val_16e &= ~(REG_CODEC_TX_EAR_DR_EN | REG_CODEC_TX_EAR_ENBIAS | REG_CODEC_TX_EAR_LPBIAS);
        analog_write(ANA_REG_16E, val_16e);
    }
}

static void analog_aud_enable_dac_pa_classab(uint32_t dac)
{
    uint16_t val_175;

    analog_read(ANA_REG_175, &val_175);

    if (dac & (AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1)) {
        val_175 |= REG_CODEC_TX_EN_S4PA;
    } else {
        val_175 &= ~REG_CODEC_TX_EN_S4PA;
    }
    analog_write(ANA_REG_175, val_175);
}

static void analog_aud_enable_dac(uint32_t dac)
{
    analog_aud_enable_dac_with_classab(dac, false);
}

static void analog_aud_enable_dac_pa_internal(uint32_t dac)
{
    analog_aud_enable_dac_pa_classab(dac);
}

static void analog_aud_enable_dac_pa(uint32_t dac)
{
    if (dac & (AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1)) {
        analog_aud_enable_dac_pa_internal(dac);

#ifdef AUDIO_OUTPUT_DC_CALIB_ANA
        hal_codec_dac_sdm_reset_clear();
#endif
    } else {
#ifdef AUDIO_OUTPUT_DC_CALIB_ANA
        hal_codec_dac_sdm_reset_set();
#endif

        analog_aud_enable_dac_pa_internal(dac);
    }
}

static void analog_aud_enable_codec_vcm_buffer(bool en)
{
    uint16_t val;

    analog_read(ANA_REG_B2, &val);
    if (en) {
        val |= REG_CODEC_EN_VCM_BUFFER;
    } else {
        val &= ~REG_CODEC_EN_VCM_BUFFER;
    }
    analog_write(ANA_REG_B2, val);
}

POSSIBLY_UNUSED
static void analog_aud_enable_codec_bias_lp(bool en)
{
    uint16_t val;

    analog_read(ANA_REG_B0, &val);
    if (en) {
        val |= REG_CODEC_EN_BIAS_LP;
    } else {
        val &= ~REG_CODEC_EN_BIAS_LP;
    }
    analog_write(ANA_REG_B0, val);
}

static void analog_aud_enable_adc(enum ANA_CODEC_USER_T user, enum AUD_CHANNEL_MAP_T ch_map, bool en)
{
    int i;
    uint16_t reg;
    uint16_t val;
    enum ANA_CODEC_USER_T old_map;
    bool set;
#if !defined(CODEC_TX_PEAK_DETECT)
    bool global_update = false;
#endif

    ANALOG_DEBUG_TRACE(3,"[%s] user=%d ch_map=0x%x", __func__, user, ch_map);

    for (i = 0; i < MAX_ANA_MIC_CH_NUM; i++) {
        if (ch_map & (AUD_CHANNEL_MAP_CH0 << i)) {
            set = false;
            if (en) {
                if (adc_map[i] == 0) {
                    set = true;
                }
                adc_map[i] |= user;
            } else {
                old_map = adc_map[i];
                adc_map[i] &= ~user;
                if (old_map != 0 && adc_map[i] == 0) {
                    set = true;
                }
            }
            if (set) {
#if !defined(CODEC_TX_PEAK_DETECT)
                if (!global_update) {
                    global_update = true;
                    if (en) {
                        analog_aud_enable_codec_vcm_buffer(true);
                        analog_aud_enable_codec_bias_lp(true);
                    }
                }
#endif
                reg = ANA_REG_61 + 0x10 * i;
                analog_read(reg, &val);
                if (adc_map[i]) {
                    val |= REG_CODEC_EN_ADCA;
                } else {
                    val &= ~REG_CODEC_EN_ADCA;
                }
                analog_write(reg, val);
            }
        }
    }

#if !defined(CODEC_TX_PEAK_DETECT)
    if (global_update && !en) {
        for (i = 0; i < MAX_ANA_MIC_CH_NUM; i++) {
            if (adc_map[i]) {
                break;
            }
        }
        if (i >= MAX_ANA_MIC_CH_NUM) {
            analog_aud_enable_codec_vcm_buffer(false);
            analog_aud_enable_codec_bias_lp(false);
        }
    }
#endif
}

static uint32_t db_to_adc_gain(int db)
{
    int i;
    uint8_t cnt;
    const int8_t *list;

    list = adc_db;
    cnt = ARRAY_SIZE(adc_db);

    for (i = 0; i < cnt - 1; i++) {
        if (db < list[i + 1]) {
            break;
        }
    }

    if (i == cnt - 1) {
        return i;
    }
    else if (db * 2 < list[i] + list[i + 1]) {
        return i;
    } else {
        return i + 1;
    }
}

static int8_t get_chan_adc_gain(uint32_t i)
{
    int8_t gain;

    gain = tgt_adc_db[i];

#ifdef DYN_ADC_GAIN
    if (adc_gain_offset[i] < 0 && -adc_gain_offset[i] > gain) {
        gain = 0;
    } else {
        gain += adc_gain_offset[i];
    }
#endif

    return gain;
}

static void analog_aud_set_adc_gain(enum AUD_IO_PATH_T input_path, enum AUD_CHANNEL_MAP_T ch_map)
{
    int i;
    int gain;
    uint16_t gain_val;
    uint16_t reg;
    uint16_t val;

    for (i = 0; i < MAX_ANA_MIC_CH_NUM; i++) {
        if (ch_map & (AUD_CHANNEL_MAP_CH0 << i)) {
            if (0) {
#ifdef ANC_APP
#ifdef ANC_FF_ENABLED
            } else if ((ANC_FF_MIC_CH_L | ANC_FF_MIC_CH_R) & (AUD_CHANNEL_MAP_CH0 << i)) {
                gain = get_chan_adc_gain(i);
#endif
#ifdef ANC_FB_ENABLED
            } else if ((ANC_FB_MIC_CH_L | ANC_FB_MIC_CH_R) & (AUD_CHANNEL_MAP_CH0 << i)) {
                gain = get_chan_adc_gain(i);
#endif
#endif
            } else if (input_path == AUD_INPUT_PATH_LINEIN) {
                gain = LINEIN_ADC_GAIN_DB;
            } else {
                gain = get_chan_adc_gain(i);
            }
            gain_val = db_to_adc_gain(gain);
            reg = ANA_REG_61 + 0x10 * i;
            analog_read(reg, &val);
            val = SET_BITFIELD(val, REG_CODEC_ADCA_GAIN_BIT, gain_val);
            analog_write(reg, val);
        }
    }
}

void analog_aud_set_adc_gain_direct(enum AUD_CHANNEL_MAP_T ch_map, int gain)
{
    int i;
    uint16_t gain_val;
    uint16_t reg;
    uint16_t val;

    for (i = 0; i < MAX_ANA_MIC_CH_NUM; i++) {
        if (ch_map & (AUD_CHANNEL_MAP_CH0 << i)) {
            gain_val = db_to_adc_gain(gain);
            reg = ANA_REG_61 + 0x10 * i;
            analog_read(reg, &val);
            val = SET_BITFIELD(val, REG_CODEC_ADCA_GAIN_BIT, gain_val);
            analog_write(reg, val);
        }
    }
}

#ifdef ANC_APP
void analog_aud_apply_anc_adc_gain_offset(enum ANC_TYPE_T type, int16_t offset_l, int16_t offset_r)
{
    enum ANC_TYPE_T single_type;
    enum AUD_CHANNEL_MAP_T ch_map;
    uint32_t l, r;
    int8_t org_l, adj_l;
    int8_t org_r, adj_r;

    // qdb to db
    offset_l /= 4;
    offset_r /= 4;

    while (type) {
        l = get_msb_pos(type);
        single_type = (1 << l);
        type &= ~single_type;

        ch_map = 0;
        l = r = 32;
        if (0) {
#if defined(ANC_FF_MIC_CH_L)
        } else if (single_type == ANC_FEEDFORWARD) {
            ch_map |= ANC_FF_MIC_CH_L;
            l = get_msb_pos(ANC_FF_MIC_CH_L);
#endif
#if defined(ANC_FB_MIC_CH_L)
        } else if (single_type == ANC_FEEDBACK) {
            ch_map = ANC_FB_MIC_CH_L;
            l = get_msb_pos(ANC_FB_MIC_CH_L);
#endif
#if defined(ANC_TT_MIC_CH_L)
        } else if (single_type == ANC_TALKTHRU) {
            ch_map = ANC_TT_MIC_CH_L;
            l = get_msb_pos(ANC_TT_MIC_CH_L);
#if defined(ANC_FF_MIC_CH_L)
            anc_tt_gain_offset_l = offset_l;
            if (adda_common_map & ANA_CODEC_USER_ANC_FF) {
                if (ANC_TT_MIC_CH_L & (ANC_FF_MIC_CH_L)) {
                    ch_map &= ~ANC_TT_MIC_CH_L;
                    l = 32;
                }
            }
#endif
#endif
        } else {
            continue;
        }

        if ((l >= MAX_ANA_MIC_CH_NUM || adc_gain_offset[l] == offset_l) &&
                (r >= MAX_ANA_MIC_CH_NUM || adc_gain_offset[r] == offset_r)) {
            continue;
        }

        ANALOG_INFO_TRACE(0, "ana: apply anc adc gain offset: type=%d offset=%d/%d", single_type, offset_l, offset_r);

        org_l = adj_l = 0;
        if (l < MAX_ANA_MIC_CH_NUM) {
            adc_gain_offset[l] = 0;
            if (offset_l) {
                org_l = adc_db[db_to_adc_gain(get_chan_adc_gain(l))];
                adc_gain_offset[l] = offset_l;
                adj_l = adc_db[db_to_adc_gain(get_chan_adc_gain(l))];
            }
        }

        org_r = adj_r = 0;
        if (r < MAX_ANA_MIC_CH_NUM) {
            adc_gain_offset[r] = 0;
            if (offset_r) {
                org_r = adc_db[db_to_adc_gain(get_chan_adc_gain(r))];
                adc_gain_offset[r] = offset_r;
                adj_r = adc_db[db_to_adc_gain(get_chan_adc_gain(r))];
            }
        }

        hal_codec_apply_anc_adc_gain_offset(single_type, (org_l - adj_l), (org_r - adj_r));
        analog_aud_set_adc_gain(AUD_INPUT_PATH_MAINMIC, ch_map);
    }
}
#endif

#ifdef DYN_ADC_GAIN
void analog_aud_apply_adc_gain_offset(enum AUD_CHANNEL_MAP_T ch_map, int16_t offset)
{
    enum AUD_CHANNEL_MAP_T map;
    int i;

#ifdef ANC_APP
#ifdef ANC_FF_ENABLED
    ch_map &= ~(ANC_FF_MIC_CH_L | ANC_FF_MIC_CH_R);
#endif
#ifdef ANC_FB_ENABLED
    ch_map &= ~(ANC_FB_MIC_CH_L | ANC_FB_MIC_CH_R);
#endif
#endif

    if (ch_map) {
        map = ch_map;

        while (map) {
            i = get_msb_pos(map);
            map &= ~(1 << i);
            if (i < MAX_ANA_MIC_CH_NUM) {
                adc_gain_offset[i] = offset;
            }
        }

        ANALOG_INFO_TRACE(2,"ana: apply adc gain offset: ch_map=0x%X offset=%d", ch_map, offset);

        analog_aud_set_adc_gain(AUD_INPUT_PATH_MAINMIC, ch_map);
    }
}
#endif

void analog_aud_set_dac_gain(int32_t v)
{
}

uint32_t analog_codec_get_dac_gain(void)
{
    return 0;
}

uint32_t analog_codec_dac_gain_to_db(int32_t gain)
{
    return 0;
}

int32_t analog_codec_dac_max_attn_db(void)
{
    return 0;
}

static int POSSIBLY_UNUSED dc_calib_checksum_valid(uint32_t efuse)
{
    int i;
    uint32_t cnt = 0;
    uint32_t chksum_mask = (1 << (16 - DAC_DC_CALIB_BIT_WIDTH)) - 1;

    for (i = 0; i < DAC_DC_CALIB_BIT_WIDTH; i++) {
        if (efuse & (1 << i)) {
            cnt++;
        }
    }

    return (((~cnt) & chksum_mask) == ((efuse >> DAC_DC_CALIB_BIT_WIDTH) & chksum_mask));
}

static int16_t dc_calib_val_decode(int16_t val)
{
#ifdef AUDIO_OUTPUT_DC_CALIB_ANA
    uint32_t sign_bit = (1 << (DAC_DC_CALIB_BIT_WIDTH - 1));
    uint32_t num_mask = sign_bit - 1;

    if (val & sign_bit) {
        val = -(val & num_mask);
    }
#endif
    return val;
}

void analog_aud_get_dc_calib_value(int16_t *dc_l, int16_t *dc_r)
{
    static const uint8_t EFUSE_PAGE_CLASSAB_1P8V = PMU_EFUSE_PAGE_DCCALIB_L;
    static const uint8_t EFUSE_PAGE_CLASSAB_1P7V = PMU_EFUSE_PAGE_DCCALIB2_L;
    uint8_t page;
    uint16_t efuse;
    uint16_t flag;

    union DC_EFUSE_T {
        struct DC_VALUE_T {
            int16_t dc          : DAC_DC_CALIB_BIT_WIDTH;
            uint16_t checksum   : (16 - DAC_DC_CALIB_BIT_WIDTH);
        } val;
        uint16_t reg;
    };
    union DC_EFUSE_T dc;

    if (vcodec_mv >= 1800) {
        page = EFUSE_PAGE_CLASSAB_1P8V;
    } else {
        page = EFUSE_PAGE_CLASSAB_1P7V;
    }

    pmu_get_efuse(PMU_EFUSE_PAGE_SW_CFG, &flag);

    pmu_get_efuse(page, &efuse);
    if (dc_calib_checksum_valid(efuse)) {
        ANALOG_INFO_TRACE(1,"Dc calib L OK: 0x%04x", efuse);
        dc.reg = efuse;
        *dc_l = dc.val.dc;

        if((flag & (1 << 6)) == 0) {
            ANALOG_INFO_TRACE(1,"Old data format and flag: 0x%x", flag);
            *dc_l = (*dc_l << 2);
        }
    } else {
        ANALOG_INFO_TRACE(1,"Warning: Bad dc calib efuse L: 0x%04x", efuse);
        *dc_l = 0;
    }

    *dc_r = 0;

    ANALOG_INFO_TRACE(2,"ANA: DC CALIB L=0x%04hX/%d", *dc_l, dc_calib_val_decode(*dc_l));

#if defined(ANA_DC_CALIB_L) || defined(ANA_DC_CALIB_R)
#ifdef ANA_DC_CALIB_L
    *dc_l = ANA_DC_CALIB_L;
#endif
    ANALOG_INFO_TRACE(2,"ANA: OVERRIDE DC CALIB L=0x%04hX/%d", *dc_l, dc_calib_val_decode(*dc_l));
#endif

    return;
}

bool analog_aud_dc_calib_valid(void)
{
    return false;
}

uint16_t analog_aud_dac_dc_diff_to_val(int32_t diff)
{
    uint16_t val;

    // BIT 13: SIGN
    // BIT 12: x256
    // BIT 11: x128
    // BIT 10: x64
    // BIT  9: x64
    // BIT  8: x32
    // BIT  7: x16
    // BIT  6: x8
    // BIT  5: x8
    // BIT  4: x4
    // BIT  3: x2
    // BIT  2: x2
    // BIT  1: x1
    // BIT  1: x1

    val = 0;
    if (diff < 0) {
        val |= (1 << 13);
        diff = -diff;
    }
    if (diff & (1 << 8)) {
        val |= (1 << 12);
    }
    if (diff & (1 << 7)) {
        val |= (1 << 11);
    }
    if (diff & (1 << 6)) {
        val |= (1 << 9);
    }
    if (diff & (1 << 5)) {
        val |= (1 << 8);
    }
    if (diff & (1 << 4)) {
        val |= (1 << 7);
    }
    if (diff & (1 << 3)) {
        val |= (1 << 5);
    }
    if (diff & (1 << 2)) {
        val |= (1 << 4);
    }
    if (diff & (1 << 1)) {
        val |= (1 << 2);
    }
    if (diff & (1 << 0)) {
        val |= (1 << 0);
    }

    return val;
}

uint16_t analog_aud_dc_calib_val_to_efuse(uint16_t val)
{
    int i;
    uint32_t cnt = 0;
    uint32_t chksum_mask = (1 << (16 - DAC_DC_CALIB_BIT_WIDTH)) - 1;
    uint32_t val_mask = (1 << DAC_DC_CALIB_BIT_WIDTH) - 1;

    for (i = 0; i < DAC_DC_CALIB_BIT_WIDTH; i++) {
        if (val & (1 << i)) {
            cnt++;
        }
    }

    return (((~cnt) & chksum_mask) << DAC_DC_CALIB_BIT_WIDTH) | (val & val_mask);
}

int16_t analog_aud_dac_dc_get_step(void)
{
    return DAC_DC_ADJUST_STEP;
}

void analog_aud_save_dc_calib(uint16_t val)
{
}

void analog_aud_dc_calib_set_value(uint16_t dc_l, uint16_t dc_r)
{
    uint16_t val;

    analog_read(ANA_REG_172, &val);
    val = SET_BITFIELD(val, REG_CODEC_TX_EAR_OFF_BITL, dc_l);
    analog_write(ANA_REG_172, val);
#if 0
    analog_read(ANA_REG_72, &val);
    val = SET_BITFIELD(val, REG_CODEC_TX_EAR_OFF_BITR, dc_r);
    analog_write(ANA_REG_72, val);
#endif
}

void analog_aud_dc_calib_get_cur_value(uint16_t *dc_l, uint16_t *dc_r)
{
    uint16_t val;

    if (dc_l) {
        analog_read(ANA_REG_172, &val);
        *dc_l = GET_BITFIELD(val, REG_CODEC_TX_EAR_OFF_BITL);
    }

    if (dc_r) {
#if 0
        analog_read(ANA_REG_72, &val);
        *dc_r = GET_BITFIELD(val, REG_CODEC_TX_EAR_OFF_BITR);
#else
        *dc_r = 0;
#endif
    }
}

bool analog_aud_dc_calib_get_large_ana_dc_value(uint16_t *ana_dc, int cur_dig_dc, int tgt_dig_dc,
    int chan, bool init)
{
#define DAC_LARGE_ANA_DC_STEP_100UV (0x080) //DC=100uV
#define DAC_LARGE_ANA_DC_STEP_200UV (0x100) //DC=230uV
#define DAC_LARGE_ANA_DC_STEP_250UV (0x140) //DC=260uV
#define DAC_LARGE_ANA_DC_STEP_450UV (0x200) //DC=450uV
#define DAC_LARGE_ANA_DC_STEP_850UV (0x800) //DC=850uV

#define DAC_LARGE_ANA_DC_INIT_R (0x0)
#define DAC_LARGE_ANA_DC_INIT_L (DAC_LARGE_ANA_DC_STEP_200UV)
#define DAC_LARGE_ANA_DC_STEP   (DAC_LARGE_ANA_DC_STEP_100UV)
#define DAC_DIG_DC_THRES_MIN    (2000)

#define DAC_ANA_DC_SIGN_BIT     (13)
#define POS_ANA_DC(v)           ((v)&(~(1<<DAC_ANA_DC_SIGN_BIT)))
#define NEG_ANA_DC(v)           ((v)|(1<<DAC_ANA_DC_SIGN_BIT))
    bool success = false;
    if (init) {
        if (ana_dc) {
            if (chan == 0) {
                *ana_dc = DAC_LARGE_ANA_DC_INIT_L;
            } else {
                *ana_dc = DAC_LARGE_ANA_DC_INIT_R;
            }
        }
    } else {
        int comp_dig_dc;
        uint16_t dc;

        comp_dig_dc = tgt_dig_dc - cur_dig_dc;
        if (ABS(comp_dig_dc) >= DAC_DIG_DC_THRES_MIN) {
            success = true;
        } else {
            if (ana_dc) {
                dc = *ana_dc;
                dc += DAC_LARGE_ANA_DC_STEP;
                dc = (cur_dig_dc<0)?NEG_ANA_DC(dc):POS_ANA_DC(dc);
                *ana_dc = dc;
            }
        }
    }
    return success;
}

#ifdef AUDIO_OUTPUT_DC_CALIB_ANA
static void analog_aud_dc_calib_init(void)
{
    uint16_t val;
    int16_t dc_l, dc_r;

    analog_aud_get_dc_calib_value(&dc_l, &dc_r);

    analog_read(ANA_REG_172, &val);
    val = SET_BITFIELD(val, REG_CODEC_TX_EAR_OFF_BITL, dc_l);
    analog_write(ANA_REG_172, val);
}
#endif

void analog_aud_dc_calib_enable(bool en)
{
    uint16_t val;

    analog_read(ANA_REG_172, &val);
    if (en) {
        val |= REG_CODEC_TX_EAR_OFFEN;
    } else {
        val &= ~REG_CODEC_TX_EAR_OFFEN;
    }
    analog_write(ANA_REG_172, val);
}

static void analog_aud_dac_dc_backup_regs(bool save)
{
    uint32_t i;
    uint16_t regaddr[] = {
        ANA_REG_61,
        ANA_REG_71,
        ANA_REG_66,
        ANA_REG_68,
        ANA_REG_78,
        ANA_REG_76};

    static bool regs_saved = false;
    static uint16_t regval[ARRAY_SIZE(regaddr)] = {0};

    if (save) {
        if (!regs_saved) {
            for(i = 0; i < ARRAY_SIZE(regaddr); i++) {
                analog_read(regaddr[i], &regval[i]);
            }
            regs_saved = true;
        }
    } else {
        if (regs_saved) {
            for(i = 0; i < ARRAY_SIZE(regaddr); i++) {
                analog_write(regaddr[i], regval[i]);
            }
            regs_saved = false;
        }
    }
}

#ifdef AUDIO_OUTPUT_DC_CALIB_RST_ANA_ADC
static void analog_aud_dac_dc_calib_rst_ana_adc(bool rst)
{
    static bool rst_done = false;

    uint16_t val;

    if (rst) {
        if (!rst_done) {
            osDelay(1);
            analog_read(ANA_REG_61, &val);
            val &= ~REG_CODEC_EN_ADCA;
            val |= REG_CODEC_RESET_ADCA | CFG_RESET_ADCA_DR;
            analog_write(ANA_REG_61, val);
            osDelay(1);

            val |= REG_CODEC_EN_ADCA;
            analog_write(ANA_REG_61, val);
            osDelay(1);

            val &= ~REG_CODEC_RESET_ADCA;
            analog_write(ANA_REG_61, val);
            osDelay(1);
            rst_done = true;
        }
    } else {
        rst_done = false;
    }
}
#endif

#define analog_aud_dac_dc_save_regs()    analog_aud_dac_dc_backup_regs(true)
#define analog_aud_dac_dc_restore_regs() analog_aud_dac_dc_backup_regs(false)

void analog_aud_dac_dc_auto_calib_enable(void)
{
    uint16_t val;

    analog_aud_dac_dc_save_regs();

    analog_aud_enable_dac(AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1);
    analog_aud_enable_dac_pa(AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1);
    analog_aud_set_adc_gain_direct((AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1), -3);
    analog_aud_enable_adc(ANA_CODEC_USER_ADC, (AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1), true);

    // Force ADC precharge = 1
    analog_read(ANA_REG_61, &val);
    val |= REG_CODEC_ADCA_PRE_CHARGE;
    analog_write(ANA_REG_61, val);

    analog_read(ANA_REG_71, &val);
    val |= REG_CODEC_ADCB_PRE_CHARGE;
    analog_write(ANA_REG_71, val);

    // precharge DR = 1
    analog_read(ANA_REG_61, &val);
    val |= CFG_PRE_CHARGE_ADCA_DR;
    analog_write(ANA_REG_61, val);

    analog_read(ANA_REG_71, &val);
    val |= CFG_PRE_CHARGE_ADCB_DR;
    analog_write(ANA_REG_71, val);

    osDelay(100);

    // precharge DR = 0
    analog_read(ANA_REG_61, &val);
    val &= ~CFG_PRE_CHARGE_ADCA_DR;
    analog_write(ANA_REG_61, val);

    analog_read(ANA_REG_71, &val);
    val &= ~CFG_PRE_CHARGE_ADCB_DR;
    analog_write(ANA_REG_71, val);

    // Force ADC precharge = 0
    analog_read(ANA_REG_61, &val);
    val &= ~REG_CODEC_ADCA_PRE_CHARGE;
    analog_write(ANA_REG_61, val);

    analog_read(ANA_REG_71, &val);
    val &= ~REG_CODEC_ADCB_PRE_CHARGE;
    analog_write(ANA_REG_71, val);

    // set ADC base voltage for DAC DC calib
    analog_read(ANA_REG_68, &val);
    val = SET_BITFIELD(val, REG_CODEC_ADCA_VREFBUF_BIT, 9);
    analog_write(ANA_REG_68, val);

    analog_read(ANA_REG_78, &val);
    val = SET_BITFIELD(val, REG_CODEC_ADCB_VREFBUF_BIT, 9);
    analog_write(ANA_REG_78, val);
}

void analog_aud_dac_dc_auto_calib_disable(void)
{
    analog_aud_dac_dc_auto_calib_set_mode(ANA_DAC_DC_CALIB_MODE_NORMAL);

    analog_aud_enable_adc(ANA_CODEC_USER_ADC, (AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1), false);
    analog_aud_enable_dac_pa(0);
    analog_aud_enable_dac(0);

    analog_aud_dac_dc_restore_regs();

#ifdef AUDIO_OUTPUT_DC_CALIB_RST_ANA_ADC
    analog_aud_dac_dc_calib_rst_ana_adc(false);
#endif
}

void analog_aud_dac_dc_auto_calib_set_mode(enum ANA_DAC_DC_CALIB_MODE_T mode)
{
    uint16_t val;

    analog_read(ANA_REG_66, &val);
    if (mode == ANA_DAC_DC_CALIB_MODE_ADC_ONLY) {
        val = SET_BITFIELD(val, REG_CODEC_ADCA_OFFSET_CAL_S, 0x06);
    } else if (mode == ANA_DAC_DC_CALIB_MODE_DAC_TO_ADC) {
        val = SET_BITFIELD(val, REG_CODEC_ADCA_OFFSET_CAL_S, 0x12);
    } else {
        val = SET_BITFIELD(val, REG_CODEC_ADCA_OFFSET_CAL_S, 0x12);
    }
    analog_write(ANA_REG_66, val);

    analog_read(ANA_REG_61, &val);
    val = SET_BITFIELD(val, REG_CODEC_ADCA_CH_SEL, 2);
    analog_write(ANA_REG_61, val);

    analog_read(ANA_REG_71, &val);
    val = SET_BITFIELD(val, REG_CODEC_ADCB_CH_SEL, 2);
    analog_write(ANA_REG_71, val);

#ifdef AUDIO_OUTPUT_DC_CALIB_RST_ANA_ADC
    if (mode == ANA_DAC_DC_CALIB_MODE_ADC_ONLY) {
        analog_aud_dac_dc_calib_rst_ana_adc(true);
    }
#endif
}

static void analog_aud_vcodec_enable(enum ANA_CODEC_USER_T user, bool en)
{
    uint32_t lock;
    bool set = false;

    lock = int_lock();
    if (en) {
        if (vcodec_map == 0) {
            set = true;
        }
        vcodec_map |= user;
    } else {
        vcodec_map &= ~user;
        if (vcodec_map == 0) {
            set = true;
        }
    }
    int_unlock(lock);

    if (set) {
        pmu_codec_config(!!vcodec_map);
    }
}

static void analog_aud_enable_common_internal(enum ANA_CODEC_USER_T user, bool en)
{
    uint32_t lock;
    uint16_t val_b2;
    uint16_t val_b3;
    bool set = false;

    lock = int_lock();
    if (en) {
        if (codec_common_map == 0) {
            set = true;
        }
        codec_common_map |= user;
    } else {
        codec_common_map &= ~user;
        if (codec_common_map == 0) {
            set = true;
        }
    }
    int_unlock(lock);

    if (set) {
        analog_read(ANA_REG_B2, &val_b2);
        analog_read(ANA_REG_B3, &val_b3);
        if (codec_common_map) {
            val_b2 |= REG_CODEC_EN_VCM;
        } else {
            val_b2 &= ~REG_CODEC_EN_VCM;
            val_b3 &= ~REG_CODEC_VCM_EN_LPF;
        }
        if (codec_common_map) {
            // RTOS application startup time is long enougth for VCM charging
#if !(defined(VCM_ON) && defined(RTOS))
            // VCM fully stable time is about 60ms/1.95V or 150ms/1.7V
            // Quick startup:
            // 1) Disable VCM LPF and target to a higher voltage than the required one
            // 2) Wait for a short time when VCM is in quick charge (high voltage)
            // 3) Enable VCM LPF and target to the required VCM LPF voltage
            analog_write(ANA_REG_B2, SET_BITFIELD(val_b2, REG_CODEC_VCM_LOW_VCM, 0));
            uint32_t delay;

#if defined(VCM_CAP_100NF)
            if (vcodec_mv >= 1900) {
                delay = 6;
            } else {
                delay = 10;
            }
#else
            if (vcodec_mv >= 1900) {
                delay = 25;
            } else {
                delay = 100;
            }
#endif
            osDelay(delay);
#if 0
            // Target to a voltage near the required one
            analog_write(ANA_REG_6B, val_6b);
            osDelay(10);
#endif
#endif // !(VCM_ON && RTOS)
            val_b3 |= REG_CODEC_VCM_EN_LPF;
        }
        analog_write(ANA_REG_B2, val_b2);
        analog_write(ANA_REG_B3, val_b3);
    }
}

static void analog_aud_enable_codec_common(enum ANA_CODEC_USER_T user, bool en)
{
#ifndef VCM_ON
    analog_aud_enable_common_internal(user, en);
#endif
}

static void analog_aud_enable_adda_common(enum ANA_CODEC_USER_T user, bool en)
{
    uint32_t lock;
    uint16_t val_b0;
    bool set = false;

    lock = int_lock();
    if (en) {
        if (adda_common_map == 0) {
            set = true;
        }
        adda_common_map |= user;
    } else {
        adda_common_map &= ~user;
        if (adda_common_map == 0) {
            set = true;
        }
    }
    int_unlock(lock);

    if (set) {
        analog_read(ANA_REG_B0, &val_b0);
        if (adda_common_map) {
            val_b0 |= REG_CODEC_EN_BIAS;
            // bypass or pu tx regulator
        } else {
            val_b0 &= ~REG_CODEC_EN_BIAS;
        }
        analog_write(ANA_REG_B0, val_b0);

#if defined(CODEC_TX_PEAK_DETECT)
        analog_aud_enable_codec_vcm_buffer(!!adda_common_map);
#endif
    }
}

static void analog_aud_enable_vmic(enum ANA_CODEC_USER_T user, uint32_t dev)
{
    uint32_t lock;
    enum ANA_CODEC_USER_T old_map;
    bool set = false;
    int i;
    uint32_t pmu_map = 0;

    lock = int_lock();

    for (i = 0; i < MAX_VMIC_CH_NUM; i++) {
        if (dev & (AUD_VMIC_MAP_VMIC1 << i)) {
            if (vmic_map[i] == 0) {
                set = true;
            }
            vmic_map[i] |= user;
        } else {
            old_map = vmic_map[i];
            vmic_map[i] &= ~user;
            if (old_map != 0 && vmic_map[i] == 0) {
                set = true;
            }
        }

        if (vmic_map[i]) {
            pmu_map |= (AUD_VMIC_MAP_VMIC1 << i);
        }
    }

    int_unlock(lock);

    if (set) {
        pmu_codec_mic_bias_enable(pmu_map);
#ifdef VOICE_DETECTOR_EN
        pmu_codec_mic_bias_lowpower_mode(pmu_map);
#endif
        if (pmu_map) {
            osDelay(1);
        }
    }
}

uint32_t analog_aud_get_max_dre_gain(void)
{
    if (vcodec_mv >= 1800) {
#if ANALOG_MAX_DRE_GAIN_VAL !=0
        return ANALOG_MAX_DRE_GAIN_VAL;
#else
        return 0x11;
#endif
    } else {
#if ANALOG_MAX_DRE_GAIN_VAL !=0
        return ANALOG_MAX_DRE_GAIN_VAL;
#else
        return 0x11;
#endif
    }
}

int analog_reset(void)
{
    return 0;
}

void analog_open(void)
{
    uint16_t i;
    uint16_t val;
    uint16_t bfv;
    uint16_t efuse;

    for (i = 0; i < MAX_ANA_MIC_CH_NUM; i++) {
        val = REG_CODEC_ADCA_CH_SEL(1);
        analog_write(ANA_REG_61 + 0x10 * i, val);
        val = REG_CODEC_ADCA_CAP_BIT1(0x7F) | REG_CODEC_ADCA_CAP_BIT2(0x1F);
        analog_write(ANA_REG_62 + 0x10 * i, val);

        if (vcodec_mv >= 1800) {
            val = REG_CODEC_ADCA_CAP_BIT3(0x1F) | REG_CODEC_ADCA_OP4_IBIT(1) | REG_CODEC_ADCA_REG_VSEL(4);
        } else {
            val = REG_CODEC_ADCA_CAP_BIT3(0x1F) | REG_CODEC_ADCA_OP1_IBIT(1) | REG_CODEC_ADCA_REG_VSEL(3);
        }
        analog_write(ANA_REG_63 + 0x10 * i, val);

        val = REG_CODEC_ADCA_IBSEL_OFFSET(8) | REG_CODEC_ADCA_IBSEL_OP1(0xC) | REG_CODEC_ADCA_IBSEL_OP2(8) | REG_CODEC_ADCA_IBSEL_OP3(8);
        analog_write(ANA_REG_64 + 0x10 * i, val);

        if (vcodec_mv >= 1800) {
            val = REG_CODEC_ADCA_IBSEL_OP4(8) | REG_CODEC_ADCA_IBSEL_REG(2) | REG_CODEC_ADCA_IBSEL_VCOMP(4) | REG_CODEC_ADCA_IBSEL_VREF(8);
        } else {
            val = REG_CODEC_ADCA_IBSEL_OP4(8) | REG_CODEC_ADCA_IBSEL_REG(8) | REG_CODEC_ADCA_IBSEL_VCOMP(8) | REG_CODEC_ADCA_IBSEL_VREF(8);
        }
        analog_write(ANA_REG_65 + 0x10 * i, val);

        if (vcodec_mv >= 1800) {
            val = REG_CODEC_ADCA_IBSEL_VREFBUF(4) | REG_CODEC_ADCA_IBSEL_IDAC2(8) | REG_CODEC_ADCA_OFFSET_CURRENT_SEL;
            analog_write(ANA_REG_66 + 0x10 * i, val);
        }

        if (vcodec_mv >= 1800) {
            val = REG_CODEC_ADCA_VREF_SEL(7) | REG_CODEC_ADCA_VREFBUF_BIT(8) | ADCA_TIMER_RSTN_DLY(0x10);
        } else {
            val = REG_CODEC_ADCA_VREF_SEL(5) | REG_CODEC_ADCA_VREFBUF_BIT(8) | ADCA_TIMER_RSTN_DLY(0x10);
        }
        analog_write(ANA_REG_68 + 0x10 * i, val);

        val = ADCA_OPEN_TIMER_DLY(4) | ADCA_PRE_CHARGE_TIMER_DLY(0x14) | REG_CODEC_ADCA_HP_MODE;
        analog_write(ANA_REG_69 + 0x10 * i, val);
    }

    val = 0;
    analog_write(ANA_REG_B0, val);

    val = REG_CODEC_BIAS_IBSEL(8) | REG_CODEC_BIAS_IBSEL_TX(2) | REG_CODEC_BIAS_IBSEL_VOICE(8);
    analog_write(ANA_REG_B1, val);

    val = RESERVED_DIG_32 | REG_CODEC_VCM_LOW_VCM(7) | REG_CODEC_VCM_LOW_VCM_LP(7) | REG_CODEC_VCM_LOW_VCM_LPF(7);
    analog_write(ANA_REG_B2, val);

    val = REG_CODEC_BUF_LOWVCM(4);
    analog_write(ANA_REG_B3, val);

    val = REG_TX_REGULATOR_BIT(0xC);
#ifdef CODEC_TX_PEAK_DETECT
    val |= REG_CODEC_TX_PEAK_NL_EN | REG_CODEC_TX_PEAK_PL_EN;
#endif
    analog_write(ANA_REG_16D, val);

    val = REG_CODEC_TX_EAR_DRE_GAIN_L(0x11);
#ifdef DAC_DRE_GAIN_DC_UPDATE
    val |= REG_CODEC_TX_EAR_DRE_GAIN_L_UPDATE;
#endif
    analog_write(ANA_REG_16E, val);

    if (vcodec_mv >= 1800) {
        bfv = 0x6;
    } else {
        bfv = 5;
    }


    uint16_t efuse_b;
    pmu_get_efuse(PMU_EFUSE_PAGE_DCCALIB_L, &efuse_b);
    TRACE(3, "%s efuse %x=0x%x", __FUNCTION__, PMU_EFUSE_PAGE_DCCALIB_L, efuse_b);

    pmu_get_efuse(PMU_EFUSE_PAGE_DCCALIB2_L, &efuse);
    //efuse_9[0]==1 , efuse_9[4:1]!=0, efuse_b==0
    if ((efuse & 0x1) && ((efuse & 0x1E) != 0) && (efuse_b == 0)) {
        bfv = ((efuse & 0x1E) >> 1);    //efuse_9 [4:1]
    }
    TRACE(2, "%s efuse_9=0x%x bfv=%d", __FUNCTION__, efuse, bfv);

    val = REG_CODEC_TX_DAC_VREF_L(bfv);
    analog_write(ANA_REG_16F, val);

    if (vcodec_mv >= 1800) {
        bfv = 0x5d;
    } else {
        bfv = 0xf3;
    }
    val = REG_CODEC_TX_EAR_COMP1(bfv);
    analog_write(ANA_REG_170, val);

    if (vcodec_mv >= 1800) {
        bfv = 2;
    } else {
        bfv = 1;
    }
    val = REG_CODEC_TX_EAR_IBSEL(bfv);
    analog_write(ANA_REG_171, val);

    val = REG_CODEC_TX_EAR_SOFTSTART(8) | REG_CODEC_TX_EAR_DR_ST(2);
    if (vcodec_mv >= 1800) {
        bfv = 2;
    } else {
        bfv = 1;
    }
    val |= REG_CODEC_TX_EAR_OUTPUTSEL(bfv);
    analog_write(ANA_REG_174, val);

    val = REG_BYPASS_TX_REGULATOR | REG_CODEC_TX_EAR_GAIN(1) | REG_CODEC_DAC_CLK_EDGE_SEL;
    analog_write(ANA_REG_175, val);

    val = REG_CODEC_TX_EAR_VCM(3) | DRE_GAIN_SEL_L;
    analog_write(ANA_REG_176, val);


    if (vcodec_mv >= 1800) {
        val = REG_CODEC_CASN_L(2) | REG_CODEC_CASP_L(2) | REG_CODEC_TX_IB_SEL_ST2(1);
    } else {
        val = REG_CODEC_CASN_L(2) | REG_CODEC_CASP_L(2) | REG_CODEC_TX_IB_SEL_ST2(0);
    }
    analog_write(ANA_REG_17B, val);

    val = REG_CODEC_TX_VREF_RZ;
    if (vcodec_mv >= 1800) {
        val |= REG_DAC_LOWGAIN(1);
    }
    analog_write(ANA_REG_17C, val);

#ifdef CODEC_TX_ZERO_CROSSING_EN_GAIN
    val = REG_PU_ZERO_DET_L | REG_ZERO_DETECT_CHANGE;
    analog_write(ANA_REG_17D, val);
#endif

#ifdef AUDIO_OUTPUT_DC_CALIB_ANA
    analog_aud_dc_calib_init();
    analog_aud_dc_calib_enable(true);
#endif

#ifdef VCM_ON
    analog_aud_enable_common_internal(ANA_CODEC_USER_DAC, true);
#endif

    analog_read(ANA_REG_16E, &val);
    val |= REG_CODEC_TX_EAR_OCEN;
    analog_write(ANA_REG_16E, val);
}

void analog_sleep(void)
{
#ifndef VCM_ON
    if (codec_common_map)
#endif
    {
        uint16_t val;

        analog_read(ANA_REG_B2, &val);
        val |= REG_CODEC_LP_VCM;
        analog_write(ANA_REG_B2, val);
    }
}

void analog_wakeup(void)
{
#ifndef VCM_ON
    if (codec_common_map)
#endif
    {
        uint16_t val;

        analog_read(ANA_REG_B2, &val);
        val &= ~REG_CODEC_LP_VCM;
        analog_write(ANA_REG_B2, val);
    }
}

void analog_aud_codec_anc_enable(enum ANC_TYPE_T type, bool en)
{
    enum ANA_CODEC_USER_T user;
    enum AUD_CHANNEL_MAP_T ch_map;

    user = 0;
    ch_map = 0;
#if defined(ANC_FF_MIC_CH_L) || defined(ANC_FF_MIC_CH_R)
    if (type & ANC_FEEDFORWARD) {
        user |= ANA_CODEC_USER_ANC_FF;
        ch_map |= ANC_FF_MIC_CH_L;
    }
#endif
#if defined(ANC_FB_MIC_CH_L) || defined(ANC_FB_MIC_CH_R)
    if (type & ANC_FEEDBACK) {
        user |= ANA_CODEC_USER_ANC_FB;
        ch_map |= ANC_FB_MIC_CH_L;
    }
#endif
#if defined(ANC_TT_MIC_CH_L)
    if (type & ANC_TALKTHRU) {
        user |= ANA_CODEC_USER_ANC_TT;
        ch_map |= ANC_TT_MIC_CH_L;
    }
#endif

    ANALOG_DEBUG_TRACE(0, "%s: type=%d en=%d ch_map=0x%x", __func__, type, en, ch_map);

    if (en) {
        analog_aud_enable_vmic(user, ANC_VMIC_CFG);
        analog_aud_enable_codec_common(user, true);
        analog_aud_enable_adda_common(user, true);
        analog_aud_set_adc_gain(AUD_INPUT_PATH_MAINMIC, ch_map);
        analog_aud_enable_adc(user, ch_map, true);
    } else {
        analog_aud_apply_anc_adc_gain_offset(type, 0, 0);
        analog_aud_enable_adc(user, ch_map, false);
        analog_aud_enable_adda_common(user, false);
        analog_aud_enable_codec_common(user, false);
        analog_aud_enable_vmic(user, 0);
#ifdef ANC_APP
#if defined(ANC_FF_MIC_CH_L)
#if defined(ANC_TT_MIC_CH_L)
        if ((type & ANC_FEEDFORWARD) && (adda_common_map & ANA_CODEC_USER_ANC_TT)) {
            analog_aud_apply_anc_adc_gain_offset(ANC_TALKTHRU, anc_tt_gain_offset_l, 0);
        }
#endif
#endif
#endif
    }
}

void analog_aud_codec_anc_boost(bool en, ANALOG_ANC_BOOST_DELAY_FUNC delay_func)
{
    uint16_t val;
    int ret;

    if (vcodec_mv >= 1800) {
        return;
    }

    if (delay_func == NULL) {
        delay_func = (ANALOG_ANC_BOOST_DELAY_FUNC)osDelay;
    }

    if (en) {
        // -2.1 dB
        hal_codec_set_anc_boost_gain_attn(0.78523563f);
        delay_func(1);
    }

    analog_read(ANA_REG_B3, &val);
    val &= ~REG_CODEC_VCM_EN_LPF;
    analog_write(ANA_REG_B3, val);
    osDelay(1);

    do {
        if (en) {
            ret = pmu_codec_volt_ramp_up();
        } else {
            ret = pmu_codec_volt_ramp_down();
        }
        delay_func(1);
    } while (ret);

    analog_read(ANA_REG_B3, &val);
    val |= REG_CODEC_VCM_EN_LPF;
    analog_write(ANA_REG_B3, val);

    if (!en) {
        delay_func(1);
        // 0 dB
        hal_codec_set_anc_boost_gain_attn(1.0f);
    }
}

void analog_aud_mickey_enable(bool en)
{
    if (en) {
        analog_aud_vcodec_enable(ANA_CODEC_USER_MICKEY, true);
        analog_aud_enable_vmic(ANA_CODEC_USER_MICKEY, CFG_HW_AUD_MICKEY_DEV);
        analog_aud_enable_codec_common(ANA_CODEC_USER_MICKEY, true);
    } else {
        analog_aud_enable_codec_common(ANA_CODEC_USER_MICKEY, false);
        analog_aud_enable_vmic(ANA_CODEC_USER_MICKEY, 0);
        analog_aud_vcodec_enable(ANA_CODEC_USER_MICKEY, false);
    }
}

void analog_aud_codec_adc_enable(enum AUD_IO_PATH_T input_path, enum AUD_CHANNEL_MAP_T ch_map, bool en)
{
    uint32_t dev;

    if (en) {
        dev = hal_codec_get_input_path_cfg(input_path);
        // Enable vmic first to overlap vmic stable time with codec vcm stable time
        analog_aud_enable_vmic(ANA_CODEC_USER_ADC, dev);
        analog_aud_enable_codec_common(ANA_CODEC_USER_ADC, true);
        analog_aud_enable_adda_common(ANA_CODEC_USER_ADC, true);
        analog_aud_set_adc_gain(input_path, ch_map);
        analog_aud_enable_adc(ANA_CODEC_USER_ADC, ch_map, true);
    } else {
        analog_aud_enable_adc(ANA_CODEC_USER_ADC, ch_map, false);
        analog_aud_enable_adda_common(ANA_CODEC_USER_ADC, false);
        analog_aud_enable_codec_common(ANA_CODEC_USER_ADC, false);
        analog_aud_enable_vmic(ANA_CODEC_USER_ADC, 0);
    }
}

static void analog_aud_codec_config_speaker(void)
{
    bool en;

    if (ana_spk_req && !ana_spk_muted) {
        en = true;
    } else {
        en = false;
    }

    if (ana_spk_enabled != en) {
        ana_spk_enabled = en;
        if (en) {
            analog_aud_enable_dac_pa(CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV);
        } else {
            analog_aud_enable_dac_pa(0);
        }
    }
}

void analog_aud_codec_speaker_enable(bool en)
{
    ana_spk_req = en;
    analog_aud_codec_config_speaker();
}

void analog_aud_codec_dac_enable(bool en)
{
    if (en) {
        analog_aud_enable_codec_common(ANA_CODEC_USER_DAC, true);
        analog_aud_enable_adda_common(ANA_CODEC_USER_DAC, true);
        pmu_codec_hppa_enable(1);
        analog_aud_enable_dac(CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV);
#if !defined(AUDIO_OUTPUT_DC_CALIB_ANA) && !defined(AUDIO_OUTPUT_DC_CALIB)
        osDelay(1);
        analog_aud_codec_speaker_enable(true);
#endif
    } else {
#if !defined(AUDIO_OUTPUT_DC_CALIB_ANA) && !defined(AUDIO_OUTPUT_DC_CALIB)
        analog_aud_codec_speaker_enable(false);
        osDelay(1);
#endif
        analog_aud_enable_dac(0);
        pmu_codec_hppa_enable(0);
        analog_aud_enable_adda_common(ANA_CODEC_USER_DAC, false);
        analog_aud_enable_codec_common(ANA_CODEC_USER_DAC, false);
    }
}

void analog_aud_codec_open(void)
{
    analog_aud_vcodec_enable(ANA_CODEC_USER_CODEC, true);
}

void analog_aud_codec_close(void)
{
    analog_aud_codec_speaker_enable(false);
    osDelay(1);
    analog_aud_codec_dac_enable(false);

    analog_aud_vcodec_enable(ANA_CODEC_USER_CODEC, false);
}

void analog_aud_codec_mute(void)
{
#ifndef AUDIO_OUTPUT_DC_CALIB
    //analog_codec_tx_pa_gain_sel(0);
#endif

    ana_spk_muted = true;
    analog_aud_codec_config_speaker();
}

void analog_aud_codec_nomute(void)
{
    ana_spk_muted = false;
    analog_aud_codec_config_speaker();

#ifndef AUDIO_OUTPUT_DC_CALIB
    //analog_aud_set_dac_gain(dac_gain);
#endif
}

int analog_debug_config_audio_output(bool diff)
{
    return 0;
}

int analog_debug_config_codec(uint16_t mv)
{
#ifdef ANC_PROD_TEST
    int ret;

    ret = pmu_debug_config_codec(mv);
    if (ret) {
        return ret;
    }

    vcodec_mv = mv;
#endif
    return 0;
}

int analog_debug_config_low_power_adc(bool enable)
{
    return 0;
}

void analog_debug_config_anc_calib_mode(bool enable)
{
    anc_calib_mode = enable;
}

bool analog_debug_get_anc_calib_mode(void)
{
    return anc_calib_mode;
}

int analog_debug_config_vad_mic(bool enable)
{
    return 0;
}

void analog_reg_codec_en_vcm_check()
{
    uint16_t val;
    analog_read(ANA_REG_B2, &val);
    if(!(val&REG_CODEC_EN_VCM))
    {
        ANALOG_DEBUG_TRACE(0,"CODEC_EN_VCM 0");
        val |= REG_CODEC_EN_VCM;
        analog_write(ANA_REG_B2, val);
    }
}