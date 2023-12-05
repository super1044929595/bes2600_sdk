#ifndef __AUDIO_PROMPT_SBC_H__
#define __AUDIO_PROMPT_SBC_H__
#include "stdint.h"
#include "cqueue.h"
#include "codec_sbc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MIX_WITH_A2DP_STREAMING  0
#define MIX_WITH_SCO_STREAMING   1

#define PENDING_TO_STOP_A2DP_STREAMING  0
#define PENDING_TO_STOP_SCO_STREAMING   1

typedef struct
{
    uint8_t    channelCnt;
    uint8_t    channelMode;
    uint8_t    bitPool;
    uint8_t    sizePerSample;
    uint8_t    sampleRate;
    uint8_t    numBlocks;
    uint8_t    numSubBands;
    uint8_t    mSbcFlag;
    uint8_t    allocMethod;
} AUDIO_PROMPT_SBC_CONFIG_T;

#ifdef TWS_PROMPT_SYNC
typedef struct {
    uint16_t promptId;
    uint16_t isMerging;
    uint32_t trigger_time;
}APP_TWS_CMD_PROMPT_SYNC_T;

typedef struct {
    uint16_t promptId;
    uint16_t promptPram;
    uint32_t sampleRate;
    uint32_t trigger_time;    
}APP_TWS_CMD_MIX_PROMPT_SYNC_T;
#endif

typedef struct
{
    uint32_t wholeEncodedDataLen;
    uint32_t leftEncodedDataLen;
    uint8_t*    promptDataBuf;
    uint16_t    tmpSourcePcmDataLen;
    uint16_t    tmpSourcePcmDataOutIndex;
    uint8_t*    tmpSourcePcmDataBuf;
    uint8_t*    tmpTargetPcmDataBuf;
    CQueue      pcmDataQueue;
    uint8_t     isResetDecoder;
    uint8_t     isAudioPromptDecodingDone;
    uint8_t     targetBytesCntPerSample;
    uint32_t    targetSampleRate;
    struct APP_RESAMPLE_T * resampler;
    uint32_t    targetPcmChunkSize;
    float       resampleRatio;
    uint8_t     pendingStopOp;
    int8_t      savedStoppedStreamId;
    uint8_t     targetChannelCnt;
    uint8_t     mixType;
    uint16_t    promptId;
    uint16_t    promptPram;
    uint8_t     isMixPromptOn;
    uint8_t     isStoppingPromptOngoing;  
} AUDIO_PROMPT_ENV_T;

extern AUDIO_PROMPT_ENV_T audio_prompt_env;

#define IS_IGNORME_SBC_FRAME_HEADER             (1)
#define AUDIO_PROMPT_SBC_FRAME_HEADER_LEN       (4)

#define AUDIO_PROMPT_SBC_CHANNEL_COUNT          (1)
#define AUDIO_PROMPT_SBC_CHANNEL_MODE           (BTIF_SBC_CHNL_MODE_MONO)

#define AUDIO_PROMPT_SOURCE_PCM_BUFFER_SIZE     (512)
#define AUDIO_PROMPT_TARGET_PCM_BUFFER_SIZE     (AUDIO_PROMPT_SOURCE_PCM_BUFFER_SIZE*2)
#define AUDIO_PROMPT_PCM_BUFFER_SIZE            (AUDIO_PROMPT_SOURCE_PCM_BUFFER_SIZE*4)

#define AUDIO_PROMPT_SBC_BLOCK_SIZE             40
#define AUDIO_PROMPT_SBC_SIZE_PER_SAMPLE        (2)// 16 bits, 1 channel


#define AUDIO_PROMPT_SBC_SAMPLE_RATE_VALUE      16000

#define AUDIO_PROMPT_SBC_FRAME_PERIOD_IN_MS     (24)

#define AUDIO_PROMPT_SBC_PCM_DATA_SIZE_PER_FRAME        256

#define AUDIO_PROMPT_PCM_FILL_UNIT_SIZE                 (AUDIO_PROMPT_SBC_PCM_DATA_SIZE_PER_FRAME*2)

#define AUDIO_PROMPT_SBC_ENCODED_DATA_SIZE_PER_FRAME    (AUDIO_PROMPT_SBC_BLOCK_SIZE*4)

#define AUDIO_PROMPT_BUF_SIZE_FOR_RESAMPLER             800

uint32_t audio_prompt_sbc_get_frame_len(void);
void audio_prompt_init_handler(void);
bool audio_prompt_is_playing_ongoing(void);
void audio_prompt_buffer_config(uint8_t mixType, uint8_t channel_cnt,
    uint8_t bitNumPerSample, uint8_t* tmpSourcePcmDataBuf, uint8_t* tmpTargetPcmDataBuf,
    uint8_t* pcmDataBuf, uint32_t pcmBufLen);
bool audio_prompt_start_playing(uint16_t promptPram, uint32_t targetSampleRate);
void audio_prompt_stop_playing(void);
void audio_prompt_processing_handler(uint32_t acquiredPcmDataLen,  
    uint8_t* pcmDataToMerge);
void audio_prompt_save_pending_stop_stream_op(uint8_t pendingStopOp, uint8_t deviceId);
void audio_prompt_forcefully_stop(void);

#ifdef TWS_PROMPT_SYNC
uint16_t audio_prompt_get_prompt_id(void);
uint32_t audio_prompt_get_sample_rate(void);
void app_tws_cmd_sync_mix_prompt_req_handler(uint8_t* ptrParam, uint16_t paramLen);
#endif

#ifdef __cplusplus
}
#endif

#endif	// __AUDIO_PROMPT_SBC_H__

