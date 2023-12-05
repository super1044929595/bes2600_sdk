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
#include "plat_types.h"
#include "hal_trace.h"
#include "hal_aud.h"
#include "app_anc_table.h"

#if defined(XXXX)
#undef XXXX
#endif
#define XXXX    (ANC_INVALID_COEF_INDEX)

static const app_anc_coef_index_cfg_t app_anc_coef_index_cfg[APP_ANC_MODE_QTY] = {
  /*  FF ,  FB , PSAP, SPK ,  TT */      
    {XXXX, XXXX, XXXX, XXXX, XXXX,},    // APP_ANC_MODE_OFF
    {0x00, 0x00, XXXX, XXXX, XXXX,},    // APP_ANC_MODE1
    {0x01, 0x01, XXXX, XXXX, XXXX,},    // APP_ANC_MODE1
    {0x02, 0x02, XXXX, XXXX, XXXX,},    // APP_ANC_MODE1
    {0x03, 0x03, XXXX, XXXX, XXXX,},    // APP_ANC_MODE
    {0x04, 0x04, XXXX, XXXX, XXXX,},    // APP_ANC_MODE1
    {0x05, 0x05, XXXX, XXXX, XXXX,},    // APP_ANC_MODE1
    {0x06, 0x06, XXXX, XXXX, XXXX,},    // APP_ANC_MODE1
    {0x07, 0x07, XXXX, XXXX, XXXX,},    // APP_ANC_MODE1
    {0x08, 0x08, XXXX, XXXX, XXXX,},    // APP_ANC_MODE1
    {0x09, 0x09, XXXX, XXXX, XXXX,},    // APP_ANC_MODE1
    // {XXXX, 0x00, 0x00, XXXX, XXXX,},    // APP_ANC_MODE2
    // {0x02, 0x02, XXXX, XXXX, XXXX,},    // APP_ANC_MODE3
    // {0x03, 0x03, XXXX, XXXX, XXXX,},    // APP_ANC_MODE4
    // {XXXX, 0x04, 0x00, XXXX, XXXX,},    // APP_ANC_MODE5
    // {XXXX, 0x04, 0x01, XXXX, XXXX,},    // APP_ANC_MODE6
    // {XXXX, 0x04, 0x02, XXXX, XXXX,},    // APP_ANC_MODE7
    // {XXXX, 0x04, 0x03, 0x00, XXXX,},    // APP_ANC_MODE8
};

int32_t app_anc_table_init(void)
{
    // Check whether app_anc_coef_index_cfg is not initialized completely.
    uint32_t index_cfg_num = sizeof(app_anc_coef_index_cfg_t) / sizeof(uint32_t);

    TRACE(0, "[%s] index_cfg_num: %d", __func__, index_cfg_num);

    for (uint32_t mode = 0; mode < APP_ANC_MODE_QTY; mode++) {
        uint32_t valid_index_num = 0;
        uint32_t *index_arry = (uint32_t *)&app_anc_coef_index_cfg[mode];
        for (uint32_t pos = 0; pos <index_cfg_num; pos++) {
            if (*(index_arry + pos) == 0) {
                valid_index_num++;
            }
        }

        ASSERT(valid_index_num < index_cfg_num, "[%s] Found app_anc_coef_index_cfg is not initialized completely!", __func__);
    }

    return 0;
}

int32_t app_anc_table_deinit(void)
{
    return 0;
}

app_anc_coef_index_cfg_t app_anc_table_get_cfg(app_anc_mode_t mode)
{
    ASSERT(mode < APP_ANC_MODE_QTY, "[%s] mode(%d) >= APP_ANC_MODE_QTY", __func__, mode);

    return app_anc_coef_index_cfg[mode];
}

uint32_t app_anc_table_get_types(app_anc_mode_t mode)
{
    ASSERT(mode < APP_ANC_MODE_QTY, "[%s] mode(%d) >= APP_ANC_MODE_QTY", __func__, mode);

    uint32_t anc_types = 0;
    app_anc_coef_index_cfg_t cfg = app_anc_coef_index_cfg[mode];
    
    if (cfg.ff != ANC_INVALID_COEF_INDEX) {
        anc_types |= ANC_FEEDFORWARD;
    }

    if (cfg.fb != ANC_INVALID_COEF_INDEX) {
        anc_types |= ANC_FEEDBACK;
    }

    if (cfg.tt != ANC_INVALID_COEF_INDEX) {
        anc_types |= ANC_TALKTHRU;
    }

    if (cfg.psap != ANC_INVALID_COEF_INDEX) {
        anc_types |= PSAP_FEEDFORWARD;
    }

    if (cfg.spk_calib != ANC_INVALID_COEF_INDEX) {
        anc_types |= ANC_SPKCALIB;
    }

    return anc_types;
}
