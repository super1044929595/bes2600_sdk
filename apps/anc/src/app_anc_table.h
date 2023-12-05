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
#ifndef __APP_ANC_TABLE_H__
#define __APP_ANC_TABLE_H__

#include "plat_types.h"
#include "app_anc.h"

typedef struct {
    uint32_t ff;
    uint32_t fb;
    uint32_t psap;
    uint32_t spk_calib;
    uint32_t tt;
} app_anc_coef_index_cfg_t;

#define ANC_INVALID_COEF_INDEX      (0xff)

#ifdef __cplusplus
extern "C" {
#endif

int32_t app_anc_table_init(void);
int32_t app_anc_table_deinit(void);
app_anc_coef_index_cfg_t app_anc_table_get_cfg(app_anc_mode_t mode);
uint32_t app_anc_table_get_types(app_anc_mode_t mode);

#ifdef __cplusplus
}
#endif

#endif
