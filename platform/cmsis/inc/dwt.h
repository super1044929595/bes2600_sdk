/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#ifndef __DWT_H__
#define __DWT_H__

#ifdef __cplusplus
extern "C" {
#endif

enum watchpoint_rw {
    WPT_READ  = 0,
    WPT_WRITE = 1,
    WPT_RW    = 2
};

int dwt_enable(void);

int dwt_disable(void);

int watchpoint_enable(uint8_t wpt_index, uint32_t *word_address,
                                            enum watchpoint_rw access);

int watchpoint_disable(uint8_t wpt_index);

static inline int dwt_reset_cycle_cnt(void)
{
    DWT->CYCCNT = 0;
    /*start the cycle count*/
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    return 0;
}

static inline uint32_t dwt_read_cycle_cnt(void)
{
    uint32_t cnt = DWT->CYCCNT;

    /*stop the cycle count*/
    DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;
    return cnt;
}

#ifdef __cplusplus
}
#endif

#endif /* __DWT_H__ */
