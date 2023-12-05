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
#ifndef __HAL_IOMUX_BEST1305_H__
#define __HAL_IOMUX_BEST1305_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"

#ifndef ROM_BUILD
#define PMU_HAS_LED_PIN
#define PMU_HAS_LED_IRQ
#endif

enum HAL_IOMUX_PIN_T {
    HAL_IOMUX_PIN_P0_0 = 0,
    HAL_IOMUX_PIN_P0_1,
    HAL_IOMUX_PIN_P0_2,
    HAL_IOMUX_PIN_P0_3,
    HAL_IOMUX_PIN_P0_4,
    HAL_IOMUX_PIN_P0_5,
    HAL_IOMUX_PIN_P0_6,
    HAL_IOMUX_PIN_P0_7,

    HAL_IOMUX_PIN_P1_0,
    HAL_IOMUX_PIN_P1_1,
    HAL_IOMUX_PIN_P1_2,
    HAL_IOMUX_PIN_P1_3,
    HAL_IOMUX_PIN_P1_4,
    HAL_IOMUX_PIN_P1_5,
    HAL_IOMUX_PIN_P1_6,
    HAL_IOMUX_PIN_P1_7,

    HAL_IOMUX_PIN_P2_0,
    HAL_IOMUX_PIN_P2_1,
    HAL_IOMUX_PIN_P2_2,
    HAL_IOMUX_PIN_P2_3,
    HAL_IOMUX_PIN_P2_4,
    HAL_IOMUX_PIN_P2_5,

    HAL_IOMUX_PIN_NUM,

    HAL_IOMUX_PIN_LED1 = HAL_IOMUX_PIN_NUM,
    HAL_IOMUX_PIN_LED2,

    HAL_IOMUX_PIN_LED_NUM,
};

enum HAL_GPIO_PIN_T {
    HAL_GPIO_PIN_P0_0 = HAL_IOMUX_PIN_P0_0,
    HAL_GPIO_PIN_P0_1 = HAL_IOMUX_PIN_P0_1,
    HAL_GPIO_PIN_P0_2 = HAL_IOMUX_PIN_P0_2,
    HAL_GPIO_PIN_P0_3 = HAL_IOMUX_PIN_P0_3,
    HAL_GPIO_PIN_P0_4 = HAL_IOMUX_PIN_P0_4,
    HAL_GPIO_PIN_P0_5 = HAL_IOMUX_PIN_P0_5,
    HAL_GPIO_PIN_P0_6 = HAL_IOMUX_PIN_P0_6,
    HAL_GPIO_PIN_P0_7 = HAL_IOMUX_PIN_P0_7,

    HAL_GPIO_PIN_P1_0 = HAL_IOMUX_PIN_P1_0,
    HAL_GPIO_PIN_P1_1 = HAL_IOMUX_PIN_P1_1,
    HAL_GPIO_PIN_P1_2 = HAL_IOMUX_PIN_P1_2,
    HAL_GPIO_PIN_P1_3 = HAL_IOMUX_PIN_P1_3,
    HAL_GPIO_PIN_P1_4 = HAL_IOMUX_PIN_P1_4,
    HAL_GPIO_PIN_P1_5 = HAL_IOMUX_PIN_P1_5,
    HAL_GPIO_PIN_P1_6 = HAL_IOMUX_PIN_P1_6,
    HAL_GPIO_PIN_P1_7 = HAL_IOMUX_PIN_P1_7,

    HAL_GPIO_PIN_P2_0 = HAL_IOMUX_PIN_P2_0,
    HAL_GPIO_PIN_P2_1 = HAL_IOMUX_PIN_P2_1,
    HAL_GPIO_PIN_P2_2 = HAL_IOMUX_PIN_P2_2,
    HAL_GPIO_PIN_P2_3 = HAL_IOMUX_PIN_P2_3,
    HAL_GPIO_PIN_P2_4 = HAL_IOMUX_PIN_P2_4,
    HAL_GPIO_PIN_P2_5 = HAL_IOMUX_PIN_P2_5,

    HAL_GPIO_PIN_NUM = HAL_IOMUX_PIN_NUM,

    HAL_GPIO_PIN_LED1 = HAL_IOMUX_PIN_LED1,
    HAL_GPIO_PIN_LED2 = HAL_IOMUX_PIN_LED2,

    HAL_GPIO_PIN_LED_NUM = HAL_IOMUX_PIN_LED_NUM,
};

enum HAL_IOMUX_FUNCTION_T {
    HAL_IOMUX_FUNC_NONE = 0,
    HAL_IOMUX_FUNC_GPIO,
    HAL_IOMUX_FUNC_AS_GPIO = HAL_IOMUX_FUNC_GPIO,
    HAL_IOMUX_FUNC_BT_UART_CTS,
    HAL_IOMUX_FUNC_BT_UART_RTS,
    HAL_IOMUX_FUNC_BT_UART_RX,
    HAL_IOMUX_FUNC_BT_UART_TX,
    HAL_IOMUX_FUNC_CLK_32K_IN,
    HAL_IOMUX_FUNC_CLK_REQ_IN,
    HAL_IOMUX_FUNC_CLK_REQ_OUT,
    HAL_IOMUX_FUNC_CLK_OUT,
    HAL_IOMUX_FUNC_I2C_M0_SCL,
    HAL_IOMUX_FUNC_I2C_M0_SDA,
    HAL_IOMUX_FUNC_I2C_M1_SCL,
    HAL_IOMUX_FUNC_I2C_M1_SDA,
    HAL_IOMUX_FUNC_I2C_M2_SCL,
    HAL_IOMUX_FUNC_I2C_M2_SDA,
    HAL_IOMUX_FUNC_I2C_SCL,
    HAL_IOMUX_FUNC_I2C_SDA,
    HAL_IOMUX_FUNC_I2S0_MCLK,
    HAL_IOMUX_FUNC_I2S0_SCK,
    HAL_IOMUX_FUNC_I2S0_SDI0,
    HAL_IOMUX_FUNC_I2S0_SDO,
    HAL_IOMUX_FUNC_I2S0_WS,
    HAL_IOMUX_FUNC_PCM_CLK,
    HAL_IOMUX_FUNC_PCM_DI,
    HAL_IOMUX_FUNC_PCM_DO,
    HAL_IOMUX_FUNC_PCM_FSYNC,
    HAL_IOMUX_FUNC_PDM0_CK,
    HAL_IOMUX_FUNC_PDM0_D,
    HAL_IOMUX_FUNC_PDM1_CK,
    HAL_IOMUX_FUNC_PDM1_D,
    HAL_IOMUX_FUNC_PWM0,
    HAL_IOMUX_FUNC_PWM1,
    HAL_IOMUX_FUNC_PWM2,
    HAL_IOMUX_FUNC_PWM3,
    HAL_IOMUX_FUNC_SPI_CLK,
    HAL_IOMUX_FUNC_SPI_CS0,
    HAL_IOMUX_FUNC_SPI_DI0,
    HAL_IOMUX_FUNC_SPI_DIO,
    HAL_IOMUX_FUNC_UART0_RX,
    HAL_IOMUX_FUNC_UART0_TX,
    HAL_IOMUX_FUNC_UART1_CTS,
    HAL_IOMUX_FUNC_UART1_RTS,
    HAL_IOMUX_FUNC_UART1_RX,
    HAL_IOMUX_FUNC_UART1_TX,

    HAL_IOMUX_FUNC_END
};

enum HAL_IOMUX_ISPI_ACCESS_T {
    HAL_IOMUX_ISPI_BT_RF            = (1 << 0),
    HAL_IOMUX_ISPI_BT_PMU           = (1 << 1),
    HAL_IOMUX_ISPI_BT_ANA           = (1 << 2),
    HAL_IOMUX_ISPI_MCU_RF           = (1 << 3),
    HAL_IOMUX_ISPI_MCU_PMU          = (1 << 4),
    HAL_IOMUX_ISPI_MCU_ANA          = (1 << 5),
};

void hal_iomux_set_i2s_mclk(void);

void hal_iomux_set_mcu_clock_out(void);

void hal_iomux_set_bt_clock_out(void);

int hal_iomux_tportopen(void);

int hal_iomux_tportclr(int port);

int hal_iomux_tportset(int port);

void hal_iomux_set_spi_slave(void);

void hal_iomux_clear_spi_slave(void);

void hal_iomux_set_pmu_uart(uint32_t uart);

void hal_iomux_clear_pmu_uart(void);

#ifdef __cplusplus
}
#endif

#endif
