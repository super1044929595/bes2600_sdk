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
#include "hal_trace.h"
#include "anc_assist.h"
#include "app_anc_assist.h"
#include "app_voice_assist_prompt_leak_detect.h"
#include "arm_math.h"
#include "speech_memory.h"
// #include "anc_process.h"
#include "assist_prompt_signal.h"
#include "app_anc.h"

static int32_t _voice_assist_prompt_leak_detect_callback(void *buf, uint32_t len, void *other);

static int is_running= 0;
static int cfg_set_flag = 0;
static int output_mode_num = 0;
int32_t app_voice_assist_prompt_leak_detect_init(void)
{
    is_running = 0;
    app_anc_assist_register(ANC_ASSIST_USER_PROMPT_LEAK_DETECT, _voice_assist_prompt_leak_detect_callback);

    return 0;
}

int32_t app_voice_assist_prompt_leak_detect_set_working_status(int32_t status)
{
    assist_prompt_set_working_status(status);
    return 0;
}

int32_t app_voice_assist_prompt_leak_detect_open(void)
{
    is_running = 1;
    output_mode_num = 0;
    TRACE(0,"[%s] @@@@@@@ open",__func__);
    TRACE(0,"@@@@@@@ before %d",output_mode_num);
    cfg_set_flag = 0;
      
    app_anc_assist_open(ANC_ASSIST_USER_PROMPT_LEAK_DETECT);
    return 0;
}

int32_t app_voice_assist_prompt_leak_detect_close(void)
{
    if(is_running == 1) {
        is_running = 0;
        TRACE(0,"[%s] @@@@@@@ close",__func__);
        
        app_anc_assist_get_prompt_anc_index(&output_mode_num,0);
        TRACE(0,"@@@@@@@ mode_index %d",output_mode_num);
        app_anc_switch(output_mode_num +1);
        app_anc_assist_close(ANC_ASSIST_USER_PROMPT_LEAK_DETECT);
    }
    return 0;
}

static int32_t _voice_assist_prompt_leak_detect_callback(void *buf, uint32_t len, void *other)
{
    
    return 0;
}

