/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#ifndef __PMU_BEST1305_H__
#define __PMU_BEST1305_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_cmu.h"

#define PMU_REG(r)                          (((r) & 0xFFF) | 0x0000)
#define ANA_REG(r)                          (((r) & 0xFFF) | 0x1000)
#define RF_REG(r)                           (((r) & 0xFFF) | 0x2000)

#define MAX_VMIC_CH_NUM                     2

enum PMU_EFUSE_PAGE_T {
    PMU_EFUSE_PAGE_SECURITY         = 0,
    PMU_EFUSE_PAGE_BOOT             = 1,
    PMU_EFUSE_PAGE_FEATURE          = 2,
    PMU_EFUSE_PAGE_BATTER_LV        = 3,

    PMU_EFUSE_PAGE_BATTER_HV        = 4,
    PMU_EFUSE_PAGE_SW_CFG           = 5,
    PMU_EFUSE_PAGE_PROD_TEST        = 6,
    PMU_EFUSE_PAGE_RESERVED_7       = 7,

    PMU_EFUSE_PAGE_BT_POWER         = 8,
    PMU_EFUSE_PAGE_DCCALIB2_L       = 9,
    PMU_EFUSE_PAGE_DCCALIB2_L_LP    = 10,
    PMU_EFUSE_PAGE_DCCALIB_L        = 11,

    PMU_EFUSE_PAGE_DCCALIB_L_LP     = 12,
    PMU_EFUSE_PAGE_RESERVED_13      = 13,
    PMU_EFUSE_PAGE_MODEL            = 14,
    PMU_EFUSE_PAGE_RESERVED_15      = 15,
};

enum PMU_IRQ_TYPE_T {
    PMU_IRQ_TYPE_GPADC,
    PMU_IRQ_TYPE_RTC,
    PMU_IRQ_TYPE_CHARGER,
    PMU_IRQ_TYPE_GPIO,
    PMU_IRQ_TYPE_WDT,

    PMU_IRQ_TYPE_QTY
};

enum PMU_PLL_DIV_TYPE_T {
    PMU_PLL_DIV_DIG,
    PMU_PLL_DIV_CODEC,
};

enum PMU_BIG_BANDGAP_USER_T {
    PMU_BIG_BANDGAP_USER_GPADC          = (1 << 0),
};

enum PMU_BOOT_CAUSE_T {
    PMU_BOOT_CAUSE_NULL                 = 0,
    PMU_BOOT_CAUSE_POWER_KEY            = (1 << 0),
    PMU_BOOT_CAUSE_DIG_WDT              = (1 << 1),
    PMU_BOOT_CAUSE_DIG_REBOOT           = (1 << 2),
    PMU_BOOT_CAUSE_CHARGER_IN           = (1 << 3),
    PMU_BOOT_CAUSE_CHARGER_OUT          = (1 << 4),
    PMU_BOOT_CAUSE_RTC                  = (1 << 5),
};

uint8_t pmu_gpio_setup_irq(enum HAL_GPIO_PIN_T pin, const struct HAL_GPIO_IRQ_CFG_T *cfg);

void pmu_codec_hppa_enable(int enable);

void pmu_codec_mic_bias_enable(uint32_t map);

void pmu_codec_mic_bias_lowpower_mode(uint32_t map, int enable);

int pmu_codec_volt_ramp_up(void);

int pmu_codec_volt_ramp_down(void);

void pmu_pll_div_reset_set(enum HAL_CMU_PLL_T pll);

void pmu_pll_div_reset_clear(enum HAL_CMU_PLL_T pll);

void pmu_pll_div_set(enum HAL_CMU_PLL_T pll, enum PMU_PLL_DIV_TYPE_T type, uint32_t div);

void pmu_pll_freq_reg_set(uint16_t low, uint16_t high, uint16_t high2);

void pmu_pll_codec_clock_enable(bool en);

void pmu_led_set_hiz(enum HAL_GPIO_PIN_T pin);

void pmu_led_uart_enable(enum HAL_IOMUX_PIN_T pin);

void pmu_led_uart_disable(enum HAL_IOMUX_PIN_T pin);

void pmu_big_bandgap_enable(enum PMU_BIG_BANDGAP_USER_T user, int enable);

void pmu_rf_ana_init(void);

void pmu_bt_reconn(bool en);

void check_efuse_for_different_chip(void);

enum PMU_BOOT_CAUSE_T pmu_boot_cause_get(void);

#ifdef __cplusplus
}
#endif

#endif

