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
#include "plat_addr_map.h"

#ifdef TRNG_BASE

#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_cmu.h"
#include "hal_sysfreq.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "reg_trng.h"
#include "hal_trng.h"
#include "string.h"

//#define TRNG_STATS

#define MIN_SAMPLE_CNT1                     17

#define MAX_ERROR_CNT                       20

#define TRNG_SYSFREQ                        HAL_CMU_FREQ_26M

static struct TRNG_T * const trng = (struct TRNG_T *)TRNG_BASE;

static uint32_t timeout_val;
static HAL_TRNG_RUN_CB_T run_cb;
static bool busy;

int hal_trng_open(const struct HAL_TRNG_CFG_T *cfg)
{
    uint32_t sample_cntr1;
    if (cfg == NULL) {
        return -1;
    }

    // SysFreq  SampleCnt   TimeConsumed(ms)
    // -------  ---------   ----------------
    // 24M      256         18
    // 24M      128         9
    // 24M      64          4.5
    // 24M      32          2.3
    if (cfg->sample_cntr1 < MIN_SAMPLE_CNT1)
        sample_cntr1 = MIN_SAMPLE_CNT1;
    else
        sample_cntr1 = cfg->sample_cntr1;

    timeout_val = MS_TO_TICKS(sample_cntr1 / 16 * 1 * 10);

    hal_cmu_clock_enable(HAL_CMU_MOD_P_TRNG);
    hal_cmu_reset_clear(HAL_CMU_MOD_P_TRNG);

    trng->TRNG_CONFIG = cfg->rnd_src_sel & RND_SRC_SEL_MASK;
    trng->SAMPLE_CNT1 = sample_cntr1;

    return 0;
}

static void hal_trng_start(int irq_en)
{
    trng->RND_SOURCE_ENABLE = 0;
    trng->RNG_ICR = 0xF; //clear int status
    trng->RST_BITS_COUNTER = 1; // reset the bits counter and TRNG valid registers
    trng->RND_SOURCE_ENABLE = RND_SOURCE_EN;
    if (irq_en) {
        trng->RNG_IMR = 0;
    } else {
        trng->RNG_IMR = 0xF;
    }
}

static void hal_trng_stop()
{
    trng->RND_SOURCE_ENABLE = 0;
    trng->RNG_ICR = 0xF;
    trng->RNG_IMR = 0xF;
}

void hal_trng_close(void)
{
    trng->RND_SOURCE_ENABLE = 0;
    trng->RST_BITS_COUNTER = 1; // reset the bits counter and TRNG valid registers

    hal_cmu_reset_set(HAL_CMU_MOD_P_TRNG);
    hal_cmu_clock_disable(HAL_CMU_MOD_P_TRNG);

    clear_bool_flag(&busy);

    return;
}

int hal_get_trngdata(uint8_t *buf, uint32_t buf_len)
{
    uint32_t i;
    uint32_t len;
    uint32_t ehr_data;
    uint32_t err_cnt;
    int ret = 0;
    uint8_t *buf_end = buf + buf_len;

#ifdef TRNG_STATS
    uint32_t total_err = 0;
#endif

    if (set_bool_flag(&busy)) {
        return 1;
    }

    err_cnt = 0;

    while (buf < buf_end) {
    uint32_t time = hal_sys_timer_get();
#ifdef TRNG_STATS
    uint32_t fast_time = hal_fast_sys_timer_get();
#endif
        hal_trng_start(false);

        while ((trng->RNG_ISR & 0xF) == 0) {
            if (hal_sys_timer_get() - time >= timeout_val) {
                ret = -2;
                goto _exit;
            }
#ifndef TRNG_STATS
            osDelay(4);
#endif
        };
        if (trng->RNG_ISR & 0xE) { //error
            if (++err_cnt > MAX_ERROR_CNT) {
                ret = -1;
                goto _exit;
            }
            continue;
        }
#ifdef TRNG_STATS
        fast_time = hal_fast_sys_timer_get() - fast_time;
        TRACE(0, "TRNG fast_time consumed: %u ticks / %u us", fast_time, FAST_TICKS_TO_US(fast_time));
        total_err += err_cnt;
#endif
        err_cnt = 0;

        if (buf + HAL_TRNG_DATA_LEN > buf_end)
            len = buf_end - buf;
        else
            len = HAL_TRNG_DATA_LEN;

        for (i = 0; i < len / 4; i++) {
            ehr_data = trng->EHR_DATA[i];
#ifdef UNALIGNED_ACCESS
            *(volatile uint32_t *)buf = ehr_data;
            buf += 4;
#else
            *buf++ = ((uint8_t *)&ehr_data)[0];
            *buf++ = ((uint8_t *)&ehr_data)[1];
            *buf++ = ((uint8_t *)&ehr_data)[2];
            *buf++ = ((uint8_t *)&ehr_data)[3];
#endif
        }
        len %= 4;
        if (len) {
            ehr_data = trng->EHR_DATA[i];
            for (i = 0; i < len; i++) {
                *buf++ = ((uint8_t *)&ehr_data)[i];
            }
        }
    }

_exit:
    hal_trng_stop();
    clear_bool_flag(&busy);
#ifdef TRNG_STATS
    TRACE(0, "TRNG total_err=%u", total_err);
#endif
    return ret;
}

static void hal_trng_irq_handler(void)
{
    uint32_t buf[(HAL_TRNG_DATA_LEN + 3) / 4];
    uint32_t status;

    status = trng->RNG_ISR;
    trng->RNG_ICR = status;

    trng->RNG_IMR = 0xF;

    if (status == 0x1) {
        for (int i = 0; i < ARRAY_SIZE(buf); i++) {
            buf[i] = trng->EHR_DATA[i];
        }
    }

    hal_sysfreq_req(HAL_SYSFREQ_USER_TRNG, HAL_CMU_FREQ_32K);

    clear_bool_flag(&busy);

    if (run_cb) {
        if (status == 0x1) {
            run_cb(buf, HAL_TRNG_DATA_LEN, 0);
        } else {
            run_cb(NULL, 0, status);
        }
    }
}

int hal_trng_run(HAL_TRNG_RUN_CB_T cb)
{
    if (!cb) {
        return -1;
    }

    if (set_bool_flag(&busy)) {
        return 1;
    }

    hal_sysfreq_req(HAL_SYSFREQ_USER_TRNG, TRNG_SYSFREQ);

    run_cb = cb;

    NVIC_SetVector(TRNG_IRQn, (uint32_t)hal_trng_irq_handler);
    NVIC_SetPriority(TRNG_IRQn, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(TRNG_IRQn);
    NVIC_EnableIRQ(TRNG_IRQn);

    hal_trng_start(true);

    return 0;
}

void hal_trng_cancel(void)
{
    hal_trng_stop();

    NVIC_DisableIRQ(TRNG_IRQn);

    hal_sysfreq_req(HAL_SYSFREQ_USER_TRNG, HAL_CMU_FREQ_32K);

    clear_bool_flag(&busy);
}

#endif
