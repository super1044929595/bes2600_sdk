#ifndef SPEECH_NS6_H
#define SPEECH_NS6_H

#include <stdint.h>

typedef struct {
    int32_t     bypass;
    float       denoise_dB;
} SpeechNs6Config;

struct SpeechNs6State_;

typedef struct SpeechNs6State_ SpeechNs6State;

#ifdef __cplusplus
extern "C" {
#endif

SpeechNs6State *speech_ns6_create(uint32_t sample_rate, uint32_t frame_size, const SpeechNs6Config *cfg);

void speech_ns6_destroy(SpeechNs6State *st);

int32_t speech_ns6_set_config(SpeechNs6State *st, const SpeechNs6Config *cfg);

int32_t speech_ns6_process(SpeechNs6State *st, int16_t *pcm_buf, uint32_t pcm_len);

#ifdef __cplusplus
}
#endif

#endif
