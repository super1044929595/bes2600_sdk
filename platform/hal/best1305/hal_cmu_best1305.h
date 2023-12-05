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
#ifndef __HAL_CMU_BEST1305_H__
#define __HAL_CMU_BEST1305_H__

#ifdef __cplusplus
extern "C" {
#endif

#define HAL_CMU_VALID_CRYSTAL_FREQ          { 26000000, 24000000, 40000000, 48000000, }

enum HAL_CMU_MOD_ID_T {
    // HCLK/HRST
    HAL_CMU_MOD_H_MCU,          // 0
    HAL_CMU_MOD_H_ROM0,         // 1
    HAL_CMU_MOD_H_SPI_AHB,      // 2
    HAL_CMU_MOD_H_I2C_AHB,      // 3
    HAL_CMU_MOD_H_RAM0,         // 4
    HAL_CMU_MOD_H_RAM1,         // 5
    HAL_CMU_MOD_H_RAM2,         // 6
    HAL_CMU_MOD_H_RAMRET,       // 7
    HAL_CMU_MOD_H_AHB0,         // 8
    HAL_CMU_MOD_H_AHB1,         // 9
    HAL_CMU_MOD_H_AH2H_BT,      // 10
    HAL_CMU_MOD_H_ADMA,         // 11
    HAL_CMU_MOD_H_GDMA,         // 12
    HAL_CMU_MOD_H_CACHE,        // 13
    HAL_CMU_MOD_H_FLASH,        // 14
    HAL_CMU_MOD_H_SDMMC,        // 15
    HAL_CMU_MOD_H_USBC,         // 16
    HAL_CMU_MOD_H_CODEC,        // 17
    HAL_CMU_MOD_H_BT_TP_DUMP,   // 18
    HAL_CMU_MOD_H_I2C_SLAVE,    // 19
    HAL_CMU_MOD_H_USBH,         // 20
    HAL_CMU_MOD_H_SENSOR_ENG,   // 21
    HAL_CMU_MOD_H_BT_DUMP,      // 22
    HAL_CMU_MOD_H_CP,           // 23
    HAL_CMU_MOD_H_RAM3,         // 24
    HAL_CMU_MOD_H_RAM4,         // 25
    HAL_CMU_MOD_H_RAM5,         // 26
    HAL_CMU_MOD_H_RAM6,         // 27
    HAL_CMU_MOD_H_SEC_ENG,      // 28
    HAL_CMU_MOD_H_ICACHE0,      // 29
    HAL_CMU_MOD_H_ICACHE1,      // 30
    // PCLK/PRST
    HAL_CMU_MOD_P_CMU,          // 31
    HAL_CMU_MOD_P_WDT,          // 32
    HAL_CMU_MOD_P_TIMER0,       // 33
    HAL_CMU_MOD_P_TIMER1,       // 34
    HAL_CMU_MOD_P_TIMER2,       // 35
    HAL_CMU_MOD_P_I2C0,         // 36
    HAL_CMU_MOD_P_I2C1,         // 37
    HAL_CMU_MOD_P_SPI,          // 38
    HAL_CMU_MOD_P_SLCD,         // 39
    HAL_CMU_MOD_P_SPI_ITN,      // 40
    HAL_CMU_MOD_P_SPI_PHY,      // 41
    HAL_CMU_MOD_P_UART0,        // 42
    HAL_CMU_MOD_P_UART1,        // 43
    HAL_CMU_MOD_P_UART2,        // 44
    HAL_CMU_MOD_P_PCM,          // 45
    HAL_CMU_MOD_P_I2S0,         // 46
    HAL_CMU_MOD_P_SPDIF0,       // 47
    HAL_CMU_MOD_P_I2S1,         // 48
    HAL_CMU_MOD_P_SEC_ENG,      // 49
    HAL_CMU_MOD_P_TRNG,         // 50
    HAL_CMU_MOD_P_I2C_HS_SLAVE, // 51
    HAL_CMU_MOD_P_I2C3_HW,      // 52
    // OCLK/ORST
    HAL_CMU_MOD_O_SLEEP,        // 53
    HAL_CMU_MOD_O_FLASH,        // 54
    HAL_CMU_MOD_O_USB,          // 55
    HAL_CMU_MOD_O_SDMMC,        // 56
    HAL_CMU_MOD_O_WDT,          // 57
    HAL_CMU_MOD_O_TIMER0,       // 58
    HAL_CMU_MOD_O_TIMER1,       // 59
    HAL_CMU_MOD_O_TIMER2,       // 60
    HAL_CMU_MOD_O_I2C0,         // 61
    HAL_CMU_MOD_O_I2C1,         // 62
    HAL_CMU_MOD_O_SPI,          // 63
    HAL_CMU_MOD_O_SLCD,         // 64
    HAL_CMU_MOD_O_SPI_ITN,      // 65
    HAL_CMU_MOD_O_SPI_PHY,      // 66
    HAL_CMU_MOD_O_UART0,        // 67
    HAL_CMU_MOD_O_UART1,        // 68
    HAL_CMU_MOD_O_UART2,        // 69
    HAL_CMU_MOD_O_I2S0,         // 70
    HAL_CMU_MOD_O_SPDIF0,       // 71
    HAL_CMU_MOD_O_PCM,          // 72
    HAL_CMU_MOD_O_USB32K,       // 73
    HAL_CMU_MOD_O_I2S1,         // 74
    HAL_CMU_MOD_O_I2C_HS_SLAVE, // 75
    HAL_CMU_MOD_O_I2C3_HW,      // 76

    // AON ACLK/ARST
    HAL_CMU_AON_A_CMU,          // 77
    HAL_CMU_AON_A_GPIO,         // 78
    HAL_CMU_AON_A_GPIO_INT,     // 79
    HAL_CMU_AON_A_WDT,          // 80
    HAL_CMU_AON_A_PWM,          // 81
    HAL_CMU_AON_A_TIMER,        // 82
    HAL_CMU_AON_A_PSC,          // 83
    HAL_CMU_AON_A_IOMUX,        // 84
    HAL_CMU_AON_A_APBC,         // 85
    HAL_CMU_AON_A_H2H_MCU,      // 86
    // AON OCLK/ORST
    HAL_CMU_AON_O_WDT,          // 87
    HAL_CMU_AON_O_TIMER,        // 88
    HAL_CMU_AON_O_GPIO,         // 89
    HAL_CMU_AON_O_PWM0,         // 90
    HAL_CMU_AON_O_PWM1,         // 91
    HAL_CMU_AON_O_PWM2,         // 92
    HAL_CMU_AON_O_PWM3,         // 93
    HAL_CMU_AON_O_IOMUX,        // 94
    HAL_CMU_AON_O_SLP32K,       // 95
    HAL_CMU_AON_O_SLP26M,       // 96
    HAL_CMU_AON_RESERVED0,      // 97
    HAL_CMU_AON_RESERVED1,      // 98
    HAL_CMU_AON_RESERVED2,      // 99
    HAL_CMU_AON_RESERVED3,      // 100
    // AON SUBSYS
    HAL_CMU_AON_MCU,            // 101
    HAL_CMU_AON_CODEC,          // 102
    HAL_CMU_AON_RESERVED4,      // 103
    HAL_CMU_AON_BT,             // 104
    HAL_CMU_AON_MCUCPU,         // 105
    HAL_CMU_AON_RESERVED5,      // 106
    HAL_CMU_AON_BTCPU,          // 107
    HAL_CMU_AON_GLOBAL,         // 108

    HAL_CMU_MOD_QTY,

    HAL_CMU_MOD_GLOBAL = HAL_CMU_AON_GLOBAL,
    HAL_CMU_MOD_BT = HAL_CMU_AON_BT,
    HAL_CMU_MOD_BTCPU = HAL_CMU_AON_BTCPU,

    HAL_CMU_MOD_P_PWM = HAL_CMU_AON_A_PWM,
    HAL_CMU_MOD_O_PWM0 = HAL_CMU_AON_O_PWM0,
    HAL_CMU_MOD_O_PWM1 = HAL_CMU_AON_O_PWM1,
    HAL_CMU_MOD_O_PWM2 = HAL_CMU_AON_O_PWM2,
    HAL_CMU_MOD_O_PWM3 = HAL_CMU_AON_O_PWM3,

    HAL_CMU_MOD_P_I2C2 = HAL_CMU_MOD_P_I2C3_HW,
    HAL_CMU_MOD_O_I2C2 = HAL_CMU_MOD_O_I2C3_HW,

    HAL_CMU_H_ICACHECP = HAL_CMU_MOD_QTY,
    HAL_CMU_H_DCACHECP = HAL_CMU_MOD_QTY,

    // TO BE REMOVED
    HAL_CMU_MOD_P_CODEC = HAL_CMU_MOD_QTY,
    HAL_CMU_MOD_O_CODEC_DA = HAL_CMU_MOD_QTY,
    HAL_CMU_MOD_O_CODEC_AD = HAL_CMU_MOD_QTY,
};

enum HAL_CMU_CLOCK_OUT_ID_T {
    HAL_CMU_CLOCK_OUT_AON_32K           = 0x00,
    HAL_CMU_CLOCK_OUT_AON_26M           = 0x01,
    HAL_CMU_CLOCK_OUT_AON_52M           = 0x02,
    HAL_CMU_CLOCK_OUT_AON_DIG_52M       = 0x03,
    HAL_CMU_CLOCK_OUT_AON_DIG_104M      = 0x04,
    HAL_CMU_CLOCK_OUT_AON_PER           = 0x05,
    HAL_CMU_CLOCK_OUT_AON_USB           = 0x06,
    HAL_CMU_CLOCK_OUT_AON_DCDC          = 0x07,
    HAL_CMU_CLOCK_OUT_AON_CHCLK         = 0x08,
    HAL_CMU_CLOCK_OUT_AON_SPDIF0        = 0x09,
    HAL_CMU_CLOCK_OUT_AON_MCU           = 0x0A,
    HAL_CMU_CLOCK_OUT_AON_FLASH         = 0x0B,
    HAL_CMU_CLOCK_OUT_AON_SYS           = 0x0C,

    HAL_CMU_CLOCK_OUT_BT_32K            = 0x40,
    HAL_CMU_CLOCK_OUT_BT_SYS            = 0x41,
    HAL_CMU_CLOCK_OUT_BT_52M            = 0x42,
    HAL_CMU_CLOCK_OUT_BT_26MI           = 0x43,
    HAL_CMU_CLOCK_OUT_BT_13M            = 0x44,
    HAL_CMU_CLOCK_OUT_BT_12M            = 0x45,
    HAL_CMU_CLOCK_OUT_BT_ADC            = 0x46,
    HAL_CMU_CLOCK_OUT_BT_ADC2           = 0x47,
    HAL_CMU_CLOCK_OUT_BT_24M            = 0x48,
    HAL_CMU_CLOCK_OUT_BT_26M            = 0x49,

    HAL_CMU_CLOCK_OUT_MCU_32K           = 0x60,
    HAL_CMU_CLOCK_OUT_MCU_SYS           = 0x61,
    HAL_CMU_CLOCK_OUT_MCU_FLASH         = 0x62,
    HAL_CMU_CLOCK_OUT_MCU_USB           = 0x63,
    HAL_CMU_CLOCK_OUT_MCU_PCLK          = 0x64,
    HAL_CMU_CLOCK_OUT_MCU_I2S           = 0x65,
    HAL_CMU_CLOCK_OUT_MCU_PCM           = 0x66,
    HAL_CMU_CLOCK_OUT_MCU_SPDIF0        = 0x67,
    HAL_CMU_CLOCK_OUT_MCU_SDMMC         = 0x68,
    HAL_CMU_CLOCK_OUT_MCU_SPI2          = 0x69,
    HAL_CMU_CLOCK_OUT_MCU_SPI0          = 0x6A,
    HAL_CMU_CLOCK_OUT_MCU_SPI1          = 0x6B,

    HAL_CMU_CLOCK_OUT_CODEC_ADC_ANA     = 0x80,
    HAL_CMU_CLOCK_OUT_CODEC_CODEC       = 0x81,
    HAL_CMU_CLOCK_OUT_CODEC_IIR         = 0x82,
    HAL_CMU_CLOCK_OUT_CODEC_RS          = 0x83,
    HAL_CMU_CLOCK_OUT_CODEC_HCLK        = 0x84,
};

enum HAL_CMU_I2S_MCLK_ID_T {
    HAL_CMU_I2S_MCLK_PLLCODEC           = 0x00,
    HAL_CMU_I2S_MCLK_CODEC              = 0x01,
    HAL_CMU_I2S_MCLK_PLLIIR             = 0x02,
    HAL_CMU_I2S_MCLK_PLLRS              = 0x03,
    HAL_CMU_I2S_MCLK_PLLSPDIF0          = 0x04,
    HAL_CMU_I2S_MCLK_PLLPCM             = 0x05,
    HAL_CMU_I2S_MCLK_PER                = 0x06,
    HAL_CMU_I2S_MCLK_CLK_OUT            = 0x07,
};

enum HAL_CMU_FREQ_T {
    HAL_CMU_FREQ_32K,
    HAL_CMU_FREQ_6P5M,
    HAL_CMU_FREQ_13M,
    HAL_CMU_FREQ_26M,
    HAL_CMU_FREQ_52M,
    HAL_CMU_FREQ_78M,
    HAL_CMU_FREQ_104M,
    HAL_CMU_FREQ_208M,

    HAL_CMU_FREQ_QTY
};
#define HAL_CMU_FREQ_T                      HAL_CMU_FREQ_T

enum HAL_I2S_ID_T {
    HAL_I2S_ID_0 = 0,
    HAL_I2S_ID_1,

    HAL_I2S_ID_QTY,
};
#define HAL_I2S_ID_T                        HAL_I2S_ID_T

enum HAL_CMU_ANC_CLK_USER_T {
    HAL_CMU_ANC_CLK_USER_ANC,

    HAL_CMU_ANC_CLK_USER_QTY
};

enum HAL_CMU_DCDC_CLOCK_USER_T {
    HAL_CMU_DCDC_CLOCK_USER_GPADC,
    HAL_CMU_DCDC_CLOCK_USER_ADCKEY,
    HAL_CMU_DCDC_CLOCK_USER_EFUSE,

    HAL_CMU_DCDC_CLOCK_USER_QTY,
};

int hal_cmu_fast_timer_offline(void);

void hal_cmu_codec_high_speed_enable(void);

void hal_cmu_codec_high_speed_disable(void);

void hal_cmu_anc_enable(enum HAL_CMU_ANC_CLK_USER_T user);

void hal_cmu_anc_disable(enum HAL_CMU_ANC_CLK_USER_T user);

int hal_cmu_anc_get_status(enum HAL_CMU_ANC_CLK_USER_T user);

uint32_t hal_cmu_get_aon_chip_id(void);

uint32_t hal_cmu_get_aon_revision_id(void);

void hal_cmu_cp_enable(uint32_t sp, uint32_t entry);

void hal_cmu_cp_disable(void);

uint32_t hal_cmu_cp_get_entry_addr(void);

uint32_t hal_cmu_get_ram_boot_addr(void);

void hal_cmu_dcdc_clock_enable(enum HAL_CMU_DCDC_CLOCK_USER_T user);

void hal_cmu_dcdc_clock_disable(enum HAL_CMU_DCDC_CLOCK_USER_T user);

#ifdef __cplusplus
}
#endif

#endif

