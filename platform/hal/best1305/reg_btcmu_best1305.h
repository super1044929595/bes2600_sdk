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
#ifndef __REG_BTCMU_BEST1305_H__
#define __REG_BTCMU_BEST1305_H__

#include "plat_types.h"

struct BTCMU_T {
    __IO uint32_t CLK_ENABLE;           // 0x00
    __IO uint32_t CLK_DISABLE;          // 0x04
    __IO uint32_t CLK_MODE;             // 0x08
    __IO uint32_t DIV_TIMER;            // 0x0C
    __IO uint32_t RESET_SET;            // 0x10
    __IO uint32_t RESET_CLR;            // 0x14
    __IO uint32_t DIV_WDT;              // 0x18
    __IO uint32_t RESET_PULSE;          // 0x1C
         uint32_t RESERVED_020[0x24 / 4]; // 0x20
    __IO uint32_t CLK_OUT;              // 0x44
         uint32_t RESERVED_048[2];      // 0x48
    __IO uint32_t ISIRQ_SET;            // 0x50
    __IO uint32_t ISIRQ_CLR;            // 0x54
};

// reg_00
#define BT_CMU_MANUAL_HCLK_ENABLE(n)                        (((n) & 0xFFFFFFFF) << 0)
#define BT_CMU_MANUAL_HCLK_ENABLE_MASK                      (0xFFFFFFFF << 0)
#define BT_CMU_MANUAL_HCLK_ENABLE_SHIFT                     (0)
#define BT_CMU_MANUAL_OCLK_ENABLE(n)                        (((n) & 0xFFFFFFFF) << BT_HCLK_NUM)
#define BT_CMU_MANUAL_OCLK_ENABLE_MASK                      (0xFFFFFFFF << BT_HCLK_NUM)
#define BT_CMU_MANUAL_OCLK_ENABLE_SHIFT                     (BT_HCLK_NUM)

// reg_04
#define BT_CMU_MANUAL_HCLK_DISABLE(n)                       (((n) & 0xFFFFFFFF) << 0)
#define BT_CMU_MANUAL_HCLK_DISABLE_MASK                     (0xFFFFFFFF << 0)
#define BT_CMU_MANUAL_HCLK_DISABLE_SHIFT                    (0)
#define BT_CMU_MANUAL_OCLK_DISABLE(n)                       (((n) & 0xFFFFFFFF) << BT_HCLK_NUM)
#define BT_CMU_MANUAL_OCLK_DISABLE_MASK                     (0xFFFFFFFF << BT_HCLK_NUM)
#define BT_CMU_MANUAL_OCLK_DISABLE_SHIFT                    (BT_HCLK_NUM)

// reg_08
#define BT_CMU_MODE_HCLK(n)                                 (((n) & 0xFFFFFFFF) << 0)
#define BT_CMU_MODE_HCLK_MASK                               (0xFFFFFFFF << 0)
#define BT_CMU_MODE_HCLK_SHIFT                              (0)
#define BT_CMU_MODE_OCLK(n)                                 (((n) & 0xFFFFFFFF) << BT_HCLK_NUM)
#define BT_CMU_MODE_OCLK_MASK                               (0xFFFFFFFF << BT_HCLK_NUM)
#define BT_CMU_MODE_OCLK_SHIFT                              (BT_HCLK_NUM)

// reg_10
#define BT_CMU_HRESETN_SET(n)                               (((n) & 0xFFFFFFFF) << 0)
#define BT_CMU_HRESETN_SET_MASK                             (0xFFFFFFFF << 0)
#define BT_CMU_HRESETN_SET_SHIFT                            (0)
#define BT_CMU_ORESETN_SET(n)                               (((n) & 0xFFFFFFFF) << BT_HRST_NUM)
#define BT_CMU_ORESETN_SET_MASK                             (0xFFFFFFFF << BT_HRST_NUM)
#define BT_CMU_ORESETN_SET_SHIFT                            (BT_HRST_NUM)
#define BT_CMU_GLOBAL_RESETN_SET                            (1 << (BT_HRST_NUM+BT_ORST_NUM+1-1))

// reg_14
#define BT_CMU_HRESETN_CLR(n)                               (((n) & 0xFFFFFFFF) << 0)
#define BT_CMU_HRESETN_CLR_MASK                             (0xFFFFFFFF << 0)
#define BT_CMU_HRESETN_CLR_SHIFT                            (0)
#define BT_CMU_ORESETN_CLR(n)                               (((n) & 0xFFFFFFFF) << BT_HRST_NUM)
#define BT_CMU_ORESETN_CLR_MASK                             (0xFFFFFFFF << BT_HRST_NUM)
#define BT_CMU_ORESETN_CLR_SHIFT                            (BT_HRST_NUM)
#define BT_CMU_GLOBAL_RESETN_CLR                            (1 << (BT_HRST_NUM+BT_ORST_NUM+1-1))

// reg_1c
#define BT_CMU_HRESETN_PULSE(n)                             (((n) & 0xFFFFFFFF) << 0)
#define BT_CMU_HRESETN_PULSE_MASK                           (0xFFFFFFFF << 0)
#define BT_CMU_HRESETN_PULSE_SHIFT                          (0)
#define BT_CMU_ORESETN_PULSE(n)                             (((n) & 0xFFFFFFFF) << BT_HRST_NUM)
#define BT_CMU_ORESETN_PULSE_MASK                           (0xFFFFFFFF << BT_HRST_NUM)
#define BT_CMU_ORESETN_PULSE_SHIFT                          (BT_HRST_NUM)
#define BT_CMU_GLOBAL_RESETN_PULSE                          (1 << (BT_HRST_NUM+BT_ORST_NUM+1-1))

// reg_18
#define BT_CMU_CFG_DIV_WDT(n)                               (((n) & 0xFFFF) << 0)
#define BT_CMU_CFG_DIV_WDT_MASK                             (0xFFFF << 0)
#define BT_CMU_CFG_DIV_WDT_SHIFT                            (0)
#define BT_CMU_CFG_HCLK_MCU_OFF_TIMER(n)                    (((n) & 0xFF) << 16)
#define BT_CMU_CFG_HCLK_MCU_OFF_TIMER_MASK                  (0xFF << 16)
#define BT_CMU_CFG_HCLK_MCU_OFF_TIMER_SHIFT                 (16)
#define BT_CMU_UART_BUS_IDLE_TIMER(n)                       (((n) & 0xFF) << 24)
#define BT_CMU_UART_BUS_IDLE_TIMER_MASK                     (0xFF << 24)
#define BT_CMU_UART_BUS_IDLE_TIMER_SHIFT                    (24)

// reg_0c
#define BT_CMU_CFG_DIV_TIMER0(n)                            (((n) & 0xFFFF) << 0)
#define BT_CMU_CFG_DIV_TIMER0_MASK                          (0xFFFF << 0)
#define BT_CMU_CFG_DIV_TIMER0_SHIFT                         (0)
#define BT_CMU_CFG_DIV_TIMER1(n)                            (((n) & 0xFFFF) << 16)
#define BT_CMU_CFG_DIV_TIMER1_MASK                          (0xFFFF << 16)
#define BT_CMU_CFG_DIV_TIMER1_SHIFT                         (16)

// reg_24
#define BT_CMU_DEEPSLEEP_START                              (1 << 0)
#define BT_CMU_DEEPSLEEP_GATE                               (1 << 1)
#define BT_CMU_CAL_LPO_INT_EN                               (1 << 2)
#define BT_CMU_HCLK_MCU_ENABLE                              (1 << 3)
#define BT_CMU_DEEPSLEEP_EN                                 (1 << 4)
#define BT_CMU_DEEPSLEEP_ROMRAM_EN                          (1 << 5)
#define BT_CMU_OSC_EN_BT_EN                                 (1 << 6)
#define BT_CMU_UART_WAKE_EN                                 (1 << 7)
#define BT_CMU_TIMER_WT26(n)                                (((n) & 0xFF) << 8)
#define BT_CMU_TIMER_WT26_MASK                              (0xFF << 8)
#define BT_CMU_TIMER_WT26_SHIFT                             (8)
#define BT_CMU_CPU_IDLE_GATE_EN                             (1 << 16)
#define BT_CMU_MANUAL_RAM_RETN                              (1 << 17)
#define BT_CMU_CPU_IDLE_GATE_DIS                            (1 << 18)
#define BT_CMU_DEEPSLEEP_MODE                               (1 << 24)
#define BT_CMU_PU_OSC                                       (1 << 25)
#define BT_CMU_UART_WAKE                                    (1 << 27)
#define BT_CMU_SLEEP_ALLOW                                  (1 << 29)
#define BT_CMU_DEEPSLEEP_ALLOWED                            (1 << 30)
#define BT_CMU_WAKEUP_DEEPSLEEP_SYNC_N                      (1 << 31)

// reg_28
#define BT_CMU_TARGET_ADDR(n)                               (((n) & 0xFFFFFFFF) << 0)
#define BT_CMU_TARGET_ADDR_MASK                             (0xFFFFFFFF << 0)
#define BT_CMU_TARGET_ADDR_SHIFT                            (0)

// reg_2c
#define BT_CMU_BM_IRQ_CLR                                   (1 << 0)
#define BT_CMU_BM_IRQ_EN                                    (1 << 1)
#define BT_CMU_TARGET_OP                                    (1 << 2)

// reg_30
#define BT_CMU_PWR_NO_SIG(n)                                (((n) & 0x3FF) << 0)
#define BT_CMU_PWR_NO_SIG_MASK                              (0x3FF << 0)
#define BT_CMU_PWR_NO_SIG_SHIFT                             (0)
#define BT_CMU_PWR_SIG_AC(n)                                (((n) & 0x3FF) << 10)
#define BT_CMU_PWR_SIG_AC_MASK                              (0x3FF << 10)
#define BT_CMU_PWR_SIG_AC_SHIFT                             (10)
#define BT_CMU_PWR_CHANNEL(n)                               (((n) & 0x7F) << 20)
#define BT_CMU_PWR_CHANNEL_MASK                             (0x7F << 20)
#define BT_CMU_PWR_CHANNEL_SHIFT                            (20)

// reg_34
#define BT_CMU_ROM_MS(n)                                    (((n) & 0xF) << 0)
#define BT_CMU_ROM_MS_MASK                                  (0xF << 0)
#define BT_CMU_ROM_MS_SHIFT                                 (0)
#define BT_CMU_ROM_MSE                                      (1 << 4)
#define BT_CMU_RAM_MS(n)                                    (((n) & 0xF) << 5)
#define BT_CMU_RAM_MS_MASK                                  (0xF << 5)
#define BT_CMU_RAM_MS_SHIFT                                 (5)
#define BT_CMU_RAM_MSE                                      (1 << 9)
#define BT_CMU_RAM_WMS(n)                                   (((n) & 0xF) << 10)
#define BT_CMU_RAM_WMS_MASK                                 (0xF << 10)
#define BT_CMU_RAM_WMS_SHIFT                                (10)
#define BT_CMU_RAM_WMSE                                     (1 << 14)
#define BT_CMU_RF_MS(n)                                     (((n) & 0xF) << 15)
#define BT_CMU_RF_MS_MASK                                   (0xF << 15)
#define BT_CMU_RF_MS_SHIFT                                  (15)
#define BT_CMU_RF_MSE                                       (1 << 19)
#define BT_CMU_RF_WMS(n)                                    (((n) & 0xF) << 20)
#define BT_CMU_RF_WMS_MASK                                  (0xF << 20)
#define BT_CMU_RF_WMS_SHIFT                                 (20)
#define BT_CMU_RF_WMSE                                      (1 << 24)

// reg_38
#define BT_CMU_BT_CLKSEL(n)                                 (((n) & 0x3F) << 0)
#define BT_CMU_BT_CLKSEL_MASK                               (0x3F << 0)
#define BT_CMU_BT_CLKSEL_SHIFT                              (0)
#define BT_CMU_SEL_PLL_SYS                                  (1 << 6)
#define BT_CMU_SEL_OSCX2_SYS                                (1 << 7)
#define BT_CMU_SEL_OSC_SYS                                  (1 << 8)
#define BT_CMU_SEL_OSC_52M                                  (1 << 9)
#define BT_CMU_SEL_PLL_52M                                  (1 << 10)
#define BT_CMU_BYPASS_LOCK_PLL                              (1 << 11)
#define BT_CMU_POL_CLK_ADC                                  (1 << 12)
#define BT_CMU_POL_CLK_DAC                                  (1 << 13)
#define BT_CMU_POL_CLK_26M                                  (1 << 14)
#define BT_CMU_SEL_TIMER_FAST                               (1 << 15)
#define BT_CMU_SEL_32K_TIMER                                (1 << 16)
#define BT_CMU_SEL_WDT_FAST                                 (1 << 17)
#define BT_CMU_SEL_32K_WDT                                  (1 << 18)
#define BT_CMU_SEL_BLE5                                     (1 << 19)
#define BT_CMU_FORCE_ADC_CLK_ON                             (1 << 20)
#define BT_CMU_FORCE_SYS_CLK_ON                             (1 << 21)
#define BT_CMU_EN_CLK_CPU_EM                                (1 << 22)
#define BT_CMU_FORCE_PU_OFF                                 (1 << 23)
#define BT_CMU_POL_CLK_NFMI                                 (1 << 24)
#define BT_CMU_EN_PWR_MON                                   (1 << 25)
#define BT_CMU_MODE_PWR_MON                                 (1 << 26)
#define BT_CMU_SEL_BLE5_26M                                 (1 << 27)

// reg_3c
#define BT_CMU_WAKEUP_IRQ_MASK(n)                           (((n) & 0xFFFFFFFF) << 0)
#define BT_CMU_WAKEUP_IRQ_MASK_MASK                         (0xFFFFFFFF << 0)
#define BT_CMU_WAKEUP_IRQ_MASK_SHIFT                        (0)

// reg_40
#define BT_CMU_WRITE_UNLOCK_H                               (1 << 0)
#define BT_CMU_WRITE_UNLOCK_STATUS                          (1 << 1)

// reg_44
#define BT_CMU_CAL_TIME(n)                                  (((n) & 0xFF) << 0)
#define BT_CMU_CAL_TIME_MASK                                (0xFF << 0)
#define BT_CMU_CAL_TIME_SHIFT                               (0)
#define BT_CMU_SMP_CMU_SEL(n)                               (((n) & 0xF) << 8)
#define BT_CMU_SMP_CMU_SEL_MASK                             (0xF << 8)
#define BT_CMU_SMP_CMU_SEL_SHIFT                            (8)
#define BT_CMU_CFG_CLK_OUT(n)                               (((n) & 0xF) << 12)
#define BT_CMU_CFG_CLK_OUT_MASK                             (0xF << 12)
#define BT_CMU_CFG_CLK_OUT_SHIFT                            (12)
#define BT_CMU_PWR_DELAY(n)                                 (((n) & 0x7F) << 16)
#define BT_CMU_PWR_DELAY_MASK                               (0x7F << 16)
#define BT_CMU_PWR_DELAY_SHIFT                              (16)
#define BT_CMU_ROM_PGEN(n)                                  (((n) & 0x3F) << 23)
#define BT_CMU_ROM_PGEN_MASK                                (0x3F << 23)
#define BT_CMU_ROM_PGEN_SHIFT                               (23)

// reg_48
#define BT_CMU_CAL_COUNT_VALUE(n)                           (((n) & 0x7FFFFFFF) << 0)
#define BT_CMU_CAL_COUNT_VALUE_MASK                         (0x7FFFFFFF << 0)
#define BT_CMU_CAL_COUNT_VALUE_SHIFT                        (0)
#define BT_CMU_CAL_LPO_DONE                                 (1 << 31)

// reg_4c
#define BT_CMU_CPU_IDLE_FLAG                                (1 << 0)

// reg_50
#define BT_CMU_MCU2BT_DATA_DONE_SET                         (1 << 0)
#define BT_CMU_MCU2BT_DATA1_DONE_SET                        (1 << 1)
#define BT_CMU_BT2MCU_DATA_IND_SET                          (1 << 2)
#define BT_CMU_BT2MCU_DATA1_IND_SET                         (1 << 3)
#define BT_CMU_WAKEUP_BT                                    (1 << 4)
#define BT_CMU_MCU_ALLIRQ_MASK_SET                          (1 << 5)
#define BT_CMU_BT2MCU_ALLIRQ_MASK_SET(n)                    (((n) & 0x1FF) << 6)
#define BT_CMU_BT2MCU_ALLIRQ_MASK_SET_MASK                  (0x1FF << 6)
#define BT_CMU_BT2MCU_ALLIRQ_MASK_SET_SHIFT                 (6)
#define BT_CMU_BT2AON_IRQ_SET                               (1 << 15)

// reg_54
#define BT_CMU_MCU2BT_DATA_DONE_CLR                         (1 << 0)
#define BT_CMU_MCU2BT_DATA1_DONE_CLR                        (1 << 1)
#define BT_CMU_BT2MCU_DATA_IND_CLR                          (1 << 2)
#define BT_CMU_BT2MCU_DATA1_IND_CLR                         (1 << 3)
#define BT_CMU_MCU_ALLIRQ_MASK_CLR                          (1 << 5)
#define BT_CMU_BT2MCU_ALLIRQ_MASK_CLR(n)                    (((n) & 0x1FF) << 6)
#define BT_CMU_BT2MCU_ALLIRQ_MASK_CLR_MASK                  (0x1FF << 6)
#define BT_CMU_BT2MCU_ALLIRQ_MASK_CLR_SHIFT                 (6)
#define BT_CMU_BT2AON_IRQ_CLR                               (1 << 15)

// reg_58
#define BT_CMU_PWR_SIG_END(n)                               (((n) & 0x3FF) << 0)
#define BT_CMU_PWR_SIG_END_MASK                             (0x3FF << 0)
#define BT_CMU_PWR_SIG_END_SHIFT                            (0)
#define BT_CMU_PWR_CHANNEL2(n)                              (((n) & 0x7F) << 10)
#define BT_CMU_PWR_CHANNEL2_MASK                            (0x7F << 10)
#define BT_CMU_PWR_CHANNEL2_SHIFT                           (10)

// reg_5c
#define BT_CMU_RAM_RET1N0                                   (1 << 0)
#define BT_CMU_RAM_RET2N0                                   (1 << 1)
#define BT_CMU_RAM_PGEN0                                    (1 << 2)
#define BT_CMU_RAM_RET1N1                                   (1 << 3)
#define BT_CMU_RAM_RET2N1                                   (1 << 4)
#define BT_CMU_RAM_PGEN1                                    (1 << 5)
#define BT_CMU_RF_RET1N0                                    (1 << 6)
#define BT_CMU_RF_RET2N0                                    (1 << 7)
#define BT_CMU_RF_PGEN0                                     (1 << 8)
#define BT_CMU_RF_RET1N1                                    (1 << 9)
#define BT_CMU_RF_RET2N1                                    (1 << 10)
#define BT_CMU_RF_PGEN1                                     (1 << 11)

// reg_60
#define BT_CMU_PG_AUTO_EN_REG                               (1 << 0)
#define BT_CMU_PG_AUTO_MODE                                 (1 << 1)
#define BT_CMU_POWER_MODE_0(n)                              (((n) & 0x7) << 2)
#define BT_CMU_POWER_MODE_0_MASK                            (0x7 << 2)
#define BT_CMU_POWER_MODE_0_SHIFT                           (2)
#define BT_CMU_POWER_MODE_1(n)                              (((n) & 0x7) << 5)
#define BT_CMU_POWER_MODE_1_MASK                            (0x7 << 5)
#define BT_CMU_POWER_MODE_1_SHIFT                           (5)
#define BT_CMU_B2TB1_TIMER_REG(n)                           (((n) & 0xF) << 8)
#define BT_CMU_B2TB1_TIMER_REG_MASK                         (0xF << 8)
#define BT_CMU_B2TB1_TIMER_REG_SHIFT                        (8)
#define BT_CMU_B2TB3_TIMER_REG(n)                           (((n) & 0xF) << 12)
#define BT_CMU_B2TB3_TIMER_REG_MASK                         (0xF << 12)
#define BT_CMU_B2TB3_TIMER_REG_SHIFT                        (12)

// reg_f0
#define BT_CMU_RESERVED(n)                                  (((n) & 0xFFFFFFFF) << 0)
#define BT_CMU_RESERVED_MASK                                (0xFFFFFFFF << 0)
#define BT_CMU_RESERVED_SHIFT                               (0)

#endif
