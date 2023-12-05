#include "anc_assist.h"
#include "app_anc_assist.h"
#include "hal_trace.h"

typedef struct
{
    uint32_t last_status;
} noise_adapt_t;

static noise_adapt_t noise;

static int32_t _voice_assist_noise_adapt_anc_callback(void *buf, uint32_t len, void *other);

int32_t app_voice_assist_noise_adapt_anc_init(void)
{
    app_anc_assist_register(ANC_ASSIST_USER_NOISE_ADAPT_ANC, _voice_assist_noise_adapt_anc_callback);

    return 0;
}

int32_t app_voice_assist_noise_adapt_anc_open(void)
{
    app_anc_assist_open(ANC_ASSIST_USER_NOISE_ADAPT_ANC);

    noise.last_status = NOISE_STATUS_INVALID;

    return 0;
}

int32_t app_voice_assist_noise_adapt_anc_close(void)
{
    app_anc_assist_close(ANC_ASSIST_USER_NOISE_ADAPT_ANC);

    return 0;
}

static int32_t _voice_assist_noise_adapt_anc_callback(void *buf, uint32_t len, void *other)
{
    AncAssistRes *assist_res = (AncAssistRes *)other;

    if (assist_res->noise_status != noise.last_status) {
        TRACE(0, "[%s] noise status changed from %d to %d", __FUNCTION__,
            noise.last_status, assist_res->noise_status);
        noise.last_status = assist_res->noise_status;
    }

    return 0;
}
