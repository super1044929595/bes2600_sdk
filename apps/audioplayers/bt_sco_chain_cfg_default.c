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
/**
// TX: Transimt process
// RX: Receive process
// 16k: base 25M/208M(1300,1302) base 28M/104M(1400,1402)
| ----- | ------------------- | --------------------------------- | --------- | ------------- | ---------------------- |
| TX/RX | Algo                | description                       | MIPS(M)   | RAM(kB)       | Note                   |
|       |                     |                                   | 16k    8k | 16k        8k |                        |
| ----- | ------------------- | --------------------------------- | --------- | ------------- | ---------------------- |
|       | SPEECH_TX_DC_FILTER | Direct Current filter             | 1~2    \  | 0.05          |
|       | SPEECH_TX_AEC       | Acoustic Echo Cancellation(old)   | 40     \  |               | HWFFT: 37              |
|       | SPEECH_TX_AEC2      | Acoustic Echo Cancellation(new)   | 39     \  |               | enable NLP             |
|       | SPEECH_TX_AEC3      | Acoustic Echo Cancellation(new)   | 14/18  \  |               | 2 filters/4 filters    |
|       | SPEECH_TX_AEC2FLOAT | Acoustic Echo Cancellation(new)   | 23/22  \  | 29.3          | nlp/af(blocks=1)       |
|       | SPEECH_TX_AEC2FLOAT | Acoustic Echo Cancellation(ns)    | 14/10  \  |               | banks=256/banks=128    |
|       | (ns_enabled)        |                                   | 8/6    \  |               | banks=64/banks=32      |
|       | SPEECH_TX_NS        | 1 mic noise suppress(old)         | 30     \  |               | HWFFT: 19              |
|       | SPEECH_TX_NS2       | 1 mic noise suppress(new)         | 16     \  |               | HWFFT: 12              |
|       | SPEECH_TX_NS3       | 1 mic noise suppress(new)         | 26     \  | 33.3          |                        |
|       | SPEECH_TX_NN_NS     | 1 mic noise suppress(new)         | 117/52 \  | 78.7/20.3     | 16ms; large/small      |
|       | SPEECH_TX_NN_NS2    | 1 mic noise suppress(new)         | 37        | 30.3          |
|       | SPEECH_TX_NS2FLOAT  | 1 mic noise suppress(new float)   | 12.5   \  |               | banks=64               |
| TX    | SPEECH_TX_2MIC_NS   | 2 mic noise suppres(long space)   | \         |               |                        |
|       | SPEECH_TX_2MIC_NS2  | 2 mic noise suppres(short space)  | 20        | 14.8          | delay_taps 5M          |
|       |                     |                                   |           |               | freq_smooth_enable 1.5 |
|       |                     |                                   |           |               | wnr_enable 1.5M        |
|       | SPEECH_TX_2MIC_NS4  | sensor mic noise suppress         | 31.5      |               |                        |
|       | SPEECH_TX_2MIC_NS3  | 2 mic noise suppres(far field)    | \         |               |                        |
|       | SPEECH_TX_2MIC_NS5  | 2 mic noise suppr(left right end) | \         |               |                        |
|       | SPEECH_TX_2MIC_NS6  | 2 mic noise suppr(far field)      | 70        |               |                        |
|       | SPEECH_TX_2MIC_NS7  | 2 mic noise suppr(Cohernt&RLS)    | 58.4      | 38.9          |                        |
|       | SPEECH_TX_2MIC_NS8  | 2 mic noise suppr(DSB)            | 3.5    \  |               |                        |
|       | SPEECH_TX_3MIC_NS   | 3 mic 2FF+FB NS(new)              | 80        | 33.1          | Wnr_enable = 0         |
|       |                     |                                   |           |               | include 3mic_preprocess|
|       | SPEECH_TX_3MIC_NS2  | 3 mic 2FF+FB NS (Cohernt&RLS)     | 62        | 58.7          |                        |
|       | SPEECH_TX_3MIC_NS4  | 3 mic 2FF+FB                      | 67        | 54.6          | Wnr_enable = 0         |
|       |                     |                                   |           |               | include 3mic_preprocess|
|       | SPEECH_TX_AGC       | Automatic Gain Control            | 3         | 0.4           |                        |
|       | SPEECH_TX_COMPEXP   | Compressor and expander           | 4         | 0.6           |                        |
|       | SPEECH_TX_EQ        | Default EQ                        | 0.5     \ | 1.1           | each section           |
| ----- | ------------------- | --------------------------------- | --------- | ------------- | ---------------------- |
|       | SPEECH_RX_NS        | 1 mic noise suppress(old)         | 30      \ |               |                        |
| RX    | SPEECH_RX_NS2       | 1 mic noise suppress(new)         | 16      \ |               |                        |
|       | SPEECH_RX_NS2FLOAT  | 1 mic noise suppress(new float)   | 12.5   \  | 17.9          | banks=64               |
|       | SPEECH_RX_AGC       | Automatic Gain Control            | 3         | 0.4           |                        |
|       | SPEECH_RX_EQ        | Default EQ                        | 0.5     \ | 1.1           | each section           |
| ----- | ------------------- | --------------------------------- | --------- | ------------- | ---------------------- |
**/
#include "plat_types.h"
#include "bt_sco_chain_cfg.h"

#if defined(SPEECH_TX_2MIC_NS4) || defined(SPEECH_TX_3MIC_NS)
static float ff_fb_h[256] = {1.f, };
#endif

#if defined(SPEECH_TX_2MIC_NS7)
static float mic_calib_filter[128] = {
     1.026921865e+00f,  1.066516063e+00f, -2.379298715e+00f,  3.295735996e+00f, -2.110916441e+00f, 
    -1.642222012e-01f,  1.765172633e+00f, -1.452705556e+00f, -1.227181087e-01f,  1.241358164e+00f, 
    -8.287661292e-01f, -2.150899634e-01f,  8.741254901e-01f, -5.820452497e-01f, -3.868203356e-01f, 
    7.055671799e-01f, -2.522748661e-01f, -4.014220806e-01f,  5.700824065e-01f,  3.040575360e-02f, 
    -4.074131900e-01f,  3.105524677e-01f,  8.818269997e-02f, -4.161901729e-01f,  1.332062890e-01f, 
    1.967544290e-01f, -2.641495186e-01f,  1.009787055e-01f,  2.694583440e-01f, -2.062478536e-01f, 
    -6.653330086e-02f,  1.982450828e-01f, -1.375411299e-01f, -1.346241010e-01f,  1.709397053e-01f, 
    4.298187947e-03f, -8.356238340e-02f,  1.457021464e-01f,  1.156270259e-02f, -1.273627686e-01f, 
    8.688512573e-02f,  8.220860475e-03f, -1.711943550e-01f,  6.301936596e-02f,  1.127090829e-01f, 
    -9.126311803e-02f,  2.585536407e-02f,  1.169793605e-01f, -6.593123180e-02f, -4.360538545e-02f, 
    5.846254620e-02f, -6.014766035e-02f, -5.023790880e-02f,  8.523763050e-02f, -3.002185503e-03f, 
    -3.723689249e-02f,  7.549734891e-02f,  1.035621012e-02f, -6.670066117e-02f,  1.287878939e-02f, 
    2.244228836e-03f, -5.772048309e-02f,  3.256585953e-02f,  5.054778815e-02f, -4.101645732e-02f, 
    1.900677906e-02f,  5.635093234e-02f, -4.938058524e-02f, -3.178041990e-02f,  3.413444562e-02f, 
    -2.015737694e-02f, -2.017761016e-02f,  5.007705234e-02f,  2.689892239e-03f, -1.558119384e-02f, 
    3.930001056e-02f, -1.381601352e-02f, -3.900001267e-02f,  3.481939094e-02f,  1.726369005e-04f, 
    -4.268811352e-02f,  3.889233283e-02f,  2.030277265e-02f, -3.876944429e-02f,  2.804667957e-02f, 
    2.197113368e-02f, -5.546218827e-02f,  1.440079790e-02f,  4.977035880e-02f, -5.294981395e-02f, 
    -1.194700007e-02f,  7.332307211e-02f, -3.920071577e-02f, -4.332868984e-02f,  7.397048433e-02f, 
    -7.683862031e-03f, -8.230224338e-02f,  4.145455148e-02f,  6.340536223e-02f, -2.119475135e-02f, 
    -1.199944823e-02f, -1.072356130e-02f, -3.257287289e-02f,  4.561776216e-02f,  2.842669494e-02f, 
    -7.016369527e-02f, -9.817063523e-03f,  4.429458532e-02f, -2.950755004e-02f,  7.957816900e-03f, 
    2.107629621e-02f, -5.289789915e-02f,  3.816761058e-02f,  5.066767189e-02f, -7.316498783e-02f, 
    1.820052229e-02f,  5.862443644e-02f, -8.710090778e-02f,  3.467955615e-02f,  5.211893546e-02f, 
    -1.324903188e-01f,  1.016770517e-01f,  3.528909176e-02f, -1.826161111e-01f,  2.578010784e-01f, 
    -2.185124073e-01f,  1.470132475e-01f, -5.371088515e-02f, 
};
#endif

#if defined(SPEECH_TX_MIC_CALIBRATION)
/****************************************************************************************************
 * Describtion:
 *     Mic Calibration Equalizer, implementation with 2 order iir filters
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     mic_num(1~4): the number of microphones. The filter num is (mic_num - 1)
 *     calib: {bypass, gain, num, {type, frequency, gain, q}}. Please refer to SPEECH_TX_EQ section
 *         in this file
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz, 0.5M/section;
 * Note:
 *     None
****************************************************************************************************/
const SpeechIirCalibConfig WEAK speech_tx_mic_calib_cfg =
{
    .bypass = 0,
    .mic_num = SPEECH_CODEC_CAPTURE_CHANNEL_NUM,
    .delay = 0,
    .calib = {
        {
            .bypass = 0,
            .gain = 0.f,
            .num = 6,
            .params = {
                {IIR_BIQUARD_RAW, {{1.202202895, 0.484307342, 2.146355473, -0.060602271, 1.226130148}}},
                {IIR_BIQUARD_RAW, {{0.629151551, 0.603253626, 1.000000000, 0.732618244, 0.698620434}}},
                {IIR_BIQUARD_RAW, {{-1.296785447, 0.638252598, 1.000000000, -1.327742114, 0.676187375}}},
                {IIR_BIQUARD_RAW, {{-0.530230923, 0.751070042, 1.000000000, -0.625266913, 0.580733666}}},
                {IIR_BIQUARD_RAW, {{1.626356015, 0.917424167, 1.000000000, 1.517616259, 0.868561249}}},
                {IIR_BIQUARD_RAW, {{-0.650912103, -0.321712730, 1.000000000, -0.500931581, -0.464875235}}},
            }
        },
    },
};
#endif

#if defined(SPEECH_TX_MIC_FIR_CALIBRATION)
/****************************************************************************************************
 * Describtion:
 *     Mic Calibration Equalizer, implementation with fir filter
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     mic_num(1~4): the number of microphones. The filter num is (mic_num - 1)
 *     calib: {filter, filter_length, nfft}. 
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz, 0.5M/section;
 * Note:
 *     None
****************************************************************************************************/
float mic2_ft_caliration[256] = {1.f, };
const SpeechFirCalibConfig WEAK speech_tx_mic_fir_calib_cfg =
{
    .bypass = 0,
    .mic_num = SPEECH_CODEC_CAPTURE_CHANNEL_NUM,
    .delay = 0,
    .calib = {
        {
            .filter = mic2_ft_caliration,
            .filter_length = ARRAY_SIZE(mic2_ft_caliration),
        },
    },
};
#endif

const SpeechConfig WEAK speech_cfg_default = {

#if defined(SPEECH_TX_DC_FILTER)
    .tx_dc_filter = {
        .bypass                 = 0,
        .gain                   = 0,
    },
#endif

#if defined(SPEECH_TX_AEC)
/****************************************************************************************************
 * Describtion:
 *     Acoustic Echo Cancellation
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     delay(>0): delay samples from mic to speak, unit(sample).
 *     leak_estimate(0~32767): echo supression value. This is a fixed mode. It has relatively large
 *         echo supression and large damage to speech signal.
 *     leak_estimate_shift(0~8): echo supression value. if leak_estimate == 0, then leak_estimate_shift
 *         can take effect. This is a adaptive mode. It has relatively small echo supression and also 
 *         small damage to speech signal.
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz, 40M;
 * Note:
 *     If possible, use SPEECH_TX_AEC2FLOAT
****************************************************************************************************/
    .tx_aec = {
        .bypass                 = 0,
        .delay                  = 60,
        .leak_estimate          = 16383,
        .leak_estimate_shift    = 4,
    },
#endif

#if defined(SPEECH_TX_AEC2)
/****************************************************************************************************
 * Describtion:
 *     Acoustic Echo Cancellation
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     .
 *     .
 *     .
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz, enNlpEnable = 1, 39M;
 * Note:
 *     If possible, use SPEECH_TX_AEC2FLOAT
****************************************************************************************************/
    .tx_aec2 = {
        .bypass                 = 0,
        .enEAecEnable           = 1,
        .enHpfEnable            = 1,
        .enAfEnable             = 0,
        .enNsEnable             = 0,
        .enNlpEnable            = 1,
        .enCngEnable            = 0,
        .shwDelayLength         = 0,
        .shwNlpBandSortIdx      = 0.75f,
        .shwNlpBandSortIdxLow   = 0.5f,
        .shwNlpTargetSupp       = 3.0f,
        .shwNlpMinOvrd          = 1.0f,
    },
#endif

#if defined(SPEECH_TX_AEC2FLOAT)
/****************************************************************************************************
 * Describtion:
 *     Acoustic Echo Cancellation
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     hpf_enabled(0/1): high pass filter enable or disable. Used to remove DC for Near and Ref signals.
 *     af_enabled(0/1): adaptive filter enable or disable. If the echo signal is very large, enable this
 *     adprop_enabled(0/1): update the filter coefficient according to the blocks enable or disable. If the number of blocks is bigger than 1, enable this.
 *     varistep_enabled(0/1): variable step size enable or disable. When dealing with the double-talk situation, enable this.
 *     nlp_enabled(0/1): non-linear process enable or disable. Enable this by default.
 *     clip_enabled(0/1): residual echo suppression enable or disable.
 *     stsupp_enabled(0/1): single-talk suppression enable or disable.
 *     hfsupp_enabled(0/1): high_frequency echo suppression enable or disable.
 *     constrain_enabled(0/1): constrain background noise enable or disable.
 *     ns_enabled(0/1): noise supression enable or disable. Enable this by default.
 *     cng_enabled(0/1):  comfort noise generator enable or disable.
 *     blocks(1~8): the length of adaptive filter = blocks * frame length
 *     delay(>0): delay samples from mic to speak, unit(sample).
 *     gamma(0~1): forgetting factor for psd estimation.
 *     echo_band_start(0~8000): start band for echo detection, unit(Hz)
 *     echo_band_end(echo_band_start~8000): end band for echo detection, unit(Hz)
 *     min_ovrd(1~6): supression factor, min_ovrd becomes larger and echo suppression becomes larger.
 *     target_supp(<0): target echo suppression, unit(dB)
 *     highfre_band_start(0~8000): start band for high_frequency echo suppression, unit(Hz)
 *     highfre_supp(>=0):high_frequency echo suppression, unit(dB) non-negative number!!!
 *     noise_supp(-30~0): noise suppression, unit(dB)
 *     cng_type(0/1): noise type(0: White noise; 1: Pink noise)
 *     cng_level(<0): noise gain, unit(dB)
 *     clip_threshold: compression threshold
 *     banks:Parameters for NS, bigger then better but cost more.
 * Resource consumption:
 *     RAM:     42K
 *     FLASH:   None
 *     MIPS:    30M(one block);62M(six blocks)    fs = 16kHz;
 * Note:
 *     This is the recommended AEC
****************************************************************************************************/
    .tx_aec2float = {
        .bypass         = 0,
        .hpf_enabled    = false,
        .af_enabled     = false,
        .adprop_enabled    = false,
        .varistep_enabled    = false,
        .nlp_enabled    = true,
        .clip_enabled   = false,
        .stsupp_enabled = false,
        .hfsupp_enabled = false,
        .constrain_enabled = false,
#if defined(SPEECH_TX_NS3) || defined(SPEECH_TX_NS4) || defined(SPEECH_TX_NS5) || defined(SPEECH_TX_DTLN)
        .ns_enabled     = false,
#else
        .ns_enabled     = true,
#endif
        .cng_enabled    = false,
        .blocks         = 1,
#if defined(SPEECH_TX_AEC_CODEC_REF)
        .delay          = 5,
#else
        .delay          = 139,
#endif
        .gamma          = 0.75,
        .echo_band_start = 300,
        .echo_band_end  = 3200,
        .min_ovrd       = 2,
        .target_supp    = -40,
        .highfre_band_start = 4000,
        .highfre_supp   = 8.f,
        .noise_supp     = -15,
        .cng_type       = 1,
        .cng_level      = -60,
        .clip_threshold = -20.f,
        .banks          = 64,
    },
#endif

#if defined(SPEECH_TX_AEC3)
    .tx_aec3 = {
        .bypass         = 0,
        .filter_size     = 16,
    },
#endif

#if defined(SPEECH_TX_2MIC_NS)
/****************************************************************************************************
 * Describtion:
 *     2 MICs Noise Suppression
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz;
 * Note:
 *     None
****************************************************************************************************/
    .tx_2mic_ns = {
        .bypass             = 0,
        .alpha_h            = 0.8,
        .alpha_slow         = 0.9,
        .alpha_fast         = 0.6,
        .thegma             = 0.01,
        .thre_corr          = 0.2,
        .thre_filter_diff   = 0.2,
        .cal_left_gain      = 1.0,
        .cal_right_gain     = 1.0,
        .delay_mono_sample  = 6,
        .wnr_enable         = 0,
    },
#endif

#if defined(SPEECH_TX_2MIC_NS2)
/****************************************************************************************************
 * Describtion:
 *     2 MICs Noise Suppression
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     delay_taps(0~4): ceil{[d(MIC1~mouth) - d(MIC2~mouth)] / 2}.
 *         ceil: Returns the largest integer less than or equal to the specified data
 *         d(MIC~mouth): The dinstance from MIC to mouth
 *         e.g. 0: 0~2cm; 1: 2~4; 2: 5~6...
 *     freq_smooth_enable(1): Must enable
 *     wnr_enable(0/1): wind noise reduction enable or disable. This is also useful for improving
 *         noise suppression, but it also has some damage to speech signal. 
 *     skip_freq_range_start(0~8000): skip dualmic process between skip_freq_range_start and skip_freq_range_end
 *     skip_freq_range_end(0~8000): skip dualmic process between skip_freq_range_start and skip_freq_range_end
 *     betalow(-0.1~0.1): suppression factor for frequency below 500Hz, large means more suppression
 *     betamid(-0.2~0.2): suppression factor for frequency between 500Hz and 2700Hz, large means more suppression
 *     betahigh(-0.3~0.3): suppression factor for frequency between 2700Hz and 8000Hz, large means more suppression
 *     vad_threshold(-1~0): threshold for vad, large means detection is more likely to be triggered
 *     vad_threshold(0~0.1): flux for threshold to avoid miss detection
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz, 32M;
 * Note:
 *     None
****************************************************************************************************/
    .tx_2mic_ns2 = {
        .bypass             = 0,
        .delay_taps         = 0.9f,  
        .dualmic_enable     = 1,
        .freq_smooth_enable = 1,
        .wnr_enable         = 1, 
        .skip_freq_range_start = 0,
        .skip_freq_range_end = 0,       // TODO: Jay: Audio developer has a bug, int type can not be negative
        .noise_supp         = -25,
        .betalow            = 0.07f, // 500
        .betamid            = -0.1f, // 2700
        .betahigh           = -0.2f, // above 2700
        .vad_threshold      = 0.f,
        .vad_threshold_flux = 0.05f,
    },
#endif

#if defined(SPEECH_TX_2MIC_NS5)
/****************************************************************************************************
 * Describtion:
 *     2 MICs Noise Suppression
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     delay_taps(0~4): ceil{[d(MIC1~mouth) - d(MIC2~mouth)] / 2}. Default as 0
 *         ceil: Returns the largest integer less than or equal to the specified data
 *         d(MIC~mouth): The dinstance from MIC to mouth
 *         e.g. 0: 0~2cm; 1: 2~4; 2: 5~6...
 *     freq_smooth_enable(1): Must enable
 *     wnr_enable(0/1): wind noise reduction enable or disable. This is also useful for improving
 *         noise suppression, but it also has some damage to speech signal. 
 *     delay_enable(0/1): enable the delay_taps or not. Ideally, never need to enable the delay and
 *          delay_taps will be a useless parameter.
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz, 32M;
 * Note:
 *     None
****************************************************************************************************/
    .tx_2mic_ns5 = {
        .bypass             = 0,
        .delay_taps         = 0.0f,
        .freq_smooth_enable = 1,
        .wnr_enable         = 0, 
        .delay_enable       = 0,
    },
#endif


#if defined(SPEECH_TX_2MIC_NS6)
/****************************************************************************************************
 * Describtion:
 *     2 MICs Noise Suppression
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     wnr_enable(0/1): wind noise reduction enable or disable. This is also useful for improving
 *         noise suppression, but it also has some damage to speech signal. 
 *     denoise_dB : The minimum gain in the MMSE NS algorithm which is integrated in wind noise.
 * Resource consumption:
 *     RAM:     TBD
 *     FLASE:   TBD
 *     MIPS:    fs = 16kHz, TBD;
 * Note:
 *     None
****************************************************************************************************/
    .tx_2mic_ns6 = {
        .bypass             = 0,
        .wnr_enable         = 0, 
        .denoise_dB       = -12,
    },
#endif

#if defined(SPEECH_TX_2MIC_NS4)
/****************************************************************************************************
 * Describtion:
 *     2 MICs Noise Suppression
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz;
 * Note:
 *     None
****************************************************************************************************/
    .tx_2mic_ns4 = {
        .bypass             = 0,
        .blend_en           = 1,
        // .left_gain          = 1.0f,
        // .right_gain         = 1.0f,
        .delay_tapsM        = 0,
        .delay_tapsS        = 0,
        // .delay_tapsC        = 32,
        //////////////////////////{{a0,a1,a2,a3,a4},{b0,b1,b2,b3,b4}}///////////////////////////
        // .coefH[0]           = {1.0, -1.88561808316413, 1.55555555555556, -0.628539361054709, 0.111111111111111},
        // .coefH[1]           = {0.323801506930344, -1.29520602772138, 1.94280904158206, -1.29520602772138, 0.323801506930344},
        // .coefL[0]           = {1.0, -1.88561808316413, 1.55555555555556, -0.628539361054709, 0.111111111111111},
        // .coefL[1]           = {0.00953182640298944, 0.0381273056119578, 0.0571909584179366, 0.0381273056119578, 0.00953182640298944},
        .crossover_freq     = 1000,
        .ff_fb_coeff        = ff_fb_h,
        .ff_fb_coeff_len    = ARRAY_SIZE(ff_fb_h),
    },
#endif

#if defined(SPEECH_TX_2MIC_NS7)
/****************************************************************************************************
* Describtion:
*     High Performance 2 MICs BF Noise Suppression
*
* Parameters:
*     bypass(0/1): bypass enable or disable.
*     wnr_enable(0/1): bypass wind enhancment enable or disable.
*     wind_thd(0.1-0.95): wind threshold for wind enhancement function.
*     wnd_pwr_thd(1:32768): wind pwr threshold for wind enhancement function.
*     wind_gamma(0.1-0.95): wind convergence speed tunning key.
*     state_trans_frame_thd(1:32768): wind state threshold for wind enhancement function.
*     af_enable(0/1): bypass denoise enable or disable.
*     filter_gamma(0.1-0.99): denoise convergence speed tunning key.
*     vad_bin_start1(0-fs/2): denoise tunning para.
*     vad_bin_end1(0-fs/2): denoise tunning para.
*     vad_bin_start2(0-fs/2): denoise tunning para.
*     vad_bin_end2(0-fs/2): denoise tunning para.
*     vad_bin_start3(0-fs/2): denoise tunning para.
*     vad_bin_end3(0-fs/2): denoise tunning para.
*     coef1_thd(0.1-0.999): denoise tunning para.
*     coef2_thd(0.1-0.999): denoise tunning para.
*     coef3_thd(0.1-32768): denoise tunning para.
*     calib_enable(0/1): bypass calibration enable or disable.
*     calib_delay(0-512): delay para for calibration.
*     filter(float point): filter coef for calibration.
*     filter_len(16-256):filter length for calibration.
*     low_ram_enable(0/1): enable low ram mode to reduce RAM size & reduce MIPS
*     low_mips_enable(0/1): enable low mips mode to reduce MIPS
*     echo_supp_enable(0/1): enable echo suppression function
*     ref_delay(-240-240): delay for ref signal (no use so far, TBD)
*     post_supp_enable(0/1): enable post denoise function
*     denoise_db(0--50): max denoise dB for post denoise

* Resource consumption:
*-----If low_ram_enable == 0 && low_mips_enable == 0
*     Basic RAM = 40.3K, MIPS = 42M
*     IF set calib_enable = 1. Need more RAM += 4.3K & MIPS += 8M
*     IF set echo_supp_enable = 1. Need more RAM += 10.7K & MIPS += 19M
*     IF set post_supp_enable = 1. Need more RAM += 12K & MIPS += 22M
*     Total: RAM = 70K, MIPS = 91M

*-----If low_ram_enable == 1 && low_mips_enable == 0
*     Basic RAM = 21K, MIPS = 21M
*     IF set calib_enable = 1. Need more RAM += 2.2K & MIPS += 8M
*     IF set echo_supp_enable = 1. Need more RAM += 5.2K & MIPS += 9M
*     IF set post_supp_enable = 1. Need more RAM += 7.5K & MIPS += 12M
*     Total: RAM = 36K, MIPS = 50M

*-----If low_ram_enable == 0 && low_mips_enable == 1
*     Basic RAM = 40K, MIPS = 21M
*     IF set calib_enable = 1. Need more RAM += 4K & MIPS += 8M
*     IF set echo_supp_enable = 1. Need more RAM += 11K & MIPS += 9M
*     IF set post_supp_enable = 1. Need more RAM += 12K & MIPS += 12M
*     Total: RAM = 70K, MIPS = 50M

****************************************************************************************************/
    .tx_2mic_ns7 = {
        .bypass =       0,
        .wnr_enable =   1,
        .wind_thd =     0.6,
        .wnd_pwr_thd =  1600,
        .wind_gamma =   0.7,
        .state_trans_frame_thd = 60,
        .af_enable =    1,
        .filter_gamma = 0.9,
        .vad_bin_start1 =   50,
        .vad_bin_end1 =   3230,
        .vad_bin_start2 =   300,
        .vad_bin_end2 =   930,
        .vad_bin_start3 =   50,
        .vad_bin_end3 =   1100,
        .coef1_thd =    0.94F,
        .coef2_thd =    0.9F,
        .coef3_thd =    30.0F,
        .supp_times =   2,

        .calib_enable=  1,
        .calib_delay=   2,
        .filter=       mic_calib_filter,
        .filter_len=  ARRAY_SIZE(mic_calib_filter),

        .low_ram_enable=    0,
        .low_mips_enable=    0,

        .echo_supp_enable=  1,
        .ref_delay= 0,

        .post_supp_enable=  1,
        .denoise_db=    -10,
        .wdrc_enable = 0,
    },
#endif

#if defined(SPEECH_TX_2MIC_NS8)
/****************************************************************************************************
 * Describtion:
 *     2 MICs Noise Suppression
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     delay_taps(0~4): ceil{[d(MIC1~mouth) - d(MIC2~mouth)] / 2}. Default as 0
 *         ceil: Returns the largest integer less than or equal to the specified data
 *         d(MIC~mouth): The dinstance from MIC to mouth
 *         e.g. 0: 0~2cm; 1: 2~4; 2: 5~6...
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz, 32M;
 * Note:
 *     None
****************************************************************************************************/
    .tx_2mic_ns8 = {
        .bypass             = 0,
        .delay_taps         = 1.5f,
        .ref_pwr_threshold  = 30.f,
    },
#endif

#if defined(SPEECH_TX_3MIC_NS)
/****************************************************************************************************
 * Describtion:
 *     3 MICs Noise Suppression
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     delay_tapsM(0~4): MIC L/R delay samples. Refer to SPEECH_TX_2MIC_NS2 delay_taps
 *     delay_tapsS(0~4): MIC L/S delay samples. Refer to SPEECH_TX_2MIC_NS2 delay_taps
 *     freq_smooth_enable(1): Must enable
 *     wnr_enable(0/1): wind noise reduction enable or disable. This is also useful for improving
 *         noise suppression, but it also has some damage to speech signal. 
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz;
 * Note:
 *     None
****************************************************************************************************/
    .tx_3mic_ns = {
		.bypass = 0,
		.blend_en = 1,
		.authen_en = 0,
		.delay_tapsM = 0.f,
		.delay_tapsS = 4.f,
		.denoise_dB = -6.0f,
		.crossover_freq = 500,
		.freq_smooth_enable = 1,
		.wnr_enable = 1,
		.noise_supp = -20,
		.betalow = -0.05f,
		.betamid = -0.1f,
		.betahigh = -0.2f,
		.ref_pwr_threshold = 2,
		.ff_fb_coeff = ff_fb_h,
		.ff_fb_coeff_len = ARRAY_SIZE(ff_fb_h),
		.eq_cfg = {
			.bypass = 1,
			.gain = 5.f,
			.num = 1,
			.params = {
				{IIR_BIQUARD_HPF, {{60, 0, 0.707}}},
			}
		}
    },
#endif

#if defined(SPEECH_TX_3MIC_NS2)
/****************************************************************************************************
 * Describtion:
 *     3 MICs Noise Suppression2
 * Parameters:

 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz;
 * Note:
 *     None
****************************************************************************************************/
    .tx_3mic_ns2 = {
		.bypass      = 0,
		.wnr_enable  = 1,
		.denoise_dB  = -6,
		.delay_taps  = 0.6,
		.freq_smooth_enable = 1,
		.crossover_freq = 1000,
	},
#endif

#if defined(SPEECH_TX_3MIC_NS3)
/****************************************************************************************************
 * Describtion:
 *     3 MICs Noise Suppression
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     delay_taps(0~4): ceil{[d(MIC1~mouth) - d(MIC2~mouth)] / 2}.
 *         ceil: Returns the largest integer less than or equal to the specified data
 *         d(MIC~mouth): The dinstance from MIC to mouth
 *         e.g. 0: 0~2cm; 1: 2~4; 2: 5~6...
 *     freq_smooth_enable(1): Must enable
 *     wnr_enable(0/1): wind noise reduction enable or disable. This is also useful for improving
 *         noise suppression, but it also has some damage to speech signal. 
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz, 32M;
 * Note:
 *     None
****************************************************************************************************/
    .tx_3mic_ns3 = {
        .bypass             = 0,
        .endfire_enable     = 1,
        .broadside_enable   = 1,
        .delay_taps         = 0.7f,  
        .freq_smooth_enable = 1,
        .wnr_enable         = 0, 
    },
#endif

#if defined(SPEECH_TX_NS)
/****************************************************************************************************
 * Describtion:
 *     Noise Suppression
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz, 30M;
 * Note:
 *     If possible, use SPEECH_TX_NS2 or SPEECH_TX_NS2FLOAT
****************************************************************************************************/
    .tx_ns = {
        .bypass     = 0,
        .denoise_dB = -12,
    },
#endif

#if defined(SPEECH_TX_NS2)
/****************************************************************************************************
 * Describtion:
 *     Noise Suppression
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     denoise_dB(-30~0): noise suppression, unit(dB). 
 *         e.g. -15: Can reduce 15dB of stationary noise.
 * Resource consumption:
 *     RAM:     fs = 16k:   RAM = 8k; 
 *              fs = 8k:    RAM = 4k;
 *              RAM = frame_size * 30
 *     FLASE:   None
 *     MIPS:    fs = 16kHz, 16M;
 * Note:
 *     None
****************************************************************************************************/
    .tx_ns2 = {
        .bypass     = 0,
        .denoise_dB = -15,
    },
#endif

#if defined(SPEECH_TX_NS2FLOAT)
/****************************************************************************************************
 * Describtion:
 *     Noise Suppression
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     denoise_dB(-30~0): noise suppression, unit(dB). 
 *         e.g. -15: Can reduce 15dB of stationary noise.
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz;
 * Note:
 *     This is a 'float' version for SPEECH_TX_NS2. 
 *     It needs more MIPS and RAM, but can redece quantization noise.
****************************************************************************************************/
    .tx_ns2float = {
        .bypass     = 0,
        .denoise_dB = -15,
        .banks      = 64,
    },
#endif

#if defined(SPEECH_TX_NS3)
/****************************************************************************************************
 * Describtion:
 *     Noise Suppression
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     mode: None
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz;
 * Note:
 *     None
****************************************************************************************************/
    .tx_ns3 = {
        .bypass = 0,
        .denoise_dB = -24,
    },
#endif

#if defined(SPEECH_TX_NS4)
/****************************************************************************************************
 * Describtion:
 *     Noise Suppression
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 * Resource consumption:
 *     COST:    FFT=512:  RAM = 22.3k, MIPS = 24M;
                FFT=256:  RAM = 11.3K, MIPS = 24M;(Need to open MACRO BT_SCO_LOW_RAM in bt_sco_chain.c)
 * Note:
****************************************************************************************************/
    .tx_ns4 = {
        .bypass     = 0,
        .denoise_dB = -10,
    },
#endif

#if defined(SPEECH_TX_NS5)
/****************************************************************************************************
 * Describtion:
 *     Noise Suppression
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 * Resource consumption:
 *     COST:    FFT=512:  RAM = k, MIPS = M;
                FFT=256:  RAM = K, MIPS = M;(Need to open MACRO BT_SCO_LOW_RAM in bt_sco_chain.c)
 * Note:
****************************************************************************************************/
    .tx_ns5 = {
        .bypass     = 0,
        .denoise_dB = -30,
        .echo_supp_enable = true,
        .ref_delay = 70,
        .gamma = 0.8,
        .echo_band_start = 300,
        .echo_band_end = 3800,
        .min_ovrd = 2,
        .target_supp = -40,
    },
#endif

#if defined(SPEECH_TX_NS7)
/****************************************************************************************************
 * Describtion:
 *     Noise Suppression
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 * Resource consumption:

 * Note:
****************************************************************************************************/
    .tx_ns7 = {
        .bypass     = 0,
        .denoise_dB = -10,
    },
#endif

#if defined(SPEECH_TX_WNR)
/****************************************************************************************************
 * Describtion:
 *     Wind Noise Suppression
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     lpf_cutoff: lowpass filter for wind noise detection
 *     hpf_cutoff: highpass filter for wind noise suppression
 *     power_ratio_thr: ratio of the power spectrum of the lower frequencies over the total power
                        spectrum for all frequencies
 *     power_thr: normalized power of the low frequencies
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz;
 * Note:
 *     None
****************************************************************************************************/
    .tx_wnr = {
        .bypass = 0,
        .lpf_cutoff   = 1000,
        .hpf_cutoff = 400,
        .power_ratio_thr = 0.9f,
        .power_thr = 1.f,
    },
#endif

#if defined(SPEECH_TX_NOISE_GATE)
/****************************************************************************************************
 * Describtion:
 *     Noise Gate
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     data_threshold(0~32767): distinguish between noise and speech, unit(sample).
 *     data_shift(1~3): 1: -6dB; 2: -12dB; 3: -18dB
 *     factor_up(float): attack time, unit(s)
 *     factor_down(float): release time, unit(s)
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz;
 * Note:
 *     None
****************************************************************************************************/
    .tx_noise_gate = {
        .bypass         = 0,
        .data_threshold = 640,
        .data_shift     = 1,
        .factor_up      = 0.001f,
        .factor_down    = 0.5f,
    },
#endif

#if defined(SPEECH_TX_COMPEXP)
/****************************************************************************************************
 * Describtion:
 *     Compressor and expander
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     ...
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz;
 * Note:
 *     None
****************************************************************************************************/
    .tx_compexp = {
        .bypass             = 0,
        .type               = 0,
        .comp_threshold     = -20.f,
        .comp_ratio         = 2.f,
        .expand_threshold   = -65.f,
        .expand_ratio       = 0.555f,
        .attack_time        = 0.008f,
        .release_time       = 0.06f,
        .makeup_gain        = 10,
        .delay              = 128,
        .tav                = 0.1f,
    },
#endif

#if defined(SPEECH_TX_AGC)
/****************************************************************************************************
 * Describtion:
 *     Automatic Gain Control
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     target_level(>0): signal can not exceed (-1 * target_level)dB.
 *     compression_gain(>0): excepted gain.
 *     limiter_enable(0/1): 0: target_level does not take effect; 1: target_level takes effect.
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz, 3M;
 * Note:
 *     None
****************************************************************************************************/
    .tx_agc = {
        .bypass             = 0,
        .target_level       = 3,
        .compression_gain   = 6,
        .limiter_enable     = 1,
    },
#endif

#if defined(SPEECH_TX_EQ)
/****************************************************************************************************
 * Describtion:
 *     Equalizer, implementation with 2 order iir filters
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 *     gain(float): normalized gain. It is usually less than or equal to 0
 *     num(0~6): the number of EQs
 *     params: {type, frequency, gain, q}. It supports a lot of types, please refer to iirfilt.h file
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz, 0.5M/section;
 * Note:
 *     None
****************************************************************************************************/
    .tx_eq = {
        .bypass     = 0,
        .gain       = 0.f,
        .num        = 1,
        .params = {
            {IIR_BIQUARD_HPF, {{100, 0, 0.707f}}},
        },
    },
#endif

#if defined(SPEECH_TX_POST_GAIN)
/****************************************************************************************************
 * Describtion:
 *     Linear Gain
 * Parameters:
 *     bypass(0/1): bypass enable or disable.
 * Resource consumption:
 *     RAM:     None
 *     FLASE:   None
 *     MIPS:    fs = 16kHz;
 * Note:
 *     None
****************************************************************************************************/
    .tx_post_gain = {
        .bypass     = 0,
        .gain_dB    = 6.0f,
    },
#endif

// #if defined(SPEECH_CS_VAD)
// /****************************************************************************************************
//  * Describtion:
//  *     Voice Activity Detector
//  * Parameters:
//  *     bypass(0/1): bypass enable or disable.
//  * Resource consumption:
//  *     RAM:     None
//  *     FLASE:   None
//  *     MIPS:    fs = 16kHz;
//  * Note:
//  *     None
// ****************************************************************************************************/
//     .tx_vad = {
//         .snrthd     = 5.f,
//         .energythd  = 100.f,
//     },
// #endif

#if defined(SPEECH_RX_NS)
/****************************************************************************************************
 * Describtion:
 *     Acoustic Echo Cancellation
 * Parameters:
 *     Refer to SPEECH_TX_NS
 * Note:
 *     None
****************************************************************************************************/
    .rx_ns = {
        .bypass     = 0,
        .denoise_dB = -12,
    },
#endif

#if defined(SPEECH_RX_NS2)
/****************************************************************************************************
 * Describtion:
 *     Acoustic Echo Cancellation
 * Parameters:
 *     Refer to SPEECH_TX_NS2
 * Note:
 *     None
****************************************************************************************************/
    .rx_ns2 = {
        .bypass     = 0,
        .denoise_dB = -15,
    },
#endif

#if defined(SPEECH_RX_NS2FLOAT)
/****************************************************************************************************
 * Describtion:
 *     Acoustic Echo Cancellation
 * Parameters:
 *     Refer to SPEECH_TX_NS2FLOAT
 * Note:
 *     None
****************************************************************************************************/
    .rx_ns2float = {
        .bypass     = 0,
        .denoise_dB = -15,
        .banks      = 64,
    },
#endif

#if defined(SPEECH_RX_NS3)
/****************************************************************************************************
 * Describtion:
 *     Acoustic Echo Cancellation
 * Parameters:
 *     Refer to SPEECH_TX_NS3
 * Note:
 *     None
****************************************************************************************************/
    .rx_ns3 = {
        .bypass = 0,
        .denoise_dB   = -18,
    },
#endif

#if defined(SPEECH_RX_NOISE_GATE)
/****************************************************************************************************
 * Describtion:
 *     Noise Gate
 * Parameters:
 *     Refer to SPEECH_TX_NOISE_GATE
 * Note:
 *     None
****************************************************************************************************/
    .rx_noise_gate = {
        .bypass         = 0,
        .data_threshold = 640,
        .data_shift     = 1,
        .factor_up      = 0.001f,
        .factor_down    = 0.5f,
    },
#endif

#if defined(SPEECH_RX_COMPEXP)
/****************************************************************************************************
 * Describtion:
 *     Compressor and expander
 * Parameters:
 *     Refer to SPEECH_TX_COMPEXP
 * Note:
 *     None
****************************************************************************************************/
    .rx_compexp = {
        .bypass = 0,
        .num = 2,
        .xover_freq = {5000},
        .order = 4,
        .params = {
            {
                .bypass             = 0,
                .type               = 0,
                .comp_threshold     = -10.f,
                .comp_ratio         = 2.f,
                .expand_threshold   = -60.f,
                .expand_ratio       = 0.5556f,
                .attack_time        = 0.001f,
                .release_time       = 0.6f,
                .makeup_gain        = 0,
                .delay              = 128,
                .tav                = 0.2f,
            },
            {
                .bypass             = 0,
                .type               = 0,
                .comp_threshold     = -10.f,
                .comp_ratio         = 2.f,
                .expand_threshold   = -60.f,
                .expand_ratio       = 0.5556f,
                .attack_time        = 0.001f,
                .release_time       = 0.6f,
                .makeup_gain        = 0,
                .delay              = 128,
                .tav                = 0.2f,
            }
        }
    },
#endif

#if defined(SPEECH_RX_AGC)
/****************************************************************************************************
 * Describtion:
 *      Automatic Gain Control
 * Parameters:
 *     Refer to SPEECH_TX_AGC
 * Note:
 *     None
****************************************************************************************************/
    .rx_agc = {
        .bypass             = 0,
        .target_level       = 3,
        .compression_gain   = 6,
        .limiter_enable     = 1,
    },
#endif

#if defined(SPEECH_RX_EQ)
/****************************************************************************************************
 * Describtion:
 *     Equalizer, implementation with 2 order iir filters
 * Parameters:
 *     Refer to SPEECH_TX_EQ
 * Note:
 *     None
****************************************************************************************************/
    .rx_eq = {
        .bypass = 0,
        .gain   = 0.f,
        .num    = 1,
        .params = {
            {IIR_BIQUARD_HPF, {{60, 0, 0.707f}}},
        },
    },
#endif

#if defined(SPEECH_RX_HW_EQ)
/****************************************************************************************************
 * Describtion:
 *     Equalizer, implementation with 2 order iir filters
 * Parameters:
 *     Refer to SPEECH_TX_EQ
 * Note:
 *     None
****************************************************************************************************/
    .rx_hw_eq = {
        .bypass = 0,
        .gain   = 0.f,
        .num    = 1,
        .params = {
            {IIR_BIQUARD_HPF, {{60, 0, 0.707f}}},
        },
    },
#endif

#if defined(SPEECH_RX_DAC_EQ)
/****************************************************************************************************
 * Describtion:
 *     Equalizer, implementation with 2 order iir filters
 * Parameters:
 *     Refer to SPEECH_TX_EQ
 * Note:
 *     None
****************************************************************************************************/
    .rx_dac_eq = {
        .bypass = 0,
        .gain   = 0.f,
        .num    = 1,
        .params = {
            {IIR_BIQUARD_HPF, {{60, 0, 0.707f}}},
        },
    },
#endif

#if defined(SPEECH_RX_POST_GAIN)
/****************************************************************************************************
 * Describtion:
 *     Linear Gain
 * Parameters:
 *     Refer to SPEECH_TX_POST_GAIN
 * Note:
 *     None
****************************************************************************************************/
    .rx_post_gain = {
        .bypass     = 0,
        .gain_dB    = 6.0f,
    },
#endif

};
