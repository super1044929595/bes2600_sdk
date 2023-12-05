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
#include "plat_addr_map.h"
#include CHIP_SPECIFIC_HDR(reg_codec)
#include "hal_codec.h"
#include "hal_cmu.h"
#include "hal_psc.h"
#include "hal_aud.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "analog.h"
#include "cmsis.h"
#include "string.h"
#include "tgt_hardware.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_chipid.h"
#include "pmu.h"
#define NO_DAC_RESET

//#define SDM_MUTE_NOISE_SUPPRESSION
//#define SDM_MUTE_NOISE_SUPPRESSION_V2

#define DAC_ZERO_CROSSING_GAIN

#ifndef CODEC_OUTPUT_DEV
#define CODEC_OUTPUT_DEV                    CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV
#endif
#if ((CODEC_OUTPUT_DEV & CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV) == 0)
#ifndef AUDIO_OUTPUT_SWAP
#define AUDIO_OUTPUT_SWAP
#endif
#endif

// Recudce 0.57dB in 100Hz@16kHz SampleRate
#define CODEC_ADC_DC_FILTER_FACTOR          (6)

// For 44.1K/48K sample rate
#ifndef CODEC_DAC_GAIN_RAMP_INTERVAL
#define CODEC_DAC_GAIN_RAMP_INTERVAL        CODEC_DAC_GAIN_RAMP_INTVL_16_SAMP
#endif

#if (defined(__TWS__) || defined(IBRT)) && defined(ANC_APP)
//#define CODEC_TIMER
#endif

#define RS_CLOCK_FACTOR                     200

// Trigger DMA request when TX-FIFO *empty* count > threshold
#define HAL_CODEC_TX_FIFO_TRIGGER_LEVEL     (3)
// Trigger DMA request when RX-FIFO count >= threshold
#define HAL_CODEC_RX_FIFO_TRIGGER_LEVEL     (4)

#define MAX_DIG_DBVAL                       (50)
#define ZERODB_DIG_DBVAL                    (0)
#define MIN_DIG_DBVAL                       (-99)

#define MAX_SIDETONE_DBVAL                  (30)
#define MIN_SIDETONE_DBVAL                  (-30)
#define SIDETONE_DBVAL_STEP                 (-2)

#define MAX_SIDETONE_REGVAL                 (0)
#define MIN_SIDETONE_REGVAL                 (30)
#define MUTE_SIDETONE_REGVAL                (31)

#ifndef MC_DELAY_COMMON
#define MC_DELAY_COMMON                     28
#endif

#ifndef CODEC_DIGMIC_PHASE
#define CODEC_DIGMIC_PHASE                  3
#endif

#ifndef CODEC_DIGMIC_CLK_DIV
#define CODEC_DIGMIC_CLK_DIV                1
#endif

#define ADC_IIR_CH_NUM                      3

#define MAX_DIG_MIC_CH_NUM                  3

#define NORMAL_ADC_CH_NUM                   3
// Echo cancel ADC channel number
#define EC_ADC_CH_NUM                       1
#define MAX_ADC_CH_NUM                      (NORMAL_ADC_CH_NUM + EC_ADC_CH_NUM)

#define MAX_DAC_CH_NUM                      2

#define NORMAL_MIC_MAP                      (AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1 | AUD_CHANNEL_MAP_CH2 | \
                                                AUD_CHANNEL_MAP_DIGMIC_CH0 | AUD_CHANNEL_MAP_DIGMIC_CH1 | AUD_CHANNEL_MAP_DIGMIC_CH2)
#define NORMAL_ADC_MAP                      (AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1 | AUD_CHANNEL_MAP_CH2)

#define EC_MIC_MAP                          (AUD_CHANNEL_MAP_ECMIC_CH0)
#define EC_ADC_MAP                          (AUD_CHANNEL_MAP_CH3)

#define VALID_MIC_MAP                       (NORMAL_MIC_MAP | EC_MIC_MAP)
#define VALID_ADC_MAP                       (NORMAL_ADC_MAP | EC_ADC_MAP)

#define VALID_SPK_MAP                       (AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1)
#define VALID_DAC_MAP                       VALID_SPK_MAP

#if (CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV & ~VALID_SPK_MAP)
#error "Invalid CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV"
#endif
#if (CODEC_OUTPUT_DEV & ~VALID_SPK_MAP)
#error "Invalid CODEC_OUTPUT_DEV"
#endif

#define EN_ADC_FREE_RUNNING_CLK             CODEC_EN_CLK_ADC(1 << MAX_ADC_CH_NUM)
#define RSTN_ADC_FREE_RUNNING_CLK           CODEC_SOFT_RSTN_ADC(1 << MAX_ADC_CH_NUM)

#if defined(SPEECH_SIDETONE) && \
        (defined(CFG_HW_AUD_SIDETONE_MIC_DEV) && (CFG_HW_AUD_SIDETONE_MIC_DEV)) && \
        defined(CFG_HW_AUD_SIDETONE_GAIN_DBVAL)
#define SIDETONE_ENABLE
#if (CFG_HW_AUD_SIDETONE_GAIN_DBVAL > MAX_SIDETONE_DBVAL) || \
        (CFG_HW_AUD_SIDETONE_GAIN_DBVAL < MIN_SIDETONE_DBVAL) || \
        defined(CFG_HW_AUD_SIDETONE_IIR_INDEX) || \
        defined(CFG_HW_AUD_SIDETONE_GAIN_RAMP)
#define SIDETONE_DEDICATED_ADC_CHAN
#define SIDETONE_RESERVED_ADC_CHAN
#endif
#endif

enum CODEC_ADC_EN_REQ_T {
    CODEC_ADC_EN_REQ_STREAM,
    CODEC_ADC_EN_REQ_MC,

    CODEC_ADC_EN_REQ_QTY,
};

enum CODEC_IRQ_TYPE_T {
    CODEC_IRQ_TYPE_BT_TRIGGER,
    CODEC_IRQ_TYPE_VAD,
    CODEC_IRQ_TYPE_ANC_FB_CHECK,
    CODEC_IRQ_TYPE_EVENT_TRIGGER,
    CODEC_IRQ_TYPE_TIMER_TRIGGER,

    CODEC_IRQ_TYPE_QTY,
};

enum CODEC_DAC_GAIN_RAMP_INTVL_T {
    CODEC_DAC_GAIN_RAMP_INTVL_1_SAMP,
    CODEC_DAC_GAIN_RAMP_INTVL_2_SAMP,
    CODEC_DAC_GAIN_RAMP_INTVL_4_SAMP,
    CODEC_DAC_GAIN_RAMP_INTVL_8_SAMP,
    CODEC_DAC_GAIN_RAMP_INTVL_16_SAMP,
    CODEC_DAC_GAIN_RAMP_INTVL_32_SAMP,

    CODEC_DAC_GAIN_RAMP_INTVL_QTY,
};

enum CODEC_VOLT_RAMP_USER_T {
    CODEC_VOLT_RAMP_USER_DIG_MIC    = (1 << 0),
    CODEC_VOLT_RAMP_USER_ANC        = (1 << 1),
};

enum {
    DRE_STEP_MODE_STEP_1,
    DRE_STEP_MODE_STEP_2,
    DRE_STEP_MODE_STEP_4,
    DRE_STEP_MODE_STEP_8,
    DRE_STEP_MODE_STEP_16,
};

struct CODEC_DAC_DRE_CFG_T {
    // C0
    uint8_t step_mode;
    uint8_t ini_ana_gain;
    uint8_t dre_delay;
    uint16_t amp_high;
    // C4
    uint32_t dre_win;
    uint8_t thd_db_offset_sign;
    uint8_t thd_db_offset;
    uint8_t gain_offset;
    // 19C
    uint8_t db_high;
    uint8_t db_low;
    uint8_t top_gain;
    uint8_t delay_dc;
};


struct CODEC_DAC_SAMPLE_RATE_T {
    enum AUD_SAMPRATE_T sample_rate;
    uint32_t codec_freq;
    uint8_t codec_div;
    uint8_t cmu_div;
    uint8_t dac_up;
    uint8_t bypass_cnt;
    uint8_t mc_delay;
};

static const struct CODEC_DAC_SAMPLE_RATE_T codec_dac_sample_rate[] = {
#ifdef __AUDIO_RESAMPLE__
    {AUD_SAMPRATE_8463,      CODEC_FREQ_CRYSTAL,               1,             1, 6, 0, MC_DELAY_COMMON + 160},
    {AUD_SAMPRATE_16927,     CODEC_FREQ_CRYSTAL,               1,             1, 3, 0, MC_DELAY_COMMON +  85},
    {AUD_SAMPRATE_50781,     CODEC_FREQ_CRYSTAL,               1,             1, 1, 0, MC_DELAY_COMMON +  29},
#endif
    {AUD_SAMPRATE_7350,   CODEC_FREQ_44_1K_SERIES, CODEC_PLL_DIV, CODEC_CMU_DIV, 6, 0, MC_DELAY_COMMON + 174},
    {AUD_SAMPRATE_8000,   CODEC_FREQ_48K_SERIES,   CODEC_PLL_DIV, CODEC_CMU_DIV, 6, 0, MC_DELAY_COMMON + 168}, // T
    {AUD_SAMPRATE_14700,  CODEC_FREQ_44_1K_SERIES, CODEC_PLL_DIV, CODEC_CMU_DIV, 3, 0, MC_DELAY_COMMON +  71},
    {AUD_SAMPRATE_16000,  CODEC_FREQ_48K_SERIES,   CODEC_PLL_DIV, CODEC_CMU_DIV, 3, 0, MC_DELAY_COMMON +  88}, // T
    {AUD_SAMPRATE_22050,  CODEC_FREQ_44_1K_SERIES, CODEC_PLL_DIV, CODEC_CMU_DIV, 2, 0, MC_DELAY_COMMON +  60},
    {AUD_SAMPRATE_24000,  CODEC_FREQ_48K_SERIES,   CODEC_PLL_DIV, CODEC_CMU_DIV, 2, 0, MC_DELAY_COMMON +  58},
    {AUD_SAMPRATE_44100,  CODEC_FREQ_44_1K_SERIES, CODEC_PLL_DIV, CODEC_CMU_DIV, 1, 0, MC_DELAY_COMMON +  31}, // T
    {AUD_SAMPRATE_48000,  CODEC_FREQ_48K_SERIES,   CODEC_PLL_DIV, CODEC_CMU_DIV, 1, 0, MC_DELAY_COMMON +  30}, // T
    {AUD_SAMPRATE_88200,  CODEC_FREQ_44_1K_SERIES, CODEC_PLL_DIV, CODEC_CMU_DIV, 1, 1, MC_DELAY_COMMON +  12},
    {AUD_SAMPRATE_96000,  CODEC_FREQ_48K_SERIES,   CODEC_PLL_DIV, CODEC_CMU_DIV, 1, 1, MC_DELAY_COMMON +  12}, // T
    {AUD_SAMPRATE_176400, CODEC_FREQ_44_1K_SERIES, CODEC_PLL_DIV, CODEC_CMU_DIV, 1, 2, MC_DELAY_COMMON +   5},
    {AUD_SAMPRATE_192000, CODEC_FREQ_48K_SERIES,   CODEC_PLL_DIV, CODEC_CMU_DIV, 1, 2, MC_DELAY_COMMON +   5},
    {AUD_SAMPRATE_352800, CODEC_FREQ_44_1K_SERIES, CODEC_PLL_DIV, CODEC_CMU_DIV, 1, 3, MC_DELAY_COMMON +   2},
    {AUD_SAMPRATE_384000, CODEC_FREQ_48K_SERIES,   CODEC_PLL_DIV, CODEC_CMU_DIV, 1, 3, MC_DELAY_COMMON +   2},
};

struct CODEC_ADC_SAMPLE_RATE_T {
    enum AUD_SAMPRATE_T sample_rate;
    uint32_t codec_freq;
    uint8_t codec_div;
    uint8_t cmu_div;
    uint8_t adc_down;
    uint8_t bypass_cnt;
};

static const struct CODEC_ADC_SAMPLE_RATE_T codec_adc_sample_rate[] = {
#ifdef __AUDIO_RESAMPLE__
    {AUD_SAMPRATE_8463,      CODEC_FREQ_CRYSTAL,               1,             1, 6, 0},
    {AUD_SAMPRATE_16927,     CODEC_FREQ_CRYSTAL,               1,             1, 3, 0},
    {AUD_SAMPRATE_50781,     CODEC_FREQ_CRYSTAL,               1,             1, 1, 0},
#endif
    {AUD_SAMPRATE_7350,   CODEC_FREQ_44_1K_SERIES, CODEC_PLL_DIV, CODEC_CMU_DIV, 6, 0},
    {AUD_SAMPRATE_8000,   CODEC_FREQ_48K_SERIES,   CODEC_PLL_DIV, CODEC_CMU_DIV, 6, 0},
    {AUD_SAMPRATE_14700,  CODEC_FREQ_44_1K_SERIES, CODEC_PLL_DIV, CODEC_CMU_DIV, 3, 0},
    {AUD_SAMPRATE_16000,  CODEC_FREQ_48K_SERIES,   CODEC_PLL_DIV, CODEC_CMU_DIV, 3, 0},
    {AUD_SAMPRATE_32000,  CODEC_FREQ_48K_SERIES,   CODEC_PLL_DIV, CODEC_CMU_DIV, 3, 1},
    {AUD_SAMPRATE_44100,  CODEC_FREQ_44_1K_SERIES, CODEC_PLL_DIV, CODEC_CMU_DIV, 1, 0},
    {AUD_SAMPRATE_48000,  CODEC_FREQ_48K_SERIES,   CODEC_PLL_DIV, CODEC_CMU_DIV, 1, 0},
    {AUD_SAMPRATE_88200,  CODEC_FREQ_44_1K_SERIES, CODEC_PLL_DIV, CODEC_CMU_DIV, 1, 1},
    {AUD_SAMPRATE_96000,  CODEC_FREQ_48K_SERIES,   CODEC_PLL_DIV, CODEC_CMU_DIV, 1, 1},
    {AUD_SAMPRATE_176400, CODEC_FREQ_44_1K_SERIES, CODEC_PLL_DIV, CODEC_CMU_DIV, 1, 2},
    {AUD_SAMPRATE_192000, CODEC_FREQ_48K_SERIES,   CODEC_PLL_DIV, CODEC_CMU_DIV, 1, 2},
    {AUD_SAMPRATE_352800, CODEC_FREQ_44_1K_SERIES, CODEC_PLL_DIV, CODEC_CMU_DIV, 1, 3},
    {AUD_SAMPRATE_384000, CODEC_FREQ_48K_SERIES,   CODEC_PLL_DIV, CODEC_CMU_DIV, 1, 3},
};

const CODEC_ADC_VOL_T WEAK codec_adc_vol[TGT_ADC_VOL_LEVEL_QTY] = {
    -99, 0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28,
};

static struct CODEC_T * const codec = (struct CODEC_T *)CODEC_BASE;

static bool codec_init = false;
static bool codec_opened = false;

static int8_t digdac_gain[MAX_DAC_CH_NUM];
static int8_t digadc_gain[NORMAL_ADC_CH_NUM];

static bool codec_mute[AUD_STREAM_NUM];

#ifdef AUDIO_OUTPUT_SWAP
static bool output_swap = true;
#endif

#ifdef ANC_APP
static float anc_boost_gain_attn;
static int8_t anc_adc_gain_offset[NORMAL_ADC_CH_NUM];
static enum AUD_CHANNEL_MAP_T anc_adc_gain_offset_map;
#endif
#if defined(NOISE_GATING) && defined(NOISE_REDUCTION)
static bool codec_nr_enabled;
static int8_t digdac_gain_offset_nr;
#endif
#ifdef AUDIO_OUTPUT_FACTROY_CALIB
static float dac_dc_gain_factory;
#endif
#ifdef AUDIO_OUTPUT_DC_CALIB
static int32_t dac_dc_l;
static float dac_dc_gain_attn;
#endif
#ifdef __AUDIO_RESAMPLE__
static uint8_t resample_rate_idx[AUD_STREAM_NUM];
#endif
#ifdef CODEC_TIMER
static uint32_t cur_codec_freq;
#endif

static uint8_t codec_rate_idx[AUD_STREAM_NUM];

//static HAL_CODEC_DAC_RESET_CALLBACK dac_reset_callback;

static uint8_t codec_irq_map;
STATIC_ASSERT(sizeof(codec_irq_map) * 8 >= CODEC_IRQ_TYPE_QTY, "codec_irq_map size too small");
static HAL_CODEC_IRQ_CALLBACK codec_irq_callback[CODEC_IRQ_TYPE_QTY];

static enum AUD_CHANNEL_MAP_T codec_dac_ch_map;
static enum AUD_CHANNEL_MAP_T codec_adc_ch_map;
static enum AUD_CHANNEL_MAP_T anc_adc_ch_map;
static enum AUD_CHANNEL_MAP_T codec_mic_ch_map;
static enum AUD_CHANNEL_MAP_T anc_mic_ch_map;
#ifdef SIDETONE_DEDICATED_ADC_CHAN
static enum AUD_CHANNEL_MAP_T sidetone_adc_ch_map;
static int8_t sidetone_adc_gain;
static int8_t sidetone_gain_offset;
#ifdef CFG_HW_AUD_SIDETONE_GAIN_RAMP
static float sidetone_ded_chan_coef;
#endif
#endif

#if defined(ANC_APP) && defined(ANC_TT_MIC_CH_L)
static enum AUD_CHANNEL_MAP_T anc_tt_adc_ch_l;
static enum ANC_TYPE_T anc_tt_mic_l_map;
#endif

#if defined(DIG_MIC_DYN_CODEC_VOLT) && defined(ANC_DYN_CODEC_VOLT)
enum CODEC_VOLT_RAMP_USER_T vcodec_ramp_user;
#endif

#ifdef ANC_PROD_TEST
#define FORCE_TT_ADC_ALLOC              true
#define OPT_TYPE
#else
#define FORCE_TT_ADC_ALLOC              false
#define OPT_TYPE                        const
#endif

static OPT_TYPE uint8_t codec_digmic_phase = CODEC_DIGMIC_PHASE;

#if defined(AUDIO_ANC_FB_MC) && defined(__AUDIO_RESAMPLE__)
#error "Music cancel cannot work with audio resample"
#endif
#ifdef AUDIO_ANC_FB_MC
static bool mc_enabled;
static bool mc_started;
static enum AUD_CHANNEL_NUM_T mc_chan_num;
static enum AUD_BITS_T mc_bits;
#endif

#if defined(AUDIO_ANC_FB_MC)
static uint8_t adc_en_map;
STATIC_ASSERT(sizeof(adc_en_map) * 8 >= CODEC_ADC_EN_REQ_QTY, "adc_en_map size too small");
#endif

#ifdef PERF_TEST_POWER_KEY
static enum HAL_CODEC_PERF_TEST_POWER_T cur_perft_power;
#endif

#ifdef AUDIO_OUTPUT_SW_GAIN
static int8_t swdac_gain;
static HAL_CODEC_SW_OUTPUT_COEF_CALLBACK sw_output_coef_callback;
#endif

static HAL_CODEC_BT_TRIGGER_CALLBACK bt_trigger_callback = NULL;
static HAL_CODEC_EVENT_TRIGGER_CALLBACK event_trigger_callback = NULL;
static HAL_CODEC_TIMER_TRIGGER_CALLBACK timer_trigger_callback = NULL;

#if defined(DAC_CLASSG_ENABLE)
static struct dac_classg_cfg _dac_classg_cfg = {
    .thd2 = 0xC0,
    .thd1 = 0x10,
    .thd0 = 0x10,
    .lr = 1,
    .step_4_3n = 0,
    .quick_down = 1,
    .wind_width = 0x400,
};
#endif

#ifdef DAC_DRE_ENABLE
static struct CODEC_DAC_DRE_CFG_T dac_dre_cfg = {
    // 350
    .step_mode = DRE_STEP_MODE_STEP_16,
    .ini_ana_gain = 0,
    .dre_delay = 0x14,
    .amp_high = 0x800,
    // 354
    .dre_win = 0x3000, //16 ms, 1.536Mhz
    .thd_db_offset_sign = 0,
    .thd_db_offset = 0,
    .gain_offset = 2,
    // 358
    .db_high = 48, //-42-0.75*48 = -78 dBFS
    .db_low  = 48, //-42-0.75*48 = -78 dBFS
    .top_gain = 0xF,
    .delay_dc = 0,
};
#endif

#if defined(DAC_DRE_ENABLE) || defined(AUDIO_OUTPUT_DC_AUTO_CALIB)
static struct HAL_CODEC_DAC_DRE_CALIB_CFG_T dac_dre_calib_cfg[] = {
    {
        .valid = 0,
        .dc_l = 0,
        .dc_r = 0,
        .gain_l = 1.0,
        .gain_r = 1.0,
        .ana_dc_l = 0,
        .ana_dc_r = 0,
        .ana_gain = 0x13,
        .ini_ana_gain = 0,
        .gain_offset = 4,
        .step_mode = DRE_STEP_MODE_STEP_16,
        .top_gain = 7,
    },
    {
        .valid = 0,
        .dc_l = 0,
        .dc_r = 0,
        .gain_l = 3.652,
        .gain_r = 3.652,
        .ana_dc_l = 0,
        .ana_dc_r = 0,
        .ana_gain = 0xC,
        .ini_ana_gain = 7,
        .gain_offset = 4,
        .step_mode = DRE_STEP_MODE_STEP_16,
        .top_gain = 7,
    },
};
#endif

#ifdef CODEC_SW_SYNC
static bool codec_sw_sync_play_status = false;
static bool codec_sw_sync_cap_status = false;
#endif

static void hal_codec_set_dig_adc_gain(enum AUD_CHANNEL_MAP_T map, int32_t gain);
static void hal_codec_set_dig_dac_gain(enum AUD_CHANNEL_MAP_T map, int32_t gain);
static void hal_codec_restore_dig_dac_gain(void);
static void hal_codec_set_dac_gain_value(enum AUD_CHANNEL_MAP_T map, uint32_t val);
static int hal_codec_set_adc_down(enum AUD_CHANNEL_MAP_T map, uint32_t val);
static int hal_codec_set_adc_hbf_bypass_cnt(enum AUD_CHANNEL_MAP_T map, uint32_t cnt);
static uint32_t hal_codec_get_adc_chan(enum AUD_CHANNEL_MAP_T mic_map);
#ifdef AUDIO_OUTPUT_SW_GAIN
static void hal_codec_set_sw_gain(int32_t gain);
#endif
#ifdef __AUDIO_RESAMPLE__
static float get_playback_resample_phase(void);
static float get_capture_resample_phase(void);
static uint32_t resample_phase_float_to_value(float phase);
static float resample_phase_value_to_float(uint32_t value);
#endif
#ifdef AUDIO_OUTPUT_DC_AUTO_CALIB
static bool hal_codec_get_dig_dc_calib_value(int32_t *dc_l, int32_t *dc_r);
static bool hal_codec_set_ana_dc_calib_value(void);
#endif

static void hal_codec_reg_update_delay(void)
{
    hal_sys_timer_delay_us(2);
}

#if defined(DAC_CLASSG_ENABLE)
void hal_codec_classg_config(const struct dac_classg_cfg *cfg)
{
    _dac_classg_cfg = *cfg;
}

static void hal_codec_classg_enable(bool en)
{
    struct dac_classg_cfg *config;

    if (en) {
        config = &_dac_classg_cfg;

        codec->REG_0B4 = SET_BITFIELD(codec->REG_0B4, CODEC_CODEC_CLASSG_THD2, config->thd2);
        codec->REG_0B4 = SET_BITFIELD(codec->REG_0B4, CODEC_CODEC_CLASSG_THD1, config->thd1);
        codec->REG_0B4 = SET_BITFIELD(codec->REG_0B4, CODEC_CODEC_CLASSG_THD0, config->thd0);

        // Make class-g set the lowest gain after several samples.
        // Class-g gain will have impact on dc.
        codec->REG_0B0 = SET_BITFIELD(codec->REG_0B0, CODEC_CODEC_CLASSG_WINDOW, 6);

        if (config->lr)
            codec->REG_0B0 |= CODEC_CODEC_CLASSG_LR;
        else
            codec->REG_0B0 &= ~CODEC_CODEC_CLASSG_LR;

        if (config->step_4_3n)
            codec->REG_0B0 |= CODEC_CODEC_CLASSG_STEP_3_4N;
        else
            codec->REG_0B0 &= ~CODEC_CODEC_CLASSG_STEP_3_4N;

        if (config->quick_down)
            codec->REG_0B0 |= CODEC_CODEC_CLASSG_QUICK_DOWN;
        else
            codec->REG_0B0 &= ~CODEC_CODEC_CLASSG_QUICK_DOWN;

        codec->REG_0B0 |= CODEC_CODEC_CLASSG_EN;

        // Restore class-g window after the gain has been updated
        hal_codec_reg_update_delay();
        codec->REG_0B0 = SET_BITFIELD(codec->REG_0B0, CODEC_CODEC_CLASSG_WINDOW, config->wind_width);
    } else {
        codec->REG_0B0 &= ~CODEC_CODEC_CLASSG_QUICK_DOWN;
    }
}
#endif

void hal_codec_dac_dc_offset_enable(int32_t dc_l, int32_t dc_r)
{
    codec->REG_0E0 &= ~CODEC_CODEC_DAC_DC_UPDATE_CH0;
    hal_codec_reg_update_delay();
    codec->REG_0E0 = SET_BITFIELD(codec->REG_0E0, CODEC_CODEC_DAC_DC_CH0, dc_l) | CODEC_CODEC_DAC_DC_UPDATE_CH0;
}

int hal_codec_open(enum HAL_CODEC_ID_T id)
{
    int i;
    bool first_open;

#ifdef CODEC_POWER_DOWN
    first_open = true;
#else
    first_open = !codec_init;
#endif

    analog_aud_pll_open(ANA_AUD_PLL_USER_CODEC);

    if (!codec_init) {
        for (i = 0; i < CFG_HW_AUD_INPUT_PATH_NUM; i++) {
            if (cfg_audio_input_path_cfg[i].cfg & AUD_CHANNEL_MAP_ALL & ~VALID_MIC_MAP) {
                ASSERT(false, "Invalid input path cfg: i=%d io_path=%d cfg=0x%X",
                    i, cfg_audio_input_path_cfg[i].io_path, cfg_audio_input_path_cfg[i].cfg);
            }
        }
#ifdef ANC_APP
        anc_boost_gain_attn = 1.0f;
#endif
        codec_init = true;
    }
    if (first_open) {
        // Codec will be powered down first
        hal_psc_codec_enable();
    }
    hal_cmu_codec_clock_enable();
    hal_cmu_codec_reset_clear();

    codec_opened = true;

    codec->REG_060 |= CODEC_EN_CLK_ADC_MASK | CODEC_EN_CLK_ADC_ANA_MASK | CODEC_POL_ADC_ANA_MASK | CODEC_POL_DAC_OUT;
    codec->REG_064 |= CODEC_SOFT_RSTN_32K | CODEC_SOFT_RSTN_IIR_MASK | CODEC_SOFT_RSTN_RS_MASK | CODEC_SOFT_RSTN_RS1 |
        CODEC_SOFT_RSTN_DAC | CODEC_SOFT_RSTN_ADC_MASK | CODEC_SOFT_RSTN_ADC_ANA_MASK;
    codec->REG_000 = 0;
    codec->REG_04C &= ~CODEC_MC_ENABLE;
    codec->REG_004 = ~0UL;
    hal_codec_reg_update_delay();
    codec->REG_004 = 0;
    codec->REG_000 |= CODEC_CODEC_IF_EN;

    codec->REG_054 |= CODEC_FAULT_MUTE_DAC_ENABLE | CODEC_FAULT_MUTE_DAC_ENABLE_SND;

#ifdef AUDIO_OUTPUT_SWAP
    if (output_swap) {
        codec->REG_0A0 |= CODEC_CODEC_DAC_OUT_SWAP;
    } else {
        codec->REG_0A0 &= ~CODEC_CODEC_DAC_OUT_SWAP;
    }
#endif

#ifdef CODEC_TIMER
    // Enable codec timer and record time by bt event instead of gpio event
    codec->REG_054 = (codec->REG_054 & ~CODEC_EVENT_SEL) | CODEC_EVENT_FOR_CAPTURE;
    codec->REG_060 |= CODEC_EN_CLK_DAC;
#endif

    if (first_open) {
#ifdef AUDIO_ANC_FB_MC
        codec->REG_04C = CODEC_DMACTRL_MC | CODEC_MC_EN_SEL | CODEC_MC_RATE_SRC_SEL;
#endif

        // ANC zero-crossing
        codec->REG_0D4 |= CODEC_CODEC_ANC_MUTE_GAIN_PASS0_FF_CH0;
        codec->REG_0D8 |= CODEC_CODEC_ANC_MUTE_GAIN_PASS0_FB_CH0;

        // Enable ADC zero-crossing gain adjustment
        for (i = 0; i < NORMAL_ADC_CH_NUM; i++) {
            *(&codec->REG_084 + i) |= CODEC_CODEC_ADC_GAIN_SEL_CH0;
        }

        // DRE ini gain and offset
        uint8_t max_gain, ini_gain, dre_offset;
        max_gain = analog_aud_get_max_dre_gain();
        ini_gain = 0;
        if (max_gain >= 0xF) {
            dre_offset = max_gain - 0xF;
        } else {
            dre_offset = 32 - (0xF - max_gain);
        }
        codec->REG_0C0 = CODEC_CODEC_DRE_INI_ANA_GAIN_CH0(ini_gain);
        codec->REG_0C4 = CODEC_CODEC_DRE_GAIN_OFFSET_CH0(dre_offset);
        codec->REG_0E0 = CODEC_CODEC_DAC_ANA_GAIN_UPDATE_DELAY_CH0(0);
        codec->REG_19C = CODEC_CODEC_DRE_GAIN_TOP_CH0(max_gain);

#ifdef ANC_PROD_TEST
#ifdef AUDIO_ANC_FB_MC
        // Enable ADC + music cancel.
        codec->REG_130 |= CODEC_CODEC_FB_CHECK_KEEP_CH0;
#elif defined(AUDIO_ANC_FB_MC_HW)
        // Enable ADC + music cancel.
        codec->REG_130 |= CODEC_CODEC_FB_CHECK_KEEP_CH0;
#endif
#endif

        // 0dB gain for EC channel
        codec->REG_094 = (codec->REG_094 & ~(CODEC_CODEC_ADC_ECHO_GAIN_SEL_CH0 | CODEC_CODEC_ADC_ECHO_GAIN_CH0_MASK)) |
            CODEC_CODEC_ADC_ECHO_GAIN_CH0(1 << 12);
        codec->REG_09C &= ~CODEC_CODEC_ADC_ECHO_GAIN_UPDATE_CH0;
        hal_codec_reg_update_delay();
        codec->REG_09C |= CODEC_CODEC_ADC_ECHO_GAIN_UPDATE_CH0;

#if defined(FIXED_CODEC_ADC_VOL) && defined(SINGLE_CODEC_ADC_VOL)
        const CODEC_ADC_VOL_T *adc_gain_db;

        adc_gain_db = hal_codec_get_adc_volume(CODEC_SADC_VOL);
        if (adc_gain_db) {
            hal_codec_set_dig_adc_gain(NORMAL_ADC_MAP, *adc_gain_db);
#ifdef SIDETONE_DEDICATED_ADC_CHAN
            sidetone_adc_gain = *adc_gain_db;
#endif
        }
#endif

#ifdef AUDIO_OUTPUT_DC_CALIB
#ifdef AUDIO_OUTPUT_DC_AUTO_CALIB
        if (hal_codec_set_ana_dc_calib_value()) {
            hal_codec_get_dig_dc_calib_value(&dac_dc_l, NULL);
        }
#endif
        hal_codec_dac_dc_offset_enable(dac_dc_l, 0);
#elif defined(SDM_MUTE_NOISE_SUPPRESSION)
        hal_codec_dac_dc_offset_enable(1, 1);
#endif

#ifdef AUDIO_OUTPUT_SW_GAIN
        const struct CODEC_DAC_VOL_T *vol_tab_ptr;

        // Init gain settings
        vol_tab_ptr = hal_codec_get_dac_volume(0);
        if (vol_tab_ptr) {
            analog_aud_set_dac_gain(vol_tab_ptr->tx_pa_gain);
            hal_codec_set_dig_dac_gain(VALID_DAC_MAP, ZERODB_DIG_DBVAL);
        }
#else
#ifdef DAC_ZERO_CROSSING_GAIN
        // Enable DAC zero-crossing gain adjustment
        codec->REG_09C |= CODEC_CODEC_DAC_GAIN_SEL_CH0;
#elif defined(DAC_RAMP_GAIN)
        // Enable DAC ramp gain (adjust 2^-14 on each sample)
        codec->REG_0EC = CODEC_CODEC_RAMP_STEP_CH0(1) | CODEC_CODEC_RAMP_EN_CH0 | CODEC_CODEC_RAMP_INTERVAL_CH0(0);
#endif
#endif

#ifdef AUDIO_OUTPUT_DC_CALIB_ANA
        // Reset SDM
        hal_codec_set_dac_gain_value(VALID_DAC_MAP, 0);
        codec->REG_0A4 |= CODEC_CODEC_DAC_SDM_CLOSE;
#endif

#ifdef SDM_MUTE_NOISE_SUPPRESSION
        codec->REG_098 = SET_BITFIELD(codec->REG_098, CODEC_CODEC_DAC_DITHER_GAIN, 0x10);
#endif

#ifdef DAC_SDM_GAIN_ENABLE
        codec->REG_098 = SET_BITFIELD(codec->REG_098, CODEC_CODEC_DAC_SDM_GAIN, 0x4);
#endif

#ifdef __AUDIO_RESAMPLE__
        codec->REG_0E4 &= ~(CODEC_CODEC_RESAMPLE_DAC_ENABLE | CODEC_CODEC_RESAMPLE_ADC_ENABLE |
            CODEC_CODEC_RESAMPLE_DAC_ENABLE_SND |
            CODEC_CODEC_RESAMPLE_DAC_PHASE_UPDATE | CODEC_CODEC_RESAMPLE_ADC_PHASE_UPDATE |
            CODEC_CODEC_RESAMPLE_DAC_PHASE_UPDATE_SND);
#endif

        // Mute DAC when cpu fault occurs
        hal_cmu_codec_set_fault_mask(0x2F);

#ifdef CODEC_TIMER
        // Disable sync stamp auto clear to avoid impacting codec timer capture
        codec->REG_054 &= ~CODEC_STAMP_CLR_USED;
#else
        // Enable sync stamp auto clear
        codec->REG_054 |= CODEC_STAMP_CLR_USED;
#endif
    }

    return 0;
}

int hal_codec_close(enum HAL_CODEC_ID_T id)
{
#ifdef CODEC_TIMER
    // Reset codec timer
    codec->REG_054 &= ~CODEC_EVENT_FOR_CAPTURE;
    codec->REG_060 &= ~CODEC_EN_CLK_DAC;
#endif

    codec->REG_054 &= ~(CODEC_FAULT_MUTE_DAC_ENABLE | CODEC_FAULT_MUTE_DAC_ENABLE_SND);

    codec->REG_000 = 0;
    codec->REG_064 &= ~(CODEC_SOFT_RSTN_32K | CODEC_SOFT_RSTN_IIR_MASK | CODEC_SOFT_RSTN_RS_MASK | CODEC_SOFT_RSTN_RS1 |
        CODEC_SOFT_RSTN_DAC | CODEC_SOFT_RSTN_ADC_MASK | CODEC_SOFT_RSTN_ADC_ANA_MASK);
    codec->REG_060 = 0;

    codec_opened = false;

#ifdef CODEC_POWER_DOWN
    hal_cmu_codec_reset_set();
    hal_cmu_codec_clock_disable();
    hal_psc_codec_disable();
#else
    // NEVER reset or power down CODEC registers, for the CODEC driver expects that last configurations
    // still exist in the next stream setup
    hal_cmu_codec_clock_disable();
#endif

    analog_aud_pll_close(ANA_AUD_PLL_USER_CODEC);

    return 0;
}

void hal_codec_crash_mute(void)
{
    if (codec_opened) {
        codec->REG_000 &= ~CODEC_CODEC_IF_EN;
    }
}

#ifdef AUDIO_OUTPUT_DC_AUTO_CALIB
static bool dac_dc_calib_status = false;
void hal_codec_set_dac_calib_status(bool status)
{
    dac_dc_calib_status = status;
}

void hal_codec_dac_dre_init_calib_cfg(void)
{
    struct HAL_CODEC_DAC_DRE_CALIB_CFG_T *cfg = &dac_dre_calib_cfg[0];
    uint8_t max_gain, ini_gain, gain_offset, top_gain;
    uint8_t gain_step;

    max_gain = analog_aud_get_max_dre_gain();
    if (max_gain < 8) {
        gain_step = max_gain;
    } else {
        gain_step = 8;
    }
    TRACE(1, "%s: max_gain=0x%x,gain_step=%d", __func__, max_gain, gain_step);
    if (cfg[0].ana_gain != max_gain) {
        ini_gain = 0;
        if (max_gain >= 0xF) {
            gain_offset = max_gain - 0xF;
        } else {
            gain_offset = 32 - (0xF - max_gain);
        }
        cfg[0].ana_gain = max_gain;
        cfg[0].ini_ana_gain = ini_gain;
        cfg[0].gain_offset = gain_offset;
        cfg[1].ana_gain = cfg[0].ana_gain - (gain_step - 1);
        if (max_gain >= 0xF) {
            cfg[1].ini_ana_gain = 15 - (cfg[1].ana_gain - gain_offset);
        } else {
            cfg[1].ini_ana_gain = cfg[0].ana_gain - cfg[1].ana_gain;
        }
        cfg[1].gain_offset = cfg[0].gain_offset;
        top_gain = cfg[0].ana_gain - cfg[1].ana_gain;
        cfg[1].top_gain = cfg[0].top_gain = top_gain;
        TRACE(1, "update max_gain:%d, ini=%d,offs=%d,top=%d", max_gain, ini_gain, gain_offset, top_gain);
    }
}

struct HAL_CODEC_DAC_DRE_CALIB_CFG_T *hal_codec_dac_dre_get_calib_cfg(uint32_t *nr)
{
    *nr = ARRAY_SIZE(dac_dre_calib_cfg);
    return dac_dre_calib_cfg;
}

int hal_codec_dac_dre_check_calib_cfg(struct HAL_CODEC_DAC_DRE_CALIB_CFG_T *cfg)
{
    int error = 0;

    if ((cfg->valid & 0xF) == 0) {
        error |= 0x1;
    }
    if ((cfg->valid & (~(0xF))) != DAC_DC_VALID_MARK) {
        error |= 0x2;
    }
    if (cfg->ana_gain > 0x1F) {
        error |= 0x4;
    }
    if (cfg->ini_ana_gain > 0xF) {
        error |= 0x8;
    }
    if (cfg->gain_offset > 0x1F) {
        error |= 0x10;
    }
    if (cfg->step_mode > 4) {
        error |= 0x20;
    }
    if (cfg->top_gain > 0xF) {
        error |= 0x40;
    }
    return error;
}

void hal_codec_set_dac_ana_gain(uint8_t ini_gain, uint8_t gain_offset)
{
    uint8_t max_gain = analog_aud_get_max_dre_gain();

    codec->REG_0C0 = CODEC_CODEC_DRE_INI_ANA_GAIN_CH0(ini_gain);
    codec->REG_0C4 = CODEC_CODEC_DRE_GAIN_OFFSET_CH0(gain_offset);
    codec->REG_0E0 = CODEC_CODEC_DAC_ANA_GAIN_UPDATE_DELAY_CH0(0);
    codec->REG_19C = CODEC_CODEC_DRE_GAIN_TOP_CH0(max_gain);
}

bool hal_codec_get_dig_dc_calib_value_high_dre_gain(int32_t *dc_l, int32_t *dc_r)
{
    struct HAL_CODEC_DAC_DRE_CALIB_CFG_T *cfg = &dac_dre_calib_cfg[0];

    if (dc_l) {
        *dc_l = (cfg->valid & (1<<0)) ? cfg->dc_l : 0;
    }
    if (dc_r) {
        *dc_r = (cfg->valid & (1<<1)) ? cfg->dc_r : 0;
    }
    return true;
}

int hal_codec_check_dac_dc_offset(bool major, int range_idx, int32_t dc_l, int32_t dc_r)
{
#define MAJOR_DC_BITS     (19)
#define MAJOR_DC_MAX      ((1<<(MAJOR_DC_BITS-1))-1)
#define MAJOR_DC_MIN      (-(1<<(MAJOR_DC_BITS-1)))
#define MINOR_DC_BITS     (16)
#define MINOR_DC_MAX      ((1<<(MINOR_DC_BITS-1))-1)
#define MINOR_DC_MIN      (-(1<<(MINOR_DC_BITS-1)))

#define MAJOR_DET_DC      (200)
#define PROD_MAJOR_DC_MAX (MAJOR_DC_MAX - MAJOR_DET_DC)
#define PROD_MAJOR_DC_MIN (MAJOR_DC_MIN + MAJOR_DET_DC)
#define MINOR_DET_DC      (200)
#define PROD_MINOR_DC_MAX (MINOR_DC_MAX - MINOR_DET_DC)
#define PROD_MINOR_DC_MIN (MINOR_DC_MIN + MINOR_DET_DC)

    int r = 0;
    int32_t max_dc;
    int32_t min_dc;

    if (range_idx == 0) {
        if (major) {
            max_dc = MAJOR_DC_MAX;
            min_dc = MAJOR_DC_MIN;
        } else {
            max_dc = MINOR_DC_MAX;
            min_dc = MINOR_DC_MIN;
        }
    } else {
        if (major) {
            max_dc = PROD_MAJOR_DC_MAX;
            min_dc = PROD_MAJOR_DC_MIN;
        } else {
            max_dc = PROD_MINOR_DC_MAX;
            min_dc = PROD_MINOR_DC_MIN;
        }
    }
    if (dc_l > max_dc) {
        r |= 0x01;
    } else if (dc_l < min_dc) {
        r |= 0x02;
    }
    if (dc_r > max_dc) {
        r |= 0x04;
    } else if (dc_r < min_dc) {
        r |= 0x08;
    }
    if (r) {
        TRACE(0, "[%s]: Invalid DAC DC, major=%d, dc_l=%d, dc_r=%d, min=%d, max=%d",
            __func__, major, dc_l, dc_r, min_dc, max_dc);
    }
    return r;
}

static bool hal_codec_get_dig_dc_calib_value(int32_t *dc_l, int32_t *dc_r)
{
    int32_t dc_val_l = 0, dc_val_r = 0;
    struct HAL_CODEC_DAC_DRE_CALIB_CFG_T *cfg = dac_dre_calib_cfg;

#ifdef DAC_DRE_ENABLE
    dc_val_l = ((int32_t)(cfg[0].dc_l) + (int32_t)(cfg[1].dc_l)) / 2;
    dc_val_r = ((int32_t)(cfg[0].dc_r) + (int32_t)(cfg[1].dc_r)) / 2;
#else
    dc_val_l = (int32_t)(cfg[0].dc_l);
    dc_val_r = (int32_t)(cfg[0].dc_r);
#endif

    if (dc_l) {
        *dc_l = dc_val_l;
    }
    if (dc_r) {
        *dc_r = dc_val_r;
    }

    TRACE(1, "CALIB_DIG_DC: L=0x%x, R=0x%x",dc_l?(*dc_l):0, dc_r?(*dc_r):0);
    return true;
}

static bool hal_codec_set_ana_dc_calib_value(void)
{
    bool success = false;
    uint16_t ana_dc_l = 0, ana_dc_r = 0;
    uint32_t i, ini_gain, gain_offs, ana_gain;
    struct HAL_CODEC_DAC_DRE_CALIB_CFG_T *cfg = dac_dre_calib_cfg;

    ini_gain  = GET_BITFIELD(codec->REG_0C0, CODEC_CODEC_DRE_INI_ANA_GAIN_CH0);
    gain_offs = GET_BITFIELD(codec->REG_0C4, CODEC_CODEC_DRE_GAIN_OFFSET_CH0);
    if (gain_offs <= 0xF) { //[0,+15]
        ana_gain  = 0xF - ini_gain + gain_offs;
    } else if (gain_offs > 0x10) { //(-16,-1]
        ana_gain =  0xF - ini_gain - (32 - gain_offs);
        ASSERT(ana_gain <= 30, "[%s]: bad ana_gain=%d,ini_gain=%d,gain_offs=%d",
            __func__, ana_gain, ini_gain, gain_offs);
    } else { //-16
        ASSERT(false, "[%s]: invalid gain_offs=%d, ini_gain=%d", __func__, gain_offs, ini_gain);
    }
    for (i = 0; i < ARRAY_SIZE(dac_dre_calib_cfg); i++, cfg++) {
        if (ana_gain == cfg->ana_gain) {
            if (cfg->valid & (1<<0)) {
                ana_dc_l = cfg->ana_dc_l;
            }
            if (cfg->valid & (1<<1)) {
                ana_dc_r = cfg->ana_dc_r;
            }
            success = true;
            break;
        }
    }
    analog_aud_dc_calib_set_value(ana_dc_l, ana_dc_r);
    analog_aud_dc_calib_enable(true);
    TRACE(1, "CALIB_ANA_DC: L=0x%x, R=0x%x, gain=0x%x",ana_dc_l, ana_dc_r, ana_gain);
    return success;
}
#endif /* AUDIO_OUTPUT_DC_AUTO_CALIB */

#ifdef DAC_DRE_ENABLE
static bool hal_codec_dac_dre_setup_calib_param(struct CODEC_DAC_DRE_CFG_T *cfg)
{
    struct HAL_CODEC_DAC_DRE_CALIB_CFG_T *cal = dac_dre_calib_cfg;
    uint32_t i;

    cfg->step_mode    = cal[1].step_mode;
    cfg->ini_ana_gain = cal[1].ini_ana_gain;
    cfg->gain_offset  = cal[1].gain_offset;
    cfg->top_gain     = cal[1].top_gain;

    for (i = 0; i < ARRAY_SIZE(dac_dre_calib_cfg); i++, cal++) {
        if (cal->valid & 0x1) {
            uint16_t dc_l = (uint16_t)((int32_t)(cal->dc_l) - dac_dc_l);
            uint8_t dc_step = cal->ini_ana_gain;
            int32_t gain_l = (int32_t)(cal->gain_l * (float)2048);
            uint32_t reg_offs[]={0,0,4,4,8,8,12,12,16,16,20,20,24,24,28,28};
            uint32_t mask_flag, dc_reg_addr, gain_reg_addr;

            ASSERT(dc_step < ARRAY_SIZE(reg_offs), "%s:invalid dc_step=%d",__func__,dc_step);
            mask_flag = dc_step % 2;
            dc_reg_addr = (uint32_t)&(codec->REG_380) + reg_offs[dc_step];
            gain_reg_addr = (uint32_t)&(codec->REG_1A0) + reg_offs[dc_step];

            if (mask_flag) {
                *(volatile uint32_t *)dc_reg_addr = CODEC_CODEC_DAC_DRE_DC1_CH0(dc_l);
                *(volatile uint32_t *)gain_reg_addr = CODEC_CODEC_DAC_DRE_GAIN_STEP1_CH0(gain_l);
            } else {
                *(volatile uint32_t *)dc_reg_addr = CODEC_CODEC_DAC_DRE_DC0_CH0(dc_l);
                *(volatile uint32_t *)gain_reg_addr = CODEC_CODEC_DAC_DRE_GAIN_STEP0_CH0(gain_l);
            }
#ifdef DRE_DBG
            TRACE(1,"[%s]: dc_step=%d, mask_flag=%d, ana_gain=%d, ini_gain=%d, gain_offset=%d", __func__,
                dc_step, mask_flag, cal->ana_gain, cal->ini_ana_gain, cal->gain_offset);
            TRACE(1,"DAC_L: dc [%x]=%x, gain [%x]=%x", dc_reg_addr, dc_l, gain_reg_addr, gain_l);
#endif
        }
    }
    return true;
}

static POSSIBLY_UNUSED void hal_codec_dac_dre_enable(void)
{
    struct CODEC_DAC_DRE_CFG_T *cfg = &dac_dre_cfg;

    hal_codec_dac_dre_setup_calib_param(cfg);

    codec->REG_0C0 = CODEC_CODEC_DRE_STEP_MODE_CH0(cfg->step_mode)
                    | CODEC_CODEC_DRE_INI_ANA_GAIN_CH0(cfg->ini_ana_gain)
                    | CODEC_CODEC_DRE_DELAY_CH0(cfg->dre_delay)
                    | CODEC_CODEC_DRE_AMP_HIGH_CH0(cfg->amp_high);

    codec->REG_0C4 = CODEC_CODEC_DRE_WINDOW_CH0(cfg->dre_win)
                    | CODEC_CODEC_DRE_THD_DB_OFFSET_CH0(cfg->thd_db_offset)
                    | CODEC_CODEC_DRE_GAIN_OFFSET_CH0(cfg->gain_offset);
    if (cfg->thd_db_offset_sign) {
        codec->REG_0C4 |= CODEC_CODEC_DRE_THD_DB_OFFSET_SIGN_CH0;
    }

    codec->REG_19C = CODEC_CODEC_DRE_DB_HIGH_CH0(cfg->db_high)
                    | CODEC_CODEC_DRE_DB_LOW_CH0(cfg->db_low)
                    | CODEC_CODEC_DRE_GAIN_TOP_CH0(cfg->top_gain)
                    | CODEC_CODEC_DRE_DELAY_DC_CH0(cfg->delay_dc);

    codec->REG_0C0 |= CODEC_CODEC_DRE_ENABLE_CH0;
}

void hal_codec_dac_dre_disable(void)
{
    struct HAL_CODEC_DAC_DRE_CALIB_CFG_T *cal = dac_dre_calib_cfg;
    uint8_t ini_ana_gain = cal[0].ini_ana_gain;
    uint8_t gain_offset  = cal[0].gain_offset;

    codec->REG_0C0 &= ~CODEC_CODEC_DRE_ENABLE_CH0;
    codec->REG_0C0 = SET_BITFIELD(codec->REG_0C0, CODEC_CODEC_DRE_INI_ANA_GAIN_CH0, ini_ana_gain);
    codec->REG_0C4 = SET_BITFIELD(codec->REG_0C4, CODEC_CODEC_DRE_GAIN_OFFSET_CH0, gain_offset);
}
#endif

#ifdef PERF_TEST_POWER_KEY
static void hal_codec_update_perf_test_power(void)
{
    int32_t nominal_vol;
    uint32_t ini_ana_gain;
    int32_t dac_vol;

    if (!codec_opened) {
        return;
    }

    dac_vol = 0;
    if (cur_perft_power == HAL_CODEC_PERF_TEST_30MW) {
        nominal_vol = 0;
        ini_ana_gain = 0;
    } else if (cur_perft_power == HAL_CODEC_PERF_TEST_10MW) {
        nominal_vol = -5;
        ini_ana_gain = 6;
    } else if (cur_perft_power == HAL_CODEC_PERF_TEST_5MW) {
        nominal_vol = -8;
        ini_ana_gain = 0xA;
    } else if (cur_perft_power == HAL_CODEC_PERF_TEST_M60DB) {
        nominal_vol = -60;
        ini_ana_gain = 0xF; // about -11 dB
        dac_vol = -49;
    } else {
        return;
    }

    if (codec->REG_0C0 & CODEC_CODEC_DRE_ENABLE_CH0) {
        dac_vol = nominal_vol;
    } else {
        codec->REG_0C0 = SET_BITFIELD(codec->REG_0C0, CODEC_CODEC_DRE_INI_ANA_GAIN_CH0, ini_ana_gain);
    }

#ifdef AUDIO_OUTPUT_SW_GAIN
    hal_codec_set_sw_gain(dac_vol);
#else
    hal_codec_set_dig_dac_gain(VALID_DAC_MAP, dac_vol);
#endif

#if defined(NOISE_GATING) && defined(NOISE_REDUCTION)
    if (codec_nr_enabled) {
        codec_nr_enabled = false;
        hal_codec_set_noise_reduction(true);
    }
#endif
}

void hal_codec_dac_gain_m60db_check(enum HAL_CODEC_PERF_TEST_POWER_T type)
{
    cur_perft_power = type;

    if (!codec_opened || (codec->REG_0A4 & CODEC_CODEC_DAC_UH_EN) == 0) {
        return;
    }

    hal_codec_update_perf_test_power();
}
#endif

#if defined(NOISE_GATING) && defined(NOISE_REDUCTION)
void hal_codec_set_noise_reduction(bool enable)
{
    uint32_t ini_ana_gain;

    if (codec_nr_enabled == enable) {
        // Avoid corrupting digdac_gain_offset_nr or using an invalid one
        return;
    }

    codec_nr_enabled = enable;

    if (!codec_opened) {
        return;
    }

    // ini_ana_gain=0   -->   0dB
    // ini_ana_gain=0xF --> -11dB
    if (enable) {
        ini_ana_gain = GET_BITFIELD(codec->REG_0C0, CODEC_CODEC_DRE_INI_ANA_GAIN_CH0);
        digdac_gain_offset_nr = ((0xF - ini_ana_gain) * 11 + 0xF / 2) / 0xF;
        ini_ana_gain = 0xF;
    } else {
        ini_ana_gain = 0xF - (digdac_gain_offset_nr * 0xF + 11 / 2) / 11;
        digdac_gain_offset_nr = 0;
    }

    codec->REG_0C0 = SET_BITFIELD(codec->REG_0C0, CODEC_CODEC_DRE_INI_ANA_GAIN_CH0, ini_ana_gain);

#ifdef AUDIO_OUTPUT_SW_GAIN
    hal_codec_set_sw_gain(swdac_gain);
#else
    hal_codec_restore_dig_dac_gain();
#endif
}
#endif

#ifdef CODEC_SW_SYNC
void hal_codec_sw_sync_play_open(void)
{
    codec_sw_sync_play_status = true;
}

void hal_codec_sw_sync_play_close(void)
{
    codec_sw_sync_play_status = false;
}

void hal_codec_sw_sync_play_enable(void)
{
    if (codec_sw_sync_play_status) {
        codec->REG_000 |= CODEC_DAC_ENABLE;
    }
}

void hal_codec_sw_sync_cap_open(void)
{
    codec_sw_sync_cap_status = true;
}

void hal_codec_sw_sync_cap_close(void)
{
    codec_sw_sync_cap_status = false;
}

void hal_codec_sw_sync_cap_enable(void)
{
    if (codec_sw_sync_cap_status) {
        codec->REG_000 |= CODEC_ADC_ENABLE;
    }
}
#endif

void hal_codec_stop_playback_stream(enum HAL_CODEC_ID_T id)
{
#if (defined(AUDIO_OUTPUT_DC_CALIB_ANA) || defined(AUDIO_OUTPUT_DC_CALIB)) && (!(defined(__TWS__) || defined(IBRT)) || defined(ANC_APP))
    // Disable PA
    analog_aud_codec_speaker_enable(false);
#endif

    codec->REG_098 &= ~CODEC_CODEC_DAC_EN_CH0;
    codec->REG_0A4 &= ~CODEC_CODEC_DAC_UH_EN;

#ifdef DAC_DRE_ENABLE
    hal_codec_dac_dre_disable();
#endif

#if defined(DAC_CLASSG_ENABLE)
    hal_codec_classg_enable(false);
#endif

#ifndef NO_DAC_RESET
    // Reset DAC
    // Avoid DAC outputing noise after it is disabled
    codec->REG_064 &= ~CODEC_SOFT_RSTN_DAC;
    codec->REG_064 |= CODEC_SOFT_RSTN_DAC;
#endif
#ifndef CODEC_TIMER
    codec->REG_09C &= ~CODEC_CODEC_DAC_GAIN_UPDATE;
    codec->REG_46C &= ~CODEC_CODEC_DAC_GAIN_UPDATE_SND;
    codec->REG_0E0 &= ~CODEC_CODEC_DAC_DC_UPDATE_CH0;
    hal_codec_reg_update_delay();
    codec->REG_060 &= ~CODEC_EN_CLK_DAC;
#endif
}

void hal_codec_start_playback_stream(enum HAL_CODEC_ID_T id)
{
#ifndef CODEC_TIMER
    codec->REG_060 |= CODEC_EN_CLK_DAC;
#endif
#ifndef NO_DAC_RESET
    // Reset DAC
    codec->REG_064 &= ~CODEC_SOFT_RSTN_DAC;
    codec->REG_064 |= CODEC_SOFT_RSTN_DAC;
#endif

#ifdef DAC_DRE_ENABLE
    if (
            //(codec->REG_044 & CODEC_MODE_16BIT_DAC) == 0 &&
#ifdef ANC_APP
            anc_adc_ch_map == 0 &&
#endif
            1
            )
    {
        hal_codec_dac_dre_enable();
    }
#endif

#ifdef PERF_TEST_POWER_KEY
    hal_codec_update_perf_test_power();
#endif

#if defined(DAC_CLASSG_ENABLE)
    hal_codec_classg_enable(true);
#endif

    codec->REG_098 |= CODEC_CODEC_DAC_EN_CH0;

    codec->REG_0A4 |= CODEC_CODEC_DAC_UH_EN;

#if (defined(AUDIO_OUTPUT_DC_CALIB_ANA) || defined(AUDIO_OUTPUT_DC_CALIB)) && (!(defined(__TWS__) || defined(IBRT)) || defined(ANC_APP))
#ifdef AUDIO_OUTPUT_DC_CALIB
    // At least delay 4ms for 8K-sample-rate mute data to arrive at DAC PA
    osDelay(5);
#endif

    // Enable PA
    analog_aud_codec_speaker_enable(true);

#ifdef AUDIO_ANC_FB_MC
    if (mc_started) {
        uint32_t lock;
        lock = int_lock();
        // MC FIFO and DAC FIFO must be started at the same time
        codec->REG_04C |= CODEC_MC_ENABLE;
        codec->REG_0A4 |= CODEC_CODEC_DAC_LH_EN;

#ifdef CODEC_SW_SYNC
        if (!codec_sw_sync_play_status)
#endif
        {
            codec->REG_000 |= CODEC_DAC_ENABLE;
        }
        int_unlock(lock);
    }
#endif
#endif
}

#ifdef AF_ADC_I2S_SYNC
static bool _hal_codec_capture_enable_delay = false;

void hal_codec_capture_enable_delay(void)
{
    _hal_codec_capture_enable_delay = true;
}

void hal_codec_capture_enable(void)
{
    codec->REG_080 |= CODEC_CODEC_ADC_EN;
}
#endif

int hal_codec_start_stream(enum HAL_CODEC_ID_T id, enum AUD_STREAM_T stream)
{
    if (stream == AUD_STREAM_PLAYBACK) {
        // Reset and start DAC
        hal_codec_start_playback_stream(id);
    } else {
#if defined(AUDIO_ANC_FB_MC)
        adc_en_map |= (1 << CODEC_ADC_EN_REQ_STREAM);
        if (adc_en_map == (1 << CODEC_ADC_EN_REQ_STREAM))
#endif
        {
            // Reset ADC ANA
            codec->REG_064 &= ~CODEC_SOFT_RSTN_ADC_ANA_MASK;
            codec->REG_064 |= CODEC_SOFT_RSTN_ADC_ANA_MASK;

#ifdef AF_ADC_I2S_SYNC
            if (_hal_codec_capture_enable_delay)
            {
                _hal_codec_capture_enable_delay = false;
            }
            else
            {
                hal_codec_capture_enable();
            }
#else
            codec->REG_080 |= CODEC_CODEC_ADC_EN;
#endif
        }
    }

    return 0;
}

int hal_codec_stop_stream(enum HAL_CODEC_ID_T id, enum AUD_STREAM_T stream)
{
    if (stream == AUD_STREAM_PLAYBACK) {
        // Stop and reset DAC
        hal_codec_stop_playback_stream(id);
    } else {
#if defined(AUDIO_ANC_FB_MC)
        adc_en_map &= ~(1 << CODEC_ADC_EN_REQ_STREAM);
        if (adc_en_map == 0)
#endif
        {
            codec->REG_080 &= ~CODEC_CODEC_ADC_EN;
#ifdef AF_ADC_I2S_SYNC
            _hal_codec_capture_enable_delay = false;
#endif
        }
    }

    return 0;
}

#ifdef __AUDIO_RESAMPLE__
void hal_codec_resample_clock_enable(enum AUD_STREAM_T stream)
{
    uint32_t clk;

    // 192K-24BIT requires 52M clock, and 384K-24BIT requires 104M clock
    if (stream == AUD_STREAM_PLAYBACK) {
        clk = codec_dac_sample_rate[resample_rate_idx[AUD_STREAM_PLAYBACK]].sample_rate * RS_CLOCK_FACTOR;
        hal_cmu_codec_rs_enable(clk);
    } else {
        clk = codec_adc_sample_rate[resample_rate_idx[AUD_STREAM_CAPTURE]].sample_rate * RS_CLOCK_FACTOR;
        hal_cmu_codec_rs_adc_enable(clk);
    }
}

void hal_codec_resample_clock_disable(enum AUD_STREAM_T stream)
{
    if (stream == AUD_STREAM_PLAYBACK) {
        hal_cmu_codec_rs_disable();
    } else {
        hal_cmu_codec_rs_adc_disable();
    }
}
#endif

#if defined(DIG_MIC_DYN_CODEC_VOLT) || defined(ANC_DYN_CODEC_VOLT)
static void hal_codec_codec_volt_ramp_up(enum CODEC_VOLT_RAMP_USER_T user)
{
#if defined(DIG_MIC_DYN_CODEC_VOLT) && defined(ANC_DYN_CODEC_VOLT)
    bool update = true;

    if (vcodec_ramp_user) {
        update = false;
    }
    vcodec_ramp_user |= user;
    if (!update) {
        return;
    }
#endif

    pmu_codec_volt_ramp_up();
}

static void hal_codec_codec_volt_ramp_down(enum CODEC_VOLT_RAMP_USER_T user)
{
#if defined(DIG_MIC_DYN_CODEC_VOLT) && defined(ANC_DYN_CODEC_VOLT)
    vcodec_ramp_user &= ~user;
    if (vcodec_ramp_user) {
        return;
    }
#endif

    pmu_codec_volt_ramp_down();
}
#endif

#if defined(DIG_MIC_DYN_CODEC_VOLT) || defined(ANC_DYN_CODEC_VOLT)
static void hal_codec_codec_volt_ramp_up(enum CODEC_VOLT_RAMP_USER_T user)
{
#if defined(DIG_MIC_DYN_CODEC_VOLT) && defined(ANC_DYN_CODEC_VOLT)
    bool update = true;

    if (vcodec_ramp_user) {
        update = false;
    }
    vcodec_ramp_user |= user;
    if (!update) {
        return;
    }
#endif

    pmu_codec_volt_ramp_up();
}

static void hal_codec_codec_volt_ramp_down(enum CODEC_VOLT_RAMP_USER_T user)
{
#if defined(DIG_MIC_DYN_CODEC_VOLT) && defined(ANC_DYN_CODEC_VOLT)
    vcodec_ramp_user &= ~user;
    if (vcodec_ramp_user) {
        return;
    }
#endif

    pmu_codec_volt_ramp_down();
}
#endif

static void hal_codec_enable_dig_mic(enum AUD_CHANNEL_MAP_T mic_map)
{
    uint32_t phase = 0;
    uint32_t line_map = 0;
    uint32_t rate_sel = 0;

#ifdef DIG_MIC_DYN_CODEC_VOLT
    if ((codec->REG_0C4 & CODEC_CODEC_PDM_ENABLE) == 0) {
        hal_codec_codec_volt_ramp_up(CODEC_VOLT_RAMP_USER_DIG_MIC);
    }
#endif


    phase = codec->REG_0A8;
    for (int i = 0; i < MAX_DIG_MIC_CH_NUM; i++) {
        if (mic_map & (AUD_CHANNEL_MAP_DIGMIC_CH0 << i)) {
            line_map |= (1 << (i / 2));
        }
        phase = (phase & ~(CODEC_CODEC_PDM_CAP_PHASE_CH0_MASK << (i * 2))) |
            (CODEC_CODEC_PDM_CAP_PHASE_CH0(codec_digmic_phase) << (i * 2));
    }
    if (CODEC_DIGMIC_CLK_DIV >= 8) {
        rate_sel = 3;
    } else if (CODEC_DIGMIC_CLK_DIV >= 4) {
        rate_sel = 2;
    } else if (CODEC_DIGMIC_CLK_DIV >= 2) {
        rate_sel = 1;
    }
    codec->REG_0A8 = phase;
    codec->REG_0A4 = SET_BITFIELD(codec->REG_0A4, CODEC_CODEC_PDM_RATE_SEL, rate_sel) | CODEC_CODEC_PDM_ENABLE;
    hal_iomux_set_dig_mic(line_map);
}

static void hal_codec_disable_dig_mic(void)
{
    codec->REG_0A4 &= ~CODEC_CODEC_PDM_ENABLE;

#ifdef DIG_MIC_DYN_CODEC_VOLT
    hal_codec_codec_volt_ramp_down(CODEC_VOLT_RAMP_USER_DIG_MIC);
#endif
}

static void hal_codec_ec_enable(void)
{
    codec->REG_094 |= CODEC_CODEC_ECHO_ENABLE_CH0;
}

static void hal_codec_ec_disable(void)
{
    codec->REG_094 &=  ~CODEC_CODEC_ECHO_ENABLE_CH0;
}

int hal_codec_start_interface(enum HAL_CODEC_ID_T id, enum AUD_STREAM_T stream, int dma)
{
    uint32_t fifo_flush = 0;

    if (stream == AUD_STREAM_PLAYBACK) {
#ifdef __AUDIO_RESAMPLE__
        if (codec->REG_0E4 & CODEC_CODEC_RESAMPLE_DAC_PHASE_UPDATE) {
            hal_codec_resample_clock_enable(stream);
            codec->REG_060 |= CODEC_EN_CLK_RS(1 << 0);
#if (defined(__TWS__) || defined(IBRT))
            enum HAL_CODEC_SYNC_TYPE_T sync_type;

            sync_type = GET_BITFIELD(codec->REG_0E4, CODEC_CODEC_RESAMPLE_DAC_UPDATE_TRIGGER_SEL);
            if (sync_type != HAL_CODEC_SYNC_TYPE_NONE) {
                codec->REG_0E4 = SET_BITFIELD(codec->REG_0E4, CODEC_CODEC_RESAMPLE_DAC_UPDATE_TRIGGER_SEL, HAL_CODEC_SYNC_TYPE_NONE);
                codec->REG_0E4 &= ~CODEC_CODEC_RESAMPLE_DAC_PHASE_UPDATE;
                hal_codec_reg_update_delay();
                codec->REG_0F4 = resample_phase_float_to_value(1.0f);
                hal_codec_reg_update_delay();
                codec->REG_0E4 |= CODEC_CODEC_RESAMPLE_DAC_PHASE_UPDATE;
                hal_codec_reg_update_delay();
                codec->REG_0E4 &= ~CODEC_CODEC_RESAMPLE_DAC_PHASE_UPDATE;
                codec->REG_0E4 = SET_BITFIELD(codec->REG_0E4, CODEC_CODEC_RESAMPLE_DAC_UPDATE_TRIGGER_SEL, sync_type);
                hal_codec_reg_update_delay();
                codec->REG_0F4 = resample_phase_float_to_value(get_playback_resample_phase());
                codec->REG_0E4 |= CODEC_CODEC_RESAMPLE_DAC_PHASE_UPDATE;
            }
#endif
            codec->REG_0E4 |= CODEC_CODEC_RESAMPLE_DAC_FIFO_ENABLE;
            codec->REG_0E4 |= CODEC_CODEC_RESAMPLE_DAC_ENABLE;
        }
#endif
        if ((codec->REG_000 & CODEC_ADC_ENABLE) && (codec_adc_ch_map & EC_ADC_MAP)) {
            hal_codec_ec_enable();
        }
#ifdef DAC_RAMP_GAIN
        if (codec->REG_0EC & CODEC_CODEC_RAMP_EN_CH0) {
            codec->REG_0EC &= ~CODEC_CODEC_RAMP_EN_CH0;
            hal_codec_reg_update_delay();
            hal_codec_set_dac_gain_value(VALID_DAC_MAP, 0);
            hal_codec_reg_update_delay();
            codec->REG_0EC |= CODEC_CODEC_RAMP_EN_CH0;
            hal_codec_reg_update_delay();
            hal_codec_restore_dig_dac_gain();
        }
#endif
#ifdef AUDIO_ANC_FB_MC
        fifo_flush |= CODEC_MC_FIFO_FLUSH;
#endif
        fifo_flush |= CODEC_TX_FIFO_FLUSH;
        codec->REG_004 |= fifo_flush;
        hal_codec_reg_update_delay();
        codec->REG_004 &= ~fifo_flush;
        if (dma) {
            codec->REG_008 = SET_BITFIELD(codec->REG_008, CODEC_CODEC_TX_THRESHOLD, HAL_CODEC_TX_FIFO_TRIGGER_LEVEL);
            codec->REG_000 |= CODEC_DMACTRL_TX;
            // Delay a little time for DMA to fill the TX FIFO before sending
            for (volatile int i = 0; i < 50; i++);
        }
#ifdef AUDIO_ANC_FB_MC
        if (mc_enabled) {
            if (mc_chan_num == AUD_CHANNEL_NUM_2) {
                codec->REG_04C |= CODEC_DUAL_CHANNEL_MC;
            } else {
                codec->REG_04C &= ~CODEC_DUAL_CHANNEL_MC;
            }
            if (mc_bits == AUD_BITS_16) {
                codec->REG_04C = (codec->REG_04C & ~CODEC_MODE_32BIT_MC) | CODEC_MODE_16BIT_MC;
            } else if (mc_bits == AUD_BITS_24) {
                codec->REG_04C &= ~(CODEC_MODE_16BIT_MC | CODEC_MODE_32BIT_MC);
            } else if (mc_bits == AUD_BITS_32) {
                codec->REG_04C = (codec->REG_04C & ~CODEC_MODE_16BIT_MC) | CODEC_MODE_32BIT_MC;
            } else {
                ASSERT(false, "%s: Bad mc bits: %u", __func__, mc_bits);
            }
            if (adc_en_map == 0) {
                // Reset ADC free running clock and ADC ANA
                codec->REG_064 &= ~(RSTN_ADC_FREE_RUNNING_CLK | CODEC_SOFT_RSTN_ADC_ANA_MASK);
                codec->REG_064 |= (RSTN_ADC_FREE_RUNNING_CLK | CODEC_SOFT_RSTN_ADC_ANA_MASK);
                codec->REG_080 |= CODEC_CODEC_ADC_EN;
            }
            adc_en_map |= (1 << CODEC_ADC_EN_REQ_MC);
            // If codec function has been enabled, start FIFOs directly;
            // otherwise, start FIFOs after PA is enabled
            if (codec->REG_0A4 & CODEC_CODEC_DAC_UH_EN) {
                uint32_t lock;
                lock = int_lock();
                // MC FIFO and DAC FIFO must be started at the same time
                codec->REG_04C |= CODEC_MC_ENABLE;

#ifdef CODEC_SW_SYNC
                if (!codec_sw_sync_play_status)
#endif
                {
                    codec->REG_000 |= CODEC_DAC_ENABLE;
                }
                codec->REG_0A4 |= CODEC_CODEC_DAC_LH_EN;
                int_unlock(lock);
            }
            mc_started = true;
        }
        else
#endif
        {

#ifdef CODEC_SW_SYNC
            if (!codec_sw_sync_play_status)
#endif
            {
                codec->REG_000 |= CODEC_DAC_ENABLE;
            }
            codec->REG_0A4 |= CODEC_CODEC_DAC_LH_EN;
        }
    } else {
#ifdef __AUDIO_RESAMPLE__
        if (codec->REG_0E4 & CODEC_CODEC_RESAMPLE_ADC_PHASE_UPDATE) {
            hal_codec_resample_clock_enable(stream);
#if (defined(__TWS__) || defined(IBRT)) && defined(ANC_APP)
            enum HAL_CODEC_SYNC_TYPE_T sync_type;

            sync_type = GET_BITFIELD(codec->REG_0E4, CODEC_CODEC_RESAMPLE_ADC_UPDATE_TRIGGER_SEL);
            if (sync_type != HAL_CODEC_SYNC_TYPE_NONE) {
                codec->REG_0E4 = SET_BITFIELD(codec->REG_0E4, CODEC_CODEC_RESAMPLE_ADC_UPDATE_TRIGGER_SEL, HAL_CODEC_SYNC_TYPE_NONE);
                codec->REG_0E4 &= ~CODEC_CODEC_RESAMPLE_ADC_PHASE_UPDATE;
                hal_codec_reg_update_delay();
                codec->REG_0F8 = resample_phase_float_to_value(1.0f);
                hal_codec_reg_update_delay();
                codec->REG_0E4 |= CODEC_CODEC_RESAMPLE_ADC_PHASE_UPDATE;
                hal_codec_reg_update_delay();
                codec->REG_0E4 &= ~CODEC_CODEC_RESAMPLE_ADC_PHASE_UPDATE;
                codec->REG_0E4 = SET_BITFIELD(codec->REG_0E4, CODEC_CODEC_RESAMPLE_ADC_UPDATE_TRIGGER_SEL, sync_type);
                hal_codec_reg_update_delay();
                codec->REG_0F8 = resample_phase_float_to_value(get_capture_resample_phase());
                codec->REG_0E4 |= CODEC_CODEC_RESAMPLE_ADC_PHASE_UPDATE;
            }
#endif
            codec->REG_0E4 |= CODEC_CODEC_RESAMPLE_ADC_ENABLE;
        }
#endif
        if (codec_mic_ch_map & AUD_CHANNEL_MAP_DIGMIC_ALL) {
            hal_codec_enable_dig_mic(codec_mic_ch_map);
        }
        if (codec_adc_ch_map & EC_ADC_MAP) {
            hal_codec_ec_enable();
        }
        int adc_cnt = 0;
        for (int i = 0; i < MAX_ADC_CH_NUM; i++) {
            if (codec_adc_ch_map & (AUD_CHANNEL_MAP_CH0 << i)) {
                codec->REG_080 |= (CODEC_CODEC_ADC_EN_CH0 << i);
                codec->REG_000 |= (CODEC_ADC_ENABLE_CH0 << adc_cnt);
                adc_cnt++;
            }
        }
        fifo_flush = CODEC_RX_FIFO_FLUSH_CH0 | CODEC_RX_FIFO_FLUSH_CH1 | CODEC_RX_FIFO_FLUSH_CH2 |
            CODEC_RX_FIFO_FLUSH_CH3;
        codec->REG_004 |= fifo_flush;
        hal_codec_reg_update_delay();
        codec->REG_004 &= ~fifo_flush;
        if (dma) {
            codec->REG_008 = SET_BITFIELD(codec->REG_008, CODEC_CODEC_RX_THRESHOLD, HAL_CODEC_RX_FIFO_TRIGGER_LEVEL);
            codec->REG_000 |= CODEC_DMACTRL_RX;
        }

#ifdef CODEC_SW_SYNC
        if (!codec_sw_sync_cap_status)
#endif
        {
            codec->REG_000 |= CODEC_ADC_ENABLE;
        }
    }

    return 0;
}

static void clear_playback_fifo_workaround(void)
{
    codec->REG_004 |= CODEC_TX_FIFO_FLUSH;

#ifdef __AUDIO_RESAMPLE__
    uint32_t resample_value = 0;

    codec->REG_0E4 &= ~CODEC_CODEC_RESAMPLE_DAC_PHASE_UPDATE;
    hal_codec_reg_update_delay();
    resample_value = codec->REG_0F4;
    codec->REG_0F4 = resample_phase_float_to_value(0.0f);
    hal_codec_reg_update_delay();
    codec->REG_0E4 |= CODEC_CODEC_RESAMPLE_DAC_PHASE_UPDATE;
    hal_codec_reg_update_delay();
#endif

    // Wait at least 1 sample time
    osDelay(1);

#ifdef __AUDIO_RESAMPLE__
    codec->REG_0E4 &= ~CODEC_CODEC_RESAMPLE_DAC_ENABLE;
#endif

    // Wait at least 1 sample time
    osDelay(1);

#ifdef __AUDIO_RESAMPLE__
    codec->REG_0E4 &= ~CODEC_CODEC_RESAMPLE_DAC_PHASE_UPDATE;
    hal_codec_reg_update_delay();
    codec->REG_0F4 = resample_value;
    hal_codec_reg_update_delay();
    codec->REG_0E4 |= CODEC_CODEC_RESAMPLE_DAC_PHASE_UPDATE;
    hal_codec_reg_update_delay();
#endif

    return;
}

int hal_codec_stop_interface(enum HAL_CODEC_ID_T id, enum AUD_STREAM_T stream)
{
    uint32_t fifo_flush = 0;

    if (stream == AUD_STREAM_PLAYBACK) {
        codec->REG_000 &= ~CODEC_DAC_ENABLE;
        codec->REG_000 &= ~CODEC_DMACTRL_TX;
        clear_playback_fifo_workaround();
        codec->REG_0A4 &= ~CODEC_CODEC_DAC_LH_EN;
#ifdef __AUDIO_RESAMPLE__
        codec->REG_0E4 &= ~CODEC_CODEC_RESAMPLE_DAC_ENABLE;
        codec->REG_0E4 &= ~CODEC_CODEC_RESAMPLE_DAC_FIFO_ENABLE;
        codec->REG_060 &= ~CODEC_EN_CLK_RS(1 << 0);
        hal_codec_resample_clock_disable(stream);
#endif
#ifdef AUDIO_ANC_FB_MC
        mc_started = false;
        codec->REG_04C &= ~CODEC_MC_ENABLE;
        adc_en_map &= ~(1 << CODEC_ADC_EN_REQ_MC);
        if (adc_en_map == 0) {
            codec->REG_080 &= ~CODEC_CODEC_ADC_EN;
        }
        fifo_flush |= CODEC_MC_FIFO_FLUSH;
#endif
        fifo_flush |= CODEC_TX_FIFO_FLUSH;
        codec->REG_004 |= fifo_flush;
        hal_codec_reg_update_delay();
        codec->REG_004 &= ~fifo_flush;
        // Cancel dac sync request
        hal_codec_sync_dac_disable();
        hal_codec_sync_dac_resample_rate_disable();
        hal_codec_sync_dac_gain_disable();
    } else {
        codec->REG_000 &= ~(CODEC_ADC_ENABLE | CODEC_ADC_ENABLE_CH0 | CODEC_ADC_ENABLE_CH1 | CODEC_ADC_ENABLE_CH2 |
            CODEC_ADC_ENABLE_CH3);
        codec->REG_000 &= ~CODEC_DMACTRL_RX;
        for (int i = 0; i < MAX_ADC_CH_NUM; i++) {
            if ((codec_adc_ch_map & (AUD_CHANNEL_MAP_CH0 << i)) &&
                    (anc_adc_ch_map & (AUD_CHANNEL_MAP_CH0 << i)) == 0) {
                codec->REG_080 &= ~(CODEC_CODEC_ADC_EN_CH0 << i);
            }
        }
        if (codec_adc_ch_map & EC_ADC_MAP) {
            hal_codec_ec_disable();
        }
        if ((codec_mic_ch_map & AUD_CHANNEL_MAP_DIGMIC_ALL) &&
                (anc_mic_ch_map & AUD_CHANNEL_MAP_DIGMIC_ALL) == 0) {
            hal_codec_disable_dig_mic();
        }
#ifdef __AUDIO_RESAMPLE__
        codec->REG_0E4 &= ~CODEC_CODEC_RESAMPLE_ADC_ENABLE;
        hal_codec_resample_clock_disable(stream);
#endif
        fifo_flush = CODEC_RX_FIFO_FLUSH_CH0 | CODEC_RX_FIFO_FLUSH_CH1 | CODEC_RX_FIFO_FLUSH_CH2 |
            CODEC_RX_FIFO_FLUSH_CH3;
        codec->REG_004 |= fifo_flush;
        hal_codec_reg_update_delay();
        codec->REG_004 &= ~fifo_flush;
        // Cancel adc sync request
        hal_codec_sync_adc_disable();
        hal_codec_sync_adc_resample_rate_disable();
        hal_codec_sync_adc_gain_disable();
    }

    return 0;
}

static void hal_codec_set_dac_gain_value(enum AUD_CHANNEL_MAP_T map, uint32_t val)
{
    codec->REG_09C &= ~CODEC_CODEC_DAC_GAIN_UPDATE;
    hal_codec_reg_update_delay();
    if (map & AUD_CHANNEL_MAP_CH0) {
        codec->REG_09C = SET_BITFIELD(codec->REG_09C, CODEC_CODEC_DAC_GAIN_CH0, val);
    }
    codec->REG_09C |= CODEC_CODEC_DAC_GAIN_UPDATE;
}

void hal_codec_get_dac_gain(float *left_gain, float *right_gain)
{
    struct DAC_GAIN_T {
        int32_t v : 20;
    };

    struct DAC_GAIN_T left;

    left.v  = GET_BITFIELD(codec->REG_09C, CODEC_CODEC_DAC_GAIN_CH0);

    *left_gain = left.v;

    // Gain format: 6.14
    *left_gain /= (1 << 14);
    if (right_gain) {
        *right_gain = *left_gain;
    }
}

void hal_codec_dac_mute(bool mute)
{
    codec_mute[AUD_STREAM_PLAYBACK] = mute;

#ifdef AUDIO_OUTPUT_SW_GAIN
    hal_codec_set_sw_gain(swdac_gain);
#else
    if (mute) {
        hal_codec_set_dac_gain_value(VALID_DAC_MAP, 0);
    } else {
        hal_codec_restore_dig_dac_gain();
    }
#endif
}

static float db_to_amplitude_ratio(int32_t db)
{
    float coef;

    if (db == ZERODB_DIG_DBVAL) {
        coef = 1;
    } else if (db <= MIN_DIG_DBVAL) {
        coef = 0;
    } else {
        if (db > MAX_DIG_DBVAL) {
            db = MAX_DIG_DBVAL;
        }
        coef = db_to_float(db);
    }

    return coef;
}

static float digdac_gain_to_float(int32_t gain)
{
    float coef;

#if defined(NOISE_GATING) && defined(NOISE_REDUCTION)
    gain += digdac_gain_offset_nr;
#endif

    coef = db_to_amplitude_ratio(gain);

#ifdef AUDIO_OUTPUT_DC_CALIB
    coef *= dac_dc_gain_attn;
#endif

#ifdef AUDIO_OUTPUT_FACTROY_CALIB
    coef *= dac_dc_gain_factory;
#endif

#ifdef ANC_APP
    coef *= anc_boost_gain_attn;
#endif

    return coef;
}

#ifdef AUDIO_OUTPUT_DC_AUTO_CALIB
void hal_codec_set_dig_dac_gain_dr(enum AUD_CHANNEL_MAP_T map, int32_t gain)
{
    uint32_t val;
    float coef;

    coef = db_to_amplitude_ratio(gain);

    // Gain format: 6.14
    int32_t s_val = (int32_t)(coef * (1 << 14));
    val = __SSAT(s_val, 20);
    hal_codec_set_dac_gain_value(map, val);
}
#endif

static void hal_codec_set_dig_dac_gain(enum AUD_CHANNEL_MAP_T map, int32_t gain)
{
    uint32_t val;
    float coef;
    bool mute;

    if (map & AUD_CHANNEL_MAP_CH0) {
        digdac_gain[0] = gain;
    }
    if (map & AUD_CHANNEL_MAP_CH1) {
        digdac_gain[1] = gain;
    }

#ifdef AUDIO_OUTPUT_SW_GAIN
    mute = false;
#else
    mute = codec_mute[AUD_STREAM_PLAYBACK];
#endif

#ifdef AUDIO_OUTPUT_DC_CALIB_ANA
    if (codec->REG_0A4 & CODEC_CODEC_DAC_SDM_CLOSE) {
        mute = true;
    }
#endif

    if (mute) {
        val = 0;
    } else {
        coef = digdac_gain_to_float(gain);

        // Gain format: 6.14
        int32_t s_val = (int32_t)(coef * (1 << 14));
        val = __SSAT(s_val, 20);
    }

    hal_codec_set_dac_gain_value(map, val);
}

static POSSIBLY_UNUSED void hal_codec_restore_dig_dac_gain(void)
{
    if (digdac_gain[0] == digdac_gain[1]) {
        hal_codec_set_dig_dac_gain(VALID_DAC_MAP, digdac_gain[0]);
    } else {
        hal_codec_set_dig_dac_gain(AUD_CHANNEL_MAP_CH0, digdac_gain[0]);
        hal_codec_set_dig_dac_gain(AUD_CHANNEL_MAP_CH1, digdac_gain[1]);
    }
}

#ifdef AUDIO_OUTPUT_SW_GAIN
static void hal_codec_set_sw_gain(int32_t gain)
{
    float coef;
    bool mute;

    swdac_gain = gain;

    mute = codec_mute[AUD_STREAM_PLAYBACK];

    if (mute) {
        coef = 0;
    } else {
        coef = digdac_gain_to_float(gain);
    }

    if (sw_output_coef_callback) {
        sw_output_coef_callback(coef);
    }
}

void hal_codec_set_sw_output_coef_callback(HAL_CODEC_SW_OUTPUT_COEF_CALLBACK callback)
{
    sw_output_coef_callback = callback;
}
#endif

#if defined(CODEC_ADC_DC_FILTER_FACTOR)
void hal_codec_enable_adc_dc_filter(enum AUD_CHANNEL_MAP_T map, uint32_t enable)
{
    uint32_t val = codec->REG_198;

    for (int i = 0; i < NORMAL_ADC_CH_NUM; i++) {
        if (map & (AUD_CHANNEL_MAP_CH0 << i)) {
            if (enable) {
                val &= ~(CODEC_CODEC_ADC_UDC_CH0_MASK << (4 * i));
                codec->REG_198 = val | CODEC_CODEC_ADC_UDC_CH0(CODEC_ADC_DC_FILTER_FACTOR) << (4 * i);

                codec->REG_0A4 &= ~(CODEC_CODEC_ADC_DCF_BYPASS_CH0 << i);
            } else {
                codec->REG_0A4 |= (CODEC_CODEC_ADC_DCF_BYPASS_CH0 << i);
            }
        }
    }
}
#endif

static void hal_codec_set_adc_gain_value(enum AUD_CHANNEL_MAP_T map, uint32_t val)
{
    uint32_t gain_update = 0;

    for (int i = 0; i < NORMAL_ADC_CH_NUM; i++) {
        if (map & (AUD_CHANNEL_MAP_CH0 << i)) {
            *(&codec->REG_084 + i) = SET_BITFIELD(*(&codec->REG_084 + i), CODEC_CODEC_ADC_GAIN_CH0, val);
            gain_update |= (CODEC_CODEC_ADC_GAIN_UPDATE_CH0 << i);
        }
    }
    codec->REG_09C &= ~gain_update;
    hal_codec_reg_update_delay();
    codec->REG_09C |= gain_update;
}

static void hal_codec_set_dig_adc_gain(enum AUD_CHANNEL_MAP_T map, int32_t gain)
{
    uint32_t val;
    float coef;
    bool mute;
    int i;
    int32_t s_val;

    for (i = 0; i < NORMAL_ADC_CH_NUM; i++) {
        if (map & (1 << i)) {
            digadc_gain[i] = gain;
        }
    }

    mute = codec_mute[AUD_STREAM_CAPTURE];

    if (mute) {
        val = 0;
    } else {
#ifdef ANC_APP
        enum AUD_CHANNEL_MAP_T adj_map;
        int32_t anc_gain;

        adj_map = map & anc_adc_gain_offset_map;
        while (adj_map) {
            i = get_msb_pos(adj_map);
            adj_map &= ~(1 << i);
            anc_gain = gain + anc_adc_gain_offset[i];
            coef = db_to_amplitude_ratio(anc_gain);
            coef /= anc_boost_gain_attn;
            // Gain format: 8.12
            s_val = (int32_t)(coef * (1 << 12));
            val = __SSAT(s_val, 20);
            hal_codec_set_adc_gain_value((1 << i), val);
        }

        map &= ~anc_adc_gain_offset_map;
#endif

        if (map) {
            coef = db_to_amplitude_ratio(gain);
#ifdef ANC_APP
            coef /= anc_boost_gain_attn;
#endif
            // Gain format: 8.12
            s_val = (int32_t)(coef * (1 << 12));
            val = __SSAT(s_val, 20);
        } else {
            val = 0;
        }
    }

    hal_codec_set_adc_gain_value(map, val);
}

static void hal_codec_restore_dig_adc_gain(void)
{
    int i;

    for (i = 0; i < NORMAL_ADC_CH_NUM; i++) {
        hal_codec_set_dig_adc_gain((1 << i), digadc_gain[i]);
    }
}

static void POSSIBLY_UNUSED hal_codec_get_adc_gain(enum AUD_CHANNEL_MAP_T map, float *gain)
{
    struct ADC_GAIN_T {
        int32_t v : 20;
    };

    struct ADC_GAIN_T adc_val;

    for (int i = 0; i < NORMAL_ADC_CH_NUM; i++) {
        if (map & (AUD_CHANNEL_MAP_CH0 << i)) {
            adc_val.v = GET_BITFIELD(*(&codec->REG_084 + i), CODEC_CODEC_ADC_GAIN_CH0);

            *gain = adc_val.v;
            // Gain format: 8.12
            *gain /= (1 << 12);
            return;
        }
    }

    *gain = 0;
}

void hal_codec_adc_mute(bool mute)
{
    codec_mute[AUD_STREAM_CAPTURE] = mute;

    if (mute) {
        hal_codec_set_adc_gain_value(NORMAL_ADC_MAP, 0);
    } else {
        hal_codec_restore_dig_adc_gain();
    }
}

int hal_codec_set_chan_vol(enum AUD_STREAM_T stream, enum AUD_CHANNEL_MAP_T ch_map, uint8_t vol)
{
    if (stream == AUD_STREAM_PLAYBACK) {
#ifdef AUDIO_OUTPUT_SW_GAIN
        ASSERT(false, "%s: Cannot set play chan vol with AUDIO_OUTPUT_SW_GAIN", __func__);
#else
#ifdef SINGLE_CODEC_DAC_VOL
        ASSERT(false, "%s: Cannot set play chan vol with SINGLE_CODEC_DAC_VOL", __func__);
#else
        const struct CODEC_DAC_VOL_T *vol_tab_ptr;

        vol_tab_ptr = hal_codec_get_dac_volume(vol);
        if (vol_tab_ptr) {
            if (ch_map & AUD_CHANNEL_MAP_CH0) {
                hal_codec_set_dig_dac_gain(AUD_CHANNEL_MAP_CH0, vol_tab_ptr->sdac_volume);
            }
            if (ch_map & AUD_CHANNEL_MAP_CH1) {
                hal_codec_set_dig_dac_gain(AUD_CHANNEL_MAP_CH1, vol_tab_ptr->sdac_volume);
            }
        }
#endif
#endif
    } else {
#ifdef SINGLE_CODEC_ADC_VOL
        ASSERT(false, "%s: Cannot set cap chan vol with SINGLE_CODEC_ADC_VOL", __func__);
#else
        uint8_t mic_ch, adc_ch;
        enum AUD_CHANNEL_MAP_T map;
        const CODEC_ADC_VOL_T *adc_gain_db;

        adc_gain_db = hal_codec_get_adc_volume(vol);
        if (adc_gain_db) {
            map = ch_map & ~EC_MIC_MAP;
            while (map) {
                mic_ch = get_lsb_pos(map);
                map &= ~(1 << mic_ch);
                adc_ch = hal_codec_get_adc_chan(1 << mic_ch);
                ASSERT(adc_ch < NORMAL_ADC_CH_NUM, "%s: Bad cap ch_map=0x%X (ch=%u)", __func__, ch_map, mic_ch);
                hal_codec_set_dig_adc_gain((1 << adc_ch), *adc_gain_db);
            }
        }
#endif
    }

    return 0;
}

static int hal_codec_set_dac_hbf_bypass_cnt(uint32_t cnt)
{
    uint32_t bypass = 0;
    uint32_t bypass_mask = CODEC_CODEC_DAC_HBF1_BYPASS | CODEC_CODEC_DAC_HBF2_BYPASS | CODEC_CODEC_DAC_HBF3_BYPASS;

    if (cnt == 0) {
    } else if (cnt == 1) {
        bypass = CODEC_CODEC_DAC_HBF3_BYPASS;
    } else if (cnt == 2) {
        bypass = CODEC_CODEC_DAC_HBF2_BYPASS | CODEC_CODEC_DAC_HBF3_BYPASS;
    } else if (cnt == 3) {
        bypass = CODEC_CODEC_DAC_HBF1_BYPASS | CODEC_CODEC_DAC_HBF2_BYPASS | CODEC_CODEC_DAC_HBF3_BYPASS;
    } else {
        ASSERT(false, "%s: Invalid dac bypass cnt: %u", __FUNCTION__, cnt);
    }

    // OSR is fixed to 512
    //codec->REG_098 = SET_BITFIELD(codec->REG_098, CODEC_CODEC_DAC_OSR_SEL, 2);

    codec->REG_098 = (codec->REG_098 & ~bypass_mask) | bypass;
    return 0;
}

static int hal_codec_set_dac_up(uint32_t val)
{
    uint32_t sel = 0;

    if (val == 2) {
        sel = 0;
    } else if (val == 3) {
        sel = 1;
    } else if (val == 4) {
        sel = 2;
    } else if (val == 6) {
        sel = 3;
    } else if (val == 1) {
        sel = 4;
    } else {
        ASSERT(false, "%s: Invalid dac up: %u", __FUNCTION__, val);
    }
    codec->REG_098 = SET_BITFIELD(codec->REG_098, CODEC_CODEC_DAC_UP_SEL, sel);
    return 0;
}

static uint32_t POSSIBLY_UNUSED hal_codec_get_dac_up(void)
{
    uint32_t sel;

    sel = GET_BITFIELD(codec->REG_098, CODEC_CODEC_DAC_UP_SEL);
    if (sel == 0) {
        return 2;
    } else if (sel == 1) {
        return 3;
    } else if (sel == 2) {
        return 4;
    } else if (sel == 3) {
        return 6;
    } else {
        return 1;
    }
}

static int hal_codec_set_adc_down(enum AUD_CHANNEL_MAP_T map, uint32_t val)
{
    uint32_t sel = 0;

    if (val == 3) {
        sel = 0;
    } else if (val == 6) {
        sel = 1;
    } else if (val == 1) {
        sel = 2;
    } else {
        ASSERT(false, "%s: Invalid adc down: %u", __FUNCTION__, val);
    }
    for (int i = 0; i < NORMAL_ADC_CH_NUM; i++) {
        if (map & (AUD_CHANNEL_MAP_CH0 << i)) {
            *(&codec->REG_084 + i) = SET_BITFIELD(*(&codec->REG_084 + i), CODEC_CODEC_ADC_DOWN_SEL_CH0, sel);
        }
    }
    if (map & AUD_CHANNEL_MAP_CH3) {
        codec->REG_094 = SET_BITFIELD(codec->REG_094, CODEC_CODEC_ADC_ECHO_DOWN_SEL_CH0, sel);
    }
    return 0;
}

static int hal_codec_set_adc_hbf_bypass_cnt(enum AUD_CHANNEL_MAP_T map, uint32_t cnt)
{
    uint32_t bypass = 0;
    uint32_t bypass_mask = CODEC_CODEC_ADC_HBF1_BYPASS_CH0 | CODEC_CODEC_ADC_HBF2_BYPASS_CH0 | CODEC_CODEC_ADC_HBF3_BYPASS_CH0;
    uint32_t bypass_ec0 = 0;
    uint32_t bypass_mask_ec0 = CODEC_CODEC_ADC_ECHO_HBF1_BYPASS_CH0 | CODEC_CODEC_ADC_ECHO_HBF2_BYPASS_CH0 | CODEC_CODEC_ADC_ECHO_HBF3_BYPASS_CH0;

    if (cnt == 0) {
    } else if (cnt == 1) {
        bypass = CODEC_CODEC_ADC_HBF3_BYPASS_CH0;
        bypass_ec0 = CODEC_CODEC_ADC_ECHO_HBF3_BYPASS_CH0;
    } else if (cnt == 2) {
        bypass = CODEC_CODEC_ADC_HBF2_BYPASS_CH0 | CODEC_CODEC_ADC_HBF3_BYPASS_CH0;
        bypass_ec0 = CODEC_CODEC_ADC_ECHO_HBF2_BYPASS_CH0 | CODEC_CODEC_ADC_ECHO_HBF3_BYPASS_CH0;
    } else if (cnt == 3) {
        bypass = CODEC_CODEC_ADC_HBF1_BYPASS_CH0 | CODEC_CODEC_ADC_HBF2_BYPASS_CH0 | CODEC_CODEC_ADC_HBF3_BYPASS_CH0;
        bypass_ec0 = CODEC_CODEC_ADC_ECHO_HBF1_BYPASS_CH0 | CODEC_CODEC_ADC_ECHO_HBF2_BYPASS_CH0 | CODEC_CODEC_ADC_ECHO_HBF3_BYPASS_CH0;
    } else {
        ASSERT(false, "%s: Invalid bypass cnt: %u", __FUNCTION__, cnt);
    }
    for (int i = 0; i < NORMAL_ADC_CH_NUM; i++) {
        if (map & (AUD_CHANNEL_MAP_CH0 << i)) {
            *(&codec->REG_084 + i) = (*(&codec->REG_084 + i) & ~bypass_mask) | bypass;
        }
    }
    if (map & AUD_CHANNEL_MAP_CH3) {
        codec->REG_094 = (codec->REG_094 & ~bypass_mask_ec0) | bypass_ec0;
    }
    return 0;
}

#ifdef __AUDIO_RESAMPLE__
static float get_playback_resample_phase(void)
{
    return (float)codec_dac_sample_rate[resample_rate_idx[AUD_STREAM_PLAYBACK]].codec_freq / hal_cmu_get_crystal_freq();
}

static float get_capture_resample_phase(void)
{
    return (float)hal_cmu_get_crystal_freq() / codec_adc_sample_rate[resample_rate_idx[AUD_STREAM_CAPTURE]].codec_freq;
}

static uint32_t resample_phase_float_to_value(float phase)
{
    if (phase >= 4.0) {
        return (uint32_t)-1;
    } else {
        // Phase format: 2.30
        return (uint32_t)(phase * (1 << 30));
    }
}

static float POSSIBLY_UNUSED resample_phase_value_to_float(uint32_t value)
{
    // Phase format: 2.30
    return (float)value / (1 << 30);
}
#endif

#ifdef SIDETONE_ENABLE
static void POSSIBLY_UNUSED hal_codec_set_sidetone_adc_chan(enum AUD_CHANNEL_MAP_T chan_map)
{
    uint8_t ch;
    ch = get_msb_pos(chan_map);
    codec->REG_080 = SET_BITFIELD(codec->REG_080, CODEC_CODEC_SIDE_TONE_MIC_SEL, ch);
}
#endif

static void hal_codec_set_dac_gain_ramp_interval(enum AUD_SAMPRATE_T rate)
{
#ifdef DAC_RAMP_GAIN
    int ramp_intvl = CODEC_DAC_GAIN_RAMP_INTERVAL;

    if ((codec->REG_0EC & CODEC_CODEC_RAMP_EN_CH0) == 0) {
        return;
    }

    if (rate >= AUD_SAMPRATE_44100 * 8) {
        ramp_intvl += 3;
    } else if (rate >= AUD_SAMPRATE_44100 * 4) {
        ramp_intvl += 2;
    } else if (rate >= AUD_SAMPRATE_44100 * 2) {
        ramp_intvl += 1;
    } else if (rate >= AUD_SAMPRATE_44100) {
        ramp_intvl += 0;
    } else if (rate >= AUD_SAMPRATE_44100 / 2) {
        ramp_intvl -= 1;
    } else if (rate >= AUD_SAMPRATE_44100 / 4) {
        ramp_intvl -= 2;
    } else {
        ramp_intvl -= 3;
    }
    if (ramp_intvl < 0) {
        ramp_intvl = 0;
    } else if (ramp_intvl >= CODEC_DAC_GAIN_RAMP_INTVL_QTY) {
        ramp_intvl = CODEC_DAC_GAIN_RAMP_INTVL_QTY - 1;
    }
    codec->REG_0EC = SET_BITFIELD(codec->REG_0EC, CODEC_CODEC_RAMP_INTERVAL_CH0, ramp_intvl);
#endif
}

static void hal_codec_set_adc_mic_sel(uint8_t adc_idx, uint8_t mic_idx)
{
    if ((1 << mic_idx) & AUD_CHANNEL_MAP_DIGMIC_ALL) {
        mic_idx = hal_codec_get_digmic_hw_index(mic_idx);
        codec->REG_0A8 = (codec->REG_0A8 & ~(CODEC_CODEC_PDM_MUX_CH0_MASK << (3 * adc_idx))) |
            (CODEC_CODEC_PDM_MUX_CH0(mic_idx) << (3 * adc_idx));
        codec->REG_0A4 |= CODEC_CODEC_PDM_ADC_SEL_CH0 << adc_idx;
    } else {
        *(&codec->REG_084 + adc_idx) = SET_BITFIELD(*(&codec->REG_084 + adc_idx), CODEC_CODEC_ADC_IN_SEL_CH0, mic_idx);
        codec->REG_0A4 &= ~(CODEC_CODEC_PDM_ADC_SEL_CH0 << adc_idx);
    }
}

int hal_codec_setup_stream(enum HAL_CODEC_ID_T id, enum AUD_STREAM_T stream, const struct HAL_CODEC_CONFIG_T *cfg)
{
    int i;
    int rate_idx;
    uint32_t ana_dig_div;
    enum AUD_SAMPRATE_T sample_rate;
    POSSIBLY_UNUSED uint32_t mask, val;

    if (stream == AUD_STREAM_PLAYBACK) {
        if ((HAL_CODEC_CONFIG_CHANNEL_MAP | HAL_CODEC_CONFIG_CHANNEL_NUM) & cfg->set_flag) {
            if (cfg->channel_num == AUD_CHANNEL_NUM_2) {
                if (cfg->channel_map != (AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1)) {
                    TRACE(2,"\n!!! WARNING:%s: Bad play stereo ch map: 0x%X\n", __func__, cfg->channel_map);
                }
                codec->REG_044 |= CODEC_DUAL_CHANNEL_DAC;
            } else {
                ASSERT(cfg->channel_num == AUD_CHANNEL_NUM_1, "%s: Bad play ch num: %u", __func__, cfg->channel_num);
                // Allow to DMA one channel but output 2 channels
                ASSERT((cfg->channel_map & ~(AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1)) == 0,
                    "%s: Bad play mono ch map: 0x%X", __func__, cfg->channel_map);
                codec->REG_044 &= ~CODEC_DUAL_CHANNEL_DAC;
            }
            codec_dac_ch_map = AUD_CHANNEL_MAP_CH0;
        }

        if (HAL_CODEC_CONFIG_BITS & cfg->set_flag) {
            if (cfg->bits == AUD_BITS_16) {
                codec->REG_044 = (codec->REG_044 & ~CODEC_MODE_32BIT_DAC) | CODEC_MODE_16BIT_DAC;
            } else if (cfg->bits == AUD_BITS_24) {
                codec->REG_044 &= ~(CODEC_MODE_16BIT_DAC | CODEC_MODE_32BIT_DAC);
            } else if (cfg->bits == AUD_BITS_32) {
                codec->REG_044 = (codec->REG_044 & ~CODEC_MODE_16BIT_DAC) | CODEC_MODE_32BIT_DAC;
            } else {
                ASSERT(false, "%s: Bad play bits: %u", __func__, cfg->bits);
            }
        }

        if (HAL_CODEC_CONFIG_SAMPLE_RATE & cfg->set_flag) {
            sample_rate = cfg->sample_rate;

            for (i = 0; i < ARRAY_SIZE(codec_dac_sample_rate); i++) {
                if (codec_dac_sample_rate[i].sample_rate == sample_rate) {
                    break;
                }
            }
            ASSERT(i < ARRAY_SIZE(codec_dac_sample_rate), "%s: Invalid playback sample rate: %u", __func__, sample_rate);
            rate_idx = i;
            ana_dig_div = codec_dac_sample_rate[rate_idx].codec_div / codec_dac_sample_rate[rate_idx].cmu_div;
            ASSERT(ana_dig_div * codec_dac_sample_rate[rate_idx].cmu_div == codec_dac_sample_rate[rate_idx].codec_div,
                "%s: Invalid playback div for rate %u: codec_div=%u cmu_div=%u", __func__, sample_rate,
                codec_dac_sample_rate[rate_idx].codec_div, codec_dac_sample_rate[rate_idx].cmu_div);

            TRACE(2,"[%s] playback sample_rate=%d", __func__, sample_rate);

#ifdef CODEC_TIMER
            cur_codec_freq = codec_dac_sample_rate[rate_idx].codec_freq;
#endif

            codec_rate_idx[AUD_STREAM_PLAYBACK] = rate_idx;

#ifdef __AUDIO_RESAMPLE__
            if (hal_cmu_get_audio_resample_status() && codec_dac_sample_rate[rate_idx].codec_freq != CODEC_FREQ_CRYSTAL) {
#ifdef CODEC_TIMER
                cur_codec_freq = CODEC_FREQ_CRYSTAL;
#endif
                if ((codec->REG_0E4 & CODEC_CODEC_RESAMPLE_DAC_PHASE_UPDATE) == 0 ||
                        resample_rate_idx[AUD_STREAM_PLAYBACK] != rate_idx) {
                    resample_rate_idx[AUD_STREAM_PLAYBACK] = rate_idx;
                    codec->REG_0E4 &= ~CODEC_CODEC_RESAMPLE_DAC_PHASE_UPDATE;
                    hal_codec_reg_update_delay();
                    codec->REG_0F4 = resample_phase_float_to_value(get_playback_resample_phase());
                    hal_codec_reg_update_delay();
                    codec->REG_0E4 |= CODEC_CODEC_RESAMPLE_DAC_PHASE_UPDATE;
                }

                mask = CODEC_CODEC_RESAMPLE_DAC_L_ENABLE;
                val = 0;
                if (codec_dac_ch_map & AUD_CHANNEL_MAP_CH0) {
                    val |= CODEC_CODEC_RESAMPLE_DAC_L_ENABLE;
                }
            } else {
                mask = CODEC_CODEC_RESAMPLE_DAC_L_ENABLE | CODEC_CODEC_RESAMPLE_DAC_PHASE_UPDATE;
                val = 0;
            }
            codec->REG_0E4 = (codec->REG_0E4 & ~mask) | val;
#endif

#ifdef __AUDIO_RESAMPLE__
            if (!hal_cmu_get_audio_resample_status())
#endif
            {
#ifdef __AUDIO_RESAMPLE__
                ASSERT(codec_dac_sample_rate[rate_idx].codec_freq != CODEC_FREQ_CRYSTAL,
                    "%s: playback sample rate %u is for resample only", __func__, sample_rate);
#endif
                analog_aud_freq_pll_config(codec_dac_sample_rate[rate_idx].codec_freq, codec_dac_sample_rate[rate_idx].codec_div);
                hal_cmu_codec_dac_set_div(codec_dac_sample_rate[rate_idx].cmu_div * CODEC_FREQ_EXTRA_DIV);
            }

            hal_codec_set_dac_gain_ramp_interval(cfg->sample_rate);

            hal_codec_set_dac_up(codec_dac_sample_rate[rate_idx].dac_up);
            hal_codec_set_dac_hbf_bypass_cnt(codec_dac_sample_rate[rate_idx].bypass_cnt);

#ifdef AUDIO_ANC_FB_MC
            codec->REG_04C = SET_BITFIELD(codec->REG_04C, CODEC_MC_DELAY, codec_dac_sample_rate[rate_idx].mc_delay);
#endif
        }

        if (HAL_CODEC_CONFIG_VOL & cfg->set_flag) {
            const struct CODEC_DAC_VOL_T *vol_tab_ptr;

            vol_tab_ptr = hal_codec_get_dac_volume(cfg->vol);
            if (vol_tab_ptr) {
#ifdef AUDIO_OUTPUT_SW_GAIN
                hal_codec_set_sw_gain(vol_tab_ptr->sdac_volume);
#else
                analog_aud_set_dac_gain(vol_tab_ptr->tx_pa_gain);
                hal_codec_set_dig_dac_gain(VALID_DAC_MAP, vol_tab_ptr->sdac_volume);
#endif
#ifdef PERF_TEST_POWER_KEY
                // Update performance test power after applying new dac volume
                hal_codec_update_perf_test_power();
#endif
            }
        }
    } else {
        enum AUD_CHANNEL_MAP_T mic_map;
        enum AUD_CHANNEL_MAP_T reserv_map;
        uint8_t cnt;
        uint8_t ch_idx;
        uint32_t cfg_set_mask;
        uint32_t cfg_clr_mask;
        bool alloc_adc = false;

        mic_map = 0;
        if ((HAL_CODEC_CONFIG_CHANNEL_MAP | HAL_CODEC_CONFIG_CHANNEL_NUM) & cfg->set_flag) {
            alloc_adc = true;
        }
#if defined(ANC_APP) && defined(ANC_TT_MIC_CH_L)
        if (ANC_TT_MIC_CH_L && (FORCE_TT_ADC_ALLOC || anc_tt_adc_ch_l == 0)) {
            alloc_adc = true;
        }
#endif

        if (alloc_adc) {
            mic_map = cfg->channel_map;
            codec_mic_ch_map = mic_map;
            codec_adc_ch_map = 0;
#ifdef SIDETONE_DEDICATED_ADC_CHAN
            sidetone_adc_ch_map = 0;
#endif
            reserv_map = 0;

#ifdef ANC_APP
#if defined(ANC_FF_MIC_CH_L)
#ifdef ANC_PROD_TEST
            if ((ANC_FF_MIC_CH_L & ~NORMAL_MIC_MAP) || (ANC_FF_MIC_CH_L & (ANC_FF_MIC_CH_L - 1))) {
                ASSERT(false, "Invalid ANC_FF_MIC_CH_L: 0x%04X", ANC_FF_MIC_CH_L);
            }
#if defined(ANC_FB_MIC_CH_L)
            if (ANC_FF_MIC_CH_L & ANC_FB_MIC_CH_L) {
                ASSERT(false, "Conflicted FF MIC (0x%04X) and FB MIC (0x%04X)",
                    ANC_FF_MIC_CH_L, ANC_FB_MIC_CH_L);
            }
#endif
#else // !ANC_PROD_TEST
#if (ANC_FF_MIC_CH_L & ~NORMAL_MIC_MAP) || (ANC_FF_MIC_CH_L & (ANC_FF_MIC_CH_L - 1))
#error "Invalid ANC_FF_MIC_CH_L"
#endif
#if defined(ANC_FB_MIC_CH_L)
#if (ANC_FF_MIC_CH_L & ANC_FB_MIC_CH_L)
#error "Conflicted ANC_FF_MIC_CH_L and ANC_FB_MIC_CH_L"
#endif
#endif
#endif // !ANC_PROD_TEST
            if (mic_map & ANC_FF_MIC_CH_L) {
                codec_adc_ch_map |= AUD_CHANNEL_MAP_CH0;
                mic_map &= ~ANC_FF_MIC_CH_L;
                ch_idx = get_msb_pos(ANC_FF_MIC_CH_L);
                if (ANC_FF_MIC_CH_L & AUD_CHANNEL_MAP_DIGMIC_ALL) {
                    ch_idx = hal_codec_get_digmic_hw_index(ch_idx);
                    codec->REG_0A8 = SET_BITFIELD(codec->REG_0A8, CODEC_CODEC_PDM_MUX_CH0, ch_idx);
                    codec->REG_0A4 |= CODEC_CODEC_PDM_ADC_SEL_CH0;
                } else {
                    codec->REG_084 = SET_BITFIELD(codec->REG_084, CODEC_CODEC_ADC_IN_SEL_CH0, ch_idx);
                    codec->REG_0A4 &= ~CODEC_CODEC_PDM_ADC_SEL_CH0;
                }
            } else if (ANC_FF_MIC_CH_L & AUD_CHANNEL_MAP_ALL) {
                reserv_map |= AUD_CHANNEL_MAP_CH0;
            }
#if defined(SIDETONE_ENABLE) && !defined(SIDETONE_DEDICATED_ADC_CHAN)
            if (CFG_HW_AUD_SIDETONE_MIC_DEV == ANC_FF_MIC_CH_L) {
                hal_codec_set_sidetone_adc_chan(AUD_CHANNEL_MAP_CH0);
            }
#endif
#endif

#if defined(ANC_FB_MIC_CH_L)
#ifdef ANC_PROD_TEST
            if ((ANC_FB_MIC_CH_L & ~NORMAL_MIC_MAP) || (ANC_FB_MIC_CH_L & (ANC_FB_MIC_CH_L - 1))) {
                ASSERT(false, "Invalid ANC_FB_MIC_CH_L: 0x%04X", ANC_FB_MIC_CH_L);
            }
#if defined(ANC_TT_MIC_CH_L)
            if (ANC_TT_MIC_CH_L & ANC_FB_MIC_CH_L) {
                ASSERT(false, "Conflicted TT MIC (0x%04X) and FB MIC (0x%04X)",
                    ANC_TT_MIC_CH_L, ANC_FB_MIC_CH_L);
            }
#endif
#else // !ANC_PROD_TEST
#if (ANC_FB_MIC_CH_L & ~NORMAL_MIC_MAP) || (ANC_FB_MIC_CH_L & (ANC_FB_MIC_CH_L - 1))
#error "Invalid ANC_FB_MIC_CH_L"
#endif
#if defined(ANC_TT_MIC_CH_L)
#if (ANC_TT_MIC_CH_L & ANC_FB_MIC_CH_L)
#error "Conflicted ANC_TT_MIC_CH_L and ANC_FB_MIC_CH_L"
#endif
#endif
#endif // !ANC_PROD_TEST
            if (mic_map & ANC_FB_MIC_CH_L) {
                codec_adc_ch_map |= AUD_CHANNEL_MAP_CH1;
                mic_map &= ~ANC_FB_MIC_CH_L;
                ch_idx = get_msb_pos(ANC_FB_MIC_CH_L);
                if (ANC_FB_MIC_CH_L & AUD_CHANNEL_MAP_DIGMIC_ALL) {
                    ch_idx = hal_codec_get_digmic_hw_index(ch_idx);
                    codec->REG_0A8 = SET_BITFIELD(codec->REG_0A8, CODEC_CODEC_PDM_MUX_CH1, ch_idx);
                    codec->REG_0A4 |= CODEC_CODEC_PDM_ADC_SEL_CH1;
                } else {
                    codec->REG_088 = SET_BITFIELD(codec->REG_088, CODEC_CODEC_ADC_IN_SEL_CH1, ch_idx);
                    codec->REG_0A4 &= ~CODEC_CODEC_PDM_ADC_SEL_CH1;
                }
            } else if (ANC_FB_MIC_CH_L & AUD_CHANNEL_MAP_ALL) {
                reserv_map |= AUD_CHANNEL_MAP_CH1;
            }

#if defined(SIDETONE_ENABLE) && !defined(SIDETONE_DEDICATED_ADC_CHAN)
            if (CFG_HW_AUD_SIDETONE_MIC_DEV == ANC_FB_MIC_CH_L) {
                hal_codec_set_sidetone_adc_chan(AUD_CHANNEL_MAP_CH1);
            }
#endif
#endif

            reserv_map |= codec_adc_ch_map;

#if defined(ANC_TT_MIC_CH_L)
#ifdef ANC_PROD_TEST
            if ((ANC_TT_MIC_CH_L & ~NORMAL_MIC_MAP) || (ANC_TT_MIC_CH_L & (ANC_TT_MIC_CH_L - 1))) {
                ASSERT(false, "Invalid ANC_TT_MIC_CH_L: 0x%04X", ANC_TT_MIC_CH_L);
            }
#else // !ANC_PROD_TEST
#if (ANC_TT_MIC_CH_L & ~NORMAL_MIC_MAP) || (ANC_TT_MIC_CH_L & (ANC_TT_MIC_CH_L - 1))
#error "Invalid ANC_TT_MIC_CH_L"
#endif
#endif // !ANC_PROD_TEST
            if (ANC_TT_MIC_CH_L && (FORCE_TT_ADC_ALLOC || anc_tt_adc_ch_l == 0)) {
                if (ANC_TT_MIC_CH_L == ANC_FF_MIC_CH_L) {
                    anc_tt_adc_ch_l = AUD_CHANNEL_MAP_CH0;
                } else {
                    anc_tt_adc_ch_l = NORMAL_ADC_MAP & ~reserv_map;
                    ASSERT(anc_tt_adc_ch_l, "%s: Cannot alloc TT CH L adc: reserv_map=0x%X", __func__, reserv_map);
                    anc_tt_adc_ch_l = (1 << get_lsb_pos(anc_tt_adc_ch_l));
                    reserv_map |= anc_tt_adc_ch_l;
                }
            }
            if (mic_map & ANC_TT_MIC_CH_L) {
                codec_adc_ch_map |= anc_tt_adc_ch_l;
                mic_map &= ~ANC_TT_MIC_CH_L;
                i = get_lsb_pos(anc_tt_adc_ch_l);
                ch_idx = get_lsb_pos(ANC_TT_MIC_CH_L);
                if (ANC_TT_MIC_CH_L & AUD_CHANNEL_MAP_DIGMIC_ALL) {
                    ch_idx = hal_codec_get_digmic_hw_index(ch_idx);
                    codec->REG_0A8 = (codec->REG_0A8 & ~(CODEC_CODEC_PDM_MUX_CH0_MASK << (3 * i))) |
                        (CODEC_CODEC_PDM_MUX_CH0(ch_idx) << (3 * i));
                    codec->REG_0A4 |= CODEC_CODEC_PDM_ADC_SEL_CH0 << i;
                } else {
                    *(&codec->REG_084 + i) = SET_BITFIELD(*(&codec->REG_084 + i), CODEC_CODEC_ADC_IN_SEL_CH0, ch_idx);
                    codec->REG_0A4 &= ~(CODEC_CODEC_PDM_ADC_SEL_CH0 << i);
                }
            } else if (ANC_TT_MIC_CH_L) {
                reserv_map |= anc_tt_adc_ch_l;
            }
#endif

#endif // ANC_APP

            if (mic_map & AUD_CHANNEL_MAP_ECMIC_CH0) {
                codec_adc_ch_map |= AUD_CHANNEL_MAP_CH3;
                mic_map &= ~AUD_CHANNEL_MAP_ECMIC_CH0;
            }

            reserv_map |= codec_adc_ch_map;

#ifdef SIDETONE_ENABLE
#if defined(SIDETONE_DEDICATED_ADC_CHAN) || defined(SIDETONE_RESERVED_ADC_CHAN)
            if (mic_map & CFG_HW_AUD_SIDETONE_MIC_DEV) {
                enum AUD_CHANNEL_MAP_T st_map;

                // Alloc sidetone adc chan
                st_map = NORMAL_ADC_MAP & ~reserv_map;
                ASSERT(st_map, "%s: Cannot alloc dedicated sidetone adc: reserv_map=0x%X", __func__, reserv_map);

                i = get_lsb_pos(st_map);
                st_map = (1 << i);

                // Associate mic and sidetone adc
                hal_codec_set_sidetone_adc_chan(st_map);
                ch_idx = get_lsb_pos(CFG_HW_AUD_SIDETONE_MIC_DEV);
                hal_codec_set_adc_mic_sel(i, ch_idx);
#ifdef SIDETONE_DEDICATED_ADC_CHAN
                sidetone_adc_ch_map = st_map;
#else
                mic_map &= ~(1 << ch_idx);
                codec_adc_ch_map |= st_map;
#endif
                // Mark sidetone adc as used
                reserv_map |= st_map;
            }
#endif
#endif

            i = 0;
            while (mic_map && i < NORMAL_ADC_CH_NUM) {
                ASSERT(i < MAX_ANA_MIC_CH_NUM || (mic_map & AUD_CHANNEL_MAP_DIGMIC_ALL),
                    "%s: Not enough ana cap chan: mic_map=0x%X adc_map=0x%X reserv_map=0x%X",
                    __func__, mic_map, codec_adc_ch_map, reserv_map);
                ch_idx = get_lsb_pos(mic_map);
                while ((reserv_map & (AUD_CHANNEL_MAP_CH0 << i)) && i < NORMAL_ADC_CH_NUM) {
                    i++;
                }
                if (i < NORMAL_ADC_CH_NUM) {
                    codec_adc_ch_map |= (AUD_CHANNEL_MAP_CH0 << i);
                    reserv_map |= codec_adc_ch_map;
                    mic_map &= ~(1 << ch_idx);
                    hal_codec_set_adc_mic_sel(i, ch_idx);
#if defined(CODEC_ADC_DC_FILTER_FACTOR)
#ifdef AUDIO_OUTPUT_DC_AUTO_CALIB
                    if (!dac_dc_calib_status)
#endif /* AUDIO_OUTPUT_DC_AUTO_CALIB */
                    {
                        hal_codec_enable_adc_dc_filter((1 << i), true);
                    }
#endif
                    i++;
                }
            }

            ASSERT(mic_map == 0, "%s: Bad cap chan map: 0x%X reserv_map=0x%X", __func__, mic_map, reserv_map);
        }

        if (HAL_CODEC_CONFIG_BITS & cfg->set_flag) {
            cfg_set_mask = 0;
            cfg_clr_mask = CODEC_MODE_16BIT_ADC | CODEC_MODE_24BIT_ADC | CODEC_MODE_32BIT_ADC;
            if (cfg->bits == AUD_BITS_16) {
                cfg_set_mask |= CODEC_MODE_16BIT_ADC;
            } else if (cfg->bits == AUD_BITS_24) {
                cfg_set_mask |= CODEC_MODE_24BIT_ADC;
            } else if (cfg->bits == AUD_BITS_32) {
                cfg_set_mask |= CODEC_MODE_32BIT_ADC;
            } else {
                ASSERT(false, "%s: Bad cap bits: %d", __func__, cfg->bits);
            }
            codec->REG_040 = (codec->REG_040 & ~cfg_clr_mask) | cfg_set_mask;
        }

        cnt = 0;
        for (i = 0; i < MAX_ADC_CH_NUM; i++) {
            if (codec_adc_ch_map & (AUD_CHANNEL_MAP_CH0 << i)) {
                cnt++;
            }
        }
        ASSERT(cnt == cfg->channel_num, "%s: Invalid capture stream chan cfg: mic_map=0x%X adc_map=0x%X num=%u",
            __func__, codec_mic_ch_map, codec_adc_ch_map, cfg->channel_num);

        if (HAL_CODEC_CONFIG_SAMPLE_RATE & cfg->set_flag) {
            sample_rate = cfg->sample_rate;

            for(i = 0; i < ARRAY_SIZE(codec_adc_sample_rate); i++) {
                if(codec_adc_sample_rate[i].sample_rate == sample_rate) {
                    break;
                }
            }
            ASSERT(i < ARRAY_SIZE(codec_adc_sample_rate), "%s: Invalid capture sample rate: %d", __func__, sample_rate);
            rate_idx = i;
            ana_dig_div = codec_adc_sample_rate[rate_idx].codec_div / codec_adc_sample_rate[rate_idx].cmu_div;
            ASSERT(ana_dig_div * codec_adc_sample_rate[rate_idx].cmu_div == codec_adc_sample_rate[rate_idx].codec_div,
                "%s: Invalid catpure div for rate %u: codec_div=%u cmu_div=%u", __func__, sample_rate,
                codec_adc_sample_rate[rate_idx].codec_div, codec_adc_sample_rate[rate_idx].cmu_div);

            TRACE(2,"[%s] capture sample_rate=%d", __func__, sample_rate);

#ifdef CODEC_TIMER
            cur_codec_freq = codec_adc_sample_rate[rate_idx].codec_freq;
#endif

            codec_rate_idx[AUD_STREAM_CAPTURE] = rate_idx;

#ifdef __AUDIO_RESAMPLE__
            if (hal_cmu_get_audio_resample_status() && codec_adc_sample_rate[rate_idx].codec_freq != CODEC_FREQ_CRYSTAL) {
#ifdef CODEC_TIMER
                cur_codec_freq = CODEC_FREQ_CRYSTAL;
#endif
                if ((codec->REG_0E4 & CODEC_CODEC_RESAMPLE_ADC_PHASE_UPDATE) == 0 ||
                        resample_rate_idx[AUD_STREAM_CAPTURE] != rate_idx) {
                    resample_rate_idx[AUD_STREAM_CAPTURE] = rate_idx;
                    codec->REG_0E4 &= ~CODEC_CODEC_RESAMPLE_ADC_PHASE_UPDATE;
                    hal_codec_reg_update_delay();
                    codec->REG_0F8 = resample_phase_float_to_value(get_capture_resample_phase());
                    hal_codec_reg_update_delay();
                    codec->REG_0E4 |= CODEC_CODEC_RESAMPLE_ADC_PHASE_UPDATE;
                }

                codec->REG_0E4 = SET_BITFIELD(codec->REG_0E4, CODEC_CODEC_RESAMPLE_ADC_CH_CNT, cfg->channel_num - 1);
            }
#endif

            uint32_t val;
            uint8_t adc_chan;

            val = 0;
            i = 0;
            mic_map = codec_mic_ch_map;
            while (mic_map && i < MAX_ADC_CH_NUM) {
                ch_idx = get_lsb_pos(mic_map);
                mic_map &= ~(1 << ch_idx);
                if ((1 << ch_idx) & EC_MIC_MAP) {
                    val |= CODEC_CODEC_RESAMPLE_ADC_CH0_ECHO << i;
                    // No need to select echo cancel channel (only 1)
                } else {
                    adc_chan = hal_codec_get_adc_chan(1 << ch_idx);
                    ASSERT(adc_chan < NORMAL_ADC_CH_NUM, "%s: Bad remap mic ch=%u mic_map=0x%X adc_map=0x%X",
                        __func__, ch_idx, codec_mic_ch_map, codec_adc_ch_map);
                    val |= CODEC_CODEC_RESAMPLE_ADC_CH0_SEL(adc_chan) << (2 * i);
                }
                i++;
            }
            ASSERT(mic_map == 0, "%s: Bad cap remap map: 0x%X", __func__, mic_map);
            codec->REG_0FC = val;
            codec->REG_0E4 |= CODEC_CODEC_ADC_REMAP_ENABLE;

#ifdef __AUDIO_RESAMPLE__
            if (!hal_cmu_get_audio_resample_status())
#endif
            {
#ifdef __AUDIO_RESAMPLE__
                ASSERT(codec_adc_sample_rate[rate_idx].codec_freq != CODEC_FREQ_CRYSTAL,
                    "%s: capture sample rate %u is for resample only", __func__, sample_rate);
#endif
                analog_aud_freq_pll_config(codec_adc_sample_rate[rate_idx].codec_freq, codec_adc_sample_rate[rate_idx].codec_div);
                hal_cmu_codec_adc_set_div(codec_adc_sample_rate[rate_idx].cmu_div * CODEC_FREQ_EXTRA_DIV);
            }

            hal_codec_set_adc_down(codec_adc_ch_map, codec_adc_sample_rate[rate_idx].adc_down);
            hal_codec_set_adc_hbf_bypass_cnt(codec_adc_ch_map, codec_adc_sample_rate[rate_idx].bypass_cnt);
        }

#if !(defined(FIXED_CODEC_ADC_VOL) && defined(SINGLE_CODEC_ADC_VOL))
        if (HAL_CODEC_CONFIG_VOL & cfg->set_flag) {
#ifdef SINGLE_CODEC_ADC_VOL
            const CODEC_ADC_VOL_T *adc_gain_db;
            adc_gain_db = hal_codec_get_adc_volume(cfg->vol);
            if (adc_gain_db) {
                hal_codec_set_dig_adc_gain(NORMAL_ADC_MAP, *adc_gain_db);
#ifdef SIDETONE_DEDICATED_ADC_CHAN
                sidetone_adc_gain = *adc_gain_db;
                hal_codec_set_dig_adc_gain(sidetone_adc_ch_map, sidetone_adc_gain + sidetone_gain_offset);
#endif
            }
#else // !SINGLE_CODEC_ADC_VOL
            uint32_t vol;

            mic_map = codec_mic_ch_map;
            while (mic_map) {
                ch_idx = get_lsb_pos(mic_map);
                mic_map &= ~(1 << ch_idx);
                vol = hal_codec_get_mic_chan_volume_level(1 << ch_idx);
                hal_codec_set_chan_vol(AUD_STREAM_CAPTURE, (1 << ch_idx), vol);
            }
#ifdef SIDETONE_DEDICATED_ADC_CHAN
            if (codec_mic_ch_map & CFG_HW_AUD_SIDETONE_MIC_DEV) {
                const CODEC_ADC_VOL_T *adc_gain_db;

                vol = hal_codec_get_mic_chan_volume_level(CFG_HW_AUD_SIDETONE_MIC_DEV);
                adc_gain_db = hal_codec_get_adc_volume(vol);
                if (adc_gain_db) {
                    sidetone_adc_gain = *adc_gain_db;
                    hal_codec_set_dig_adc_gain(sidetone_adc_ch_map, sidetone_adc_gain + sidetone_gain_offset);
                }
            }
#endif
#endif // !SINGLE_CODEC_ADC_VOL
        }
#endif
    }

    return 0;
}

int hal_codec_anc_adc_enable(enum ANC_TYPE_T type)
{
#ifdef ANC_APP
    enum AUD_CHANNEL_MAP_T map;
    enum AUD_CHANNEL_MAP_T mic_map;
    uint8_t ch_idx;

    map = 0;
    mic_map = 0;
    if (type & ANC_FEEDFORWARD) {
#if defined(ANC_FF_MIC_CH_L)
        if (ANC_FF_MIC_CH_L) {
            ch_idx = get_msb_pos(ANC_FF_MIC_CH_L);
            if (ANC_FF_MIC_CH_L & AUD_CHANNEL_MAP_DIGMIC_ALL) {
                ch_idx = hal_codec_get_digmic_hw_index(ch_idx);
                codec->REG_0A8 = SET_BITFIELD(codec->REG_0A8, CODEC_CODEC_PDM_MUX_CH0, ch_idx);
                codec->REG_0A4 |= CODEC_CODEC_PDM_ADC_SEL_CH0;
            } else {
                codec->REG_084 = SET_BITFIELD(codec->REG_084, CODEC_CODEC_ADC_IN_SEL_CH0, ch_idx);
                codec->REG_0A4 &= ~CODEC_CODEC_PDM_ADC_SEL_CH0;
            }
            map |= AUD_CHANNEL_MAP_CH0;
            mic_map |= ANC_FF_MIC_CH_L;
        }
#if defined(ANC_TT_MIC_CH_L)
        if (ANC_FF_MIC_CH_L && (ANC_FF_MIC_CH_L == ANC_TT_MIC_CH_L)) {
            anc_tt_mic_l_map |= ANC_FEEDFORWARD;
        }
#endif
#else
        ASSERT(false, "No ana adc ff ch defined");
#endif
    }
    if (type & ANC_FEEDBACK) {
#if defined(ANC_FB_MIC_CH_L)
        if (ANC_FB_MIC_CH_L) {
            ch_idx = get_msb_pos(ANC_FB_MIC_CH_L);
            if (ANC_FB_MIC_CH_L & AUD_CHANNEL_MAP_DIGMIC_ALL) {
                ch_idx = hal_codec_get_digmic_hw_index(ch_idx);
                codec->REG_0A8 = SET_BITFIELD(codec->REG_0A8, CODEC_CODEC_PDM_MUX_CH1, ch_idx);
                codec->REG_0A4 |= CODEC_CODEC_PDM_ADC_SEL_CH1;
            } else {
                codec->REG_088 = SET_BITFIELD(codec->REG_088, CODEC_CODEC_ADC_IN_SEL_CH1, ch_idx);
                codec->REG_0A4 &= ~CODEC_CODEC_PDM_ADC_SEL_CH1;
            }
            map |= AUD_CHANNEL_MAP_CH1;
            mic_map |= ANC_FB_MIC_CH_L;
        }
#else
        ASSERT(false, "No ana adc fb ch defined");
#endif
    }

#if defined( AUDIO_ANC_TT_HW)
    if (type & ANC_TALKTHRU) {
#if defined(ANC_TT_MIC_CH_L)
        int i;

        if (ANC_TT_MIC_CH_L) {
            ch_idx = get_msb_pos(ANC_TT_MIC_CH_L);
            i = get_msb_pos(anc_tt_adc_ch_l);
            if (ANC_TT_MIC_CH_L & AUD_CHANNEL_MAP_DIGMIC_ALL) {
                ch_idx -= get_msb_pos(AUD_CHANNEL_MAP_DIGMIC_CH0);
                codec->REG_0A8 = (codec->REG_0A8 & ~(CODEC_CODEC_PDM_MUX_CH0_MASK << (3 * i))) |
                    (CODEC_CODEC_PDM_MUX_CH0(ch_idx) << (3 * i));
                codec->REG_0A4 |= CODEC_CODEC_PDM_ADC_SEL_CH0 << i;
            } else {
                *(&codec->REG_084 + i) = SET_BITFIELD(*(&codec->REG_084 + i), CODEC_CODEC_ADC_IN_SEL_CH0, ch_idx);
                codec->REG_0A4 &= ~(CODEC_CODEC_PDM_ADC_SEL_CH0 << i);
            }
            codec->REG_22C = SET_BITFIELD(codec->REG_22C, CODEC_CODEC_TT_ADC_SEL_CH0, i);
            map |= anc_tt_adc_ch_l;
            mic_map |= ANC_TT_MIC_CH_L;
        }
#if defined(ANC_FF_MIC_CH_L)
        if (ANC_TT_MIC_CH_L && (ANC_TT_MIC_CH_L == ANC_FF_MIC_CH_L)) {
            anc_tt_mic_l_map |= ANC_TALKTHRU;
        }
#endif
#endif
    }
#endif

#ifdef ANC_DYN_CODEC_VOLT
    if (anc_adc_ch_map == 0) {
        hal_codec_codec_volt_ramp_up(CODEC_VOLT_RAMP_USER_ANC);
    }
#endif

    anc_adc_ch_map |= map;
    anc_mic_ch_map |= mic_map;

    if (anc_mic_ch_map & AUD_CHANNEL_MAP_DIGMIC_ALL) {
        hal_codec_enable_dig_mic(anc_mic_ch_map);
    }

    for (int i = 0; i < NORMAL_ADC_CH_NUM; i++) {
        if (map & (AUD_CHANNEL_MAP_CH0 << i)) {
            codec->REG_080 |= (CODEC_CODEC_ADC_EN_CH0 << i);
        }
    }

#if 0//def DAC_DRE_ENABLE
    if (anc_adc_ch_map && (codec->REG_0A4 & CODEC_CODEC_DAC_UH_EN)) {
        hal_codec_dac_dre_disable();
    }
#endif
#endif

    return 0;
}

int hal_codec_anc_adc_disable(enum ANC_TYPE_T type)
{
#ifdef ANC_APP
    enum AUD_CHANNEL_MAP_T map;
    enum AUD_CHANNEL_MAP_T mic_map;

    map = 0;
    mic_map = 0;
    if (type & ANC_FEEDFORWARD) {
#if defined(ANC_FF_MIC_CH_L)
        if (ANC_FF_MIC_CH_L) {
#if defined(ANC_TT_MIC_CH_L)
            if (ANC_FF_MIC_CH_L && (ANC_FF_MIC_CH_L == ANC_TT_MIC_CH_L)) {
                anc_tt_mic_l_map &= ~ANC_FEEDFORWARD;
            }
            if (anc_tt_mic_l_map == 0)
#endif
            {
                map |= AUD_CHANNEL_MAP_CH0;
                mic_map |= ANC_FF_MIC_CH_L;
            }
        }
#endif
    }
    if (type & ANC_FEEDBACK) {
#if defined(ANC_FB_MIC_CH_L)
        if (ANC_FB_MIC_CH_L) {
            map |= AUD_CHANNEL_MAP_CH1;
            mic_map |= ANC_FB_MIC_CH_L;
        }
#endif
    }

#if defined(AUDIO_ANC_TT_HW)
    if (type & ANC_TALKTHRU) {
#if defined(ANC_TT_MIC_CH_L)
        if (ANC_TT_MIC_CH_L) {
#if defined(ANC_FF_MIC_CH_L)
            if (ANC_TT_MIC_CH_L && (ANC_TT_MIC_CH_L == ANC_FF_MIC_CH_L)) {
                anc_tt_mic_l_map &= ~ANC_TALKTHRU;
            }
            if (anc_tt_mic_l_map == 0)
#endif
            {
                map |= anc_tt_adc_ch_l;
                mic_map |= ANC_TT_MIC_CH_L;
            }
        }
#endif
    }
#endif

    anc_adc_ch_map &= ~map;
    anc_mic_ch_map &= ~mic_map;

    if ((anc_mic_ch_map & AUD_CHANNEL_MAP_DIGMIC_ALL) == 0 &&
            ((codec_mic_ch_map & AUD_CHANNEL_MAP_DIGMIC_ALL) == 0 || (codec->REG_000 & CODEC_ADC_ENABLE) == 0)) {
        hal_codec_disable_dig_mic();
    }

    for (int i = 0; i < NORMAL_ADC_CH_NUM; i++) {
        if ((map & (AUD_CHANNEL_MAP_CH0 << i)) == 0) {
            continue;
        }
        if (codec->REG_000 & CODEC_ADC_ENABLE) {
            if (codec_adc_ch_map & (AUD_CHANNEL_MAP_CH0 << i)) {
                continue;
            }
        }
        codec->REG_080 &= ~(CODEC_CODEC_ADC_EN_CH0 << i);
    }

#if 0//def DAC_DRE_ENABLE
    if (anc_adc_ch_map == 0 && (codec->REG_0A4 & CODEC_CODEC_DAC_UH_EN) &&
            //(codec->REG_044 & CODEC_MODE_16BIT_DAC) == 0 &&
            1) {
        hal_codec_dac_dre_enable();
    }
#endif
#endif

#ifdef ANC_DYN_CODEC_VOLT
    if (anc_adc_ch_map == 0) {
        hal_codec_codec_volt_ramp_down(CODEC_VOLT_RAMP_USER_ANC);
    }
#endif

    return 0;
}

enum AUD_SAMPRATE_T hal_codec_anc_convert_rate(enum AUD_SAMPRATE_T rate)
{
    if (hal_cmu_get_audio_resample_status()) {
        return AUD_SAMPRATE_50781;
    } else if (CODEC_FREQ_48K_SERIES / rate * rate == CODEC_FREQ_48K_SERIES) {
        return AUD_SAMPRATE_48000;
    } else /* if (CODEC_FREQ_44_1K_SERIES / rate * rate == CODEC_FREQ_44_1K_SERIES) */ {
        return AUD_SAMPRATE_44100;
    }
}

int hal_codec_anc_dma_enable(enum HAL_CODEC_ID_T id)
{
    return 0;
}

int hal_codec_anc_dma_disable(enum HAL_CODEC_ID_T id)
{
    return 0;
}

int hal_codec_aux_mic_dma_enable(enum HAL_CODEC_ID_T id)
{
    return 0;
}

int hal_codec_aux_mic_dma_disable(enum HAL_CODEC_ID_T id)
{
    return 0;
}

uint32_t hal_codec_get_alg_dac_shift(void)
{
    return 0;
}

#ifdef ANC_APP
void hal_codec_set_anc_boost_gain_attn(float attn)
{
    anc_boost_gain_attn = attn;

#ifdef AUDIO_OUTPUT_SW_GAIN
    hal_codec_set_sw_gain(swdac_gain);
#else
    hal_codec_restore_dig_dac_gain();
#endif
    hal_codec_restore_dig_adc_gain();
}

void hal_codec_apply_anc_adc_gain_offset(enum ANC_TYPE_T type, int8_t offset_l, int8_t offset_r)
{
    enum AUD_CHANNEL_MAP_T map_l;
    enum AUD_CHANNEL_MAP_T ch_map;
    uint8_t ch_idx;

    if (analog_debug_get_anc_calib_mode()) {
        return;
    }

    map_l = 0;

#if defined(ANC_FF_MIC_CH_L)
    if (type & ANC_FEEDFORWARD) {
        if (ANC_FF_MIC_CH_L) {
            map_l |= AUD_CHANNEL_MAP_CH0;
        }
    }
#endif
#if defined(ANC_FB_MIC_CH_L)
    if (type & ANC_FEEDBACK) {
        if (ANC_FB_MIC_CH_L) {
            map_l |= AUD_CHANNEL_MAP_CH1;
        }
    }
#endif
#if defined(ANC_TT_MIC_CH_L)
    if (type & ANC_TALKTHRU) {
        if (ANC_FB_MIC_CH_L) {
            map_l |= anc_tt_adc_ch_l;
        }
    }
#endif

    if (map_l) {
        ch_map = map_l;
        while (ch_map) {
            ch_idx = get_msb_pos(ch_map);
            ch_map &= ~(1 << ch_idx);
            anc_adc_gain_offset[ch_idx] = offset_l;
        }
        if (offset_l) {
            anc_adc_gain_offset_map |= map_l;
        } else {
            anc_adc_gain_offset_map &= ~map_l;
        }
    }
    if (map_l) {
        hal_codec_restore_dig_adc_gain();
    }
}
#endif

#ifdef AUDIO_OUTPUT_DC_CALIB
void hal_codec_set_dac_dc_gain_attn(float attn)
{
    dac_dc_gain_attn = attn;
}

void hal_codec_set_dac_dc_offset(int16_t dc_l, int16_t dc_r)
{
    // DC calib values are based on 16-bit, but hardware compensation is based on 24-bit
    dac_dc_l = dc_l << 8;
#ifdef SDM_MUTE_NOISE_SUPPRESSION
    if (dac_dc_l == 0) {
        dac_dc_l = 1;
    }
#endif
}
#endif

#ifdef AUDIO_OUTPUT_FACTROY_CALIB
void hal_codec_set_dac_dc_gain_factory(float factory)
{
    dac_dc_gain_factory = factory;
}
#endif

void hal_codec_set_dac_reset_callback(HAL_CODEC_DAC_RESET_CALLBACK callback)
{
    //dac_reset_callback = callback;
}

static uint32_t POSSIBLY_UNUSED hal_codec_get_adc_chan(enum AUD_CHANNEL_MAP_T mic_map)
{
    uint8_t adc_ch;
    uint8_t mic_ch;
    uint8_t digmic_ch0;
    uint8_t en_ch;
    bool digmic;
    int i;

    adc_ch = MAX_ADC_CH_NUM;

    mic_ch = get_lsb_pos(mic_map);

    if (((1 << mic_ch) & codec_mic_ch_map) == 0) {
        return adc_ch;
    }

    digmic_ch0 = get_lsb_pos(AUD_CHANNEL_MAP_DIGMIC_CH0);

    if (mic_ch >= digmic_ch0) {
        mic_ch -= digmic_ch0;
        digmic = true;
    } else {
        digmic = false;
    }

    for (i = 0; i < NORMAL_ADC_CH_NUM; i++) {
        if (codec_adc_ch_map & (1 << i)) {
            if (digmic ^ !!(codec->REG_0A4 & (CODEC_CODEC_PDM_ADC_SEL_CH0 << i))) {
                continue;
            }
            if (digmic) {
                en_ch = (codec->REG_0A8 & (CODEC_CODEC_PDM_MUX_CH0_MASK << (3 * i))) >> (CODEC_CODEC_PDM_MUX_CH0_SHIFT + 3 * i);
            } else {
                en_ch = GET_BITFIELD(*(&codec->REG_084 + i), CODEC_CODEC_ADC_IN_SEL_CH0);
            }
            if (mic_ch == en_ch) {
                adc_ch = i;
                break;
            }
        }
    }

    return adc_ch;
}

void hal_codec_sidetone_enable(void)
{
#ifdef SIDETONE_ENABLE
#if (CFG_HW_AUD_SIDETONE_MIC_DEV & (CFG_HW_AUD_SIDETONE_MIC_DEV - 1))
#error "Invalid CFG_HW_AUD_SIDETONE_MIC_DEV: only 1 mic can be defined"
#endif
#if (CFG_HW_AUD_SIDETONE_MIC_DEV == 0) || (CFG_HW_AUD_SIDETONE_MIC_DEV & ~NORMAL_MIC_MAP)
#error "Invalid CFG_HW_AUD_SIDETONE_MIC_DEV: bad mic channel"
#endif
    int gain = CFG_HW_AUD_SIDETONE_GAIN_DBVAL;
    uint32_t val;

#ifdef SIDETONE_DEDICATED_ADC_CHAN
    sidetone_gain_offset = 0;
    if (gain > MAX_SIDETONE_DBVAL) {
        sidetone_gain_offset = gain - MAX_SIDETONE_DBVAL;
    } else if (gain < MIN_SIDETONE_DBVAL) {
        sidetone_gain_offset = gain - MIN_SIDETONE_DBVAL;
    }
#endif

    if (gain > MAX_SIDETONE_DBVAL) {
        gain = MAX_SIDETONE_DBVAL;
    } else if (gain < MIN_SIDETONE_DBVAL) {
        gain = MIN_SIDETONE_DBVAL;
    }

    val = MIN_SIDETONE_REGVAL + (gain - MIN_SIDETONE_DBVAL) / SIDETONE_DBVAL_STEP;

    codec->REG_080 = SET_BITFIELD(codec->REG_080, CODEC_CODEC_SIDE_TONE_GAIN, val);

#ifdef SIDETONE_DEDICATED_ADC_CHAN
    uint8_t adc_ch;
    uint8_t a;

    adc_ch = get_lsb_pos(sidetone_adc_ch_map);
    if (adc_ch >= NORMAL_ADC_CH_NUM) {
        ASSERT(false, "%s: Bad sidetone_adc_ch_map=0x%X", __func__, sidetone_adc_ch_map);
        return;
    }

    a = codec_rate_idx[AUD_STREAM_CAPTURE];
    hal_codec_set_adc_down(sidetone_adc_ch_map, codec_adc_sample_rate[a].adc_down);
    hal_codec_set_adc_hbf_bypass_cnt(sidetone_adc_ch_map, codec_adc_sample_rate[a].bypass_cnt);

    hal_codec_set_dig_adc_gain(sidetone_adc_ch_map, sidetone_adc_gain + sidetone_gain_offset);
#ifdef CFG_HW_AUD_SIDETONE_GAIN_RAMP
    hal_codec_get_adc_gain(sidetone_adc_ch_map, &sidetone_ded_chan_coef);
    hal_codec_set_dig_adc_gain(sidetone_adc_ch_map, MIN_DIG_DBVAL);
#endif
    codec->REG_080 |= (CODEC_CODEC_ADC_EN_CH0 << adc_ch);

#ifdef CFG_HW_AUD_SIDETONE_IIR_INDEX
#if (CFG_HW_AUD_SIDETONE_IIR_INDEX >= ADC_IIR_CH_NUM + 0UL)
#error "Invalid CFG_HW_AUD_SIDETONE_IIR_INDEX"
#endif
    uint32_t mask;

    mask = CODEC_CODEC_ADC_IIR_CH0_SEL_MASK << (4 * CFG_HW_AUD_SIDETONE_IIR_INDEX);
    val = CODEC_CODEC_ADC_IIR_CH0_SEL(4) << (4 * CFG_HW_AUD_SIDETONE_IIR_INDEX);
    codec->REG_0DC = (codec->REG_0DC & ~mask) | val;
#endif
#endif
#endif
}

void hal_codec_sidetone_disable(void)
{
#ifdef SIDETONE_ENABLE
    codec->REG_080 = SET_BITFIELD(codec->REG_080, CODEC_CODEC_SIDE_TONE_GAIN, MUTE_SIDETONE_REGVAL);
#ifdef SIDETONE_DEDICATED_ADC_CHAN
    if (sidetone_adc_ch_map) {
        uint8_t adc_ch;

        adc_ch = get_lsb_pos(sidetone_adc_ch_map);
        if (adc_ch >= NORMAL_ADC_CH_NUM) {
            ASSERT(false, "%s: Bad sidetone_adc_ch_map=0x%X", __func__, sidetone_adc_ch_map);
            return;
        }
        codec->REG_080 &= ~(CODEC_CODEC_ADC_EN_CH0 << adc_ch);
    }
#endif
#endif
}

int hal_codec_sidetone_gain_ramp_up(float step)
{
    int ret = 0;
#ifdef CFG_HW_AUD_SIDETONE_GAIN_RAMP
    float coef;
    uint32_t val;

    hal_codec_get_adc_gain(sidetone_adc_ch_map, &coef);
    coef += step;
    if (coef >= sidetone_ded_chan_coef) {
        coef = sidetone_ded_chan_coef;
        ret = 1;
    }
    // Gain format: 8.12
    int32_t s_val = (int32_t)(coef * (1 << 12));
    val = __SSAT(s_val, 20);
    hal_codec_set_adc_gain_value(sidetone_adc_ch_map, val);
#endif
    return ret;
}

int hal_codec_sidetone_gain_ramp_down(float step)
{
    int ret = 0;
#ifdef CFG_HW_AUD_SIDETONE_GAIN_RAMP
    float coef;
    uint32_t val;

    hal_codec_get_adc_gain(sidetone_adc_ch_map, &coef);
    coef -= step;
    if (coef <= 0) {
        coef = 0;
        ret = 1;
    }

    // Gain format: 8.12
    int32_t s_val = (int32_t)(coef * (1 << 12));
    val = __SSAT(s_val, 20);
    hal_codec_set_adc_gain_value(sidetone_adc_ch_map, val);
#endif
    return ret;
}

void hal_codec_select_adc_iir_mic(uint32_t index, enum AUD_CHANNEL_MAP_T mic_map)
{
    uint32_t mask, val;
    uint8_t adc_ch;

    ASSERT(index < ADC_IIR_CH_NUM, "%s: Bad index=%u", __func__, index);
    ASSERT(mic_map && (mic_map & (mic_map - 1)) == 0, "%s: Bad mic_map=0x%X", __func__, mic_map);
#ifdef CFG_HW_AUD_SIDETONE_IIR_INDEX
    ASSERT(index != CFG_HW_AUD_SIDETONE_IIR_INDEX, "%s: Adc iir index conflicts with sidetone", __func__);
#endif

    // TODO: How to select iir adc index?
    adc_ch = hal_codec_get_adc_chan(mic_map);
    if (adc_ch < NORMAL_ADC_CH_NUM) {
        mask = CODEC_CODEC_ADC_IIR_CH0_SEL_MASK << (4 * index);
        val = CODEC_CODEC_ADC_IIR_CH0_SEL(adc_ch) << (4 * index);
        codec->REG_0DC = (codec->REG_0DC & ~mask) | val;
    }
}

void hal_codec_min_phase_mode_enable(enum AUD_STREAM_T stream)
{
}

void hal_codec_min_phase_mode_disable(enum AUD_STREAM_T stream)
{
}

void hal_codec_sync_dac_enable(enum HAL_CODEC_SYNC_TYPE_T type)
{
    //hal_codec_sync_dac_resample_rate_enable(type);
    codec->REG_054 = SET_BITFIELD(codec->REG_054, CODEC_CODEC_DAC_LH_ENABLE_SEL, type);
}

void hal_codec_sync_dac_disable(void)
{
    //hal_codec_sync_dac_resample_rate_disable();
    codec->REG_054 = SET_BITFIELD(codec->REG_054, CODEC_CODEC_DAC_LH_ENABLE_SEL, HAL_CODEC_SYNC_TYPE_NONE);
}

void hal_codec_sync_dac2_enable(enum HAL_CODEC_SYNC_TYPE_T type)
{
    //hal_codec_sync_dac2_resample_rate_enable(type);
    codec->REG_054 = SET_BITFIELD(codec->REG_054, CODEC_DAC_ENABLE_SEL_SND, type);
}

void hal_codec_sync_dac2_disable(void)
{
    //hal_codec_sync_dac2_resample_rate_disable();
    codec->REG_054 = SET_BITFIELD(codec->REG_054, CODEC_DAC_ENABLE_SEL_SND, HAL_CODEC_SYNC_TYPE_NONE);
}

void hal_codec_sync_adc_enable(enum HAL_CODEC_SYNC_TYPE_T type)
{
#if defined(ANC_APP)
    //hal_codec_sync_adc_resample_rate_enable(type);
    codec->REG_054 = SET_BITFIELD(codec->REG_054, CODEC_ADC_ENABLE_SEL, type);
#else
    codec->REG_054 = SET_BITFIELD(codec->REG_054, CODEC_CODEC_ADC_ENABLE_SEL, type);
#endif
}

void hal_codec_sync_adc_disable(void)
{
#if defined(ANC_APP)
    //hal_codec_sync_adc_resample_rate_disable();
    codec->REG_054 = SET_BITFIELD(codec->REG_054, CODEC_ADC_ENABLE_SEL, HAL_CODEC_SYNC_TYPE_NONE);
#else
    codec->REG_054 = SET_BITFIELD(codec->REG_054, CODEC_CODEC_ADC_ENABLE_SEL, HAL_CODEC_SYNC_TYPE_NONE);
#endif
}

void hal_codec_sync_dac_resample_rate_enable(enum HAL_CODEC_SYNC_TYPE_T type)
{
    codec->REG_0E4 = SET_BITFIELD(codec->REG_0E4, CODEC_CODEC_RESAMPLE_DAC_UPDATE_TRIGGER_SEL, type);
}

void hal_codec_sync_dac_resample_rate_disable(void)
{
    codec->REG_0E4 = SET_BITFIELD(codec->REG_0E4, CODEC_CODEC_RESAMPLE_DAC_UPDATE_TRIGGER_SEL, HAL_CODEC_SYNC_TYPE_NONE);
}

void hal_codec_sync_dac2_resample_rate_enable(enum HAL_CODEC_SYNC_TYPE_T type)
{
    codec->REG_0E4 = SET_BITFIELD(codec->REG_0E4, CODEC_CODEC_RESAMPLE_DAC_UPDATE_TRIGGER_SEL_SND, type);
}

void hal_codec_sync_dac2_resample_rate_disable(void)
{
    codec->REG_0E4 = SET_BITFIELD(codec->REG_0E4, CODEC_CODEC_RESAMPLE_DAC_UPDATE_TRIGGER_SEL_SND, HAL_CODEC_SYNC_TYPE_NONE);
}

void hal_codec_sync_adc_resample_rate_enable(enum HAL_CODEC_SYNC_TYPE_T type)
{
    codec->REG_0E4 = SET_BITFIELD(codec->REG_0E4, CODEC_CODEC_RESAMPLE_ADC_UPDATE_TRIGGER_SEL, type);
}

void hal_codec_sync_adc_resample_rate_disable(void)
{
    codec->REG_0E4 = SET_BITFIELD(codec->REG_0E4, CODEC_CODEC_RESAMPLE_ADC_UPDATE_TRIGGER_SEL, HAL_CODEC_SYNC_TYPE_NONE);
}

void hal_codec_sync_dac_gain_enable(enum HAL_CODEC_SYNC_TYPE_T type)
{
    codec->REG_09C = SET_BITFIELD(codec->REG_09C, CODEC_CODEC_DAC_GAIN_TRIGGER_SEL, type);
}

void hal_codec_sync_dac_gain_disable(void)
{
    codec->REG_09C = SET_BITFIELD(codec->REG_09C, CODEC_CODEC_DAC_GAIN_TRIGGER_SEL, HAL_CODEC_SYNC_TYPE_NONE);
}

void hal_codec_sync_dac2_gain_enable(enum HAL_CODEC_SYNC_TYPE_T type)
{
    codec->REG_46C = SET_BITFIELD(codec->REG_46C, CODEC_CODEC_DAC_GAIN_TRIGGER_SEL_SND, type);
}

void hal_codec_sync_dac2_gain_disable(void)
{
    codec->REG_46C = SET_BITFIELD(codec->REG_46C, CODEC_CODEC_DAC_GAIN_TRIGGER_SEL_SND, HAL_CODEC_SYNC_TYPE_NONE);
}

void hal_codec_sync_adc_gain_enable(enum HAL_CODEC_SYNC_TYPE_T type)
{
}

void hal_codec_sync_adc_gain_disable(void)
{
}

void hal_codec_gpio_trigger_debounce_enable(void)
{
    if (codec_opened) {
        codec->REG_054 |= CODEC_GPIO_TRIGGER_DB_ENABLE;
    }
}

void hal_codec_gpio_trigger_debounce_disable(void)
{
    if (codec_opened) {
        codec->REG_054 &= ~CODEC_GPIO_TRIGGER_DB_ENABLE;
    }
}

#ifdef CODEC_TIMER
int hal_codec_timer_trig_i2s_enable(enum HAL_CODEC_TIMER_TRIG_MODE_T mode, uint32_t ticks, bool periodic)
{
    uint32_t val;

    if (!codec_opened) {
        return 1;
    }

    if (periodic) {
        val = get_msb_pos(ticks);
        if (val < 31) {
            ticks += (1 << val) / 2;
        }
        val = get_msb_pos(ticks);
        if (val <= 16 || val >= 32) {
            val = 0;
        } else {
            val -= 16;
        }
        codec->REG_390 = SET_BITFIELD(codec->REG_390, CODEC_CALIB_INTERVAL, val) | CODEC_CALIB_INTERVAL_DR;
    } else {
        ticks >>= 5;
        if (ticks == 0) {
            ticks = 1;
        }

        // reset codec timer
        codec->REG_054 &= ~CODEC_EVENT_FOR_CAPTURE;
        hal_codec_reg_update_delay();
        codec->REG_054 |= CODEC_EVENT_FOR_CAPTURE;

        // 0: DAC, 1: ADC, 2/3: DAC or ADC
        codec->REG_0A4 = SET_BITFIELD(codec->REG_0A4, CODEC_EN_48KX64_MODE, mode);
    }

    codec->REG_06C = (codec->REG_06C & ~(CODEC_TRIG_TIME_MASK | CODEC_TRIG_MODE)) |
        CODEC_TRIG_TIME(ticks) | CODEC_TRIG_TIME_ENABLE | (periodic ? CODEC_TRIG_MODE : 0);

    return 0;
}

int hal_codec_timer_trig_i2s_disable(void)
{
    if (!codec_opened) {
        return 1;
    }

    codec->REG_06C &= ~CODEC_TRIG_TIME_ENABLE;
    codec->REG_390 &= ~CODEC_CALIB_INTERVAL_DR;

    return 0;
}

uint32_t hal_codec_timer_get(void)
{
    if (codec_opened) {
        return codec->REG_050;
    }

    return 0;
}

uint32_t hal_codec_timer_ticks_to_us(uint32_t ticks)
{
    uint32_t timer_freq;

    timer_freq = cur_codec_freq / 4 / CODEC_FREQ_EXTRA_DIV;

    return (uint32_t)((float)ticks * 1000000 / timer_freq);
}

uint32_t hal_codec_timer_us_to_ticks(uint32_t us)
{
    uint32_t timer_freq;

    timer_freq = cur_codec_freq / 4 / CODEC_FREQ_EXTRA_DIV;

    return (uint32_t)((float)us * timer_freq / 1000000);
}

void hal_codec_timer_trigger_read(void)
{
    if (codec_opened) {
        codec->REG_06C ^= CODEC_GET_CNT_TRIG;
        hal_codec_reg_update_delay();
    }
}

void hal_codec_timer_set_ws_trigger_cnt(uint32_t cnt)
{
    uint32_t val;

    val = get_msb_pos(cnt);
    if (val < 31) {
        cnt += (1 << val) / 2;
    }
    val = get_msb_pos(cnt);
    if (val <= 6 || val >= 32) {
        val = 0;
    } else {
        val -= 6;
        if (val > 13) {
            val = 13;
        }
    }
    codec->REG_390 = SET_BITFIELD(codec->REG_390, CODEC_WS_TRIGGER_INTERVAL, val);
}
#endif

#ifdef AUDIO_OUTPUT_DC_CALIB_ANA
int hal_codec_dac_sdm_reset_set(void)
{
    if (codec_opened) {
#ifdef DAC_RAMP_GAIN
        bool ramp = false;

        if (codec->REG_0EC & CODEC_CODEC_RAMP_EN_CH0) {
            codec->REG_0EC &= ~CODEC_CODEC_RAMP_EN_CH0;
            hal_codec_reg_update_delay();
            ramp = true;
        }
#endif
#ifndef ANC_PROD_TEST
        hal_codec_set_dac_gain_value(VALID_DAC_MAP, 0);
#endif
#ifdef DAC_RAMP_GAIN
        if (ramp) {
            hal_codec_reg_update_delay();
            codec->REG_0EC |= CODEC_CODEC_RAMP_EN_CH0;
        }
#endif
#if 0
        if (codec->REG_0A4 & CODEC_CODEC_DAC_UH_EN) {
            osDelay(dac_delay_ms);
        }
#endif
#ifdef SDM_MUTE_NOISE_SUPPRESSION_V2
        for (int i = 0x200; i >= 0; i -= 0x100) {
            hal_codec_dac_dc_offset_enable(i, i);
            osDelay(1);
        }
#endif
        codec->REG_0A4 |= CODEC_CODEC_DAC_SDM_CLOSE;
        osDelay(1);
    }

    return 0;
}

int hal_codec_dac_sdm_reset_clear(void)
{
    if (codec_opened) {
        osDelay(1);
        codec->REG_0A4 &= ~CODEC_CODEC_DAC_SDM_CLOSE;
#ifdef SDM_MUTE_NOISE_SUPPRESSION_V2
        for (int i = 0x100; i <= 0x300; i += 0x100) {
            hal_codec_dac_dc_offset_enable(i, i);
            osDelay(1);
        }
#endif
        hal_codec_restore_dig_dac_gain();
    }

    return 0;
}
#endif

void hal_codec_tune_resample_rate(enum AUD_STREAM_T stream, float ratio)
{
#ifdef __AUDIO_RESAMPLE__
    uint32_t val;

    if (!codec_opened) {
        return;
    }

    if (stream == AUD_STREAM_PLAYBACK) {
        if (codec->REG_0E4 & CODEC_CODEC_RESAMPLE_DAC_PHASE_UPDATE) {
            codec->REG_0E4 &= ~CODEC_CODEC_RESAMPLE_DAC_PHASE_UPDATE;
            hal_codec_reg_update_delay();
            val = resample_phase_float_to_value(get_playback_resample_phase());
            val += (int)(val * ratio);
            codec->REG_0F4 = val;
            hal_codec_reg_update_delay();
            codec->REG_0E4 |= CODEC_CODEC_RESAMPLE_DAC_PHASE_UPDATE;
        }
    } else {
        if (codec->REG_0E4 & CODEC_CODEC_RESAMPLE_ADC_PHASE_UPDATE) {
            codec->REG_0E4 &= ~CODEC_CODEC_RESAMPLE_ADC_PHASE_UPDATE;
            hal_codec_reg_update_delay();
            val = resample_phase_float_to_value(get_capture_resample_phase());
            val -= (int)(val * ratio);
            codec->REG_0F8 = val;
            hal_codec_reg_update_delay();
            codec->REG_0E4 |= CODEC_CODEC_RESAMPLE_ADC_PHASE_UPDATE;
        }
    }
#endif
}

void hal_codec_tune_both_resample_rate(float ratio)
{
#ifdef __AUDIO_RESAMPLE__
    bool update[2];
    uint32_t val[2];
    uint32_t lock;

    if (!codec_opened) {
        return;
    }

    update[0] = !!(codec->REG_0E4 & CODEC_CODEC_RESAMPLE_DAC_PHASE_UPDATE);
    update[1] = !!(codec->REG_0E4 & CODEC_CODEC_RESAMPLE_ADC_PHASE_UPDATE);

    val[0] = val[1] = 0;

    if (update[0]) {
        codec->REG_0E4 &= ~CODEC_CODEC_RESAMPLE_DAC_PHASE_UPDATE;
        val[0] = resample_phase_float_to_value(get_playback_resample_phase());
        val[0] += (int)(val[0] * ratio);
    }
    if (update[1]) {
        codec->REG_0E4 &= ~CODEC_CODEC_RESAMPLE_ADC_PHASE_UPDATE;
        val[1] = resample_phase_float_to_value(get_capture_resample_phase());
        val[1] -= (int)(val[1] * ratio);
    }

    hal_codec_reg_update_delay();

    if (update[0]) {
        codec->REG_0F4 = val[0];
    }
    if (update[1]) {
        codec->REG_0F8 = val[1];
    }

    hal_codec_reg_update_delay();

    lock = int_lock();
    if (update[0]) {
        codec->REG_0E4 |= CODEC_CODEC_RESAMPLE_DAC_PHASE_UPDATE;
    }
    if (update[1]) {
        codec->REG_0E4 |= CODEC_CODEC_RESAMPLE_ADC_PHASE_UPDATE;
    }
    int_unlock(lock);
#endif
}

int hal_codec_select_clock_out(uint32_t cfg)
{
    uint32_t lock;
    int ret = 1;

    lock = int_lock();

    if (codec_opened) {
        codec->REG_060 = SET_BITFIELD(codec->REG_060, CODEC_CFG_CLK_OUT, cfg);
        ret = 0;
    }

    int_unlock(lock);

    return ret;
}

#ifdef AUDIO_ANC_FB_MC
void hal_codec_mc_enable(void)
{
    mc_enabled = true;
}

void hal_codec_mc_disable(void)
{
    mc_enabled = false;
}

void hal_codec_setup_mc(enum AUD_CHANNEL_NUM_T channel_num, enum AUD_BITS_T bits)
{
    mc_chan_num = channel_num;
    mc_bits = bits;
}
#endif

void hal_codec_swap_output(bool swap)
{
#ifdef AUDIO_OUTPUT_SWAP
    output_swap = swap;

    if (codec_opened) {
        if (output_swap) {
            codec->REG_0A0 |= CODEC_CODEC_DAC_OUT_SWAP;
        } else {
            codec->REG_0A0 &= ~CODEC_CODEC_DAC_OUT_SWAP;
        }
    }
#endif
}

int hal_codec_config_digmic_phase(uint8_t phase)
{
#ifdef ANC_PROD_TEST
    codec_digmic_phase = phase;
#endif
    return 0;
}

static void hal_codec_general_irq_handler(void)
{
    uint32_t status;

    status = codec->REG_00C;
    codec->REG_00C = status;

    status &= codec->REG_010;

    for (int i = 0; i < CODEC_IRQ_TYPE_QTY; i++) {
        if (codec_irq_callback[i]) {
            codec_irq_callback[i](status);
        }
    }
}

static void hal_codec_set_irq_handler(enum CODEC_IRQ_TYPE_T type, HAL_CODEC_IRQ_CALLBACK cb)
{
    uint32_t lock;

    ASSERT(type < CODEC_IRQ_TYPE_QTY, "%s: Bad type=%d", __func__, type);

    lock = int_lock();

    codec_irq_callback[type] = cb;

    if (cb) {
        if (codec_irq_map == 0) {
            NVIC_SetVector(CODEC_IRQn, (uint32_t)hal_codec_general_irq_handler);
            NVIC_SetPriority(CODEC_IRQn, IRQ_PRIORITY_NORMAL);
            NVIC_ClearPendingIRQ(CODEC_IRQn);
            NVIC_EnableIRQ(CODEC_IRQn);
        }
        codec_irq_map |= (1 << type);
    } else {
        codec_irq_map &= ~(1 << type);
        if (codec_irq_map == 0) {
            NVIC_DisableIRQ(CODEC_IRQn);
            NVIC_ClearPendingIRQ(CODEC_IRQn);
        }
    }

    int_unlock(lock);
}

void hal_codec_anc_fb_check_set_irq_handler(HAL_CODEC_IRQ_CALLBACK cb)
{
    hal_codec_set_irq_handler(CODEC_IRQ_TYPE_ANC_FB_CHECK, cb);
}

int hal_codec_dac_dc_auto_calib_enable(void)
{
    uint8_t mic_ch, adc_ch;
    enum AUD_CHANNEL_MAP_T map;

    map = AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1;
    while (map) {
        mic_ch = get_lsb_pos(map);
        map &= ~(1 << mic_ch);
        adc_ch = hal_codec_get_adc_chan(1 << mic_ch);
        ASSERT(adc_ch < NORMAL_ADC_CH_NUM, "%s: Bad cap map=0x%X (ch=%u)", __func__, map, mic_ch);
        hal_codec_set_dig_adc_gain((1 << adc_ch), 0);
    }
#if defined(DAC_RAMP_GAIN)
    codec->REG_0EC &= ~CODEC_CODEC_RAMP_EN_CH0;
#endif
#ifdef DAC_DRE_ENABLE
    hal_codec_dac_dre_disable();
#endif
    return 0;
}

int hal_codec_dac_dc_auto_calib_disable(void)
{
    return 0;
}

//********************BT trigger functions: START********************
static void hal_codec_bt_trigger_isr(uint32_t irq_status)
{
    if ((irq_status & CODEC_BT_TRIGGER) == 0) {
        return;
    }

    if (bt_trigger_callback) {
        TRACE(1,"[%s] bt_trigger_callback Start...", __func__);
        bt_trigger_callback();
    } else {
        TRACE(1,"[%s] bt_trigger_callback = NULL", __func__);
    }
}

static inline void hal_codec_bt_trigger_irq_en(int enable)
{
    if (enable)
        codec->REG_010 |= CODEC_BT_TRIGGER_MSK;
    else
        codec->REG_010 &= ~CODEC_BT_TRIGGER_MSK;

    codec->REG_00C = CODEC_BT_TRIGGER;
}

void hal_codec_set_bt_trigger_callback(HAL_CODEC_BT_TRIGGER_CALLBACK callback)
{
    bt_trigger_callback = callback;
}

int hal_codec_bt_trigger_start(void)
{
    uint32_t lock;

    TRACE(1,"[%s] Start", __func__);

    lock = int_lock();

    hal_codec_set_irq_handler(CODEC_IRQ_TYPE_BT_TRIGGER, hal_codec_bt_trigger_isr);
    hal_codec_bt_trigger_irq_en(1);

    int_unlock(lock);

    return 0;
}

int hal_codec_bt_trigger_stop(void)
{
    uint32_t lock;

    TRACE(1,"[%s] Stop", __func__);

    lock = int_lock();

    hal_codec_bt_trigger_irq_en(0);
    hal_codec_set_irq_handler(CODEC_IRQ_TYPE_BT_TRIGGER, NULL);

    int_unlock(lock);

    return 0;
}
//********************BT trigger functions: END********************

static void hal_codec_event_trigger_isr(uint32_t irq_status)
{
    if ((irq_status & CODEC_EVENT_TRIGGER) == 0) {
        return;
    }

    if (event_trigger_callback) {
        event_trigger_callback();
    }
}

static inline void hal_codec_event_trigger_irq_en(int enable)
{
    if (enable) {
        codec->REG_010 |= CODEC_EVENT_TRIGGER_MSK;
    } else {
        codec->REG_010 &= ~CODEC_EVENT_TRIGGER_MSK;
    }

    codec->REG_00C = CODEC_EVENT_TRIGGER;
}

void hal_codec_set_event_trigger_callback(HAL_CODEC_EVENT_TRIGGER_CALLBACK callback)
{
    uint32_t lock;

    event_trigger_callback = callback;

    lock = int_lock();
    if (callback) {
        hal_codec_set_irq_handler(CODEC_IRQ_TYPE_EVENT_TRIGGER, hal_codec_event_trigger_isr);
        hal_codec_event_trigger_irq_en(1);
    } else {
        hal_codec_event_trigger_irq_en(0);
        hal_codec_set_irq_handler(CODEC_IRQ_TYPE_EVENT_TRIGGER, NULL);
    }
    int_unlock(lock);
}

static void hal_codec_timer_trigger_isr(uint32_t irq_status)
{
    if ((irq_status & CODEC_TIME_TRIGGER) == 0) {
        return;
    }

    if (timer_trigger_callback) {
        timer_trigger_callback();
    }
}

static inline void hal_codec_timer_trigger_irq_en(int enable)
{
    if (enable) {
        codec->REG_010 |= CODEC_TIME_TRIGGER_MSK;
    } else {
        codec->REG_010 &= ~CODEC_TIME_TRIGGER_MSK;
    }

    codec->REG_00C = CODEC_TIME_TRIGGER;
}

void hal_codec_set_timer_trigger_callback(HAL_CODEC_TIMER_TRIGGER_CALLBACK callback)
{
    uint32_t lock;

    timer_trigger_callback = callback;

    lock = int_lock();
    if (callback) {
        hal_codec_set_irq_handler(CODEC_IRQ_TYPE_TIMER_TRIGGER, hal_codec_timer_trigger_isr);
        hal_codec_timer_trigger_irq_en(1);
    } else {
        hal_codec_timer_trigger_irq_en(0);
        hal_codec_set_irq_handler(CODEC_IRQ_TYPE_TIMER_TRIGGER, NULL);
    }
    int_unlock(lock);
}

