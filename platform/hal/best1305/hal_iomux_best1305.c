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
#include "plat_addr_map.h"
#include CHIP_SPECIFIC_HDR(reg_iomux)
#include "hal_iomux.h"
#include "hal_chipid.h"
#include "hal_gpio.h"
#include "hal_location.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_uart.h"
#include "pmu.h"

#define UART_HALF_DUPLEX

#ifdef I2S0_VOLTAGE_VMEM
#define I2S0_VOLTAGE_SEL                    HAL_IOMUX_PIN_VOLTAGE_MEM
#else
#define I2S0_VOLTAGE_SEL                    HAL_IOMUX_PIN_VOLTAGE_VIO
#endif

#ifdef I2S1_VOLTAGE_VMEM
#define I2S1_VOLTAGE_SEL                    HAL_IOMUX_PIN_VOLTAGE_MEM
#else
#define I2S1_VOLTAGE_SEL                    HAL_IOMUX_PIN_VOLTAGE_VIO
#endif

#ifdef SPDIF0_VOLTAGE_VMEM
#define SPDIF0_VOLTAGE_SEL                  HAL_IOMUX_PIN_VOLTAGE_MEM
#else
#define SPDIF0_VOLTAGE_SEL                  HAL_IOMUX_PIN_VOLTAGE_VIO
#endif

#ifdef DIGMIC_VOLTAGE_VMEM
#define DIGMIC_VOLTAGE_SEL                  HAL_IOMUX_PIN_VOLTAGE_MEM
#else
#define DIGMIC_VOLTAGE_SEL                  HAL_IOMUX_PIN_VOLTAGE_VIO
#endif

#ifdef SPI_VOLTAGE_VMEM
#define SPI_VOLTAGE_SEL                     HAL_IOMUX_PIN_VOLTAGE_MEM
#else
#define SPI_VOLTAGE_SEL                     HAL_IOMUX_PIN_VOLTAGE_VIO
#endif

#ifdef SPILCD_VOLTAGE_VMEM
#define SPILCD_VOLTAGE_SEL                  HAL_IOMUX_PIN_VOLTAGE_MEM
#else
#define SPILCD_VOLTAGE_SEL                  HAL_IOMUX_PIN_VOLTAGE_VIO
#endif

#ifdef I2C0_VOLTAGE_VMEM
#define I2C0_VOLTAGE_SEL                    HAL_IOMUX_PIN_VOLTAGE_MEM
#else
#define I2C0_VOLTAGE_SEL                    HAL_IOMUX_PIN_VOLTAGE_VIO
#endif

#ifdef I2C1_VOLTAGE_VMEM
#define I2C1_VOLTAGE_SEL                    HAL_IOMUX_PIN_VOLTAGE_MEM
#else
#define I2C1_VOLTAGE_SEL                    HAL_IOMUX_PIN_VOLTAGE_VIO
#endif

#ifdef CLKOUT_VOLTAGE_VMEM
#define CLKOUT_VOLTAGE_SEL                  HAL_IOMUX_PIN_VOLTAGE_MEM
#else
#define CLKOUT_VOLTAGE_SEL                  HAL_IOMUX_PIN_VOLTAGE_VIO
#endif

#ifndef I2S0_IOMUX_INDEX
#define I2S0_IOMUX_INDEX                    0
#endif

#ifndef I2S1_IOMUX_INDEX
#define I2S1_IOMUX_INDEX                    0
#endif

#ifndef I2S_MCLK_IOMUX_INDEX
#define I2S_MCLK_IOMUX_INDEX                0
#endif

#ifndef SPDIF0_IOMUX_INDEX
#define SPDIF0_IOMUX_INDEX                  0
#endif

#ifndef DIG_MIC2_CK_IOMUX_INDEX
#define DIG_MIC2_CK_IOMUX_INDEX             0
#endif

#ifndef DIG_MIC3_CK_IOMUX_INDEX
#define DIG_MIC3_CK_IOMUX_INDEX             0
#endif

#ifndef DIG_MIC_CK_IOMUX_PIN
#define DIG_MIC_CK_IOMUX_PIN                0
#endif

#ifndef DIG_MIC_D0_IOMUX_PIN
#define DIG_MIC_D0_IOMUX_PIN                1
#endif

#ifndef DIG_MIC_D1_IOMUX_PIN
#define DIG_MIC_D1_IOMUX_PIN                2
#endif

#ifndef DIG_MIC_D2_IOMUX_PIN
#define DIG_MIC_D2_IOMUX_PIN                3
#endif

#ifndef SPI_IOMUX_INDEX
#define SPI_IOMUX_INDEX                     0
#endif

#ifndef SPILCD_IOMUX_INDEX
#define SPILCD_IOMUX_INDEX                  0
#endif

#ifndef I2C0_IOMUX_INDEX
#define I2C0_IOMUX_INDEX                    0
#endif

#ifndef I2C1_IOMUX_INDEX
#define I2C1_IOMUX_INDEX                    0
#endif

#ifndef CLKOUT_IOMUX_INDEX
#define CLKOUT_IOMUX_INDEX                  0
#endif

#ifndef JTAG_IOMUX_INDEX
#define JTAG_IOMUX_INDEX                    0
#endif

#define IOMUX_FUNC_VAL_GPIO                 0

#define IOMUX_ALT_FUNC_NUM                  6

// Other func values: 0 -> gpio, 4 -> bt_sw, 5 -> jtag, 6 -> btdm, 14 -> ana_test
static const uint8_t index_to_func_val[IOMUX_ALT_FUNC_NUM] = { 1, 2, 3, };

static const enum HAL_IOMUX_FUNCTION_T pin_func_map[HAL_IOMUX_PIN_NUM][IOMUX_ALT_FUNC_NUM] = {
    // P0_0
    { HAL_IOMUX_FUNC_I2S0_SCK, HAL_IOMUX_FUNC_SPI_CLK, HAL_IOMUX_FUNC_PWM0, },
    // P0_1
    { HAL_IOMUX_FUNC_I2S0_WS, HAL_IOMUX_FUNC_SPI_CS0, HAL_IOMUX_FUNC_PWM1, },
    // P0_2
    { HAL_IOMUX_FUNC_I2S0_SDI0, HAL_IOMUX_FUNC_SPI_DI0, HAL_IOMUX_FUNC_PDM0_D, },
    // P0_3
    { HAL_IOMUX_FUNC_I2S0_SDO, HAL_IOMUX_FUNC_SPI_DIO, HAL_IOMUX_FUNC_PDM0_CK, },
    // P0_4
    { HAL_IOMUX_FUNC_I2C_M0_SCL, HAL_IOMUX_FUNC_CLK_REQ_OUT, HAL_IOMUX_FUNC_NONE, },
    // P0_5
    { HAL_IOMUX_FUNC_I2C_M0_SDA, HAL_IOMUX_FUNC_CLK_REQ_IN, HAL_IOMUX_FUNC_NONE, },
    // P0_6
    { HAL_IOMUX_FUNC_I2C_M1_SCL, HAL_IOMUX_FUNC_PWM2, HAL_IOMUX_FUNC_PDM1_D, },
    // P0_7
    { HAL_IOMUX_FUNC_I2C_M1_SDA, HAL_IOMUX_FUNC_PWM3, HAL_IOMUX_FUNC_PDM1_CK, },
    // P1_0
    { HAL_IOMUX_FUNC_UART1_RX, HAL_IOMUX_FUNC_I2S0_MCLK, HAL_IOMUX_FUNC_NONE, },
    // P1_1
    { HAL_IOMUX_FUNC_UART1_TX, HAL_IOMUX_FUNC_CLK_OUT, HAL_IOMUX_FUNC_NONE, },
    // P1_2
    { HAL_IOMUX_FUNC_I2C_M2_SCL, HAL_IOMUX_FUNC_I2S0_MCLK, HAL_IOMUX_FUNC_BT_UART_RX, },
    // P1_3
    { HAL_IOMUX_FUNC_I2C_M2_SDA, HAL_IOMUX_FUNC_CLK_OUT, HAL_IOMUX_FUNC_BT_UART_TX, },
    // P1_4
    { HAL_IOMUX_FUNC_SPI_CLK, HAL_IOMUX_FUNC_PWM0, HAL_IOMUX_FUNC_NONE, },
    // P1_5
    { HAL_IOMUX_FUNC_SPI_CS0, HAL_IOMUX_FUNC_PWM1, HAL_IOMUX_FUNC_NONE, },
    // P1_6
    { HAL_IOMUX_FUNC_SPI_DI0, HAL_IOMUX_FUNC_I2S0_MCLK, HAL_IOMUX_FUNC_NONE, },
    // P1_7
    { HAL_IOMUX_FUNC_SPI_DIO, HAL_IOMUX_FUNC_CLK_OUT, HAL_IOMUX_FUNC_NONE, },
    // P2_0
    { HAL_IOMUX_FUNC_PDM0_D, HAL_IOMUX_FUNC_I2S0_MCLK, HAL_IOMUX_FUNC_BT_UART_CTS, },
    // P2_1
    { HAL_IOMUX_FUNC_PDM0_CK, HAL_IOMUX_FUNC_CLK_OUT, HAL_IOMUX_FUNC_BT_UART_RTS, },
    // P2_2
    { HAL_IOMUX_FUNC_UART0_RX, HAL_IOMUX_FUNC_NONE, HAL_IOMUX_FUNC_BT_UART_RX, },
    // P2_3
    { HAL_IOMUX_FUNC_UART0_TX, HAL_IOMUX_FUNC_NONE, HAL_IOMUX_FUNC_BT_UART_TX, },
    // P2_4
    { HAL_IOMUX_FUNC_UART0_RX, HAL_IOMUX_FUNC_UART1_RX, HAL_IOMUX_FUNC_BT_UART_RX, },
    // P2_5
    { HAL_IOMUX_FUNC_UART0_TX, HAL_IOMUX_FUNC_UART1_TX, HAL_IOMUX_FUNC_BT_UART_TX, },
};

static struct IOMUX_T * const iomux = (struct IOMUX_T *)IOMUX_BASE;

#ifdef ANC_PROD_TEST
#define OPT_TYPE
#else
#define OPT_TYPE                        const
#endif

static OPT_TYPE enum HAL_IOMUX_PIN_T digmic_ck_pin = DIG_MIC_CK_IOMUX_PIN;

static OPT_TYPE enum HAL_IOMUX_PIN_T digmic_d0_pin = DIG_MIC_D0_IOMUX_PIN;
static OPT_TYPE enum HAL_IOMUX_PIN_T digmic_d1_pin = DIG_MIC_D1_IOMUX_PIN;

void hal_iomux_set_default_config(void)
{
    uint32_t i;

    // Set all unused GPIOs to pull-down by default
    for (i = 0; i < 8; i++) {
        if (((iomux->REG_004 & (0xF << (i * 4))) >> (i * 4)) == 0xF) {
            iomux->REG_02C &= ~(1 << i);
            iomux->REG_030 |= (1 << i);
        }
    }
    for (i = 0; i < 8; i++) {
        if (((iomux->REG_008 & (0xF << (i * 4))) >> (i * 4)) == 0xF) {
            iomux->REG_02C &= ~(1 << (i + 8));
            iomux->REG_030 |= (1 << (i + 8));
        }
    }
    for (i = 0; i < 6; i++) {
        if (i == 2 || i == 3) {
            // This is uart0
            continue;
        }
        if (((iomux->REG_00C & (0xF << (i * 4))) >> (i * 4)) == 0xF) {
            iomux->REG_02C &= ~(1 << (i + 16));
            iomux->REG_030 |= (1 << (i + 16));
        }
    }
}

uint32_t hal_iomux_check(const struct HAL_IOMUX_PIN_FUNCTION_MAP *map, uint32_t count)
{
    uint32_t i;
    for (i = 0; i < count; ++i) {
    }
    return 0;
}

uint32_t hal_iomux_init(const struct HAL_IOMUX_PIN_FUNCTION_MAP *map, uint32_t count)
{
    uint32_t i;
    uint32_t ret;

    for (i = 0; i < count; ++i) {
        ret = hal_iomux_set_function(map[i].pin, map[i].function, HAL_IOMUX_OP_CLEAN_OTHER_FUNC_BIT);
        if (ret) {
            return (i << 8) + 1;
        }
		ret = hal_iomux_set_io_voltage_domains(map[i].pin, map[i].volt);
        if (ret) {
            return (i << 8) + 2;
        }
		ret = hal_iomux_set_io_pull_select(map[i].pin, map[i].pull_sel);
        if (ret) {
            return (i << 8) + 3;
        }
    }

    return 0;
}

#ifdef ANC_PROD_TEST
void hal_iomux_set_dig_mic_clock_pin(enum HAL_IOMUX_PIN_T pin)
{
    digmic_ck_pin = pin;
}
void hal_iomux_set_dig_mic_data0_pin(enum HAL_IOMUX_PIN_T pin)
{
    digmic_d0_pin = pin;
}

void hal_iomux_set_dig_mic_data1_pin(enum HAL_IOMUX_PIN_T pin)
{
    digmic_d1_pin = pin;
}

void hal_iomux_set_dig_mic_data2_pin(enum HAL_IOMUX_PIN_T pin)
{
//    digmic_d2_pin = pin;
}
#endif

uint32_t hal_iomux_set_function(enum HAL_IOMUX_PIN_T pin, enum HAL_IOMUX_FUNCTION_T func, enum HAL_IOMUX_OP_TYPE_T type)
{
    int i;
    uint8_t val;
    __IO uint32_t *reg;
    uint32_t shift;

    if (pin >= HAL_IOMUX_PIN_LED_NUM) {
        return 1;
    }
    if (func >= HAL_IOMUX_FUNC_END) {
        return 2;
    }

    if (pin == HAL_IOMUX_PIN_P2_2 || pin == HAL_IOMUX_PIN_P2_3) {
        if (func ==  HAL_IOMUX_FUNC_I2C_SCL || func == HAL_IOMUX_FUNC_I2C_SDA) {
            // Enable analog I2C slave
            iomux->REG_050 &= ~IOMUX_GPIO_I2C_MODE;
            // Set mcu GPIO func
            iomux->REG_00C = (iomux->REG_00C & ~(IOMUX_GPIO_P22_SEL_MASK | IOMUX_GPIO_P23_SEL_MASK)) |
                IOMUX_GPIO_P22_SEL(IOMUX_FUNC_VAL_GPIO) | IOMUX_GPIO_P23_SEL(IOMUX_FUNC_VAL_GPIO);
            return 0;
        } else {
            iomux->REG_050 |= IOMUX_GPIO_I2C_MODE;
            // Continue to set the alt func
        }
    } else if (pin == HAL_IOMUX_PIN_P1_2) {
        if (func ==  HAL_IOMUX_FUNC_UART1_CTS) {
            iomux->REG_008 = SET_BITFIELD(iomux->REG_008, IOMUX_GPIO_P12_SEL, 4);
            return 0;
        }
    } else if (pin == HAL_IOMUX_PIN_P1_3) {
        if (func ==  HAL_IOMUX_FUNC_UART1_CTS) {
            iomux->REG_008 = SET_BITFIELD(iomux->REG_008, IOMUX_GPIO_P03_SEL, 4);
            return 0;
        } else if (func == HAL_IOMUX_FUNC_CLK_32K_IN) {
            iomux->REG_008 = SET_BITFIELD(iomux->REG_008, IOMUX_GPIO_P03_SEL, 5);
            return 0;
        }
    } else if (pin == HAL_IOMUX_PIN_LED1 || pin == HAL_IOMUX_PIN_LED2) {
        if (func == HAL_IOMUX_FUNC_GPIO){
            pmu_led_uart_disable(pin);
            hal_iomux_clear_pmu_uart();
        }
        ASSERT(func == HAL_IOMUX_FUNC_GPIO, "Bad func=%d for IOMUX pin=%d", func, pin);
        return 0;
    }

    if (func == HAL_IOMUX_FUNC_GPIO) {
        val = IOMUX_FUNC_VAL_GPIO;
    } else {
        for (i = 0; i < IOMUX_ALT_FUNC_NUM; i++) {
            if (pin_func_map[pin][i] == func) {
                break;
            }
        }

        if (i == IOMUX_ALT_FUNC_NUM) {
            return 3;
        }
        val = index_to_func_val[i];
    }

    reg = &iomux->REG_004 + pin / 8;
    shift = (pin % 8) * 4;

    *reg = (*reg & ~(0xF << shift)) | (val << shift);

    return 0;
}

enum HAL_IOMUX_FUNCTION_T hal_iomux_get_function(enum HAL_IOMUX_PIN_T pin)
{
    return HAL_IOMUX_FUNC_NONE;
}

uint32_t hal_iomux_set_io_voltage_domains(enum HAL_IOMUX_PIN_T pin, enum HAL_IOMUX_PIN_VOLTAGE_DOMAINS_T volt)
{
    if (pin >= HAL_IOMUX_PIN_LED_NUM) {
        return 1;
    }

    if (pin == HAL_IOMUX_PIN_LED1 || pin == HAL_IOMUX_PIN_LED2) {
        pmu_led_set_voltage_domains(pin, volt);
    }

    return 0;
}

uint32_t hal_iomux_set_io_pull_select(enum HAL_IOMUX_PIN_T pin, enum HAL_IOMUX_PIN_PULL_SELECT_T pull_sel)
{
    if (pin >= HAL_IOMUX_PIN_LED_NUM) {
        return 1;
    }

    if (pin < HAL_IOMUX_PIN_LED1) {
        iomux->REG_02C &= ~(1 << pin);
        iomux->REG_030 &= ~(1 << pin);
        if (pull_sel == HAL_IOMUX_PIN_PULLUP_ENABLE) {
            iomux->REG_02C |= (1 << pin);
        } else if (pull_sel == HAL_IOMUX_PIN_PULLDOWN_ENABLE) {
            iomux->REG_030 |= (1 << pin);
        }
    } else if (pin == HAL_IOMUX_PIN_LED1 || pin == HAL_IOMUX_PIN_LED2) {
        pmu_led_set_pull_select(pin, pull_sel);
    }

    return 0;
}

void hal_iomux_set_sdmmc_dt_n_out_group(int enable)
{
}

void hal_iomux_set_uart0_voltage(enum HAL_IOMUX_PIN_VOLTAGE_DOMAINS_T volt)
{
}

void hal_iomux_set_uart1_voltage(enum HAL_IOMUX_PIN_VOLTAGE_DOMAINS_T volt)
{
}

bool hal_iomux_uart0_connected(void)
{
    uint32_t reg_050, reg_00c, reg_02c, reg_030;
    uint32_t mask;
    int val;

    // Save current iomux settings
    reg_050 = iomux->REG_050;
    reg_00c = iomux->REG_00C;
    reg_02c = iomux->REG_02C;
    reg_030 = iomux->REG_030;

    // Disable analog I2C slave & master
    iomux->REG_050 |= IOMUX_GPIO_I2C_MODE | IOMUX_I2C0_M_SEL_GPIO;
    // Set uart0-rx as gpio
    iomux->REG_00C = SET_BITFIELD(iomux->REG_00C, IOMUX_GPIO_P22_SEL, IOMUX_FUNC_VAL_GPIO);

    mask = (1 << HAL_IOMUX_PIN_P2_2);
    // Clear pullup
    iomux->REG_02C &= ~mask;
    // Setup pulldown
    iomux->REG_030 |= mask;

    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)HAL_IOMUX_PIN_P2_2, HAL_GPIO_DIR_IN, 0);

    hal_sys_timer_delay(MS_TO_TICKS(2));

    val = hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)HAL_IOMUX_PIN_P2_2);

    // Restore iomux settings
    iomux->REG_030 = reg_030;
    iomux->REG_02C = reg_02c;
    iomux->REG_00C = reg_00c;
    iomux->REG_050 = reg_050;

    hal_sys_timer_delay(MS_TO_TICKS(2));

    return !!val;
}

bool hal_iomux_uart1_connected(void)
{
    uint32_t reg_008, reg_02c, reg_030;
    uint32_t mask;
    int val;

    // Save current iomux settings
    reg_008 = iomux->REG_008;
    reg_02c = iomux->REG_02C;
    reg_030 = iomux->REG_030;

    // Set uart1-rx as gpio
    iomux->REG_008 = SET_BITFIELD(iomux->REG_008, IOMUX_GPIO_P10_SEL, IOMUX_FUNC_VAL_GPIO);

    mask = (1 << HAL_IOMUX_PIN_P1_0);
    // Clear pullup
    iomux->REG_02C &= ~mask;
    // Setup pulldown
    iomux->REG_030 |= mask;

    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)HAL_IOMUX_PIN_P1_0, HAL_GPIO_DIR_IN, 0);

    hal_sys_timer_delay(MS_TO_TICKS(2));

    val = hal_gpio_pin_get_val((enum HAL_GPIO_PIN_T)HAL_IOMUX_PIN_P1_0);

    // Restore iomux settings
    iomux->REG_030 = reg_030;
    iomux->REG_02C = reg_02c;
    iomux->REG_008 = reg_008;

    hal_sys_timer_delay(MS_TO_TICKS(2));

    return !!val;
}

static void hal_iomux_set_uart0_common(uint8_t func)
{
    uint32_t mask;

    // Disable analog I2C slave & master
    iomux->REG_050 |= IOMUX_GPIO_I2C_MODE | IOMUX_I2C0_M_SEL_GPIO;
    // Set uart0 func
    iomux->REG_00C = (iomux->REG_00C & ~(IOMUX_GPIO_P22_SEL_MASK | IOMUX_GPIO_P23_SEL_MASK)) |
        IOMUX_GPIO_P22_SEL(func) | IOMUX_GPIO_P23_SEL(func);

    mask = (1 << HAL_IOMUX_PIN_P2_2) | (1 << HAL_IOMUX_PIN_P2_3);
    // Setup pullup
    iomux->REG_02C |= (1 << HAL_IOMUX_PIN_P2_2);
    iomux->REG_02C &= ~(1 << HAL_IOMUX_PIN_P2_3);
    // Clear pulldown
    iomux->REG_030 &= ~mask;
}

void hal_iomux_set_uart0(void)
{
    hal_iomux_set_uart0_common(1);
}

void hal_iomux_set_uart1(void)
{
    uint32_t mask;

    // Set uart1 func
    iomux->REG_008 = (iomux->REG_008 & ~(IOMUX_GPIO_P10_SEL_MASK | IOMUX_GPIO_P11_SEL_MASK)) |
        IOMUX_GPIO_P10_SEL(1) | IOMUX_GPIO_P11_SEL(1);

    mask = (1 << HAL_IOMUX_PIN_P1_0) | (1 << HAL_IOMUX_PIN_P1_1);
    // Setup pullup
    iomux->REG_02C |= (1 << HAL_IOMUX_PIN_P1_0);
    iomux->REG_02C &= ~(1 << HAL_IOMUX_PIN_P1_1);
    // Clear pulldown
    iomux->REG_030 &= ~mask;
}

void hal_iomux_set_analog_i2c(void)
{
    uint32_t mask;

    // Disable analog I2C master
    iomux->REG_050 |= IOMUX_I2C0_M_SEL_GPIO;
    // Set mcu GPIO func
    iomux->REG_00C = (iomux->REG_00C & ~(IOMUX_GPIO_P22_SEL_MASK | IOMUX_GPIO_P23_SEL_MASK)) |
        IOMUX_GPIO_P22_SEL(IOMUX_FUNC_VAL_GPIO) | IOMUX_GPIO_P23_SEL(IOMUX_FUNC_VAL_GPIO);
    // Enable analog I2C slave
    iomux->REG_050 &= ~IOMUX_GPIO_I2C_MODE;

    mask = (1 << HAL_IOMUX_PIN_P2_2) | (1 << HAL_IOMUX_PIN_P2_3);
    // Setup pullup
    iomux->REG_02C |= mask;
    // Clear pulldown
    iomux->REG_030 &= ~mask;
}

void hal_iomux_set_jtag(void)
{
    uint32_t mask;

#if (JTAG_IOMUX_INDEX == 22)
    hal_iomux_set_uart0_common(2);

    mask = (1 << HAL_IOMUX_PIN_P2_2) | (1 << HAL_IOMUX_PIN_P2_3);
    // Clear pullup
    iomux->REG_02C &= ~mask;
    // Clear pulldown
    iomux->REG_030 &= ~mask;
#else
    uint32_t val;

    // SWCLK/TCK, SWDIO/TMS
    mask = IOMUX_GPIO_P00_SEL_MASK | IOMUX_GPIO_P04_SEL_MASK;
    val = IOMUX_GPIO_P00_SEL(5) | IOMUX_GPIO_P04_SEL(5);

    // TDI, TDO
#ifdef JTAG_TDI_TDO_PIN
    mask |= IOMUX_GPIO_P02_SEL_MASK | IOMUX_GPIO_P03_SEL_MASK;
    val |= IOMUX_GPIO_P02_SEL(5) | IOMUX_GPIO_P03_SEL(5);
#endif
    iomux->REG_004 = (iomux->REG_004 & ~mask) | val;

    // RESET
#if defined(JTAG_RESET_PIN)
    iomux->REG_004 = (iomux->REG_004 & ~(IOMUX_GPIO_P01_SEL_MASK)) | IOMUX_GPIO_P01_SEL(5);
#endif

    mask = (1 << HAL_IOMUX_PIN_P0_0) | (1 << HAL_IOMUX_PIN_P0_4);
#ifdef JTAG_TDI_TDO_PIN
    mask |= (1 << HAL_IOMUX_PIN_P0_2) | (1 << HAL_IOMUX_PIN_P0_3);
#endif
#if defined(JTAG_RESET_PIN) || defined(JTAG_TDI_TDO_PIN)
    mask |= (1 << HAL_IOMUX_PIN_P0_1);
#endif
    // Clear pullup
    iomux->REG_02C &= ~mask;
    // Clear pulldown
    iomux->REG_030 &= ~mask;
#endif
}

enum HAL_IOMUX_ISPI_ACCESS_T hal_iomux_ispi_access_enable(enum HAL_IOMUX_ISPI_ACCESS_T access)
{
    uint32_t v;

    v = iomux->REG_044;
    iomux->REG_044 |= access;

    return v;
}

enum HAL_IOMUX_ISPI_ACCESS_T hal_iomux_ispi_access_disable(enum HAL_IOMUX_ISPI_ACCESS_T access)
{
    uint32_t v;

    v = iomux->REG_044;
    iomux->REG_044 &= ~access;

    return v;
}

void hal_iomux_ispi_access_init(void)
{
    // Disable bt spi access ana/pmu interface
    hal_iomux_ispi_access_disable(HAL_IOMUX_ISPI_BT_ANA | HAL_IOMUX_ISPI_BT_PMU);
}

void hal_iomux_set_i2s0(void)
{
    static const struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_i2s[] = {
        {HAL_IOMUX_PIN_P0_0, HAL_IOMUX_FUNC_I2S0_SCK,  I2S0_VOLTAGE_SEL, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P0_1, HAL_IOMUX_FUNC_I2S0_WS,   I2S0_VOLTAGE_SEL, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P0_2, HAL_IOMUX_FUNC_I2S0_SDI0, I2S0_VOLTAGE_SEL, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P0_3, HAL_IOMUX_FUNC_I2S0_SDO,  I2S0_VOLTAGE_SEL, HAL_IOMUX_PIN_NOPULL},
    };

    hal_iomux_init(pinmux_i2s, ARRAY_SIZE(pinmux_i2s));
}

void hal_iomux_set_i2s1(void)
{
}

void hal_iomux_set_i2s_mclk(void)
{
    static const struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux[] = {
#if (I2S_MCLK_IOMUX_INDEX == 12)
        {HAL_IOMUX_PIN_P1_2, HAL_IOMUX_FUNC_I2S0_MCLK, I2S0_VOLTAGE_SEL, HAL_IOMUX_PIN_NOPULL},
#elif (I2S_MCLK_IOMUX_INDEX == 14)
        {HAL_IOMUX_PIN_P1_4, HAL_IOMUX_FUNC_I2S0_MCLK, I2S0_VOLTAGE_SEL, HAL_IOMUX_PIN_NOPULL},
#elif (I2S_MCLK_IOMUX_INDEX == 16)
        {HAL_IOMUX_PIN_P1_6, HAL_IOMUX_FUNC_I2S0_MCLK, I2S0_VOLTAGE_SEL, HAL_IOMUX_PIN_NOPULL},
#else
        {HAL_IOMUX_PIN_P1_0, HAL_IOMUX_FUNC_I2S0_MCLK, I2S0_VOLTAGE_SEL, HAL_IOMUX_PIN_NOPULL},
#endif
    };

    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
}

void hal_iomux_set_spdif0(void)
{
}

void hal_iomux_set_spdif1(void)
{
}

void hal_iomux_set_dig_mic(uint32_t map)
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_digitalmic_clk[] = {
        {HAL_IOMUX_PIN_P0_3, HAL_IOMUX_FUNC_PDM0_CK, DIGMIC_VOLTAGE_SEL, HAL_IOMUX_PIN_NOPULL},
    };
    struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_digitalmic0[] = {
        {HAL_IOMUX_PIN_P0_2, HAL_IOMUX_FUNC_PDM0_D,  DIGMIC_VOLTAGE_SEL, HAL_IOMUX_PIN_NOPULL},
    };
    struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_digitalmic1[] = {
        {HAL_IOMUX_PIN_P0_6, HAL_IOMUX_FUNC_PDM1_D,  DIGMIC_VOLTAGE_SEL, HAL_IOMUX_PIN_NOPULL},
    };

    if (digmic_ck_pin == HAL_IOMUX_PIN_P0_3) {
        pinmux_digitalmic_clk[0].pin = HAL_IOMUX_PIN_P0_3;
        pinmux_digitalmic_clk[0].function = HAL_IOMUX_FUNC_PDM0_CK;
    } else if (digmic_ck_pin == HAL_IOMUX_PIN_P0_4) {
        pinmux_digitalmic_clk[0].pin = HAL_IOMUX_PIN_P0_4;
        pinmux_digitalmic_clk[0].function = HAL_IOMUX_FUNC_PDM1_CK;
    } else if (digmic_ck_pin == HAL_IOMUX_PIN_P2_1) {
        pinmux_digitalmic_clk[0].pin = HAL_IOMUX_PIN_P2_1;
        pinmux_digitalmic_clk[0].function = HAL_IOMUX_FUNC_PDM0_CK;
    }

    if (digmic_d0_pin == HAL_IOMUX_PIN_P0_2) {
        pinmux_digitalmic0[0].pin = HAL_IOMUX_PIN_P0_2;
    } else if (digmic_d0_pin == HAL_IOMUX_PIN_P2_0) {
        pinmux_digitalmic0[0].pin = HAL_IOMUX_PIN_P2_0;
    }

    if (digmic_d1_pin == HAL_IOMUX_PIN_P0_6) {
        pinmux_digitalmic1[0].pin = HAL_IOMUX_PIN_P0_6;
    }

    if ((map & 0xF) == 0) {
        pinmux_digitalmic_clk[0].function = HAL_IOMUX_FUNC_GPIO;
    }
    hal_iomux_init(pinmux_digitalmic_clk, ARRAY_SIZE(pinmux_digitalmic_clk));
    if (map & (1 << 0)) {
        hal_iomux_init(pinmux_digitalmic0, ARRAY_SIZE(pinmux_digitalmic0));
    }
    if (map & (1 << 1)) {
        hal_iomux_init(pinmux_digitalmic1, ARRAY_SIZE(pinmux_digitalmic1));
    }
}

void hal_iomux_set_spi(void)
{
    static const struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_spi_3wire[3] = {
#if (SPI_IOMUX_INDEX == 14)
        {HAL_IOMUX_PIN_P1_4, HAL_IOMUX_FUNC_SPI_CLK, SPI_VOLTAGE_SEL, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P1_5, HAL_IOMUX_FUNC_SPI_CS0,  SPI_VOLTAGE_SEL, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P1_7, HAL_IOMUX_FUNC_SPI_DIO, SPI_VOLTAGE_SEL, HAL_IOMUX_PIN_NOPULL},
#else
        {HAL_IOMUX_PIN_P2_0, HAL_IOMUX_FUNC_SPI_CLK, SPI_VOLTAGE_SEL, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P2_1, HAL_IOMUX_FUNC_SPI_CS0,  SPI_VOLTAGE_SEL, HAL_IOMUX_PIN_NOPULL},
        {HAL_IOMUX_PIN_P2_3, HAL_IOMUX_FUNC_SPI_DIO, SPI_VOLTAGE_SEL, HAL_IOMUX_PIN_NOPULL},
#endif
    };
#ifdef SPI_IOMUX_4WIRE
    static const struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_spi_4wire[1] = {
#if (SPI_IOMUX_INDEX == 14)
        {HAL_IOMUX_PIN_P1_6, HAL_IOMUX_FUNC_SPI_DI0, SPI_VOLTAGE_SEL, HAL_IOMUX_PIN_NOPULL},
#else
        {HAL_IOMUX_PIN_P2_2, HAL_IOMUX_FUNC_SPI_DI0, SPI_VOLTAGE_SEL, HAL_IOMUX_PIN_NOPULL},
#endif
    };
#endif

    hal_iomux_init(pinmux_spi_3wire, ARRAY_SIZE(pinmux_spi_3wire));
#ifdef SPI_IOMUX_4WIRE
    hal_iomux_init(pinmux_spi_4wire, ARRAY_SIZE(pinmux_spi_4wire));
#endif
}

void hal_iomux_set_spilcd(void)
{
}

void hal_iomux_set_i2c0(void)
{
#if (I2C0_IOMUX_INDEX == 22)
    hal_iomux_set_analog_i2c();
    // IOMUX_GPIO_I2C_MODE should be kept in disabled state
    iomux->REG_050 &= ~IOMUX_I2C0_M_SEL_GPIO;
#else
    static const struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_i2c[] = {
        {HAL_IOMUX_PIN_P0_4, HAL_IOMUX_FUNC_I2C_M0_SCL, I2C0_VOLTAGE_SEL, HAL_IOMUX_PIN_PULLUP_ENABLE},
        {HAL_IOMUX_PIN_P0_5, HAL_IOMUX_FUNC_I2C_M0_SDA, I2C0_VOLTAGE_SEL, HAL_IOMUX_PIN_PULLUP_ENABLE},
    };

    hal_iomux_init(pinmux_i2c, ARRAY_SIZE(pinmux_i2c));
#endif
}

void hal_iomux_set_i2c1(void)
{
    static const struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_i2c[] = {
        {HAL_IOMUX_PIN_P0_6, HAL_IOMUX_FUNC_I2C_M1_SCL, I2C1_VOLTAGE_SEL, HAL_IOMUX_PIN_PULLUP_ENABLE},
        {HAL_IOMUX_PIN_P0_7, HAL_IOMUX_FUNC_I2C_M1_SDA, I2C1_VOLTAGE_SEL, HAL_IOMUX_PIN_PULLUP_ENABLE},
    };

    hal_iomux_init(pinmux_i2c, ARRAY_SIZE(pinmux_i2c));
    iomux->REG_050 |= IOMUX_I2C1_M_SEL_GPIO;
}

void hal_iomux_set_i2c2(void)
{
    static const struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_i2c[] = {
        {HAL_IOMUX_PIN_P1_2, HAL_IOMUX_FUNC_I2C_M2_SCL, HAL_IOMUX_PIN_VOLTAGE_MEM, HAL_IOMUX_PIN_PULLUP_ENABLE},
        {HAL_IOMUX_PIN_P1_3, HAL_IOMUX_FUNC_I2C_M2_SDA, HAL_IOMUX_PIN_VOLTAGE_MEM, HAL_IOMUX_PIN_PULLUP_ENABLE},
    };

    hal_iomux_init(pinmux_i2c, ARRAY_SIZE(pinmux_i2c));
}

void hal_iomux_set_clock_out(void)
{
    static const struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_clkout[] = {
#if (CLKOUT_IOMUX_INDEX == 13)
        {HAL_IOMUX_PIN_P1_3, HAL_IOMUX_FUNC_CLK_OUT, CLKOUT_VOLTAGE_SEL, HAL_IOMUX_PIN_NOPULL},
#elif (CLKOUT_IOMUX_INDEX == 17)
        {HAL_IOMUX_PIN_P1_7, HAL_IOMUX_FUNC_CLK_OUT, CLKOUT_VOLTAGE_SEL, HAL_IOMUX_PIN_NOPULL},
#elif (CLKOUT_IOMUX_INDEX == 21)
        {HAL_IOMUX_PIN_P2_1, HAL_IOMUX_FUNC_CLK_OUT, CLKOUT_VOLTAGE_SEL, HAL_IOMUX_PIN_NOPULL},
#else
        {HAL_IOMUX_PIN_P1_1, HAL_IOMUX_FUNC_CLK_OUT, CLKOUT_VOLTAGE_SEL, HAL_IOMUX_PIN_NOPULL},
#endif
    };

    hal_iomux_init(pinmux_clkout, ARRAY_SIZE(pinmux_clkout));
}

void hal_iomux_set_mcu_clock_out(void)
{
}

void hal_iomux_set_bt_clock_out(void)
{
}

void hal_iomux_set_bt_tport(void)
{
    // P0_0 ~ P0_3,
    iomux->REG_004 = (iomux->REG_004 & ~(IOMUX_GPIO_P00_SEL_MASK | IOMUX_GPIO_P01_SEL_MASK | IOMUX_GPIO_P02_SEL_MASK | IOMUX_GPIO_P03_SEL_MASK)) |
    IOMUX_GPIO_P00_SEL(0xE) | IOMUX_GPIO_P01_SEL(0xE) | IOMUX_GPIO_P02_SEL(0xE) |IOMUX_GPIO_P03_SEL(0xE);
#ifdef TPORTS_KEY_COEXIST
    //P1_0 ~ P1_1,
    iomux->REG_008 = (iomux->REG_008 & ~(IOMUX_GPIO_P10_SEL_MASK | IOMUX_GPIO_P11_SEL_MASK)) |
    IOMUX_GPIO_P10_SEL(0xE) | IOMUX_GPIO_P11_SEL(0xE);
#else
    //P1_0 ~ P1_3,
    iomux->REG_008 = (iomux->REG_008 & ~(IOMUX_GPIO_P10_SEL_MASK | IOMUX_GPIO_P11_SEL_MASK | IOMUX_GPIO_P12_SEL_MASK | IOMUX_GPIO_P13_SEL_MASK )) |
    IOMUX_GPIO_P10_SEL(0xE) | IOMUX_GPIO_P11_SEL(0xE) | IOMUX_GPIO_P12_SEL(0xE) | IOMUX_GPIO_P13_SEL(0xE);
#endif
    // ANA TEST DIR
    iomux->REG_014 = 0x0f0f;
    // ANA TEST SEL
    iomux->REG_018 = IOMUX_ANA_TEST_SEL(3);
}

void hal_iomux_set_bt_rf_sw(int rx_on, int tx_on)
{
    uint32_t val;
    uint32_t dir;

    //iomux->REG_004 = (iomux->REG_004 & ~(IOMUX_GPIO_P00_SEL_MASK | IOMUX_GPIO_P01_SEL_MASK)) |
    //    IOMUX_GPIO_P00_SEL(6) | IOMUX_GPIO_P01_SEL(6);

    val = iomux->REG_004;
    dir = 0;
    if (rx_on) {
        val = SET_BITFIELD(val, IOMUX_GPIO_P00_SEL, 0xE);
        dir = (1 << HAL_IOMUX_PIN_P0_0);
    }
    if (tx_on) {
#if (TXON_IOMUX_INDEX == 17)
        uint32_t val2 = iomux->REG_008;
        val2 = SET_BITFIELD(val2, IOMUX_GPIO_P17_SEL, 0x4);
        dir = (1 << HAL_IOMUX_PIN_P1_7);
        iomux->REG_008 = val2;
#else
        val = SET_BITFIELD(val, IOMUX_GPIO_P01_SEL, 0xE);
        dir = (1 << HAL_IOMUX_PIN_P0_1);
#endif
    }
    iomux->REG_004 = val;
    // ANA TEST DIR
    iomux->REG_014 |= dir;
    // ANA TEST SEL
    iomux->REG_018 = IOMUX_ANA_TEST_SEL(5);
}

static void _hal_iomux_config_spi_slave(uint32_t val)
{
    iomux->REG_008 = SET_BITFIELD(iomux->REG_008, IOMUX_GPIO_P12_SEL, val);
    iomux->REG_00C = (iomux->REG_00C & ~(IOMUX_GPIO_P25_SEL_MASK | IOMUX_GPIO_P26_SEL_MASK | IOMUX_GPIO_P27_SEL_MASK)) |
        IOMUX_GPIO_P25_SEL(val) | IOMUX_GPIO_P26_SEL(val) | IOMUX_GPIO_P27_SEL(val);
}

void hal_iomux_set_spi_slave(void)
{
    _hal_iomux_config_spi_slave(8);
}

void hal_iomux_clear_spi_slave(void)
{
    _hal_iomux_config_spi_slave(IOMUX_FUNC_VAL_GPIO);
}

int hal_pwrkey_set_irq(enum HAL_PWRKEY_IRQ_T type)
{
    uint32_t v;

    if (type == HAL_PWRKEY_IRQ_NONE) {
        v = IOMUX_PWR_KEY_DOWN_INT_CLR | IOMUX_PWR_KEY_UP_INT_CLR;
    } else if (type == HAL_PWRKEY_IRQ_FALLING_EDGE) {
        v = IOMUX_PWR_KEY_DOWN_INT_EN | IOMUX_PWR_KEY_DOWN_INT_MASK;
    } else if (type == HAL_PWRKEY_IRQ_RISING_EDGE) {
        v = IOMUX_PWR_KEY_UP_INT_EN | IOMUX_PWR_KEY_UP_INT_MASK;
    } else if (type == HAL_PWRKEY_IRQ_BOTH_EDGE) {
        v = IOMUX_PWR_KEY_DOWN_INT_EN | IOMUX_PWR_KEY_DOWN_INT_MASK |
            IOMUX_PWR_KEY_UP_INT_EN | IOMUX_PWR_KEY_UP_INT_MASK;
    } else {
        return 1;
    }

    iomux->REG_040 = v;

    return 0;
}

bool hal_pwrkey_pressed(void)
{
    uint32_t v = iomux->REG_040;
    return !!(v & IOMUX_PWR_KEY_VAL);
}

bool hal_pwrkey_startup_pressed(void)
{
    return hal_pwrkey_pressed();
}

enum HAL_PWRKEY_IRQ_T hal_pwrkey_get_irq_state(void)
{
    enum HAL_PWRKEY_IRQ_T state = HAL_PWRKEY_IRQ_NONE;
    uint32_t v = iomux->REG_040;

    if (v & IOMUX_PWR_KEY_DOWN_INT_ST) {
        state |= HAL_PWRKEY_IRQ_FALLING_EDGE;
    }

    if (v & IOMUX_PWR_KEY_UP_INT_ST) {
        state |= HAL_PWRKEY_IRQ_RISING_EDGE;
    }

    return state;
}

const struct HAL_IOMUX_PIN_FUNCTION_MAP iomux_tport[] = {
/*    {HAL_IOMUX_PIN_P1_1, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},*/
    {HAL_IOMUX_PIN_P1_5, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
};

int hal_iomux_tportopen(void)
{
    int i;

    for (i=0;i<sizeof(iomux_tport)/sizeof(struct HAL_IOMUX_PIN_FUNCTION_MAP);i++){
        hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&iomux_tport[i], 1);
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)iomux_tport[i].pin, HAL_GPIO_DIR_OUT, 0);
    }
    return 0;
}

int hal_iomux_tportset(int port)
{
    hal_gpio_pin_set((enum HAL_GPIO_PIN_T)iomux_tport[port].pin);
    return 0;
}

int hal_iomux_tportclr(int port)
{
    hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)iomux_tport[port].pin);
    return 0;
}

void hal_iomux_set_codec_gpio_trigger(enum HAL_IOMUX_PIN_T pin, bool polarity)
{
    iomux->REG_064 = SET_BITFIELD(iomux->REG_064, IOMUX_CFG_CODEC_TRIG_SEL, pin);
    if (polarity) {
        iomux->REG_064 &= ~IOMUX_CFG_CODEC_TRIG_POL;
    } else {
        iomux->REG_064 |= IOMUX_CFG_CODEC_TRIG_POL;
    }
}

void hal_iomux_set_pmu_uart(uint32_t uart)
{
    iomux->REG_048 &= ~IOMUX_DR_PMU_UART_OENB;
    iomux->REG_048 = SET_BITFIELD(iomux->REG_048, IOMUX_SEL_PMU_UART, uart) | IOMUX_EN_PMU_UART;
}

void hal_iomux_clear_pmu_uart(void)
{
    iomux->REG_048 &= ~IOMUX_EN_PMU_UART;
}

void hal_iomux_single_wire_pmu_uart_rx(uint32_t uart)
{
    pmu_led_uart_enable(HAL_IOMUX_PIN_LED2);
    hal_iomux_set_pmu_uart(uart);
}

void hal_iomux_single_wire_pmu_uart_tx(uint32_t uart)
{
}

void hal_iomux_single_wire_dig_uart_rx(uint32_t uart)
{
#ifdef UART_HALF_DUPLEX
#define SUART_TX_PIN_PULL_SEL_IN_RX         HAL_IOMUX_PIN_NOPULL
#else
#define SUART_TX_PIN_PULL_SEL_IN_RX         HAL_IOMUX_PIN_PULLUP_ENABLE
#endif

    const struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_uart[] =
    {
        {HAL_IOMUX_PIN_P1_0, HAL_IOMUX_FUNC_UART1_RX, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
        {HAL_IOMUX_PIN_P1_1, HAL_IOMUX_FUNC_GPIO,     HAL_IOMUX_PIN_VOLTAGE_VIO, SUART_TX_PIN_PULL_SEL_IN_RX},
    };

    iomux->REG_050 &= ~IOMUX_UART1_MCU_HALFN;

    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)pinmux_uart[0].pin, HAL_GPIO_DIR_IN, 1);
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)pinmux_uart[1].pin, HAL_GPIO_DIR_IN, 1);

    hal_iomux_init(pinmux_uart, ARRAY_SIZE(pinmux_uart));

#ifndef UART_HALF_DUPLEX
    hal_uart_flush(uart, 0);
#endif
}

void hal_iomux_single_wire_dig_uart_tx(uint32_t uart)
{
#ifndef UART_HALF_DUPLEX
    struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_uart[] =
    {
        {HAL_IOMUX_PIN_P1_0, HAL_IOMUX_FUNC_GPIO,     HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
        {HAL_IOMUX_PIN_P1_1, HAL_IOMUX_FUNC_UART1_TX, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };

    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)pinmux_uart[0].pin, HAL_GPIO_DIR_IN, 1);
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)pinmux_uart[1].pin, HAL_GPIO_DIR_IN, 1);

    hal_iomux_init(pinmux_uart, ARRAY_SIZE(pinmux_uart));
#endif
}

void hal_iomux_single_wire_uart_rx(uint32_t uart)
{
#ifdef SINGLE_WIRE_UART_DIG
    hal_iomux_single_wire_dig_uart_rx(uart);
#else
    hal_iomux_single_wire_pmu_uart_rx(uart);
#endif
}

void hal_iomux_single_wire_uart_tx(uint32_t uart)
{
#ifdef SINGLE_WIRE_UART_DIG
    hal_iomux_single_wire_dig_uart_tx(uart);
#else
    hal_iomux_single_wire_pmu_uart_tx(uart);
#endif
}
