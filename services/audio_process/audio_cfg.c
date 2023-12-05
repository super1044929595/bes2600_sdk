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
#include "audio_cfg.h"
#include "string.h"
#include "hal_trace.h"
#include "iir_process.h"
#include "drc.h"
#include "limiter.h"
#include "aud_section.h"
#include "speech_eq.h"
#include "tgt_hardware.h"
#include "bt_sco_chain_cfg.h"
#include "crc32.h"

/****************************************************************
sizeof struct:
AUDIO_SECTION_AUDIO_CFG_T:6408
AUDIO_CFG_FLASH_T: 3440
DrcConfig:92
LimiterConfig:28
EqConfig:492
HFP_CFG_T:2952
IIR_CFG_T:332
****************************************************************/

struct AUDIO_CFG_T_{
    IIR_CFG_T       iir_eq;       // Now just support one type.
    DrcConfig       drc;
    LimiterConfig   limiter;
};

typedef struct {
    IIR_CFG_T       iir_eq[TOOL_SUPPORT_MAX_IIR_EQ_BAND_NUM];       // Now just support one type.
    DrcConfig       drc;
    LimiterConfig   limiter;
}AUDIO_CFG_FLASH_T;

typedef struct {
    EqConfig EqConfigList[SCO_CODEC_NUM][SCO_MODE_NUM];
}HFP_CFG_T;

typedef struct {
    section_head_t  head;
    AUDIO_CFG_FLASH_T audio_cfg;
    HFP_CFG_T       hfp_cfg;
}AUDIO_SECTION_AUDIO_CFG_T;

#if defined(__HW_IIR_EQ_PROCESS__)
extern const IIR_CFG_T * const POSSIBLY_UNUSED audio_eq_hw_iir_cfg_list[];
#endif
#if defined(__HW_DAC_IIR_EQ_PROCESS__)
extern const IIR_CFG_T * const POSSIBLY_UNUSED audio_eq_hw_dac_iir_cfg_list[];
#endif
#if defined(__SW_IIR_EQ_PROCESS__)
extern const IIR_CFG_T * const audio_eq_sw_iir_cfg_list[];
#endif

#ifdef __AUDIO_DRC__
extern const DrcConfig audio_drc_cfg;
#endif

#ifdef __AUDIO_LIMITER__
extern const LimiterConfig audio_limiter_cfg;
#endif

#if defined(SPEECH_RX_EQ)
extern const SpeechConfig speech_cfg_default;
#endif

static AUDIO_SECTION_AUDIO_CFG_T audio_section_audio_cfg;




int sizeof_audio_cfg(void)
{
    return sizeof(AUDIO_CFG_T);
}

int sizeof_audio_section(void)
{
    return sizeof(AUDIO_SECTION_AUDIO_CFG_T);
}

//function for audio tools and usb tunning tools, just for compatibility
int store_audio_cfg_into_audio_section(AUDIO_CFG_T *cfg)
{
    int res = 0;

    memset((uint8_t*)&audio_section_audio_cfg, 0x0, sizeof(AUDIO_SECTION_AUDIO_CFG_T));
    memcpy(&audio_section_audio_cfg.audio_cfg.iir_eq[0], cfg, sizeof(IIR_CFG_T));
    memcpy(&audio_section_audio_cfg.audio_cfg.drc, (uint8_t*)&cfg->drc, sizeof(DrcConfig) + sizeof(HFP_CFG_T));

    res = audio_section_store_cfg(AUDIO_SECTION_DEVICE_AUDIO,
                                (uint8_t *)&audio_section_audio_cfg,
                                sizeof(AUDIO_SECTION_AUDIO_CFG_T));
    if(res)
    {
        TRACE(2,"[%s] ERROR: res = %d", __func__, res);
    }
    else
    {
        TRACE(1,"[%s] Store audio cfg into audio section!!!", __func__); 
    }
    return res;
}

void *load_audio_cfg_from_audio_section(enum AUDIO_PROCESS_TYPE_T type, uint8_t index)
{
    void *res_ptr = NULL;
    int res = 0;

    ASSERT(type < AUDIO_PROCESS_TYPE_NUM, "[%s] type(%d) is invalid", __func__, type);
    if((type == AUDIO_PROCESS_TYPE_IIR_EQ) && (index >= TOOL_SUPPORT_MAX_IIR_EQ_BAND_NUM))
    {
        TRACE(3,"[%s] ERROR: type %d has not index %d", __func__, type, index);
    }
    if((type == AUDIO_PROCESS_TYPE_HFP_CFG) && (index >= SCO_CODEC_NUM))
    {
        TRACE(3,"[%s] ERROR: type %d has not index %d", __func__, type, index);
    }

    res = audio_section_load_cfg(AUDIO_SECTION_DEVICE_AUDIO,
                                (uint8_t *)&audio_section_audio_cfg,
                                sizeof(AUDIO_SECTION_AUDIO_CFG_T));   
    if(res)
    {
        TRACE(2,"[%s] ERROR: res = %d", __func__, res);
        res_ptr = NULL;          
    }
    else 
    {
        if (type == AUDIO_PROCESS_TYPE_IIR_EQ)
        {
            TRACE(1,"[%s] Load iir_eq from audio section!!!", __func__);
            res_ptr = (void *)&audio_section_audio_cfg.audio_cfg.iir_eq[index];
        }
        else if (type == AUDIO_PROCESS_TYPE_DRC)
        {
            TRACE(1,"[%s] Load drc from audio section!!!", __func__);
            res_ptr = (void *)&audio_section_audio_cfg.audio_cfg.drc;
        }
        else if (type == AUDIO_PROCESS_TYPE_LIMITER)
        {
            TRACE(1,"[%s] Load limiter from audio section!!!", __func__);
            res_ptr = (void *)&audio_section_audio_cfg.audio_cfg.limiter;
        }
        else if(type == AUDIO_PROCESS_TYPE_HFP_CFG)
        {
            TRACE(1,"[%s] Load hfp from audio section!!!", __func__);
            res_ptr = (void *)&audio_section_audio_cfg.hfp_cfg.EqConfigList[index];
        }
        else 
        {
            TRACE(2,"[%s] ERROR: Invalid type(%d)", __func__, type);
            res_ptr = NULL;
        }
    }
    return res_ptr;
}

void audio_cfg_get_eq_section_info(uint32_t *startAddr, uint32_t *length, uint16_t *version)
{
    *startAddr = audio_section_get_device_flash_addr(AUDIO_SECTION_DEVICE_AUDIO);
    *length = audio_section_get_device_size(AUDIO_SECTION_DEVICE_AUDIO);
    *version = (uint16_t)aud_section_struct_version;
}

void *audio_cfg_get_default_audio_section(void)
{
    POSSIBLY_UNUSED uint8_t default_eq_num = 0;
    memset((uint8_t*)&audio_section_audio_cfg, 0x0, sizeof(AUDIO_SECTION_AUDIO_CFG_T));

#if defined(__HW_DAC_IIR_EQ_PROCESS__)
    TRACE(1, "%s, HW_DAC_IIR_EQ", __func__);
    default_eq_num = EQ_HW_DAC_IIR_LIST_NUM > 10 ? 10 : EQ_HW_DAC_IIR_LIST_NUM;
    for(uint8_t i = 0; i < default_eq_num; i++)
    {
        memcpy((uint8_t*)&audio_section_audio_cfg.audio_cfg.iir_eq[i], (uint8_t*)audio_eq_hw_dac_iir_cfg_list[i], sizeof(IIR_CFG_T));
    }
#endif

#if defined(__HW_IIR_EQ_PROCESS__)
    TRACE(1, "%s, HW_IIR_EQ", __func__);
    default_eq_num = EQ_HW_IIR_LIST_NUM > 10 ? 10 : EQ_HW_IIR_LIST_NUM;
    for(uint8_t i = 0; i < default_eq_num; i++)
    {
        memcpy((uint8_t*)&audio_section_audio_cfg.audio_cfg.iir_eq[i], (uint8_t*)&audio_eq_hw_iir_cfg_list[i], sizeof(IIR_CFG_T));
    }
#endif

#if defined(__SW_IIR_EQ_PROCESS__)
    TRACE(1, "%s, SW_IIR_EQ", __func__);
    default_eq_num = EQ_SW_IIR_LIST_NUM > 10 ? 10 : EQ_SW_IIR_LIST_NUM;
    for(uint8_t i = 0; i < default_eq_num; i++)
    {
        memcpy((uint8_t*)&audio_section_audio_cfg.audio_cfg.iir_eq[i], (uint8_t*)&audio_eq_sw_iir_cfg_list[i], sizeof(IIR_CFG_T));
    }
#endif

    TRACE(3,"iir_eq[0],gain0=.2%f,gain1=.2%f,num=%d",(double)audio_section_audio_cfg.audio_cfg.iir_eq[0].gain0,
            (double)audio_section_audio_cfg.audio_cfg.iir_eq[0].gain1, audio_section_audio_cfg.audio_cfg.iir_eq[0].num);
    
#ifdef __AUDIO_DRC__
    memcpy((uint8_t*)&audio_section_audio_cfg.audio_cfg.drc, (uint8_t*)&audio_drc_cfg, sizeof(DrcConfig));
#endif
#ifdef __AUDIO_LIMITER__
    memcpy((uint8_t*)&audio_section_audio_cfg.audio_cfg.limiter, (uint8_t*)&audio_limiter_cfg, sizeof(LimiterConfig));
#endif

    // TRACE(3, "drc,knee=%d, band_num=%d, look_ahead_time=%d", audio_section_audio_cfg.audio_cfg.drc.knee,
    //             audio_section_audio_cfg.audio_cfg.drc.band_num,audio_section_audio_cfg.audio_cfg.drc.look_ahead_time);
    // TRACE(3, "limiter,knee=%d, threshold=%d, release_time=%d", audio_section_audio_cfg.audio_cfg.limiter.knee,
    //         audio_section_audio_cfg.audio_cfg.limiter.threshold, audio_section_audio_cfg.audio_cfg.limiter.release_time);


#if defined(SPEECH_RX_EQ)
    memcpy((uint8_t*)&audio_section_audio_cfg.hfp_cfg.EqConfigList[0][0], (uint8_t*)&speech_cfg_default.rx_eq, sizeof(EqConfig));
#endif

    TRACE(2, "Eqconfig, bypass=%d,num=%d",audio_section_audio_cfg.hfp_cfg.EqConfigList[0][0].bypass,
            audio_section_audio_cfg.hfp_cfg.EqConfigList[0][0].num);

    audio_section_audio_cfg.head.magic = aud_section_magic;
    audio_section_audio_cfg.head.version = aud_section_struct_version;
    audio_section_audio_cfg.head.crc = crc32(0,(uint8_t*)&audio_section_audio_cfg.audio_cfg, 
                                sizeof(AUDIO_SECTION_AUDIO_CFG_T) - sizeof(section_head_t));
    return (void*)&audio_section_audio_cfg;
}
