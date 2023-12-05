
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

#include "cmsis.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_wdt.h"
#include "pmu.h"
#include "analog.h"
#include "app_utils.h"
#include "cmsis_os.h"
#include "watchdog/watchdog.h"

#ifdef APP_WATCHDOG
extern int task_idle_health_check(void);
static void watchdog_ping_handler(void const *n);
static osTimerId wdt_ping_timer_id = NULL;
osTimerDef(wdt_ping_timer, watchdog_ping_handler);
static uint32_t wdt_ping_period;

static void app_wdt_irq_handle(enum HAL_WDT_ID_T id, enum HAL_WDT_EVENT_T event)
{
    analog_aud_codec_mute();
    ASSERT(0, "%s id:%d event:%d",__func__, id, event);
}

static void pmu_wdt_irq_handle(void)
{
    analog_aud_codec_mute();
    ASSERT(0, "%s", __func__);
}

static int hung_task_check = true;

int app_wdt_hung_task_check_enable(int enable)
{
    hung_task_check = !!enable;
    return 0;
}

static void watchdog_ping_handler(void const *unused)
{
    int ret;

    watchdog_ping();

    if (hung_task_check) {
        ret = task_idle_health_check();
        if (ret < 0) {
            ASSERT(0, "System soft lockup");
        }
    }
}

int app_wdt_open(int seconds)
{
    uint32_t lock;
    int ret;
    static int wdt_opened = 0;

    lock = int_lock();
    if (wdt_opened) {
        int_unlock(lock);
        return -1;
    }

    wdt_opened = 1;
    hal_wdt_set_irq_callback(HAL_WDT_ID_0, app_wdt_irq_handle);
#ifndef CHIP_BEST2000
    pmu_wdt_set_irq_handler(pmu_wdt_irq_handle);
#endif
    watchdog_hw_start(seconds);
    int_unlock(lock);

    wdt_ping_timer_id = osTimerCreate(osTimer(wdt_ping_timer), osTimerPeriodic, NULL);
    if (!wdt_ping_timer_id) {
        TRACE(0,"Warning: can not create watchdog ping timer");
        return -1;
    }
    wdt_ping_period = seconds * 1000 / 4;

    ret = osTimerStart(wdt_ping_timer_id, wdt_ping_period);
    if (ret != osOK)
        ASSERT(0, "watchdog start timer error");

    return 0;
}

int app_wdt_reopen(int seconds)
{
    int ret;
    uint32_t lock = int_lock();

    watchdog_hw_stop();
    watchdog_hw_start(seconds);
    int_unlock(lock);

    ret = osTimerStart(wdt_ping_timer_id, wdt_ping_period);
    if(ret != osOK)
        return -1;

    return 0;
}

int app_wdt_close(void)
{
    uint32_t lock;

    osTimerStop(wdt_ping_timer_id);

    lock = int_lock();
    watchdog_hw_stop();
    int_unlock(lock);

    return 0;
}

#endif

#ifdef __WATCHER_DOG_RESET__

int watchdog_hw_stop(void)
{
    hal_wdt_stop(HAL_WDT_ID_0);
#ifndef CHIP_BEST2000
    pmu_wdt_stop();
#endif
    return 0;
}

int watchdog_hw_start(int seconds)
{
#ifdef NO_WDT
    watchdog_hw_stop();
#else
    hal_wdt_set_timeout(HAL_WDT_ID_0, seconds);
    hal_wdt_start(HAL_WDT_ID_0);
#ifndef CHIP_BEST2000
    pmu_wdt_config(seconds * 1100, seconds * 1100);
    pmu_wdt_start();
#endif
#endif
    return 0;
}

void watchdog_ping(void)
{
    hal_wdt_ping(HAL_WDT_ID_0);
#ifndef CHIP_BEST2000
    pmu_wdt_feed();
#endif
}

#endif

