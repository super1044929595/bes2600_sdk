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
#include "app_anc_assist.h"
#include "app_voice_assist_oneshot_adapt_anc.h"
#include "anc_select_mode.h"
#include "anc_assist.h"

#include "aud_section.h"
#include "anc_process.h"
#include "arm_math.h"
#include "audio_dump.h"
#include "app_utils.h"
// #define ONESHOT_ADAPT_ANC_DUMP


#ifdef ONESHOT_ADAPT_ANC_DUMP
static int tmp_data[120];
#endif
static int local_close_cnt = 0;
static int g_best_mode_index;
static int g_best_gain_index;

// local anc settings for adaptive anc
static struct_anc_cfg POSSIBLY_UNUSED AncFirCoef_50p7k_mode0 = {
    .anc_cfg_ff_l = {
        .total_gain = 512,
        .iir_bypass_flag = 0,
        .iir_counter = 5,
        .iir_coef[0].coef_b = {0xebdfa052,0x27a25ad1,0xec673052},
        .iir_coef[0].coef_a = {0x08000000,0xf1464592,0x06c235e2},
        .iir_coef[1].coef_b = {0x017db6bf,0xfd439dd8,0x01472094},
        .iir_coef[1].coef_a = {0x08000000,0xf089e726,0x077e8e06},
        .iir_coef[2].coef_b = {0x09d6db7c,0xec5fb594,0x09c988fc},
        .iir_coef[2].coef_a = {0x08000000,0xf00c1cda,0x07f3fd31},
        .iir_coef[3].coef_b = {0x08001cf6,0xf000dfb4,0x07ff0369},
        .iir_coef[3].coef_a = {0x08000000,0xf000dfb1,0x07ff205b},
        .iir_coef[4].coef_b = {0,0,0},
        .iir_coef[4].coef_a = {0,0,0},
        .iir_coef[5].coef_b = {0,0,0},
        .iir_coef[5].coef_a = {0,0,0},
        .iir_coef[6].coef_b = {0,0,0},
        .iir_coef[6].coef_a = {0,0,0},
        .iir_coef[7].coef_b = {0,0,0},
        .iir_coef[7].coef_a = {0,0,0},
        .dac_gain_offset = 0,
        .adc_gain_offset = -24,
    },
    .anc_cfg_fb_l = {
        .total_gain = 512,
        .iir_bypass_flag = 0,
        .iir_counter = 4,
        .iir_coef[0].coef_b = {0xf5db4b12,0x1420727c,0xf6043e47},
        .iir_coef[0].coef_a = {0x08000000,0xf0034405,0x07fcbf4a},
        .iir_coef[1].coef_b = {0x04ece606,0xf66a94c5,0x04b97e05},
        .iir_coef[1].coef_a = {0x08000000,0xf66a94c5,0x01a6640b},
        .iir_coef[2].coef_b = {0x080c6b31,0xf0207bd5,0x07d3ab03},
        .iir_coef[2].coef_a = {0x08000000,0xf0207bd5,0x07e01634},
        .iir_coef[3].coef_b = {0x07c930fd,0xf10112ce,0x073c5880},
        .iir_coef[3].coef_a = {0x08000000,0xf10112ce,0x0705897e},
        .iir_coef[4].coef_b = {0,0,0},
        .iir_coef[4].coef_a = {0,0,0},
        .iir_coef[5].coef_b = {0,0,0},
        .iir_coef[5].coef_a = {0,0,0},
        .iir_coef[6].coef_b = {0,0,0},
        .iir_coef[6].coef_a = {0,0,0},
        .iir_coef[7].coef_b = {0,0,0},
        .iir_coef[7].coef_a = {0,0,0},
        .dac_gain_offset = 0,
        .adc_gain_offset = -24,
    },
    .anc_cfg_tt_l = {
        .total_gain = 0,
        .iir_bypass_flag = 0,
        .iir_counter = 1,
        .iir_coef[0].coef_b = {0x08000000,0xf022991b,0x07ddf8db},
        .iir_coef[0].coef_a = {0x08000000,0xf022991b,0x07ddf8db},
        .iir_coef[1].coef_b = {0,0,0},
        .iir_coef[1].coef_a = {0,0,0},
        .iir_coef[2].coef_b = {0,0,0},
        .iir_coef[2].coef_a = {0,0,0},
        .iir_coef[3].coef_b = {0,0,0},
        .iir_coef[3].coef_a = {0,0,0},
        .iir_coef[4].coef_b = {0,0,0},
        .iir_coef[4].coef_a = {0,0,0},
        .iir_coef[5].coef_b = {0,0,0},
        .iir_coef[5].coef_a = {0,0,0},
        .iir_coef[6].coef_b = {0,0,0},
        .iir_coef[6].coef_a = {0,0,0},
        .iir_coef[7].coef_b = {0,0,0},
        .iir_coef[7].coef_a = {0,0,0},
        .dac_gain_offset = 0,
        .adc_gain_offset = -24,
    },
    .anc_cfg_mc_l = {
        .total_gain = 0,
        .iir_bypass_flag = 0,
        .iir_counter = 1,
        .iir_coef[0].coef_b = {0x08000000,0xf022991b,0x07ddf8db},
        .iir_coef[0].coef_a = {0x08000000,0xf022991b,0x07ddf8db},
        .iir_coef[1].coef_b = {0,0,0},
        .iir_coef[1].coef_a = {0,0,0},
        .iir_coef[2].coef_b = {0,0,0},
        .iir_coef[2].coef_a = {0,0,0},
        .iir_coef[3].coef_b = {0,0,0},
        .iir_coef[3].coef_a = {0,0,0},
        .iir_coef[4].coef_b = {0,0,0},
        .iir_coef[4].coef_a = {0,0,0},
        .iir_coef[5].coef_b = {0,0,0},
        .iir_coef[5].coef_a = {0,0,0},
        .iir_coef[6].coef_b = {0,0,0},
        .iir_coef[6].coef_a = {0,0,0},
        .iir_coef[7].coef_b = {0,0,0},
        .iir_coef[7].coef_a = {0,0,0},
        .dac_gain_offset = 0,
        .adc_gain_offset = 0,
    }
};




// gain search list for algo
static POSSIBLY_UNUSED const short pnc_gain_list[46]={
256,
 264,
 271,
 279,
 287,
 296,
 304,
 313,
 323,
 332,
 342,
 352,
 362,
 373,
 383,
 395,
 406,
 418,
 430,
 443,
 456,
 469,
 483,
 497,
 512,
 526,
 542,
 558,
 574,
 591,
 608,
 626,
 644,
 663,
 682,
 702,
 723,
 744,
 766,
 788,
 811,
 835,
 859,
 884,
 910
};
// mode search list for mode gain
static POSSIBLY_UNUSED const float pnc_mode_list[50]={
0.0,
 0.25,
 0.5,
 0.75,
 1.0,
 1.25,
 1.5,
 1.75,
 2.0,
 2.25,
 2.5,
 2.75,
 3.0,
 3.25,
 3.5,
 3.75,
 4.0,
 4.25,
 4.5,
 4.75,
 5.0,
 5.25,
 5.5,
 5.75,
 6.0,
 6.25,
 6.5,
 6.75,
 7.0,
 7.25,
 7.5,
 7.75,
 8.0,
 8.25,
 8.5,
 8.75,
 9.0,
 9.25,
 9.5,
 9.75,
 10.0,
 10.25,
 10.5,
 10.75,
 11.0,
 11.25,
 11.5,
 11.75,
 12.0
};

// update anc filter by calcualting filter by this function
static float ANC_SR = 50781.25*8;
static void peak_filter(float gain, float fn, float Q, float *coefs)
{
    float A = (float)sqrt(pow(10, gain/20));
    float w0 = 2* PI * fn / ANC_SR; 
    float alpha = (float)sin(w0)/(2*Q); 

    float a0 = 1 + alpha/A; 
    float a1 = -2*(float)cos(w0); 
    float a2 = 1 - alpha/A;
    float b0 = 1 + alpha*A; 
    float b1 = -2*(float)cos(w0); 
    float b2 = 1 - alpha*A; 

    coefs[0] = 1;
    coefs[1] = a1 / a0;
    coefs[2] = a2 / a0;
    coefs[3] = b0 / a0;
    coefs[4] = b1 / a0;
    coefs[5] = b2 / a0;
}

// update anc filter, change it according to the filter settings
static void update_paramerter(struct_anc_cfg * filter, float gain, float fn,float Q){
    float coef[6]= {0};
    peak_filter(gain,fn,Q,coef);
    for (int i = 0; i < 3; i++)
    {
        filter->anc_cfg_ff_l.iir_coef[4].coef_a[i] = (int)((double)coef[i]*(double)134217728.0);
        filter->anc_cfg_ff_l.iir_coef[4].coef_b[i] = (int)((double)coef[i+3]*(double)134217728.0);
    }
}



AncSelectModeState * anc_select_mode_st = NULL;
SELECT_MODE_CFG_T anc_select_mode_cfg = {
		.sample_rate = 16000, // sample rate, same as anc_assist_module
		.frame_size = 120, // frame size, same as anc_assist_modult

        .process_debug = 1,  // show debug log for process
        .snr_debug = 0,   // not show debug lof for snr
        .snr_on = 0,  // set snr function not affect procedure

        .wait_period = 15, // wait period for change anc, here is about 30*7.5 ms
        .test_period = 80, // time for whole test, total time is 80*7.5ms

        .gain_start_point = 4,  // the start gain index, use pnc_gain_list[4] as start point
        .gain_shift_upper_db = 45, // the upper bound for gain adapt, pnc_gain_list[45]
        .gain_shift_lower_db = 0, // the lower bound for gain adapt, pnc_gain_list[0]
        .gain_shift_db = 4,  // not use this time
        .gain_fb_power_thd = 0, // the fb energy lower bound for gain adapt, the lower fb energy result to less accurate result
        .gain_ff_power_thd = 45, // the fb energy upper bound for gain adapt, the lower fb energy result to less accurate result
        .previous_denoise_value = -25, // not use this time


        .mode_start_point = 0,  // the start gain place, use pnc_mode_list[0] as start point
        .mode_shift_upper_db = 49, // the upper bound for gain adapt, pnc_mode_list[49]
        .mode_shift_lower_db = 0, // the lower bound for gain adapt, pnc_mode_list[0]
        .mode_shift_db = 4,  // not use this time

        .mode_fb_power_thd = 0, // the fb energy lower bound for mode adapt
        .mode_ff_power_thd = 45, // the fb energy upper bound for mode adapt

        .mode_freq_upper = 1000, 
        .mode_freq_lower = 100,

        .gain_freq_upper = 600,
        .gain_freq_lower = 100,

        .min_db_diff = 100, // the min diff for adaption, avoid the mistaken from in-ear detection
};

static int32_t _voice_assist_oneshot_adapt_anc_callback(void *buf, uint32_t len, void *other);

int32_t app_voice_assist_oneshot_adapt_anc_init(void)
{
    
    app_anc_assist_register(ANC_ASSIST_USER_ONESHOT_ADAPT_ANC, _voice_assist_oneshot_adapt_anc_callback);
    return 0;
}

int32_t app_voice_assist_oneshot_adapt_anc_open(bool gain_on, bool mode_on)
{
    TRACE(0, "[%s] oneshot adapt anc start stream", __func__);
  
    app_sysfreq_req(APP_SYSFREQ_USER_APP_0, APP_SYSFREQ_104M);

    app_anc_assist_open(ANC_ASSIST_USER_ONESHOT_ADAPT_ANC);
    local_close_cnt = 0;
    
    //create st
    anc_select_mode_st = anc_select_mode_create(&anc_select_mode_cfg);

    // set working status and init state
    anc_select_gain_set_auto_status(anc_select_mode_st,gain_on);
    anc_select_mode_set_auto_status(anc_select_mode_st,mode_on);

    //better to set when anc init, not here
    anc_set_switching_delay(ANC_SWITCHING_DELAY_50ms,ANC_FEEDFORWARD);
    anc_set_switching_delay(ANC_SWITCHING_DELAY_50ms,ANC_FEEDBACK);
    // anc_set_cfg(&AncFirCoef_50p7k_mode0,ANC_FEEDFORWARD,ANC_GAIN_NO_DELAY);
    // anc_set_cfg(&AncFirCoef_50p7k_mode0,ANC_FEEDBACK,ANC_GAIN_NO_DELAY);

    // close fb anc for adaptive anc, it is better not to open it during the init state
    anc_set_gain(0,0,ANC_FEEDBACK);
#ifdef ONESHOT_ADAPT_ANC_DUMP
    audio_dump_init(120,sizeof(int),2);
#endif
    return 0;
}

int32_t app_voice_assist_oneshot_adapt_anc_close(void)
{
    TRACE(0, "[%s] oneshot adapt anc close stream", __func__);
    anc_select_mode_destroy(anc_select_mode_st);
    app_anc_assist_close(ANC_ASSIST_USER_ONESHOT_ADAPT_ANC);
    app_sysfreq_req(APP_SYSFREQ_USER_APP_0, APP_SYSFREQ_32K);

    // open fb anc again, better use longer smoothing time, not just like this
    // anc_set_gain(512,512,ANC_FEEDBACK);
#ifdef ONESHOT_ADAPT_ANC_DUMP
    audio_dump_deinit();
#endif
    return 0;
}

// TODO: update for multi-channel feedforward/feedback microphone
static int32_t _voice_assist_oneshot_adapt_anc_callback(void * buf, uint32_t len, void *other)
{
    // deal with input values
    float ** input_data = buf;
    float * ff_data = input_data[0];
    float * fb_data = input_data[1];
    AncAssistRes * assist_res = (AncAssistRes *)other;
    int wind_status = 0; // will not pause algo
    // check working status
    if(assist_res->wind_status != WIND_STATUS_NONE
    || assist_res->noise_status == NOISE_STATUS_SILENCE
    || any_of_u32(assist_res->ff_howling_status, assist_res->ff_ch_num, HOWLING_STATUS_HOWLING)
    || any_of_u32(assist_res->fb_howling_status, assist_res->fb_ch_num, HOWLING_STATUS_HOWLING)
    || any_of_u32(assist_res->hybrid_howling_status, assist_res->fb_ch_num, HOWLING_STATUS_HOWLING)){ 
        wind_status = 1; // will pause algo
    }

#ifdef ONESHOT_ADAPT_ANC_DUMP
    audio_dump_clear_up();
    for(int i = 0; i< len; i++){
        tmp_data[i] = (int)ff_data[i];
    }
    audio_dump_add_channel_data(0,tmp_data,len);
    for(int i = 0; i< len; i++){
        tmp_data[i] = (int)fb_data[i];
    }
    audio_dump_add_channel_data(1,tmp_data,len); 
    audio_dump_run();
#endif    

    for(int i = 0; i< len; i++){
        ff_data[i] = ff_data[i]/ 256;
        fb_data[i] = fb_data[i]/ 256;
    }

    SELECT_MODE_RESULT_T res = anc_select_process(anc_select_mode_st,ff_data,fb_data,len,wind_status);

    if(res.current_mode >= 0){ 
        TRACE(2,"[adapt_anc_action] change mode %d mode index = %d",(int)(10*pnc_mode_list[res.current_mode]),res.current_mode);
        update_paramerter(&AncFirCoef_50p7k_mode0,pnc_mode_list[res.current_mode],1000,0.5); // filter params
        anc_set_cfg(&AncFirCoef_50p7k_mode0, ANC_FEEDFORWARD, ANC_GAIN_NO_DELAY);
    }

    if(res.best_mode >= 0 && res.normal_stop_flag == 1){
        TRACE(2,"[adapt_anc_action] change mode %d",res.best_mode);
        g_best_mode_index = res.best_mode;
    }
    if(res.current_gain >= 0 ){
        TRACE(2,"[adapt_anc_action]  change gain %d",pnc_gain_list[res.current_gain]);
        anc_set_gain(pnc_gain_list[res.current_gain],pnc_gain_list[res.current_gain],ANC_FEEDFORWARD);
    }
    if(res.best_gain >= 0 && res.normal_stop_flag == 1){
        TRACE(2,"[adapt_anc_action]  best gain %d",pnc_gain_list[res.best_gain]);
        g_best_gain_index = res.best_gain;

    }

    // close stream
    if(anc_select_mode_get_auto_status(anc_select_mode_st) == 0 && anc_select_gain_get_auto_status(anc_select_mode_st) == 0){
        local_close_cnt ++;   
    }
    if(local_close_cnt >= 5){
        app_voice_assist_oneshot_adapt_anc_close();
    }

    return 0;
}
