#ifndef ASSIST_PROMPT_H
#define ASSIST_PROMPT_H

#include "anc_assist.h"
#ifndef VQE_SIMULATE
#include "anc_process.h"
#endif

#define PROMPT_FRAME_LEN            (60)
#define DELAY_SAMPLE_MAX            (512)
#define Average                     (10)

typedef struct
{
    float fb_filter_mem0[6];
    float fb_filter_mem1[9];
    float fb_filter_mem2[9];
    float fb_filter_mem3[9];
    float cache_fb_buf[PROMPT_FRAME_LEN];

    float fb_energy_band1;
    float fb_energy_band2;
    float fb_energy_band3;
    float fb_band_sum1;
    float fb_band_sum2;
    float fb_band_sum3;
} prompt_fb_inst;

typedef struct
{
    float ref_filter_mem0[6];
    float ref_filter_mem1[9];
    float ref_filter_mem2[9];
    float ref_filter_mem3[9];

    //float delay_buf[_FRAME_LEN + DELAY_SAMPLE_MAX];
    //float cache_ref_buf[_FRAME_LEN/2];
    float delay_buf[PROMPT_FRAME_LEN + DELAY_SAMPLE_MAX];
    float cache_ref_buf[PROMPT_FRAME_LEN];
    float ref_band_sum1;
    float ref_band_sum2;
    float ref_band_sum3;
} prompt_ref_inst;

#define FILTER_NUM (2)
typedef struct
{
    float env_energy;
    float env_mem[FILTER_NUM][10];
    float max_env_energy;
    float coef[FILTER_NUM][10];
    float mem[FILTER_NUM][10];
    float env_value[FILTER_NUM];
    float final_index;
    float best_result;

} prompt_denoise_inst;

typedef struct
{
    float band1_calib;
    float band2_calib;
    float band3_calib;
} prompt_calib_inst;

#define PROMPT_CURVE_NUM_MAX (20)

typedef struct
{
    /* config */
    assist_prompt_cfg_t g_prompt_cfg;

    /* fb inst */
    prompt_fb_inst fb_inst;

    prompt_ref_inst ref_inst;

    //uint32_t cal_cnt;
    prompt_denoise_inst denoise_inst;
    
    prompt_calib_inst calib_inst;
    //float g_input_fb_buf[_FRAME_LEN];
    //float g_input_ref_buf[_FRAME_LEN];
    //float g_input_ff_buf[_FRAME_LEN];
    uint32_t cal_cnt;
    int32_t test_cnt;

    float g_input_fb_buf[PROMPT_FRAME_LEN];
    float g_input_ref_buf[PROMPT_FRAME_LEN];
    float g_input_ff_buf[PROMPT_FRAME_LEN];
    float fb_real_energy;
    float ff_real_energy;
    int32_t ff_fb_balance_flag;
    float ff_fb_balance_ind;
    int balance_cnt;
    int test_filter_gain_cnt;
    float best_filter_ratio;
    float best_filter_gain;
    float filter_coef[6];
    float filter_mem[6];
    float lowpass_mem2[6];
    float lowpass_mem1[6];
    // float lowpass_filter[6];
    float candidate_gain[6];
    int calib_process;

    int pilot_playback_cnt;
    int acc_flag;
    int32_t g_sample_rate;
    int32_t working_status;
    float distance[PROMPT_CURVE_NUM_MAX];
    float g_band1;
    float g_band2;
    float g_band3;
#ifndef VQE_SIMULATE
    int32_t mode_index;
#endif
} assist_prompt_inst;



int32_t assist_prompt_reset(assist_prompt_inst *inst, uint32_t sample_rate, uint32_t frame_len, const AncAssistConfig *cfg);
int32_t assist_prompt_process(assist_prompt_inst *inst, AncAssistState *st, float *fb_mic, float *ref,float *ff, uint32_t pcm_len);

int32_t assist_prompt_set_calib_param(assist_prompt_inst *inst,float band1_calib, float band2_calib, float band3_calib);
#ifndef VQE_SIMULATE
int32_t assist_prompt_get_anc_index(assist_prompt_inst *inst,int * mode_index, float *band1, float *band2, float *band3);
#endif

#endif