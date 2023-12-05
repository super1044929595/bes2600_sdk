
#ifndef __SPEECH_NS5_H__
#define __SPEECH_NS5_H__

#ifdef __cplusplus
extern "C" {
#endif

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
} SpeechNs5Config;

struct SpeechNs5State_;

typedef struct SpeechNs5State_ SpeechNs5State;

typedef struct DenoiseState DenoiseState;


SpeechNs5State *speech_ns5_create(int32_t sample_rate, int32_t frame_size, void *model, const SpeechNs5Config *cfg);
int32_t speech_ns5_destroy(SpeechNs5State *st);
int32_t speech_ns5_set_config(SpeechNs5State *st, const SpeechNs5Config *cfg);
int32_t speech_ns5_process(SpeechNs5State *st, int16_t *pcm_buf, int16_t *ref_buf, int32_t pcm_len);

void *speech_get_ns5_small_model(void);
void *speech_get_ns5_large_model(void);

#ifdef __cplusplus
}
#endif

#endif
