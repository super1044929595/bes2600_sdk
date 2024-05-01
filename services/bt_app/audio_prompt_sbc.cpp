#include "audio_prompt_sbc.h"
#include "hal_trace.h"
#include "string.h"
#include "app_bt_stream.h"
#include "app_bt_media_manager.h"
#include "app_utils.h"
#include "cmsis.h"
#include "app_tws_ibrt.h"
#include "app_tws_ibrt_cmd_handler.h"
#include "app_tws_ctrl_thread.h"
#include "bt_drv_interface.h"
#include "audioflinger.h"
#include "app_media_player.h"
#include "app_audio.h"




#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED

#define AUDIO_PROMPT_RESAMPLE_ITER_NUM               256


#ifdef TWS_PROMPT_SYNC
extern void tws_sync_mix_prompt_start_handling(void);
extern void tws_reset_mix_prompt_trigger_ticks(void);
extern bool tws_sync_mix_prompt_handling(void);
extern void tws_enable_mix_prompt(bool isEnable);
#endif


extern void app_stop_a2dp_media_stream(uint8_t devId);
extern void app_stop_sco_media_stream(uint8_t devId);
extern void app_bt_stream_copy_track_one_to_two_16bits(int16_t *dst_buf, int16_t *src_buf, uint32_t src_len);

static uint32_t audio_prompt_sbc_decode(uint8_t* pcm_buffer, uint32_t expectedOutputSize, uint8_t isReset);

static btif_sbc_decoder_t audio_prompt_sbc_decoder;
static uint32_t audio_prompt_sbc_frame_len = 0;

static float audio_prompt_sbc_eq_band_gain[8] = {1, 1, 1, 1, 1, 1, 1, 1};

#define COEFF_FOR_MIX_PROMPT_FOR_MUSIC    0.7 
#define COEFF_FOR_MIX_PROMPT_FOR_CALL     0.7 

static uint8_t __attribute__((aligned(4))) resampler_buf[AUDIO_PROMPT_BUF_SIZE_FOR_RESAMPLER];

static int audio_prompt_sbc_init_decoder(void)
{
    btif_sbc_init_decoder(&audio_prompt_sbc_decoder);
    return 0;
}


AUDIO_PROMPT_ENV_T audio_prompt_env;

void audio_prompt_init_handler(void)
{
    memset((uint8_t *)&audio_prompt_env, 0, sizeof(audio_prompt_env));
    audio_prompt_env.savedStoppedStreamId = -1;
}

bool audio_prompt_is_playing_ongoing(void)
{
#if 0
    return (audio_prompt_env.leftEncodedDataLen > 0);
#else
    bool isPlayingOnGoing = false;
    uint32_t lock = int_lock_global();
    if (audio_prompt_env.isMixPromptOn)
    {
        isPlayingOnGoing = true;
    }
    int_unlock_global(lock);
    return isPlayingOnGoing;

#endif
}


#ifdef TWS_PROMPT_SYNC
#define MEDIA_NONE_TRIGGER_DELAY        0x5
#define MEDIA_SHORT_TRIGGER_DELAY       0x500
#define MEDIA_LONG_TRIGGER_DELAY        0x800

#define PROMPT_TICKS_OFFSET_TO_TRIGGER_MIX  10  // 3.25 ms

static uint32_t mix_prompt_trigger_ticks = 0;
static uint32_t playback_interval_in_ticks = 0;
static uint32_t playback_last_irq_ticks = 0;
static uint8_t isStartMixPrompt = false;

static uint32_t get_prompt_trigger_delay(uint16_t promptPram)
{
    if(IS_PROMPT_CHNLSEl_LOCAL(promptPram))
        return MEDIA_NONE_TRIGGER_DELAY; 
    else
        return MEDIA_LONG_TRIGGER_DELAY; 
}

bool tws_calculate_mix_prompt_trigger_ticks(uint16_t promptPram)
{
    if (0 == mix_prompt_trigger_ticks)
    {
        if ((playback_interval_in_ticks > 0)&&(playback_last_irq_ticks > 0))
        {
            mix_prompt_trigger_ticks = playback_last_irq_ticks+
                (get_prompt_trigger_delay(promptPram)/playback_interval_in_ticks)*playback_interval_in_ticks;
            TRACE(1,"playback_last_irq_ticks %d",playback_last_irq_ticks);
            TRACE(2,"playback_interval_in_ticks %d mix_prompt_trigger_ticks %d",playback_interval_in_ticks, mix_prompt_trigger_ticks);
            return true;
        }
    }

    return false;
}

void tws_enable_mix_prompt(bool isEnable)
{
    TRACE(1,"isStartMixPrompt to %d.", isEnable);
    isStartMixPrompt = isEnable;
}

void tws_set_mix_prompt_trigger_ticks(uint32_t ticks)
{
    mix_prompt_trigger_ticks = ticks;
}

void tws_sync_mix_prompt_start_handling(void)
{
    if (!app_tws_ibrt_tws_link_connected())
    {
        tws_enable_mix_prompt(true);
    }
    else
    {
        tws_enable_mix_prompt(false);
    }
}

bool tws_is_mix_prompt_allowed_to_start(void)
{
    return isStartMixPrompt;
}


void app_ibrt_send_mix_prompt_req()
{
    if(app_tws_ibrt_tws_link_connected())
    {
        APP_TWS_CMD_MIX_PROMPT_SYNC_T req;
        req.promptId = audio_prompt_get_prompt_id();
        req.promptPram = audio_prompt_env.promptPram;
        req.trigger_time = mix_prompt_trigger_ticks;
        req.sampleRate = audio_prompt_get_sample_rate();
        tws_ctrl_send_cmd(APP_TWS_CMD_SYNC_MIX_PROMPT_REQ,(uint8_t*)&req,sizeof(APP_TWS_CMD_MIX_PROMPT_SYNC_T));
    }
}

bool tws_sync_mix_prompt_handling(void)
{
    if (!tws_is_mix_prompt_allowed_to_start())
    {
        if (tws_calculate_mix_prompt_trigger_ticks(audio_prompt_env.promptPram))
        {
            // get the trigger ticks, send request to slave
            if(!IS_PROMPT_CHNLSEl_LOCAL(audio_prompt_env.promptPram))
                app_ibrt_send_mix_prompt_req();
        }
        return false;
    }
    else
    {
        return true;
    }
    
}

uint32_t tws_get_mix_prompt_trigger_ticks(void)
{
    return mix_prompt_trigger_ticks;
}

void tws_reset_mix_prompt_trigger_ticks(void)
{
    mix_prompt_trigger_ticks = 0;
    playback_interval_in_ticks = 0;
    playback_last_irq_ticks = 0;
    isStartMixPrompt = false;
}

extern "C" void tws_playback_ticks_check_for_mix_prompt(void)
{
    if ((!audio_prompt_is_playing_ongoing()) || tws_is_mix_prompt_allowed_to_start())
    {
        //TRACE(0,"check for mix prompt<1>");
        return;
    }

    uint32_t curr_ticks = 0;
    uint16_t conhandle = INVALID_HANDLE;
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();

    uint32_t btclk;
    uint16_t btcnt;
    int32_t offset;

    if (app_tws_ibrt_mobile_link_connected()){
        conhandle = p_ibrt_ctrl->mobile_conhandle;
    }else if (app_tws_ibrt_slave_ibrt_link_connected()){
        conhandle = p_ibrt_ctrl->ibrt_conhandle;
    }else{
        TRACE(0,"check for mix prompt<2>");
        return;
    }

    bt_drv_reg_op_dma_tc_clkcnt_get(&btclk, &btcnt);
    if (btdrv_is_link_index_valid(btdrv_conhdl_to_linkid(conhandle))){
        offset = 2 * bt_drv_reg_op_get_clkoffset(btdrv_conhdl_to_linkid(conhandle));
    }else{
        offset = 0;
    }
    //in BT clock(312.5us)
    curr_ticks = (2*btclk + offset) & 0x0fffffff;

//    TRACE(4,"irq curr ticks:%d/%d %d/%d",curr_ticks, bt_syn_get_curr_ticks(conhandle),
//                                 btclk, btdrv_syn_get_curr_ticks());

    if (mix_prompt_trigger_ticks > 0)
    {
        if (curr_ticks >= mix_prompt_trigger_ticks)
        {
            TRACE(2,"ticks<1> %d - trigger ticks %d", curr_ticks,mix_prompt_trigger_ticks);
            tws_enable_mix_prompt(true);
        }
        else if ((curr_ticks < mix_prompt_trigger_ticks) && 
                ((mix_prompt_trigger_ticks - curr_ticks) < PROMPT_TICKS_OFFSET_TO_TRIGGER_MIX))
        {
            TRACE(2,"ticks<2> %d - trigger ticks %d", curr_ticks,mix_prompt_trigger_ticks);
            tws_enable_mix_prompt(true);
        }
    }
    
    if (0 != playback_last_irq_ticks)
    {
       playback_interval_in_ticks = curr_ticks-playback_last_irq_ticks;
       //playback_interval_in_ticks = 75;
    }

    playback_last_irq_ticks = curr_ticks;
}
void app_tws_cmd_sync_mix_prompt_req_handler(uint8_t* ptrParam, uint16_t paramLen)
{
    uint16_t promptPram;

    TRACE(1,"%s", __func__);
    // app_ibrt_ui_t *p_ibrt_ui = app_ibrt_ui_get_ctx();
    // app_tws_info_t local_tws_info;
    // ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    // memcpy((uint8_t*)&p_ibrt_ui->peer_tws_info, p_buff, length);//update peer tws info

    APP_TWS_CMD_MIX_PROMPT_SYNC_T prompt_info;
    memcpy((uint8_t*)&prompt_info,ptrParam,paramLen);
    //TRACE(2,"pReq->promptId,pReq->trigger_time:0x%x,%d", pReq->promptId, pReq->trigger_time);
    tws_set_mix_prompt_trigger_ticks(prompt_info.trigger_time);
    promptPram = prompt_info.promptId | prompt_info.promptPram;
    audio_prompt_start_playing(promptPram, prompt_info.sampleRate);
}
#endif



void audio_prompt_save_pending_stop_stream_op(uint8_t pendingStopOp, uint8_t deviceId)
{
    audio_prompt_env.pendingStopOp = pendingStopOp;
    audio_prompt_env.savedStoppedStreamId = deviceId;
}

static int audio_prompt_resample_iter(uint8_t *buf, uint32_t len)
{
    uint32_t leftLen = audio_prompt_env.tmpSourcePcmDataLen - audio_prompt_env.tmpSourcePcmDataOutIndex;
    uint32_t lenToFetch;
    
    if (leftLen >= len)
    {
        lenToFetch = len;
    }
    else
    {
        lenToFetch = leftLen;
    }
    
    memcpy(buf, audio_prompt_env.tmpSourcePcmDataBuf + audio_prompt_env.tmpSourcePcmDataOutIndex,
        lenToFetch);
    audio_prompt_env.tmpSourcePcmDataOutIndex += lenToFetch;

    memset(buf + lenToFetch, 0, len - lenToFetch);
    
    return 0;
}

void audio_prompt_buffer_config(uint8_t mixType, uint8_t channel_cnt,
    uint8_t bitNumPerSample, uint8_t* tmpSourcePcmDataBuf, uint8_t* tmpTargetPcmDataBuf,
    uint8_t* pcmDataBuf, uint32_t pcmBufLen)
{
    audio_prompt_env.mixType = mixType;
    audio_prompt_env.targetChannelCnt = channel_cnt;
    if (24 == bitNumPerSample)
    {
        audio_prompt_env.targetBytesCntPerSample = 4;
    }
    else if (16 == bitNumPerSample)
    {
        audio_prompt_env.targetBytesCntPerSample = 2;
    }
    else
    {
        ASSERT(false, "bitNumPerSample %d is not supported by prompt mixer yet!", bitNumPerSample);
    }
    
    audio_prompt_env.tmpSourcePcmDataBuf = tmpSourcePcmDataBuf;
    audio_prompt_env.tmpTargetPcmDataBuf = tmpTargetPcmDataBuf;

    InitCQueue(&(audio_prompt_env.pcmDataQueue), pcmBufLen,
     (CQItemType *)(pcmDataBuf));
}


uint16_t audio_prompt_get_prompt_id(void)
{
    return audio_prompt_env.promptId;
}
uint32_t audio_prompt_get_sample_rate(void)
{
    return audio_prompt_env.targetSampleRate;
}


#if 0
bool audio_prompt_start_playing(uint32_t targetSampleRate, 
    uint8_t* promptDataPtr, uint32_t promptDataLen)
{
    if (audio_prompt_is_playing_ongoing())
    {
        return false;
    }

    TRACE(1,"start audio prompt. target sample rate %d", targetSampleRate);

    audio_prompt_env.promptDataBuf = promptDataPtr;
    audio_prompt_env.isResetDecoder = true;
    audio_prompt_env.isAudioPromptDecodingDone = false;
    audio_prompt_env.wholeEncodedDataLen = promptDataLen;
    audio_prompt_env.leftEncodedDataLen = audio_prompt_env.wholeEncodedDataLen;
    audio_prompt_env.targetSampleRate = targetSampleRate;

    audio_prompt_env.resampleRatio = ((float)AUDIO_PROMPT_SBC_SAMPLE_RATE_VALUE) / audio_prompt_env.targetSampleRate;
    audio_prompt_env.targetPcmChunkSize = (uint32_t)(AUDIO_PROMPT_SBC_PCM_DATA_SIZE_PER_FRAME/audio_prompt_env.resampleRatio);

    audio_prompt_env.resampler = app_playback_resample_any_open_with_pre_allocated_buffer((enum AUD_CHANNEL_NUM_T)AUDIO_PROMPT_SBC_CHANNEL_COUNT, 
        audio_prompt_resample_iter, AUDIO_PROMPT_RESAMPLE_ITER_NUM,
        audio_prompt_env.resampleRatio, 
        resampler_buf, AUDIO_PROMPT_BUF_SIZE_FOR_RESAMPLER);

    audio_prompt_env.savedStoppedStreamId = -1;

    app_sysfreq_req(APP_SYSFREQ_USER_PROMPT_MIXER, APP_SYSFREQ_104M);


#ifdef TWS_PROMPT_SYNC
        if (!isPlayingLocally)
        {
            tws_sync_mix_prompt_start_handling();
        }
        else
        {
            tws_enable_mix_prompt(true);
        }
#endif


    return true;
}
#else
bool audio_prompt_start_playing(uint16_t promptPram, uint32_t targetSampleRate)
{
    uint16_t promptId = PROMPT_ID_FROM_ID_VALUE(promptPram);

    if (audio_prompt_is_playing_ongoing())
    {
        return false;
    }

#ifdef TWS_PROMPT_SYNC
    bool isPlayingLocally = false;
    if(app_tws_ibrt_tws_link_connected())
    {
        isPlayingLocally = false;
    }
    else
    {
        isPlayingLocally = true;
    }
#endif


   // promptId = PROMPT_ID_FROM_ID_VALUE(promptId); 
    af_lock_thread();
    audio_prompt_env.isMixPromptOn = true;
    audio_prompt_env.isStoppingPromptOngoing = false;
    

    uint8_t* promptDataPtr;
    uint32_t promptDataLen;
    media_runtime_audio_prompt_update(promptId, &promptDataPtr, &promptDataLen);
    TRACE(3,"start audio prompt. target sample rate %d id:%d pram:%04x", targetSampleRate, promptId, PROMPT_PRAM_FROM_ID_VALUE(promptPram));


    audio_prompt_env.promptId = promptId;
    audio_prompt_env.promptPram = PROMPT_PRAM_FROM_ID_VALUE(promptPram);
    audio_prompt_env.promptDataBuf = promptDataPtr;
    audio_prompt_env.isResetDecoder = true;
    audio_prompt_env.isAudioPromptDecodingDone = false;
    audio_prompt_env.wholeEncodedDataLen = promptDataLen;
    audio_prompt_env.leftEncodedDataLen = audio_prompt_env.wholeEncodedDataLen;
    audio_prompt_env.targetSampleRate = targetSampleRate;

    audio_prompt_env.resampleRatio = ((float)AUDIO_PROMPT_SBC_SAMPLE_RATE_VALUE) / audio_prompt_env.targetSampleRate;
    audio_prompt_env.targetPcmChunkSize = (uint32_t)(AUDIO_PROMPT_SBC_PCM_DATA_SIZE_PER_FRAME/audio_prompt_env.resampleRatio);

    audio_prompt_env.resampler = app_playback_resample_any_open_with_pre_allocated_buffer((enum AUD_CHANNEL_NUM_T)AUDIO_PROMPT_SBC_CHANNEL_COUNT, 
        audio_prompt_resample_iter, AUDIO_PROMPT_RESAMPLE_ITER_NUM,
        audio_prompt_env.resampleRatio, 
        resampler_buf, AUDIO_PROMPT_BUF_SIZE_FOR_RESAMPLER);

    audio_prompt_env.savedStoppedStreamId = -1;

    af_unlock_thread();

    app_sysfreq_req(APP_SYSFREQ_USER_PROMPT_MIXER, APP_SYSFREQ_104M);

#ifdef TWS_PROMPT_SYNC
        if (!isPlayingLocally)
        {
            tws_sync_mix_prompt_start_handling();
        }
        else
        {
            tws_enable_mix_prompt(true);
        }
#endif


    return true;
}

#endif

void audio_prompt_forcefully_stop(void)
{
    app_playback_resample_close(audio_prompt_env.resampler);
    audio_prompt_env.savedStoppedStreamId = -1;
    audio_prompt_env.isAudioPromptDecodingDone  = true; 
    audio_prompt_env.leftEncodedDataLen = 0;
    app_sysfreq_req(APP_SYSFREQ_USER_PROMPT_MIXER, APP_SYSFREQ_32K);
}

void audio_prompt_stop_playing(void)
{
//    TRACE(0,"Stop audio prompt.");

#ifdef TWS_PROMPT_SYNC
    tws_reset_mix_prompt_trigger_ticks();
	audio_prompt_env.isMixPromptOn = false;
#endif
    audio_prompt_env.promptPram = PROMOT_ID_BIT_MASK_CHNLSEl_ALL;
    app_playback_resample_close(audio_prompt_env.resampler);
    if (audio_prompt_env.savedStoppedStreamId >= 0)
    {        
        if (PENDING_TO_STOP_A2DP_STREAMING == audio_prompt_env.pendingStopOp)
        {
            TRACE(0,"Stop the pending stopped a2dp media stream.");
            app_stop_a2dp_media_stream(audio_prompt_env.savedStoppedStreamId);
        }
        else if (PENDING_TO_STOP_SCO_STREAMING == audio_prompt_env.pendingStopOp)
        {
            TRACE(0,"Stop the pending stopped sco media stream.");
            app_stop_sco_media_stream(audio_prompt_env.savedStoppedStreamId);
        }
    }
    audio_prompt_env.savedStoppedStreamId = -1;
    audio_prompt_env.leftEncodedDataLen = 0;

}

static void audio_prompt_processing_handler_func(uint32_t acquiredPcmDataLen, uint8_t* pcmDataToMerge)
{
    uint32_t pcmDataToGetFromPrompt = acquiredPcmDataLen/(audio_prompt_env.targetChannelCnt*audio_prompt_env.targetBytesCntPerSample/2);
    if (audio_prompt_env.isAudioPromptDecodingDone)
    {
        return;
    }

    while ((uint32_t)LengthOfCQueue(&(audio_prompt_env.pcmDataQueue)) < pcmDataToGetFromPrompt)
    {
        if (audio_prompt_env.isAudioPromptDecodingDone)
        {
            break;
        }
        
        // decode the audio prompt
        uint32_t returnedPcmDataLen = audio_prompt_sbc_decode(audio_prompt_env.tmpSourcePcmDataBuf, 
            AUDIO_PROMPT_SBC_PCM_DATA_SIZE_PER_FRAME, 
            audio_prompt_env.isResetDecoder);
        
        if (returnedPcmDataLen < AUDIO_PROMPT_SBC_PCM_DATA_SIZE_PER_FRAME)
        {        
            audio_prompt_env.isAudioPromptDecodingDone = true;
        }
        
        audio_prompt_env.isResetDecoder = false;
        audio_prompt_env.tmpSourcePcmDataLen = returnedPcmDataLen;
        audio_prompt_env.tmpSourcePcmDataOutIndex = 0;

        // do resmpling
        if (audio_prompt_env.targetSampleRate != AUDIO_PROMPT_SBC_SAMPLE_RATE_VALUE)
        {
            uint32_t targetPcmSize = returnedPcmDataLen;
            if (AUDIO_PROMPT_SBC_PCM_DATA_SIZE_PER_FRAME == returnedPcmDataLen)
            {
                targetPcmSize = audio_prompt_env.targetPcmChunkSize;
            }
            else
            {
                targetPcmSize = (uint32_t)(returnedPcmDataLen/audio_prompt_env.resampleRatio);
            }

            targetPcmSize = (targetPcmSize/4)*4;
            
            app_playback_resample_run(audio_prompt_env.resampler, audio_prompt_env.tmpTargetPcmDataBuf,
                targetPcmSize);

            // fill into pcm data queue
            EnCQueue(&(audio_prompt_env.pcmDataQueue), audio_prompt_env.tmpTargetPcmDataBuf, targetPcmSize);
        }
        else
        {
            EnCQueue(&(audio_prompt_env.pcmDataQueue), audio_prompt_env.tmpSourcePcmDataBuf, returnedPcmDataLen);            
        }
    }

    uint32_t pcmDataLenToMerge = pcmDataToGetFromPrompt;
    if ((uint32_t)LengthOfCQueue(&(audio_prompt_env.pcmDataQueue)) < pcmDataToGetFromPrompt)
    {
        pcmDataLenToMerge = LengthOfCQueue(&(audio_prompt_env.pcmDataQueue));
    }
    
    // copy to multiple channel if needed
    if (audio_prompt_env.targetChannelCnt > 1)
    {         
        // get the data
        DeCQueue(&(audio_prompt_env.pcmDataQueue), audio_prompt_env.tmpSourcePcmDataBuf, pcmDataLenToMerge);
        app_bt_stream_copy_track_one_to_two_16bits((int16_t *)audio_prompt_env.tmpTargetPcmDataBuf, 
            (int16_t *)audio_prompt_env.tmpSourcePcmDataBuf, pcmDataLenToMerge/sizeof(uint16_t));
    }
    else
    {
        DeCQueue(&(audio_prompt_env.pcmDataQueue), audio_prompt_env.tmpTargetPcmDataBuf, pcmDataLenToMerge);
    }

#ifdef TWS_PROMPT_SYNC
    uint16_t prompt_chnlsel = PROMPT_CHNLSEl_FROM_ID_VALUE(audio_prompt_env.promptPram);
    if (IS_PROMPT_CHNLSEl_ALL(prompt_chnlsel) || app_ibrt_voice_report_is_me(prompt_chnlsel))
#endif
    {
        // merge the data
        int16_t *src_buf0 = (int16_t *)audio_prompt_env.tmpTargetPcmDataBuf;
        uint32_t src_len = pcmDataLenToMerge*audio_prompt_env.targetChannelCnt/sizeof(uint16_t);

        if (2 == audio_prompt_env.targetBytesCntPerSample)
        {
            int16_t *src_buf1 = (int16_t *)pcmDataToMerge;
            int16_t *dst_buf = (int16_t *)pcmDataToMerge;
            if (MIX_WITH_A2DP_STREAMING == audio_prompt_env.mixType)
            {
                for (uint32_t i = 0; i < src_len; i++) {
                    dst_buf[i] = (int16_t)((float)src_buf0[i]*COEFF_FOR_MIX_PROMPT_FOR_MUSIC) + src_buf1[i] -
                        (int16_t)((float)src_buf1[i]*COEFF_FOR_MIX_PROMPT_FOR_MUSIC);
                }    
            }
            else
            {
                for (uint32_t i = 0; i < src_len; i++) {
                    dst_buf[i] = (int16_t)((float)src_buf0[i]*COEFF_FOR_MIX_PROMPT_FOR_CALL) + src_buf1[i] -
                        (int16_t)((float)src_buf1[i]*COEFF_FOR_MIX_PROMPT_FOR_CALL);
                }  
            }
        }
        else if (4 == audio_prompt_env.targetBytesCntPerSample)
        {
            int32_t *src_buf1 = (int32_t *)pcmDataToMerge;
            int32_t *dst_buf = (int32_t *)pcmDataToMerge;

            if (MIX_WITH_A2DP_STREAMING == audio_prompt_env.mixType)
            {
                for (uint32_t i = 0; i < src_len; i++) {
                    dst_buf[i] = (int32_t)((float)((int32_t)src_buf0[i] << 8)*COEFF_FOR_MIX_PROMPT_FOR_MUSIC) + src_buf1[i] -
                        (int32_t)((float)src_buf1[i]*COEFF_FOR_MIX_PROMPT_FOR_MUSIC);
                }    
            }
            else
            {
                for (uint32_t i = 0; i < src_len; i++) {
                    dst_buf[i] = (int32_t)((float)((int32_t)src_buf0[i] << 8)*COEFF_FOR_MIX_PROMPT_FOR_CALL) + src_buf1[i] -
                        (int32_t)((float)src_buf1[i]*COEFF_FOR_MIX_PROMPT_FOR_CALL);
                }  
            }        
        }
    }
    if (audio_prompt_env.isAudioPromptDecodingDone)
    {
        // prompt playing is completed
        audio_prompt_stop_playing();        
        app_sysfreq_req(APP_SYSFREQ_USER_PROMPT_MIXER, APP_SYSFREQ_32K);
    }
    
}

void audio_prompt_processing_handler(uint32_t acquiredPcmDataLen, uint8_t* pcmDataToMerge)
{
    uint32_t gotDataLen = 0;

#ifdef TWS_PROMPT_SYNC
        if (!tws_sync_mix_prompt_handling())
        {
            return;
        }
#endif

    
    while (gotDataLen < acquiredPcmDataLen)
    {
        uint32_t lenToGet;
        if ((acquiredPcmDataLen - gotDataLen) > AUDIO_PROMPT_PCM_FILL_UNIT_SIZE)
        {
            lenToGet = AUDIO_PROMPT_PCM_FILL_UNIT_SIZE;
        }
        else
        {   lenToGet = acquiredPcmDataLen - gotDataLen;  
        }
        
        audio_prompt_processing_handler_func(lenToGet, pcmDataToMerge+gotDataLen);
        gotDataLen += lenToGet;
    }
}

uint32_t audio_prompt_sbc_get_frame_len(void)
{
    return audio_prompt_sbc_frame_len;
}

static uint32_t audio_prompt_sbc_decode(uint8_t* pcm_buffer, uint32_t expectedOutputSize, uint8_t isReset)
{
    if (isReset)
    {
        audio_prompt_sbc_init_decoder();
    }

    uint32_t sbcDataBytesToDecode;
    unsigned int pcm_offset = 0;
    uint16_t byte_decode;
    int8_t ret;
    btif_sbc_pcm_data_t audio_prompt_PcmDecData;
    
get_again:    
    audio_prompt_PcmDecData.data = pcm_buffer+pcm_offset;
    audio_prompt_PcmDecData.dataLen = 0;
    
    if (audio_prompt_env.leftEncodedDataLen > AUDIO_PROMPT_SBC_ENCODED_DATA_SIZE_PER_FRAME)
    {
        sbcDataBytesToDecode = AUDIO_PROMPT_SBC_ENCODED_DATA_SIZE_PER_FRAME;
    }
    else
    {
        sbcDataBytesToDecode = audio_prompt_env.leftEncodedDataLen;
    }
    
    ret = btif_sbc_decode_frames(&audio_prompt_sbc_decoder, 
        audio_prompt_env.promptDataBuf+audio_prompt_env.wholeEncodedDataLen-audio_prompt_env.leftEncodedDataLen, 
        sbcDataBytesToDecode, &byte_decode,
        &audio_prompt_PcmDecData, expectedOutputSize-pcm_offset, 
        audio_prompt_sbc_eq_band_gain);


    audio_prompt_env.leftEncodedDataLen -= byte_decode;
    
    pcm_offset += audio_prompt_PcmDecData.dataLen;

    if (0 == audio_prompt_env.leftEncodedDataLen)
    {
        goto exit;
    }
    
    if (expectedOutputSize == pcm_offset)
    {
        goto exit;
    }
    
    if ((ret == BT_STS_CONTINUE) || (ret == BT_STS_SUCCESS))  {
        goto get_again;
    }
    
exit:
    return pcm_offset;
}
#endif // MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
