#ifndef __SPEECH_2MIC_NS7_H__
#define __SPEECH_2MIC_NS7_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    int32_t     bypass;
    int32_t     wnr_enable;
    float       wind_thd;
    int32_t     wnd_pwr_thd;
    float       wind_gamma;
    int32_t     state_trans_frame_thd;

    int32_t     af_enable;
    float       filter_gamma;
    int32_t     vad_bin_start1;
    int32_t     vad_bin_end1;
    int32_t     vad_bin_start2;
    int32_t     vad_bin_end2;
    int32_t     vad_bin_start3;
    int32_t     vad_bin_end3;
    float       coef1_thd;
    float       coef2_thd;
    float       coef3_thd;
    int32_t     supp_times;
    int32_t     post_gain;

    int32_t     calib_enable;
    int32_t     calib_delay;
    float*      filter;
    int32_t     filter_len;

    int32_t     low_ram_enable;
    int32_t     low_mips_enable;

    int32_t     echo_supp_enable;
    int32_t     ref_delay;

    int32_t     post_supp_enable;
    float       denoise_db;
} Speech2MicNs7Config;
struct Speech2MicNs7State_;
typedef struct Speech2MicNs7State_ Speech2MicNs7State;
Speech2MicNs7State *speech_2mic_ns7_create(int sample_rate, int frame_size, const Speech2MicNs7Config *cfg);
int32_t speech_2mic_ns7_destroy(Speech2MicNs7State *st);
int32_t speech_2mic_ns7_process(Speech2MicNs7State *st, short *pcm_buf, short *ref_buf, int32_t pcm_len, short *out_buf);
int speech_2mic_ns7_set_config(Speech2MicNs7State *st, const Speech2MicNs7Config *cfg);
float speech_2mic_ns7_get_required_mips(Speech2MicNs7State *st);
#ifdef __cplusplus
}
#endif

#endif
