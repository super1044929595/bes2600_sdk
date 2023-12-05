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
#ifndef __APP_IBRT_IF_CUSTOM_CMD__
#define __APP_IBRT_IF_CUSTOM_CMD__

#include "app_ibrt_custom_cmd.h"

typedef enum
{
    APP_IBRT_CUSTOM_CMD_TEST1 = APP_IBRT_CMD_BASE|APP_IBRT_CUSTOM_CMD_PREFIX|0x01,
    APP_IBRT_CUSTOM_CMD_TEST2 = APP_IBRT_CMD_BASE|APP_IBRT_CUSTOM_CMD_PREFIX|0x02,

    APP_TWS_CMD_SHARE_FASTPAIR_INFO = APP_IBRT_CMD_BASE|APP_IBRT_CUSTOM_CMD_PREFIX|0x03,

#if defined(USE_LOWLATENCY_LIB)
    APP_IBRT_CUSTOM_CMD_LLY_SYNC = APP_IBRT_CMD_BASE|APP_IBRT_CUSTOM_CMD_PREFIX | 0x06,
#endif
#ifdef GFPS_ENABLED
    APP_TWS_CMD_GFPS_RING_STOP_SYNC = APP_IBRT_CMD_BASE|APP_IBRT_CUSTOM_CMD_PREFIX|0x08,
#endif
} app_ibrt_custom_cmd_code_e;

typedef struct
{
    uint8_t rsv;
    uint8_t buff[6];
} __attribute__((packed))ibrt_custom_cmd_test_t;

void app_ibrt_customif_cmd_test(ibrt_custom_cmd_test_t *cmd_test);


#endif
#if defined(USE_LOWLATENCY_LIB)
void app_ibrt_lly_cmd(uint8_t *p_buff, uint16_t length);
#endif
