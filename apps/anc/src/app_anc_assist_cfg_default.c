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
#include "plat_types.h"
#include "anc_assist.h"


const static int16_t custom_leak_detect_pcm_data[] = {
   #include "res/ld/tone_peco.h"
};

AncAssistConfig anc_assist_cfg = {
    .bypass = 0,
    .debug_en = 0,

    .ff_howling_en  = 0,
    .fb_howling_en  = 0,
    .noise_en   = 0,
    .wind_en    = 0,
	.pilot_en   = 0,
	.pnc_en     = 0,
	.noise_classify_en  = 0,
    .wsd_en     = 0,
    .extern_kws_en     = 0,
    .extern_adaptive_eq_en     = 0,

    .ff_howling_cfg = {
        .ind0 = 32,         // 62.5Hz per 62.5*32=2k
        .ind1 = 120,        // 62.5*120=7.5k
        .time_thd = 500,    // ff recover time 500*7.5ms=3.75s
        .power_thd = 1e4f,
    },

    .fb_howling_cfg = {
        .ind0 = 32,         // 62.5Hz per 62.5*32=2k
        .ind1 = 120,        // 62.5*120=7.5k
        .time_thd = 500,    // ff recover time 500*7.5ms=3.75s
        .power_thd = 1e5f,
    },

    .noise_cfg = {
        .power_noise_dual_thd_upper = 4, // value * 100 = value in log
        .power_noise_dual_thd_lower = 2, // value * 100 = value in log
        .power_silence_dual_thd_upper = 0.5,
        .power_silence_dual_thd_lower = 0.1,
        .snr_thd = 100, // thd to set slint
        .silence_gain_fb = 0.3, // fb gain change
        .silence_gain_ff = 1.0, // fb gain change
        .extreme_silence_gain_fb = 0.0, // fb gain change
        .extreme_silence_gain_ff = 0.0, // fb gain change
        .period = 16,  // base time
        .noise2silence_cnt_thd = 5,  // change period at least 5 state to change 
        .silence2noise_cnt_thd = 15,
        .snr_pwr_thd = 1600,
        .snr_window_size = 15, // 
        .pwr_window_size = 1,

        .band_freq = {200, 650, 1000, 1000},
        .band_weight = {1.0, 0.0, 0.0},
    },
	
    .wind_cfg = {
        .scale_size = 8,           // freq range,8/scale=1k
        .to_none_targettime = 500, // time=500*7.5ms=3.75s
        .power_thd = 0.0005, 
        .no_thd = 0.8,
        .small_thd = 0.6,
        .strong_thd = 0.3,
        .gain_none = 1.0,
        .gain_small_to_none = 0.75,
        .gain_small = 0.5,
        .gain_strong_to_small = 0.25,
        .gain_strong = 0.0,
    },

    .pilot_cfg = {
        .dump_en = 0,
	    .delay = 310,
#ifdef VOICE_ASSIST_WD_ENABLED
	    .cal_period = 25,
#else
        .cal_period = 25,
#endif
        .gain_smooth_ms = 300,

        .adaptive_anc_en = 0,
        .playback_ms = 0,
	    .thd_on = {5.6400,  1.5648,  0.6956,  0.4672,  0.4089,  0.2694, 0.1853,0.1373,0.0629,0.0325,0.0043},
	    .thd_off = {5.6400,  1.5648,  0.6956,  0.4672,  0.4089,  0.2694, 0.1853,0.1373,0.0629,0.0325,0.0043},

        .wd_en =0,
        .ultrasound_stable_tick = 2,
        .ultrasound_stop_tick = 3,
        .ultrasound_thd = 0.1,
        .inear_thd = 1, //0.1,
        .outear_thd = 0.5, // 0.02,

        .custom_leak_detect_en = 0,
        .custom_leak_detect_playback_loop_num = 2,
        .custom_pcm_data = custom_leak_detect_pcm_data,
        .custom_pcm_data_len = sizeof(custom_leak_detect_pcm_data) / sizeof(int16_t),
        .gain_local_pilot_signal = 0.0,
    },

    .prompt_cfg = {
        .dump_en = 1,
        //.delay = 0,
	    .cal_period = 10,
        .curve_num = 10,
        .max_env_energy = 1e9,
        .start_index = 1,
        .end_index = 10,

#if defined PORMPT_ANC_FUNC_VIVO_L
        .thd1 = {
-7.096,
-8.154333333,
-9.212666667,
-10.271,
-11.32933333,
-12.38766667,
-13.446,
-14.50433333,
-15.56266667,
-16.621
        },
        .thd2 = {
-9.103,
-9.664,
-10.225,
-10.786,
-11.347,
-11.908,
-12.469,
-13.03,
-13.591,
-14.152
        },
        .thd3 = {
-9.423,
-9.759666667,
-10.09633333,
-10.433,
-10.76966667,
-11.10633333,
-11.443,
-11.77966667,
-12.11633333,
-12.453
        }
#elif defined PORMPT_ANC_FUNC_VIVO_R
        .thd1 = {
-10.625,
-11.71033333,
-12.79566667,
-13.881,
-14.96633333,
-16.05166667,
-17.137,
-18.22233333,
-19.30766667,
-20.393
        },
        .thd2 = {
-13.66,
-14.15666667,
-14.65333333,
-15.15,
-15.64666667,
-16.14333333,
-16.64,
-17.13666667,
-17.63333333,
-18.13
        },
        .thd3 = {
-13.615,
-13.95477778,
-14.29455556,
-14.63433333,
-14.97411111,
-15.31388889,
-15.65366667,
-15.99344444,
-16.33322222,
-16.673

        }
#else
        .thd1 = {1,0,-2,-4,-6,-8,-10,-12},
        .thd2 = {1.5,-0.5,-2.5,-4.5,-6.5,-8.5,-10.5,-12.5},
        .thd3 = {1,-1,-3,-5,-7,-9,-11,-13}
#endif
    },

    .pnc_cfg = {
        .pnc_lower_bound = 22,
        .pnc_upper_bound = 25,
	    .out_lower_bound = 34,
	    .out_upper_bound = 38,
        .cal_period = 25,
        .out_thd = 8.0,
    },

    .noise_classify_cfg = {
        .slope_thd1 = 3.5,
        .slope_thd2 = 1.5,
        .cache_num = 300,
        .ps_index1 = 8,
        .ps_index2 = 16,
    },
};