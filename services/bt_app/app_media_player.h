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
#ifndef __APP_MEDIA_PLAYER_H__
#define __APP_MEDIA_PLAYER_H__

#include "resources.h"
#include "app_bt_stream.h"
#ifdef PROMPT_IN_FLASH
#define MEDIA_DEFAULT_LANGUAGE (1)
#else
#define MEDIA_DEFAULT_LANGUAGE (1 - 1)
#endif

typedef enum
{
    T_AUDIO_ID = 0x0,
    T_AUDIO_NUMBER,
}MEDIA_AUDIO_TYPE;

#define MAX_PHB_NUMBER 20

#define PROMOT_ID_BIT_MASK                      (0xff << 8)
#define PROMOT_ID_BIT_MASK_MERGING              (1 << 15)
#define PROMOT_ID_BIT_MASK_CHNLSEl_LCHNL        (1 << 14)
#define PROMOT_ID_BIT_MASK_CHNLSEl_RCHNL        (1 << 13)
#define PROMOT_ID_BIT_MASK_CHNLSEl_LOCAL        (1 << 12)
#define PROMOT_ID_BIT_MASK_CHNLSEl_ALL          (PROMOT_ID_BIT_MASK_CHNLSEl_LCHNL|PROMOT_ID_BIT_MASK_CHNLSEl_RCHNL)

#define IS_PROMPT_NEED_MERGING(promptId)          ((promptId)&PROMOT_ID_BIT_MASK_MERGING ? true : false)
#define PROMPT_ID_FROM_ID_VALUE(promptIdVal)      ((promptIdVal)&(~PROMOT_ID_BIT_MASK))
#define PROMPT_CHNLSEl_FROM_ID_VALUE(promptIdVal) ((promptIdVal)&PROMOT_ID_BIT_MASK_CHNLSEl_ALL)
#define PROMPT_PRAM_FROM_ID_VALUE(promptIdVal)    ((promptIdVal)&PROMOT_ID_BIT_MASK)
#define IS_PROMPT_CHNLSEl_ALL(promptId)           (((promptId)&PROMOT_ID_BIT_MASK_CHNLSEl_ALL) == PROMOT_ID_BIT_MASK_CHNLSEl_ALL  ? true : false)
#define IS_PROMPT_CHNLSEl_LCHNL(promptId)         (((promptId)&PROMOT_ID_BIT_MASK_CHNLSEl_ALL) == PROMOT_ID_BIT_MASK_CHNLSEl_RCHNL  ? true : false)
#define IS_PROMPT_CHNLSEl_RCHNL(promptId)         (((promptId)&PROMOT_ID_BIT_MASK_CHNLSEl_ALL) == PROMOT_ID_BIT_MASK_CHNLSEl_LCHNL  ? true : false)
#define IS_PROMPT_CHNLSEl_LOCAL(promptId)         (((promptId)&PROMOT_ID_BIT_MASK_CHNLSEl_LOCAL) == PROMOT_ID_BIT_MASK_CHNLSEl_LOCAL  ? true : false)

typedef struct 
{
    uint16_t id;
    uint16_t status;
    //>>>>>>>>>>>>>>>>>>>>>>>   APP_AUDIO_STATUS<<<<<<<<<<<<<<<
    uint16_t aud_type;
    uint16_t aud_id;
   char phb_number[MAX_PHB_NUMBER];
}msg_num_ptr;

#ifdef __cplusplus
extern "C" {
#endif
uint32_t media_PlayAudio(AUD_ID_ENUM id,uint8_t device_id);

#ifdef __cplusplus
}
#endif
void media_Set_IncomingNumber(const char* pNumber);

void media_FreeMemory(void* data);

int app_play_audio_stop(void);

int app_play_audio_onoff(bool on, APP_AUDIO_STATUS* status);

void app_play_audio_set_lang(int L);

int app_play_audio_get_lang();

void app_audio_playback_done(void);

int app_play_audio_get_aud_id(void);

void media_runtime_audio_prompt_update(uint16_t id, uint8_t** ptr, uint32_t* len);

void trigger_media_play(AUD_ID_ENUM id, uint8_t device_id, uint16_t aud_pram);
void trigger_media_stop(AUD_ID_ENUM id, uint8_t device_id);

#ifdef AUDIO_LINEIN
uint32_t app_play_audio_lineinmode_more_data(uint8_t *buf, uint32_t len);

int app_play_audio_lineinmode_init(uint8_t mode, uint32_t buff_len);

int app_play_audio_lineinmode_start(APP_AUDIO_STATUS* status);

int app_play_audio_lineinmode_stop(APP_AUDIO_STATUS* status);


#endif

#endif

