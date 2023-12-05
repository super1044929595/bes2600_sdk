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
#ifndef __SIDETONE_PROCESS_H__
#define __SIDETONE_PROCESS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"
#include "hw_codec_iir_process.h"


#define AUD_SIDETONE_IIR_NUM_EQ                        (8)


void sidetone_open(void);
void sidetone_close(void);
int sidetone_set_cfg(HW_CODEC_IIR_CFG_T *cfg);
int sidetone_set_gain_f32(float gain_l, float gain_r);


#ifdef __cplusplus
}
#endif

#endif
