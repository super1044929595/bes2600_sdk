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
#ifndef __REG_IOMUX_BEST1305_H_
#define __REG_IOMUX_BEST1305_H_

#include "plat_types.h"

struct IOMUX_T {
    __IO uint32_t REG_000;
    __IO uint32_t REG_004;
    __IO uint32_t REG_008;
    __IO uint32_t REG_00C;
    __IO uint32_t REG_010;
    __IO uint32_t REG_014;
    __IO uint32_t REG_018;
    __IO uint32_t REG_01C;
    __IO uint32_t REG_020;
    __IO uint32_t REG_024;
    __IO uint32_t REG_028;
    __IO uint32_t REG_02C;
    __IO uint32_t REG_030;
    __IO uint32_t REG_034;
    __IO uint32_t REG_038;
    __IO uint32_t REG_03C;
    __IO uint32_t REG_040;
    __IO uint32_t REG_044;
    __IO uint32_t REG_048;
    __IO uint32_t REG_04C;
    __IO uint32_t REG_050;
    __IO uint32_t REG_054;
    __IO uint32_t REG_058;
    __IO uint32_t REG_05C;
    __IO uint32_t REG_060;
    __IO uint32_t REG_064;
    __IO uint32_t REG_068;
    __IO uint32_t REG_06C;
    __IO uint32_t REG_070;
    __IO uint32_t REG_074;
    __IO uint32_t REG_078;
};

// reg_00
#define IOMUX_32_H00001305(n)                               (((n) & 0xFFFFFFFF) << 0)
#define IOMUX_32_H00001305_MASK                             (0xFFFFFFFF << 0)
#define IOMUX_32_H00001305_SHIFT                            (0)

// reg_04
#define IOMUX_GPIO_P00_SEL(n)                               (((n) & 0xF) << 0)
#define IOMUX_GPIO_P00_SEL_MASK                             (0xF << 0)
#define IOMUX_GPIO_P00_SEL_SHIFT                            (0)
#define IOMUX_GPIO_P01_SEL(n)                               (((n) & 0xF) << 4)
#define IOMUX_GPIO_P01_SEL_MASK                             (0xF << 4)
#define IOMUX_GPIO_P01_SEL_SHIFT                            (4)
#define IOMUX_GPIO_P02_SEL(n)                               (((n) & 0xF) << 8)
#define IOMUX_GPIO_P02_SEL_MASK                             (0xF << 8)
#define IOMUX_GPIO_P02_SEL_SHIFT                            (8)
#define IOMUX_GPIO_P03_SEL(n)                               (((n) & 0xF) << 12)
#define IOMUX_GPIO_P03_SEL_MASK                             (0xF << 12)
#define IOMUX_GPIO_P03_SEL_SHIFT                            (12)
#define IOMUX_GPIO_P04_SEL(n)                               (((n) & 0xF) << 16)
#define IOMUX_GPIO_P04_SEL_MASK                             (0xF << 16)
#define IOMUX_GPIO_P04_SEL_SHIFT                            (16)
#define IOMUX_GPIO_P05_SEL(n)                               (((n) & 0xF) << 20)
#define IOMUX_GPIO_P05_SEL_MASK                             (0xF << 20)
#define IOMUX_GPIO_P05_SEL_SHIFT                            (20)
#define IOMUX_GPIO_P06_SEL(n)                               (((n) & 0xF) << 24)
#define IOMUX_GPIO_P06_SEL_MASK                             (0xF << 24)
#define IOMUX_GPIO_P06_SEL_SHIFT                            (24)
#define IOMUX_GPIO_P07_SEL(n)                               (((n) & 0xF) << 28)
#define IOMUX_GPIO_P07_SEL_MASK                             (0xF << 28)
#define IOMUX_GPIO_P07_SEL_SHIFT                            (28)

// reg_08
#define IOMUX_GPIO_P10_SEL(n)                               (((n) & 0xF) << 0)
#define IOMUX_GPIO_P10_SEL_MASK                             (0xF << 0)
#define IOMUX_GPIO_P10_SEL_SHIFT                            (0)
#define IOMUX_GPIO_P11_SEL(n)                               (((n) & 0xF) << 4)
#define IOMUX_GPIO_P11_SEL_MASK                             (0xF << 4)
#define IOMUX_GPIO_P11_SEL_SHIFT                            (4)
#define IOMUX_GPIO_P12_SEL(n)                               (((n) & 0xF) << 8)
#define IOMUX_GPIO_P12_SEL_MASK                             (0xF << 8)
#define IOMUX_GPIO_P12_SEL_SHIFT                            (8)
#define IOMUX_GPIO_P13_SEL(n)                               (((n) & 0xF) << 12)
#define IOMUX_GPIO_P13_SEL_MASK                             (0xF << 12)
#define IOMUX_GPIO_P13_SEL_SHIFT                            (12)
#define IOMUX_GPIO_P14_SEL(n)                               (((n) & 0xF) << 16)
#define IOMUX_GPIO_P14_SEL_MASK                             (0xF << 16)
#define IOMUX_GPIO_P14_SEL_SHIFT                            (16)
#define IOMUX_GPIO_P15_SEL(n)                               (((n) & 0xF) << 20)
#define IOMUX_GPIO_P15_SEL_MASK                             (0xF << 20)
#define IOMUX_GPIO_P15_SEL_SHIFT                            (20)
#define IOMUX_GPIO_P16_SEL(n)                               (((n) & 0xF) << 24)
#define IOMUX_GPIO_P16_SEL_MASK                             (0xF << 24)
#define IOMUX_GPIO_P16_SEL_SHIFT                            (24)
#define IOMUX_GPIO_P17_SEL(n)                               (((n) & 0xF) << 28)
#define IOMUX_GPIO_P17_SEL_MASK                             (0xF << 28)
#define IOMUX_GPIO_P17_SEL_SHIFT                            (28)

// reg_0c
#define IOMUX_GPIO_P20_SEL(n)                               (((n) & 0xF) << 0)
#define IOMUX_GPIO_P20_SEL_MASK                             (0xF << 0)
#define IOMUX_GPIO_P20_SEL_SHIFT                            (0)
#define IOMUX_GPIO_P21_SEL(n)                               (((n) & 0xF) << 4)
#define IOMUX_GPIO_P21_SEL_MASK                             (0xF << 4)
#define IOMUX_GPIO_P21_SEL_SHIFT                            (4)
#define IOMUX_GPIO_P22_SEL(n)                               (((n) & 0xF) << 8)
#define IOMUX_GPIO_P22_SEL_MASK                             (0xF << 8)
#define IOMUX_GPIO_P22_SEL_SHIFT                            (8)
#define IOMUX_GPIO_P23_SEL(n)                               (((n) & 0xF) << 12)
#define IOMUX_GPIO_P23_SEL_MASK                             (0xF << 12)
#define IOMUX_GPIO_P23_SEL_SHIFT                            (12)
#define IOMUX_GPIO_P24_SEL(n)                               (((n) & 0xF) << 16)
#define IOMUX_GPIO_P24_SEL_MASK                             (0xF << 16)
#define IOMUX_GPIO_P24_SEL_SHIFT                            (16)
#define IOMUX_GPIO_P25_SEL(n)                               (((n) & 0xF) << 20)
#define IOMUX_GPIO_P25_SEL_MASK                             (0xF << 20)
#define IOMUX_GPIO_P25_SEL_SHIFT                            (20)
#define IOMUX_GPIO_P26_SEL(n)                               (((n) & 0xF) << 24)
#define IOMUX_GPIO_P26_SEL_MASK                             (0xF << 24)
#define IOMUX_GPIO_P26_SEL_SHIFT                            (24)
#define IOMUX_GPIO_P27_SEL(n)                               (((n) & 0xF) << 28)
#define IOMUX_GPIO_P27_SEL_MASK                             (0xF << 28)
#define IOMUX_GPIO_P27_SEL_SHIFT                            (28)

// reg_14
#define IOMUX_ANA_TEST_DIR(n)                               (((n) & 0xFFFF) << 0)
#define IOMUX_ANA_TEST_DIR_MASK                             (0xFFFF << 0)
#define IOMUX_ANA_TEST_DIR_SHIFT                            (0)

// reg_18
#define IOMUX_ANA_TEST_SEL(n)                               (((n) & 0xF) << 0)
#define IOMUX_ANA_TEST_SEL_MASK                             (0xF << 0)
#define IOMUX_ANA_TEST_SEL_SHIFT                            (0)

// reg_1c
#define IOMUX_RESERVED(n)                                   (((n) & 0xFFFFFFFF) << 0)
#define IOMUX_RESERVED_MASK                                 (0xFFFFFFFF << 0)
#define IOMUX_RESERVED_SHIFT                                (0)

// reg_20
#define IOMUX_RESERVED2(n)                                  (((n) & 0xFFFFFFFF) << 0)
#define IOMUX_RESERVED2_MASK                                (0xFFFFFFFF << 0)
#define IOMUX_RESERVED2_SHIFT                               (0)

// reg_24
#define IOMUX_RESERVED3(n)                                  (((n) & 0xFFFFFFFF) << 0)
#define IOMUX_RESERVED3_MASK                                (0xFFFFFFFF << 0)
#define IOMUX_RESERVED3_SHIFT                               (0)

// reg_28
#define IOMUX_RESERVED4(n)                                  (((n) & 0xFFFFFFFF) << 0)
#define IOMUX_RESERVED4_MASK                                (0xFFFFFFFF << 0)
#define IOMUX_RESERVED4_SHIFT                               (0)

// reg_2c
#define IOMUX_R_GPIO_P0_PU(n)                               (((n) & 0xFF) << 0)
#define IOMUX_R_GPIO_P0_PU_MASK                             (0xFF << 0)
#define IOMUX_R_GPIO_P0_PU_SHIFT                            (0)
#define IOMUX_R_GPIO_P1_PU(n)                               (((n) & 0xFF) << 8)
#define IOMUX_R_GPIO_P1_PU_MASK                             (0xFF << 8)
#define IOMUX_R_GPIO_P1_PU_SHIFT                            (8)
#define IOMUX_R_GPIO_P2_PU(n)                               (((n) & 0x3F) << 16)
#define IOMUX_R_GPIO_P2_PU_MASK                             (0x3F << 16)
#define IOMUX_R_GPIO_P2_PU_SHIFT                            (16)

// reg_30
#define IOMUX_R_GPIO_P0_PD(n)                               (((n) & 0xFF) << 0)
#define IOMUX_R_GPIO_P0_PD_MASK                             (0xFF << 0)
#define IOMUX_R_GPIO_P0_PD_SHIFT                            (0)
#define IOMUX_R_GPIO_P1_PD(n)                               (((n) & 0xFF) << 8)
#define IOMUX_R_GPIO_P1_PD_MASK                             (0xFF << 8)
#define IOMUX_R_GPIO_P1_PD_SHIFT                            (8)
#define IOMUX_R_GPIO_P2_PD(n)                               (((n) & 0x3F) << 16)
#define IOMUX_R_GPIO_P2_PD_MASK                             (0x3F << 16)
#define IOMUX_R_GPIO_P2_PD_SHIFT                            (16)

// reg_34
#define IOMUX_AGPIO_EN(n)                                   (((n) & 0x3FFFFF) << 0)
#define IOMUX_AGPIO_EN_MASK                                 (0x3FFFFF << 0)
#define IOMUX_AGPIO_EN_SHIFT                                (0)

// reg_38
#define IOMUX_MCUIO_DB_ACTIVE(n)                            (((n) & 0x3FFFFF) << 0)
#define IOMUX_MCUIO_DB_ACTIVE_MASK                          (0x3FFFFF << 0)
#define IOMUX_MCUIO_DB_ACTIVE_SHIFT                         (0)

// reg_3c
#define IOMUX_MCUIO_DB_VALUE(n)                             (((n) & 0xFF) << 0)
#define IOMUX_MCUIO_DB_VALUE_MASK                           (0xFF << 0)
#define IOMUX_MCUIO_DB_VALUE_SHIFT                          (0)
#define IOMUX_PWK_DB_VALUE(n)                               (((n) & 0xFF) << 8)
#define IOMUX_PWK_DB_VALUE_MASK                             (0xFF << 8)
#define IOMUX_PWK_DB_VALUE_SHIFT                            (8)
#define IOMUX_PWK_DB_ACTIVE                                 (1 << 16)

// reg_40
#define IOMUX_POWER_KEY_ON_INT_STATUS                       (1 << 0)
#define IOMUX_POWER_KEY_OFF_INT_STATUS                      (1 << 1)
#define IOMUX_R_POWER_KEY_INTR_U                            (1 << 2)
#define IOMUX_R_POWER_KEY_INTR_D                            (1 << 3)
#define IOMUX_POWER_KEY_ON_INT_EN                           (1 << 4)
#define IOMUX_POWER_KEY_OFF_INT_EN                          (1 << 5)
#define IOMUX_POWER_KEY_ON_INT_MSK                          (1 << 6)
#define IOMUX_POWER_KEY_OFF_INT_MSK                         (1 << 7)
#define IOMUX_POWER_KEY_DB                                  (1 << 30)
#define IOMUX_POWER_ON_FEEDOUT                              (1 << 31)

#define IOMUX_PWR_KEY_DOWN_INT_CLR                          (1 << 0)
#define IOMUX_PWR_KEY_UP_INT_CLR                            (1 << 1)
#define IOMUX_PWR_KEY_DOWN_INT_ST                           (1 << 2)
#define IOMUX_PWR_KEY_UP_INT_ST                             (1 << 3)
#define IOMUX_PWR_KEY_DOWN_INT_EN                           (1 << 4)
#define IOMUX_PWR_KEY_UP_INT_EN                             (1 << 5)
#define IOMUX_PWR_KEY_DOWN_INT_MASK                         (1 << 6)
#define IOMUX_PWR_KEY_UP_INT_MASK                           (1 << 7)
#define IOMUX_PWR_KEY_DBOUNCE_ST                            (1 << 30)
#define IOMUX_PWR_KEY_VAL                                   (1 << 31)

// reg_44
#define IOMUX_BT_SPI_RF_ENABLE                              (1 << 0)
#define IOMUX_BT_SPI_PMU_ENABLE                             (1 << 1)
#define IOMUX_BT_SPI_ANA_ENABLE                             (1 << 2)
#define IOMUX_MCU_APB_SPI_RF_ENABLE                         (1 << 3)
#define IOMUX_MCU_APB_SPI_PMU_ENABLE                        (1 << 4)
#define IOMUX_MCU_APB_SPI_ANA_ENABLE                        (1 << 5)

// reg_48
#define IOMUX_EN_PMU_UART                                   (1 << 0)
#define IOMUX_SEL_PMU_UART(n)                               (((n) & 0x3) << 1)
#define IOMUX_SEL_PMU_UART_MASK                             (0x3 << 1)
#define IOMUX_SEL_PMU_UART_SHIFT                            (1)
#define IOMUX_DR_PMU_UART_OENB                              (1 << 3)
#define IOMUX_REG_PMU_UART_OENB                             (1 << 4)

// reg_50
#define IOMUX_GPIO_I2C_MODE                                 (1 << 0)
#define IOMUX_I2C0_M_SEL_GPIO                               (1 << 1)
#define IOMUX_GPIO_PCM_MODE                                 (1 << 2)
#define IOMUX_BT_RXTX_SW_EN                                 (1 << 3)
#define IOMUX_I2C1_M_SEL_GPIO                               (1 << 4)
#define IOMUX_SPI0_MCU_WM3                                  (1 << 5)
#define IOMUX_SPILCD1_WM3                                   (1 << 6)
#define IOMUX_CFG_EN_CLK_REQIN                              (1 << 7)
#define IOMUX_CFG_POL_CLK_REQIN                             (1 << 8)
#define IOMUX_CFG_EN_CLK_REQOUT                             (1 << 9)
#define IOMUX_CFG_POL_CLK_REQOUT                            (1 << 10)
#define IOMUX_BT_UART_HALFN                                 (1 << 11)
#define IOMUX_UART0_MCU_HALFN                               (1 << 12)
#define IOMUX_UART1_MCU_HALFN                               (1 << 13)
#define IOMUX_UART2_MCU_HALFN                               (1 << 14)
#define IOMUX_SEL_I2C_INTR(n)                               (((n) & 0x3) << 15)
#define IOMUX_SEL_I2C_INTR_MASK                             (0x3 << 15)
#define IOMUX_SEL_I2C_INTR_SHIFT                            (15)

// reg_54
#define IOMUX_CFG_GPIO_OENB_P0_POL(n)                       (((n) & 0xFF) << 0)
#define IOMUX_CFG_GPIO_OENB_P0_POL_MASK                     (0xFF << 0)
#define IOMUX_CFG_GPIO_OENB_P0_POL_SHIFT                    (0)
#define IOMUX_CFG_GPIO_OENB_P1_POL(n)                       (((n) & 0xFF) << 8)
#define IOMUX_CFG_GPIO_OENB_P1_POL_MASK                     (0xFF << 8)
#define IOMUX_CFG_GPIO_OENB_P1_POL_SHIFT                    (8)
#define IOMUX_CFG_GPIO_OENB_P2_POL(n)                       (((n) & 0x3F) << 16)
#define IOMUX_CFG_GPIO_OENB_P2_POL_MASK                     (0x3F << 16)
#define IOMUX_CFG_GPIO_OENB_P2_POL_SHIFT                    (16)

// reg_58
#define IOMUX_SPI_INTR_RESETN                               (1 << 0)

// reg_64
#define IOMUX_CFG_CODEC_TRIG_SEL(n)                         (((n) & 0x7) << 0)
#define IOMUX_CFG_CODEC_TRIG_SEL_MASK                       (0x7 << 0)
#define IOMUX_CFG_CODEC_TRIG_SEL_SHIFT                      (0)
#define IOMUX_CFG_CODEC_TRIG_POL                            (1 << 3)

// reg_70
#define IOMUX_GPIO_P1_RXHZ_EN(n)                            (((n) & 0xFF) << 0)
#define IOMUX_GPIO_P1_RXHZ_EN_MASK                          (0xFF << 0)
#define IOMUX_GPIO_P1_RXHZ_EN_SHIFT                         (0)
#define IOMUX_GPIO_P1_PWS(n)                                (((n) & 0x3) << 8)
#define IOMUX_GPIO_P1_PWS_MASK                              (0x3 << 8)
#define IOMUX_GPIO_P1_PWS_SHIFT                             (8)

// reg_74
#define IOMUX_GPIO_P0_DRV0_SEL(n)                           (((n) & 0x3) << 0)
#define IOMUX_GPIO_P0_DRV0_SEL_MASK                         (0x3 << 0)
#define IOMUX_GPIO_P0_DRV0_SEL_SHIFT                        (0)
#define IOMUX_GPIO_P0_DRV1_SEL(n)                           (((n) & 0x3) << 2)
#define IOMUX_GPIO_P0_DRV1_SEL_MASK                         (0x3 << 2)
#define IOMUX_GPIO_P0_DRV1_SEL_SHIFT                        (2)
#define IOMUX_GPIO_P0_DRV2_SEL(n)                           (((n) & 0x3) << 4)
#define IOMUX_GPIO_P0_DRV2_SEL_MASK                         (0x3 << 4)
#define IOMUX_GPIO_P0_DRV2_SEL_SHIFT                        (4)
#define IOMUX_GPIO_P0_DRV3_SEL(n)                           (((n) & 0x3) << 6)
#define IOMUX_GPIO_P0_DRV3_SEL_MASK                         (0x3 << 6)
#define IOMUX_GPIO_P0_DRV3_SEL_SHIFT                        (6)
#define IOMUX_GPIO_P0_DRV4_SEL(n)                           (((n) & 0x3) << 8)
#define IOMUX_GPIO_P0_DRV4_SEL_MASK                         (0x3 << 8)
#define IOMUX_GPIO_P0_DRV4_SEL_SHIFT                        (8)
#define IOMUX_GPIO_P0_DRV5_SEL(n)                           (((n) & 0x3) << 10)
#define IOMUX_GPIO_P0_DRV5_SEL_MASK                         (0x3 << 10)
#define IOMUX_GPIO_P0_DRV5_SEL_SHIFT                        (10)
#define IOMUX_GPIO_P0_DRV6_SEL(n)                           (((n) & 0x3) << 12)
#define IOMUX_GPIO_P0_DRV6_SEL_MASK                         (0x3 << 12)
#define IOMUX_GPIO_P0_DRV6_SEL_SHIFT                        (12)
#define IOMUX_GPIO_P0_DRV7_SEL(n)                           (((n) & 0x3) << 14)
#define IOMUX_GPIO_P0_DRV7_SEL_MASK                         (0x3 << 14)
#define IOMUX_GPIO_P0_DRV7_SEL_SHIFT                        (14)
#define IOMUX_GPIO_P1_DRV0_SEL(n)                           (((n) & 0x3) << 16)
#define IOMUX_GPIO_P1_DRV0_SEL_MASK                         (0x3 << 16)
#define IOMUX_GPIO_P1_DRV0_SEL_SHIFT                        (16)
#define IOMUX_GPIO_P1_DRV1_SEL(n)                           (((n) & 0x3) << 18)
#define IOMUX_GPIO_P1_DRV1_SEL_MASK                         (0x3 << 18)
#define IOMUX_GPIO_P1_DRV1_SEL_SHIFT                        (18)
#define IOMUX_GPIO_P1_DRV2_SEL(n)                           (((n) & 0x3) << 20)
#define IOMUX_GPIO_P1_DRV2_SEL_MASK                         (0x3 << 20)
#define IOMUX_GPIO_P1_DRV2_SEL_SHIFT                        (20)
#define IOMUX_GPIO_P1_DRV3_SEL(n)                           (((n) & 0x3) << 22)
#define IOMUX_GPIO_P1_DRV3_SEL_MASK                         (0x3 << 22)
#define IOMUX_GPIO_P1_DRV3_SEL_SHIFT                        (22)
#define IOMUX_GPIO_P1_DRV4_SEL(n)                           (((n) & 0x3) << 24)
#define IOMUX_GPIO_P1_DRV4_SEL_MASK                         (0x3 << 24)
#define IOMUX_GPIO_P1_DRV4_SEL_SHIFT                        (24)
#define IOMUX_GPIO_P1_DRV5_SEL(n)                           (((n) & 0x3) << 26)
#define IOMUX_GPIO_P1_DRV5_SEL_MASK                         (0x3 << 26)
#define IOMUX_GPIO_P1_DRV5_SEL_SHIFT                        (26)
#define IOMUX_GPIO_P1_DRV6_SEL(n)                           (((n) & 0x3) << 28)
#define IOMUX_GPIO_P1_DRV6_SEL_MASK                         (0x3 << 28)
#define IOMUX_GPIO_P1_DRV6_SEL_SHIFT                        (28)
#define IOMUX_GPIO_P1_DRV7_SEL(n)                           (((n) & 0x3) << 30)
#define IOMUX_GPIO_P1_DRV7_SEL_MASK                         (0x3 << 30)
#define IOMUX_GPIO_P1_DRV7_SEL_SHIFT                        (30)

// reg_78
#define IOMUX_GPIO_P2_DRV0_SEL(n)                           (((n) & 0x3) << 0)
#define IOMUX_GPIO_P2_DRV0_SEL_MASK                         (0x3 << 0)
#define IOMUX_GPIO_P2_DRV0_SEL_SHIFT                        (0)
#define IOMUX_GPIO_P2_DRV1_SEL(n)                           (((n) & 0x3) << 2)
#define IOMUX_GPIO_P2_DRV1_SEL_MASK                         (0x3 << 2)
#define IOMUX_GPIO_P2_DRV1_SEL_SHIFT                        (2)
#define IOMUX_GPIO_P2_DRV2_SEL(n)                           (((n) & 0x3) << 4)
#define IOMUX_GPIO_P2_DRV2_SEL_MASK                         (0x3 << 4)
#define IOMUX_GPIO_P2_DRV2_SEL_SHIFT                        (4)
#define IOMUX_GPIO_P2_DRV3_SEL(n)                           (((n) & 0x3) << 6)
#define IOMUX_GPIO_P2_DRV3_SEL_MASK                         (0x3 << 6)
#define IOMUX_GPIO_P2_DRV3_SEL_SHIFT                        (6)
#define IOMUX_GPIO_P2_DRV4_SEL(n)                           (((n) & 0x3) << 8)
#define IOMUX_GPIO_P2_DRV4_SEL_MASK                         (0x3 << 8)
#define IOMUX_GPIO_P2_DRV4_SEL_SHIFT                        (8)
#define IOMUX_GPIO_P2_DRV5_SEL(n)                           (((n) & 0x3) << 10)
#define IOMUX_GPIO_P2_DRV5_SEL_MASK                         (0x3 << 10)
#define IOMUX_GPIO_P2_DRV5_SEL_SHIFT                        (10)

#endif
