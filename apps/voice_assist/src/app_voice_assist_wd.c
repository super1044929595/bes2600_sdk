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
#if defined(ANC_ASSIST_USE_PILOT)
#include "hal_trace.h"
#include "anc_assist.h"
#include "app_anc_assist.h"
#include "app_voice_assist_wd.h"
#include "cmsis.h"
#include "app_voice_assist_ultrasound.h"

static int32_t _voice_assist_wd_callback(void *buf, uint32_t len, void *other);
bool infrasound_fadeout_flag;

static uint32_t out_ear_ticks = 0;
static uint32_t in_ear_ticks = 0;

#define MAX_WD_TICKS (133 * 3)

static wd_status_t last_wd_status[2] = {WD_STATUS_OUT_EAR, WD_STATUS_OUT_EAR};

int32_t app_voice_assist_wd_init(void)
{
    app_anc_assist_register(ANC_ASSIST_USER_WD, _voice_assist_wd_callback);

    return 0;
}

int32_t app_voice_assist_wd_open(void)
{
    app_anc_assist_open(ANC_ASSIST_USER_WD);

    out_ear_ticks = 0;
    in_ear_ticks = 0;

    for (uint32_t i = 0; i < ARRAY_SIZE(last_wd_status); i++) {
        last_wd_status[i] = WD_STATUS_OUT_EAR;
    }

    return 0;
}

int32_t app_voice_assist_wd_close(void)
{
    app_anc_assist_close(ANC_ASSIST_USER_WD);

    return 0;
}

extern bool g_gain_switch_flag;
static int32_t _voice_assist_wd_callback(void *buf, uint32_t len, void *other)
{
    // TRACE(0, "[%s] len = %d", __func__, len);

    uint32_t *res = (uint32_t *)buf;

    for (uint8_t i = 0, j = 0; i < len; i += 2, j++) {
        anc_assist_algo_status_t wd_changed = res[i];
        wd_status_t wd_status = res[i + 1];

        if (wd_changed == ANC_ASSIST_ALGO_STATUS_CHANGED) {
            if (wd_status == WD_STATUS_IN_EAR) {
                TRACE(0, "[%s] FB %d change to WD_STATUS_IN_EAR", __func__, j);
                // ultrasound_process_flag = false;
            } else {
                TRACE(0, "[%s] FB %d change to WD_STATUS_OUT_EAR", __func__, j);
                // ultrasound_process_flag = true;
            }

            out_ear_ticks = 0;
            in_ear_ticks = 0;
            last_wd_status[i] = wd_status;
        } else if (last_wd_status[i] == WD_STATUS_OUT_EAR) {
            // TRACE(2, "[%s] tick %d", __FUNCTION__, out_ear_ticks);
            if (out_ear_ticks++ == MAX_WD_TICKS) {
                g_gain_switch_flag = true;
                app_voice_assist_wd_close();
                infrasound_fadeout_flag = true;
                break;
            }
        } else if (last_wd_status[i] == WD_STATUS_IN_EAR) {
            // TRACE(2, "[%s] in_ear tick %d", __FUNCTION__, in_ear_ticks);
            if (in_ear_ticks++ == MAX_WD_TICKS) {
                g_gain_switch_flag = true;
                app_voice_assist_wd_close();
                infrasound_fadeout_flag = true;
                break;
            }
        }
    }

    return 0;
}
#endif
