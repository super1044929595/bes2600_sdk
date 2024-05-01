#ifndef SPEECH_NS7_H
#define SPEECH_NS7_H

#include <stdint.h>

typedef struct {
    int32_t     bypass;
    float       denoise_dB;
    bool        echo_supp_enable;
    int32_t     ref_delay;
    float       gamma;
    int32_t     echo_band_start;
    int32_t     echo_band_end;
    float       min_ovrd;
    float       target_supp;
    float       ga_thr;
    float       en_thr;
} SpeechNs7Config;

struct SpeechNs7State_;

typedef struct SpeechNs7State_ SpeechNs7State;

#ifdef __cplusplus
extern "C" {
#endif

int32_t speech_ns7_process(SpeechNs7State *st, int16_t *pcm_buf, int16_t *ref_buf, uint32_t pcm_len);
void speech_ns7_destory(SpeechNs7State *st);
SpeechNs7State* speech_ns7_create(uint32_t sample_rate, uint32_t frame_size,  const SpeechNs7Config *cfg);
int32_t speech_ns7_set_config(SpeechNs7State *st, const SpeechNs7Config *cfg);

#ifdef __cplusplus
}
#endif

#endif
